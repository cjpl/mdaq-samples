/* -*- mode: c -*- **************************************************\
 * Name:    ana4cspu.c
 * Purpose: analyzer code for (n, 2n\gamma) experiment
 * Author:  Exaos Lee ( Exaos.Lee@gmail.com )
 * Date:    2008.03
\********************************************************************/

#include "cvadcexp.h"
#include <time.h>
#include <cstring>

#ifdef __linux__
#define f2cFortran
#endif

const char *analyzer_name  = "cvadcana";
INT   analyzer_loop_period = 0;
INT   odb_size             = DEFAULT_ODB_SIZE;

/* ODB structures */
RUNINFO           runinfo;
EXPCVADC_SETTINGS exp_set;
extern ANA_MODULE adc_raw_module;

ANA_MODULE *expcvadc_module[] = {
  &adc_raw_module,

  NULL
};

BANK_LIST ana_expcvadc_bank_list[] = {

  /* online banks */
  {"ADC0", TID_WORD, N_ADC, NULL},
  {"ADC1", TID_WORD, N_ADC, NULL},
  {"ADC2", TID_WORD, N_ADC, NULL},
  {"ADC3", TID_WORD, N_ADC, NULL},
  
  /* calculated banks */

  {""},
};

BANK_LIST ana_scaler_bank_list[] = {
  {"SCLR", TID_DWORD, N_SCLR, NULL},

  {""},
};

ANALYZE_REQUEST analyze_request[] = {
  {"Expcvadc",
   {3,
    TRIGGER_ALL,
    GET_NONBLOCKING,
    "SYSTEM",
    TRUE,
    "", "", 
   },
   NULL,
   expcvadc_module,
   ana_expcvadc_bank_list,
   1000,
   TRUE,
  },

  {"Scaler",
   {4,
    TRIGGER_ALL,
    GET_ALL,
    "SYSTEM",
    TRUE,
    "", "",
   },
   NULL,
   NULL,
   ana_scaler_bank_list,
   100,
  },

  {""},
};

INT analyzer_init()
{
  HNDLE hDB, hKey;

  RUNINFO_STR(runinfo_str);
  EXPCVADC_SETTINGS_STR(expcvadc_settings_str);

  /* Open ODB structure */
  cm_get_experiment_database(&hDB, NULL);

  db_create_record(hDB, 0, "/Runinfo", strcomb(runinfo_str));
  db_find_key(hDB, 0, "/Runinfo", &hKey);
  if( db_open_record(hDB, hKey, &runinfo, sizeof(runinfo), MODE_READ,
		     NULL, NULL) != DB_SUCCESS ) {
    cm_msg(MERROR, "analyzer_init", "Cannot open \"/Runinfo\" tree in ODB");
    return 0;
  }

  db_create_record(hDB, 0, "/Equipment/Expcvadc/Settings",
		   strcomb(expcvadc_settings_str));
  db_find_key(hDB, 0, "/Equipment/Expcvadc/Settings",  &hKey);
  if( db_open_record( hDB, hKey, &exp_set, sizeof(exp_set),
		      MODE_READ, NULL, NULL) != DB_SUCCESS ) {
    cm_msg(MERROR, "analyzer_init",
	   "Cannot open \"/Equipment/Expcvadc/Settings\" tree in ODB");
    return 0;
  }

  return SUCCESS;
}

INT analyzer_exit()
{
  return CM_SUCCESS;
}

INT ana_begin_of_run(INT run_number, char *error)
{
  return CM_SUCCESS;
}

INT ana_end_of_run(INT run_number, char* error)
{
  FILE  *f;
  time_t now;
  char   str[256];
  INT    size;
  double n;
  HNDLE  hDB;
  BOOL   flag;

  cm_get_experiment_database(&hDB, NULL);

  size = sizeof(flag);
  db_get_value( hDB, 0, "/Logger/Write data", &flag, &size, TID_BOOL, TRUE);
  if( flag && runinfo.online_mode == 1 ) {
    /* update run log */
    size = sizeof(str);
    str[0] = 0;
    db_get_value( hDB, 0, "/Logger/Data Dir", str, &size, TID_STRING, TRUE);
    if( str[0] != 0 )
      if( str[strlen(str)-1] != DIR_SEPARATOR )
	strcat(str, DIR_SEPARATOR_STR);
    strcat(str, "runlog.txt");

    f = fopen(str, "a");

    time(&now);
    strcpy(str, ctime(&now));
    str[10] = 0;

    fprintf(f, "%s\t%3d\t", str, runinfo.run_number);

    strcpy(str, runinfo.start_time);
    str[19] = 0;
    fprintf(f, "%s\t", str+11);

    strcpy(str, ctime(&now));
    str[19] = 0;
    fprintf(f, "%s\t", str+11);

    size = sizeof(n);
    db_get_value(hDB, 0, "/Equipment/Expcvadc/Statistics/Events sent",
		 &n, &size, TID_DOUBLE, TRUE);

    fprintf(f, "%11.5f\n", n/1000);

    fclose(f);
  }

  return CM_SUCCESS;
}

INT ana_pause_run(INT run_number, char* error)
{
  return CM_SUCCESS;
}

INT ana_resume_run(INT run_number, char* error)
{
  return CM_SUCCESS;
}

INT analyzer_loop()
{
  return CM_SUCCESS;
}
