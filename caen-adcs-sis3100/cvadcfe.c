/* -*- C -*- */
/******************************************************************\
   $Name$ - $Id$

   Name:       fe4cspu.c
   Created by: Exaos Lee ( Exaos.Lee@gmail.com )
   Contents:   DAQ Code for Pu239(n,2n gamma)

   Hardware:   VME bus, SIS3100 controller, SIS1100 PCI-VME,
               CAEN V785N Peak Sensing ADC, CAEN V775N TDC
               CAEN V977 I/O Register

\*******************************************************************/

#include <fcntl.h>
#include <sys/ioctl.h>

#include "cvadcexp.h"
#include "vadc_caen.h"
#include "cv560.h"

/*-- Globals ----------------------------------------------------*/

/* frontend name */
char *frontend_name      = "Frontend using CAEN ADCs";
char *frontend_file_name = __FILE__;
BOOL  frontend_call_loop = FALSE;

INT display_period       = 3000; /* micro-seconds, = 3 s */
INT max_event_size       = 10000;
INT max_event_size_frag  = 0x10 * 1024 * 1024; /* EQ_FRAGMENTED */
INT event_buffer_size    = 10 * 10000;

/* VME crate handle */
MVME_INTERFACE *pvme;

HNDLE              hSet;
extern INT         run_state;
extern HNDLE       hDB;
EXPCVADC_SETTINGS  exp_set; /* Expcvadc Settings */

/* VME hardware address: Using EXPCVADC_SETTINGS */

/*-- Function declarations -----------------------------------------*/
INT frontend_init();
INT frontend_exit();
INT  begin_of_run(INT rnum, char *error);
INT    end_of_run(INT rnum, char *error);
INT     pause_run(INT rnum, char *error);
INT    resume_run(INT rnum, char *error);
INT frontend_loop();

INT read_expcvadc_event(char *pevent, INT off);
INT   read_scaler_event(char *pevent, INT off);

/*---  Bank definitions --------------------------------------------*/
BANK_LIST expcvadc_bank_list[] = {
  {"ADC0", TID_WORD, N_ADC, NULL},
//  {"ADC1", TID_WORD, N_ADC, NULL},
//  {"ADC2", TID_WORD, N_ADC, NULL},
//  {"ADC3", TID_WORD, N_ADC, NULL},

  {""},
};

BANK_LIST scaler_bank_list[] = {
  {"SCLR", TID_DWORD, N_SCLR, NULL},

  {""},
};

/*-- Equipment list ------------------------------------------------*/
#undef USE_INT

EQUIPMENT equipment[] = {
  /* ADC event: ID=3 */
  { "Expcvadc",         /* Equipment name */

    { 3, 0,             /* Event ID, Expcvadc mask */
      "SYSTEM",         /* Event buffer */

#ifdef USE_INT
      EQ_INTERRUPT,     /* Equipment type */
#else
      EQ_POLLED,
#endif

      0,                /* Event source */
      "MIDAS",          /* Format */
      TRUE,             /* Enabled */
      RO_RUNNING 
      | RO_ODB,         /* Read only when running and update ODB */
      1,                /* poll for 1ms */
      0,                /* Stop run after this event limit */
      0,                /* Number of sub-events */
      0,                /* don't log history */
      "", "", "", 
    },

    read_expcvadc_event, /* readout routine */
    NULL,                /* Class driver routine */
    NULL,                /* Device driver list */
    expcvadc_bank_list,  /* Bank list */
  },

  /* Scaler event: ID=4 */
  { "Scaler",
    {4, 0,
     "SYSTEM",
     EQ_PERIODIC | EQ_MANUAL_TRIG,  /* Equipment type */
     0,
     "MIDAS",
     TRUE,
     RO_RUNNING | RO_TRANSITIONS |  /* read when running and no transitions */
     RO_ODB,                        /* and update ODB */
     2000,                         /* read every 2 sec */
     0, 0, 0,
     "", "", "",},
    read_scaler_event,  /* Readout scaler event */
    NULL, NULL,
    scaler_bank_list,
  },

  {""}
};

/*---- sequencer callback info --------------------------*/
void seq_callback(INT hDB, INT hseq, void *info)
{
  printf("odb ... expcvadc settings touched\n");
}

/*---- Frontend Initialize ------------------------------*/
INT frontend_init()
{
  char set_str[80];
  INT  status, size; 

  /*---- Book Setting Space -------------------*/
  EXPCVADC_SETTINGS_STR(expcvadc_settings_str);

  /*--- ODB setup ---*/
  sprintf(set_str, "/Equipment/Expcvadc/Settings");
  status = db_create_record(hDB, 0, set_str, strcomb(expcvadc_settings_str));
  status = db_find_key(hDB, 0, set_str, &hSet);
  if( status != DB_SUCCESS )
    cm_msg(MINFO, "FE", "Key %s not found", set_str);

  /* Enable hot-link on settings of the equipment */
  size = sizeof(EXPCVADC_SETTINGS);
  if((status = db_open_record(hDB, hSet, &exp_set, size, MODE_READ,
			      seq_callback, NULL)) != DB_SUCCESS)
    return status;

  /*--- Hardware initialization ---*/
  /* open VME */
  status = mvme_open( &pvme, 0 );
  if( status != MVME_SUCCESS )
    return status;
  else
    cm_msg(MINFO, "FE", "The 1st VME interface opened successfully.");
  status = mvme_sysreset(pvme);
  mvme_set_am(pvme, MVME_AM_A32_ND);
  mvme_set_dmode(pvme, MVME_DMODE_D32);

  /* V785N Setup */
  if( exp_set.v785n_1.enable ) { /* Software reset V785N 1 */
    cm_msg(MINFO, "FE", "Reseting the 1st CAEN V785N");
    vadc_SingleShotReset(pvme, exp_set.v785n_1.address);
  }
  if( exp_set.v785n_2.enable ) { /* Software reset V785N 2 */
    cm_msg(MINFO, "FE", "Reseting the 2nd CAEN V785N");
    vadc_SingleShotReset(pvme, exp_set.v785n_2.address); 
  }
  if( exp_set.v785n_3.enable ) { /* Software reset V785N 3 */
    cm_msg(MINFO, "FE", "Reseting the 3rd CAEN V785N");
    vadc_SingleShotReset(pvme, exp_set.v785n_3.address);
  }
  if( exp_set.v785n_4.enable ) { /* Software reset V785N 4 */
    cm_msg(MINFO, "FE", "Reseting the 4th CAEN V785N");
    vadc_SingleShotReset(pvme, exp_set.v785n_4.address); 
  }

  /* V560 Status */
  if( exp_set.v560_1.enable ) {
    cm_msg(MINFO, "FE", "Reseting the CAEN V560");
    cv560_Reset(pvme, exp_set.v560_1.address);
    cv560_Status(pvme, exp_set.v560_1.address);
  }

  return SUCCESS;
}

/*---- Frontend Exit -----------------------------------*/
INT frontend_exit() { return SUCCESS; }

/*---- Begin of Run ------------------------------------*/
INT begin_of_run( INT rnum, char *error) 
{
  INT  status, size; 

  /* read Expcvadc settings */
  size = sizeof(EXPCVADC_SETTINGS);
  if( (status = db_get_record(hDB, hSet, &exp_set, &size, 0)) != DB_SUCCESS )
    return status;

  /*--- Initialize V785N ---*/
  if( exp_set.v785n_1.enable ) {
    cm_msg(MINFO, "FE", "Initializing the 1st CAEN V785N");
    vadc_SingleShotReset( pvme, exp_set.v785n_1.address );
    vadc_Setup( pvme, exp_set.v785n_1.address, exp_set.v785n_1.mode );
    vadcn_ThresholdWrite( pvme, exp_set.v785n_1.address, 
			  exp_set.v785n_1.threshold );
  }
  if( exp_set.v785n_2.enable ) {
    cm_msg(MINFO, "FE", "Initializing the 2nd CAEN V785N");
    vadc_SingleShotReset(pvme, exp_set.v785n_2.address);
    vadc_Setup(pvme, exp_set.v785n_2.address, exp_set.v785n_2.mode);
    vadcn_ThresholdWrite( pvme, exp_set.v785n_2.address, 
			  exp_set.v785n_2.threshold ); 
  }
  if( exp_set.v785n_3.enable ) {
    cm_msg(MINFO, "FE", "Initializing the 3rd CAEN V785N");
    vadc_SingleShotReset( pvme, exp_set.v785n_3.address );
    vadc_Setup( pvme, exp_set.v785n_3.address, exp_set.v785n_3.mode );
    vadcn_ThresholdWrite( pvme, exp_set.v785n_3.address, 
			  exp_set.v785n_3.threshold );
  }
  if( exp_set.v785n_4.enable ) {
    cm_msg(MINFO, "FE", "Initializing the 4th CAEN V785N");
    vadc_SingleShotReset(pvme, exp_set.v785n_4.address);
    vadc_Setup(pvme, exp_set.v785n_4.address, exp_set.v785n_4.mode);
    vadcn_ThresholdWrite( pvme, exp_set.v785n_4.address, 
			  exp_set.v785n_4.threshold ); 
  }

  /*--- Reset scaler V560 ---*/
  if( exp_set.v560_1.enable ) {
    cv560_Reset(pvme, exp_set.v560_1.address);
    cv560_Status(pvme, exp_set.v560_1.address);
  }

  return SUCCESS;
}

/*---- End of Run --------------------------------------*/
INT end_of_run( INT rnum, char *error)
{
  if( exp_set.v785n_1.enable )
    vadc_SingleShotReset(pvme, exp_set.v785n_1.address);
  if( exp_set.v785n_2.enable )
    vadc_SingleShotReset(pvme, exp_set.v785n_2.address);
  if( exp_set.v785n_3.enable )
    vadc_SingleShotReset(pvme, exp_set.v785n_3.address);
  if( exp_set.v785n_4.enable )
    vadc_SingleShotReset(pvme, exp_set.v785n_4.address);

  return SUCCESS;
}

/*---- Pause Run --------------------------------------*/
INT pause_run( INT rnum, char *error) {  return SUCCESS; }

/*---- Resume Run ------------------------------------*/
INT resume_run( INT rnum, char *error) { return SUCCESS; }

/*---- Frontend Loop ---------------------------------*/
INT frontend_loop() {  return SUCCESS; }

/*---- Expcvadc event routines ------------------------*/
INT poll_event( INT source, INT count, BOOL test)
{
  int i;
  for(i=0; i<count; i++)
    if((exp_set.v785n_1.enable && vadc_isDataReady(pvme, exp_set.v785n_1.address)) ||
       (exp_set.v785n_2.enable && vadc_isDataReady(pvme, exp_set.v785n_2.address)) ||
       (exp_set.v785n_1.enable && vadc_isDataReady(pvme, exp_set.v785n_1.address)) ||
       (exp_set.v785n_2.enable && vadc_isDataReady(pvme, exp_set.v785n_2.address)) )
      if(!test) return 1;

  return 0;
}

/*---- Interrupt configuration ----------------------*/
INT interrupt_configure( INT cmd, INT source, POINTER_T adr)
{
  return SUCCESS;
}

/*---- Event Readout -------------------------------*/
INT read_expcvadc_event( char *pevent, INT off)
{
  int    evt_dat[32], i;
  WORD  *pdata;

  /* =================================== */
  /*  cm_msg(MINFO, "FE", "Read From V785N ADC0"); */
  /* =================================== */

  bk_init(pevent);

  /* Create ADC0 bank for Raw ADCs */
  bk_create(pevent, "ADC0", TID_WORD, &pdata);
  for( i=0; i<N_ADC; i++ ) { pdata[i] = 0L; evt_dat[i] = 0; }

  /* ====================== */
  if( exp_set.v785n_1.enable && 
      vadc_isDataReady( pvme, exp_set.v785n_1.address ) ) {

    if( ! vadcn_GetEvent( pvme, exp_set.v785n_1.address, evt_dat ) )
      for( i=0; i<N_ADC; i++ )
	if( evt_dat[i]>0 ) pdata[i] = evt_dat[i] ;
  }
  bk_close(pevent, pdata+N_ADC);

  /* Create ADC1 bank for Raw ADCs */
/*
  bk_create(pevent, "ADC1", TID_WORD, &pdata);
  for( i=0; i<N_ADC; i++ ) { pdata[i] = 0L; evt_dat[i] = 0; }

  if( exp_set.v785n_2.enable && 
      vadc_isDataReady(pvme, exp_set.v785n_2.address) ) {
    if( ! vadcn_GetEvent( pvme, exp_set.v785n_2.address, evt_dat ) )
      for( i=0; i<N_ADC; i++ )
	if( evt_dat[i]>0 ) pdata[i] = evt_dat[i] ;
  }

  bk_close(pevent, pdata+N_ADC);
*/

  /* Create ADC2 bank for Raw TDCs */
/*
  bk_create(pevent, "ADC2", TID_WORD, &pdata);
  for( i=0; i<N_TDC; i++ ) { pdata[i] = 0L; evt_dat[i] = 0; }
  
  if( exp_set.v785n_3.enable &&
      vadc_isDataReady(pvme, exp_set.v785n_3.address) ) {
    if( ! vadcn_GetEvent( pvme, exp_set.v785n_3.address, evt_dat ) )
      for( i=0; i<N_TDC; i++ )
	if( evt_dat[i]>0 ) pdata[i] = evt_dat[i] ;
  }

  bk_close(pevent, pdata+N_TDC);
*/

  /* Create ADC3 bank for Raw TDCs */
/*
  bk_create(pevent, "ADC3", TID_WORD, &pdata);
  for( i=0; i<N_TDC; i++ ) { pdata[i] = 0L; evt_dat[i] = 0; }

  if( exp_set.v785n_4.enable &&
      vadc_isDataReady(pvme, exp_set.v785n_4.address) ) {
    if( ! vadcn_GetEvent( pvme, exp_set.v785n_4.address, evt_dat ) )
      for( i=0; i<N_TDC; i++ )
	if( evt_dat[i]>0 ) pdata[i] = evt_dat[i];
  }

  bk_close(pevent, pdata+N_TDC);
*/

  return bk_size(pevent); /* return bank size ... */
}

/*---- Scaler event --------------------------------*/
INT read_scaler_event(char *pevent, INT off)
{
   DWORD *pdata;
   int i;

   /* init bank structure */
   bk_init(pevent);

   /* create SCLR bank */
   bk_create(pevent, "SCLR", TID_DWORD, &pdata);
   for(i=0; i<N_SCLR; i++) pdata[i] = 0L;
 
   /* read scaler bank */
   if( exp_set.v560_1.enable )
     cv560_Read( pvme, exp_set.v560_1.address, pdata );

   bk_close(pevent, pdata+N_SCLR);

   return bk_size(pevent);
}

