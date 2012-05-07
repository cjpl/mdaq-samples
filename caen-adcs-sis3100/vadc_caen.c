/*********************************************************************\
  Name:         vadc_caen.c  $Name$ - $Id$
  Created by:   Exaos Lee

  Contents:  CAEN ADC-modules --
             V785   ----  32 channels 12-bit peak sensing ADC
             V775   ----  32 channels 12-bit TDC
             V792   ----  32 channels 12-bit QDC
             V785N  ----  16 channels 12-bit peak sensing ADC
             V775N  ----  16 channels 12-bit TDC
             V792N  ----  16 channels 12-bit QDC

  $Log:$
\*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

#if defined(OS_LINUX)
#include <unistd.h>
#endif

#include "vadc_caen.h"

/************** Generic *******************/

vadc_ChannelData vadc_ReformData(vadc_Data *v)
{
  vadc_ChannelData formatDat;
  formatDat.data = v->data.adc;
  formatDat.channel = v->data.channel;

  return formatDat;
}

vadc_ChannelData vadcn_ReformData(vadc_Data *v)
{
  vadc_ChannelData formatDat;
  formatDat.data = v->data.adc;
  formatDat.channel = v->data.channel/2;

  return formatDat;
}

void vadc_Write16(MVME_INTERFACE *mvme, DWORD base, int offset, WORD value)
{
  int cmode;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base+offset, value);
  mvme_set_dmode(mvme, cmode);
}

WORD vadc_Read16(MVME_INTERFACE *mvme, DWORD base, int offset)
{
  int  cmode;
  WORD data;
  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  data = mvme_read_value(mvme, base+offset);
  mvme_set_dmode(mvme, cmode);
  return data;
}

/************** Registers and parameters *******************/

/*** Interrupts ***/

void vadc_IntSet(MVME_INTERFACE *mvme, DWORD base, int level, int vector)
{
  vadc_Write16(mvme, base, VADC_INT_VECTOR_WO, (vector & 0xFF));
  vadc_Write16(mvme, base, VADC_INT_LEVEL_WO,  (level  & 0x7));
}

void vadc_IntEnable(MVME_INTERFACE *mvme, DWORD base, int level)
{
  vadc_Write16(mvme, base, VADC_EVTRIG_REG_RW, (level & 0x1F));
}

void vadc_IntDisable(MVME_INTERFACE *mvme, DWORD base)
{
  vadc_Write16(mvme, base, VADC_EVTRIG_REG_RW, 0);
}

/* Status registers */

int vadc_CSR1Read(MVME_INTERFACE *mvme, DWORD base)
{ return vadc_Read16(mvme, base, VADC_CSR1_RO); }

int vadc_CSR2Read(MVME_INTERFACE *mvme, DWORD base)
{ return vadc_Read16(mvme, base, VADC_CSR2_RO); }

/*****************************************************************/
/*** Bit Set 2 and all options ***/

int vadc_BitSet2Read(MVME_INTERFACE *mvme, DWORD base)
{ return vadc_Read16(mvme, base, VADC_BIT_SET2_RW); }

void vadc_BitSet2Set(MVME_INTERFACE *mvme, DWORD base, WORD pat)
{ vadc_Write16(mvme, base, VADC_BIT_SET2_RW, pat & 0xFFFF); }

void vadc_BitSet2Clear(MVME_INTERFACE *mvme, DWORD base, WORD pat)
{ vadc_Write16(mvme, base, VADC_BIT_CLEAR2_WO, pat & 0xFFFF); }

void vadc_MemTestEnable (MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Clear(mvme, base, 0x1); }
void vadc_MemTestDisable(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Set(mvme, base, 0x1); }

void vadc_SetOnline(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Clear(mvme, base, 0x2); }
void vadc_SetOffline(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Set(mvme, base, 0x2); }

void vadc_OverCheckEnable (MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Clear(mvme,base,0x8); }
void vadc_OverCheckDisable(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Set(mvme, base, 0x8); }

void vadc_LowCheckEnable(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Clear(mvme,base,0x10); }
void vadc_LowCheckDisable(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Set(mvme, base, 0x10); }

void v775_ValidControlEnable (MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Clear(mvme,base,0x20); }
void v775_ValidControlDisable(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Set(mvme, base, 0x20); }

void vadc_SlideEnable (MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Set(mvme, base, 0x80); }
void vadc_SlideDisable(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Clear(mvme,base,0x80); }

/* If step==2, then Thresholdx2; else Thresholdx16 */
void vadc_ThresholdStep(MVME_INTERFACE *mvme, DWORD base, WORD step)
{
  if( 2 == step )
    vadc_BitSet2Set(mvme, base, 0x100);
  else
    vadc_BitSet2Clear(mvme, base, 0x100);
}

void vadc_WriteEmptyEnable(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Set(mvme, base, 0x1000); }
void vadc_WriteEmptyDisable(MVME_INTERFACE *mvme, DWORD base)
{ vadc_BitSet2Clear(mvme,base,0x1000); }

/*****************************************************************/

void vadc_EvtTriggerSet(MVME_INTERFACE *mvme, DWORD base, int count)
{ vadc_Write16(mvme, base, VADC_EVTRIG_REG_RW, (count & 0x1F)); }

/**
 * Write Thresholds and read them back
 */
int vadc_ThresholdWrite(MVME_INTERFACE *mvme, DWORD base, WORD *threshold)
{
  int k, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  for (k=0; k<VADC_MAX_CHANNELS ; k++) {
    mvme_write_value(mvme, base+VADC_THRES_BASE+2*k, threshold[k] & 0x1FF);
  }

  for (k=0; k<VADC_MAX_CHANNELS ; k++) {
    threshold[k] = mvme_read_value(mvme, base+VADC_THRES_BASE+2*k) & 0x1FF;
  }

  mvme_set_dmode(mvme, cmode);
  return VADC_MAX_CHANNELS;
}

int vadcn_ThresholdWrite(MVME_INTERFACE *mvme, DWORD base, WORD *threshold)
{
  int k, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  for (k=0; k<VADCN_MAX_CHANNELS ; k++) {
    mvme_write_value(mvme, base+VADC_THRES_BASE+4*k, threshold[k] & 0x1FF);
  }

  for (k=0; k<VADCN_MAX_CHANNELS ; k++) {
    threshold[k] = mvme_read_value(mvme, base+VADC_THRES_BASE+4*k) & 0x1FF;
  }

  mvme_set_dmode(mvme, cmode);
  return VADCN_MAX_CHANNELS;
}

/**
 * Read Thresholds
 */
int vadc_ThresholdRead(MVME_INTERFACE *mvme, DWORD base, WORD *threshold)
{
  int k, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);

  for (k=0; k<VADC_MAX_CHANNELS ; k++) {
    threshold[k] = mvme_read_value(mvme, base+VADC_THRES_BASE+2*k) & 0x1FF;
  }
  mvme_set_dmode(mvme, cmode);
  return VADC_MAX_CHANNELS;
}

int vadcn_ThresholdRead(MVME_INTERFACE *mvme, DWORD base, WORD *threshold)
{
  int k, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);

  for (k=0; k<VADCN_MAX_CHANNELS ; k++) {
    threshold[k] = mvme_read_value(mvme, base+VADC_THRES_BASE+4*k) & 0x1FF;
  }
  mvme_set_dmode(mvme, cmode);
  return VADCN_MAX_CHANNELS;
}

/**
   Sets all the necessary paramters for a given configuration.  The
   configuration is provided by the mode argument.  Add your own configuration
   in the case statement. Let me know your setting if you want to include it in
   the distribution.
 */
int  vadc_Setup(MVME_INTERFACE *mvme, DWORD base, int mode)
{
  switch (mode) {
  case 0x1:
    printf("Default setting after power up (mode:%d)\n\n", mode);
    printf("WriteEmpty Disable, Over Range Enable, Low Th Enable\n");
    vadc_SetOnline(mvme, base);
    vadc_WriteEmptyDisable(mvme, base);
    vadc_OverCheckEnable(mvme, base);
    vadc_LowCheckEnable(mvme, base);
    break;
  case 0x2:
    printf("Modified setting (mode:%d)\n", mode);
    printf("WriteEmpty Enable, Over Range disable, Low Th Enable\n");
    vadc_SetOnline(mvme, base);
    vadc_WriteEmptyEnable(mvme, base);
    vadc_OverCheckDisable(mvme, base);
    vadc_LowCheckEnable(mvme, base);
    break;
  case 0x3:
    printf("Modified setting (mode:%d)\n", mode);
    printf("WriteEmpty enable, Over Range disable, LowTh disable");
    vadc_SetOnline(mvme, base);
    vadc_WriteEmptyEnable(mvme, base);
    vadc_OverCheckDisable(mvme, base);
    vadc_LowCheckDisable(mvme, base);
    break;
  case 0x4:
    printf("Modified setting (mode:%d)\n", mode);
    printf("WriteEmpty enable, Over Range enable, LowTh enable");
    vadc_SetOnline(mvme, base);
    vadc_WriteEmptyEnable(mvme, base);
    vadc_OverCheckEnable(mvme, base);
    vadc_LowCheckEnable(mvme, base);
    break;
  default:
    printf("Unknown setup mode\n");
    return -1;
  }
  return 0;
}

/************** Status checking and switching *******************/

void  vadc_PrintStatus0(MVME_INTERFACE *mvme, DWORD base, int mode)
{
  int  status, cmode, i;
  WORD threshold[VADC_MAX_CHANNELS];

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  printf("CAEN ADC-like module at VME A32 0x%06x:\n", base);
  status = mvme_read_value(mvme, base+VADC_FIRM_REV_RO);
  printf("Firmware revision: 0x%x\n", status);
  if (status == 0xFFFF) {
    printf("Module not present!\n");
    return;
  }

  status = vadc_CSR1Read(mvme, base);
  printf("CSR1: 0x%x\n", status);
  printf("DataReady    :%s\t", status & 0x1 ? "Y" : "N");
  printf(" - Global Dready:%s\t", status & 0x2 ? "Y" : "N");
  printf(" - Busy         :%s\n", status & 0x4 ? "Y" : "N");
  printf("Global Busy  :%s\t", status & 0x8 ? "Y" : "N");
  printf(" - Amnesia      :%s\t", status & 0x10 ? "Y" : "N");
  printf(" - Purge        :%s\n", status & 0x20 ? "Y" : "N");
  printf("Term ON      :%s\t", status & 0x40 ? "Y" : "N");
  printf(" - TermOFF      :%s\t", status & 0x80 ? "Y" : "N");
  printf(" - Event Ready  :%s\n", status & 0x100 ? "Y" : "N");

  status = vadc_CSR2Read(mvme, base);
  printf("CSR2: 0x%x\n", status);
  printf("Buffer WriteEmpty :%s\t", status & 0x2 ? "Y" : "N");
  printf(" - Buffer Full  :%s\n", status & 0x4 ? "Y" : "N");
  int dtype = (status & 0xF0) >> 4;
  printf("Daughter card type (CSEL/DSEL)  :%d%d%d%d (0x%x) ",
         status & 0x80 ? 1 : 0, 
         status & 0x40 ? 1 : 0,
         status & 0x20 ? 1 : 0,
         status & 0x10 ? 1 : 0,
         dtype);
  switch (dtype) {
  default: printf("\n");   break;
  case 2:  printf("CAEN ... 32ch\n"); break;
  }

  status = vadc_BitSet2Read(mvme, base);
  printf("BitSet2: 0x%x\n", status);
  printf("Test Mem     :%s\t", status & 0x1 ? "Y" : "N");
  printf(" - Offline      :%s\t", status & 0x2 ? "Y" : "N");
  printf(" - Clear Data   :%s\n", status & 0x4  ? "Y" : "N");
  printf("Over Range En:%s\t", status & 0x8  ? "Y" : "N");
  printf(" - Low Thres En :%s\t", status & 0x10 ? "Y" : "N");
  printf(" - Auto Incr    :%s\n", status & 0x20 ? "Y" : "N");
  printf("WriteEmpty Enable :%s\t", status & 0x1000 ? "Y" : "N");
  printf(" - Slide sub En :%s\t", status & 0x2000 ? "Y" : "N");
  printf(" - All Triggers :%s\n", status & 0x4000 ? "Y" : "N");

  vadc_EvtCntRead(mvme, base, &status);
  printf("Event counter: %d\n", status);
  printf("Iped value: %d\n", vadc_Read16(mvme, base, V792_IPED_RW));

  if(mode == 0) {
    vadc_ThresholdRead(mvme, base, threshold);
    for (i=0;i<VADC_MAX_CHANNELS;i+=2) {
      printf("Threshold[%2i] = 0x%4.4x\t   -  ", i, threshold[i]);
      printf("Threshold[%2i] = 0x%4.4x\n", i+1, threshold[i+1]);
    }
  } else {
    vadcn_ThresholdRead(mvme, base, threshold);
    for (i=0;i<VADCN_MAX_CHANNELS;i+=2) {
      printf("Threshold[%2i] = 0x%4.4x\t   -  ", i, threshold[i]);
      printf("Threshold[%2i] = 0x%4.4x\n", i+1, threshold[i+1]);
    }
  }
  mvme_set_dmode(mvme, cmode);
}

void  vadcn_PrintStatus(MVME_INTERFACE *mvme, DWORD base)
{ vadc_PrintStatus0(mvme, base, 1); }

void  vadc_PrintStatus(MVME_INTERFACE *mvme, DWORD base)
{ vadc_PrintStatus0(mvme, base, 0); }

void vadc_SoftReset(MVME_INTERFACE *mvme, DWORD base)
{
  vadc_Write16(mvme, base, VADC_BIT_SET1_RW,   0xA0);
  vadc_Write16(mvme, base, VADC_BIT_CLEAR1_WO, 0xA0);
}

void vadc_DataClear(MVME_INTERFACE *mvme, DWORD base)
{
  vadc_Write16(mvme, base, VADC_BIT_SET2_RW,   0x4);
  vadc_Write16(mvme, base, VADC_BIT_CLEAR2_WO, 0x4);
}

void vadc_SingleShotReset(MVME_INTERFACE *mvme, DWORD base)
{
  vadc_Write16(mvme, base, VADC_SINGLE_RST_WO, 0x1);
}

/**
 * generate a software trigger
 */
void vadc_GenSoftTrigger(MVME_INTERFACE *mvme, DWORD base)
{ vadc_Write16(mvme, base, VADC_SW_COMM_WO, 0); }

void vadc_EvtCntRead(MVME_INTERFACE *mvme, DWORD base, DWORD *evtcnt)
{
  int cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);

  *evtcnt  = mvme_read_value(mvme, base+VADC_EVT_CNT_L_RO) & 0xFFFF;
  *evtcnt += (mvme_read_value(mvme, base+VADC_EVT_CNT_H_RO) << 0x10 ) & 0xFFFF;

  mvme_set_dmode(mvme, cmode);
}

void vadc_EvtCntReset(MVME_INTERFACE *mvme, DWORD base)
{ vadc_Write16(mvme, base, VADC_EVT_CNT_RST_WO, 1); }

int vadc_isBusy(MVME_INTERFACE *mvme, DWORD base)
{
  int status, busy, timeout, cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  timeout = 1000;
  do {
    status = mvme_read_value(mvme, base+VADC_CSR1_RO);
    busy = status & 0x4;
    timeout--;
  } while (busy || timeout);
  mvme_set_dmode(mvme, cmode);
  return (busy != 0 ? 1 : 0);
}

int  vadc_isDataReady(MVME_INTERFACE *mvme, DWORD base)
{
  return vadc_CSR1Read(mvme, base) & 0x1;
}

int  vadc_isEvtReady(MVME_INTERFACE *mvme, DWORD base)
{
  return vadc_CSR1Read(mvme, base) & 0x100;
}

void v775_SetFullScaleRange(MVME_INTERFACE *mvme, DWORD base, WORD fsr)
{ vadc_Write16(mvme, base, V775_FSR_RW, fsr); }

/**
 * Set common start or stop for CAEN V775/V775N:
 * mode = 0 --- Common start
        = 1 --- Common stop
 */
void v775_CommonSet(MVME_INTERFACE *mvme, DWORD base, int mode)
{
  if( mode == 0)
    vadc_BitSet2Clear(mvme, base, 0x400);
  else
    vadc_BitSet2Set(mvme, base, 0x400);
}

/**********************************************************/
void v792_SetI1Pedestal(MVME_INTERFACE *mvme, DWORD base, WORD i1ped)
{ vadc_Write16(mvme, base, V792_IPED_RW, i1ped); }

/************** Data handling *******************/

/**
  Read nentry of data from the data buffer. Will use the DMA engine if size is
  larger then 127 bytes.
 */
int vadc_DataRead(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int *nentry)
{
  int  cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
  *nentry = 128;
  if (vadc_isDataReady(mvme, base)) {
    mvme_read(mvme, pdest, base, *nentry*4);
  }
  mvme_set_dmode(mvme, cmode);
  return *nentry;
}

/**
   Read single event, return event length (number of entries)
   Uses single vme access! (1us/D32)
 */
int vadc_EventRead(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int *nentry)
{
  DWORD hdata;
  int   cmode;

  mvme_get_dmode(mvme, &cmode);
  mvme_set_dmode(mvme, MVME_DMODE_D32);

  *nentry = 0;
  if (vadc_isDataReady(mvme, base)) {
    do {
      hdata = mvme_read_value(mvme, base);
    } while (!(hdata & 0x02000000)); // skip up to the header

    pdest[*nentry] = hdata;
    *nentry += 1;
    do {
      pdest[*nentry] = mvme_read_value(mvme, base);
      *nentry += 1;
    } while (!(pdest[*nentry-1] & 0x04000000)); // copy until the trailer

    nentry--;
  }

  mvme_set_dmode(mvme, cmode);
  return *nentry;
}

int vadc_GetEvent(MVME_INTERFACE *mvme, DWORD base, int *event)
{
  int   count, i, ch;
  DWORD dest[100];
  vadc_Data* v;

  if( vadc_EventRead(mvme, base, dest, &count) ) {
    for( i=0; i<count; i++ ) {
      v = (vadc_Data*)(&dest[i]);
      if( v->data.type == vadc_typeMeasurement ) {
	event[v->data.channel] = v->data.adc;
      }
    }
  } else
    return 1;

  return 0;
}

int vadcn_GetEvent(MVME_INTERFACE *mvme, DWORD base, int *event)
{
  int   count, i, ch;
  DWORD dest[100];
  vadc_Data* v;

  if( vadc_EventRead(mvme, base, dest, &count) ) {
    for( i=0; i<count; i++ ) {
      v = (vadc_Data*)(&dest[i]);
      if( v->data.type == vadc_typeMeasurement ) {
	event[v->data.channel/2] = v->data.adc;
      }
    }
  } else
    return 1;

  return 0;
}

/*** Print data entry ***/

void vadc_PrintEntry(const vadc_Data* v) {
  int ch;
  switch (v->data.type) {
  case vadc_typeMeasurement:
    printf("Data=0x%08x Measurement ch=%3d v=%6d over=%1d under=%1d\n",
	   v->raw,v->data.channel,v->data.adc,v->data.ov,v->data.un);
    break;
  case vadc_typeHeader:
    printf("Data=0x%08x Header geo=%2x crate=%2x cnt=%2d\n",
	   v->raw,v->header.geo,v->header.crate,v->header.cnt);
    break;
  case vadc_typeFooter:
    printf("Data=0x%08x Footer geo=%2x evtCnt=%7d\n",
	   v->raw,v->footer.geo,v->footer.evtCnt);
    break;
  case vadc_typeFiller:
    printf("Data=0x%08x Filler\n",v->raw);
    break;
  default:
    printf("Data=0x%08x Unknown %04x\n",v->raw,v->data.type);
    break;
  }
}

void vadcn_PrintEntry(const vadc_Data* v) {
  int ch;
  switch (v->data.type) {
  case vadc_typeMeasurement:
    printf("Data=0x%08x Measurement ch=%3d v=%6d over=%1d under=%1d\n",
	   v->raw,v->data.channel/2,v->data.adc,v->data.ov,v->data.un);
    break;
  case vadc_typeHeader:
    printf("Data=0x%08x Header geo=%2x crate=%2x cnt=%2d\n",
	   v->raw,v->header.geo,v->header.crate,v->header.cnt);
    break;
  case vadc_typeFooter:
    printf("Data=0x%08x Footer geo=%2x evtCnt=%7d\n",
	   v->raw,v->footer.geo,v->footer.evtCnt);
    break;
  case vadc_typeFiller:
    printf("Data=0x%08x Filler\n",v->raw);
    break;
  default:
    printf("Data=0x%08x Unknown %04x\n",v->raw,v->data.type);
    break;
  }
}

/*****************************************************************/
#ifdef MAIN_ENABLE

int main (int argc, char* argv[])
{
  DWORD VADC_BASE;
  MVME_INTERFACE *mvme;

  int   status,csr,i, cth;
  DWORD cnt;
  DWORD dest[1000];
  WORD  threshold[32];

  if (argc<2) {
    fprintf(stdout, "Usage: %s <VADC-BASE> [Threshold]\n", argv[0]);
    return 0;
  }

  /* Open VME: SIS3100 -- /dev/sis1100_00remote */
  status = mvme_open(&mvme, 0);
  mvme_set_am(mvme, MVME_AM_A32_ND);
  mvme_set_dmode(mvme, MVME_DMODE_D32);

  sscanf(argv[1],"%x",&VADC_BASE);

  vadc_SingleShotReset(mvme, VADC_BASE);

  if(argc>2) sscanf(argv[2], "%d", &cth);
  else cth=8;
  for (i=0;i<VADCN_MAX_CHANNELS;i++) {
    threshold[i] = cth;
  }

  vadcn_ThresholdWrite(mvme, VADC_BASE, threshold);
  vadc_DataClear(mvme, VADC_BASE);
  vadc_Setup(mvme, VADC_BASE, 2);
  vadc_Status(mvme, VADC_BASE);

  while (1) {
    do {
      csr = vadc_CSR1Read(mvme, VADC_BASE);
      printf("CSR1: 0x%x ----  ", csr);
      csr = vadc_CSR2Read(mvme, VADC_BASE);
      printf("CSR2: 0x%x === ", csr);
      printf("Busy : %d\n", vadc_isBusy(mvme, VADC_BASE));
#if defined(OS_LINUX)
      usleep(1);
#endif
    } while (!csr);

    /*  Read Event Counter */
    vadc_EvtCntRead(mvme, VADC_BASE, &cnt);
    printf("Event counter: 0x%x\n", cnt);

    if( vadc_EventRead(mvme, VADC_BASE, dest, &status) ) {
      printf("Event counter: 0x%x\n", cnt);
      printf("count: 0x%x\n", status);
      
      for (i=0;i<status;i++) {
	printf("%06d) ",i);
	vadc_PrintEntry((vadc_Data*)&dest[i]);
      }
    }

    /* Soft reset 
    vadc_SingleShotReset(mvme, VADC_BASE); */
    vadc_DataClear(mvme, VADC_BASE);
  }

  return 1;
}
#endif

/* emacs
 * Local Variables:
 * mode:C
 * mode:font-lock
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
