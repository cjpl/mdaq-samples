/******************************************************************\
  Name:      vadc_caen.h  $Name$ - $Id$
  Create by: Exaos Lee <Exaos DOT Lee AT gmail DOT com>

  Contents:  CAEN ADC-modules --
             V785   ----  32 channels 12-bit peak sensing ADC
             V775   ----  32 channels 12-bit TDC
             V792   ----  32 channels 12-bit QDC
             V785N  ----  16 channels 12-bit peak sensing ADC
             V775N  ----  16 channels 12-bit TDC
             V792N  ----  16 channels 12-bit QDC

  Conventions:
      WORD:  u_int16_t, unsigned short int, 16-bit
      DWORD: u_int32_t, unsigned int, 32-bit
      MVME_INTERFACE: defined in "mvmestd.h", and others started with MVME_
      "vadc_" and "vadcn_":
         1. The purpose of a function is after the prefix "vadc_" or "vadcn_".
         2. Functions starting with "vadc_" apply to all modules except those
            have the same name but started with prefix "vadcn_". The functions
            which have the same purpose but with different prefix means those
            with "vadc_" only apply to modules with 32-channels.
         3. Functions with prefix "vadcn_" only apply to modules end with "N",
            i.e., V775N, V785N, V792N.
      v775_:  means commands only applying to V775, V775N
      v792_:  means commands only applying to V792, V792N

  History:
    2007.12   Created "cvadc.h/c" using APIs from lib_sis3100.a (APIs defiined
              in sis3100_vme_calls.h). The functions were copied from
              "v792.h/c" included in MIDAS/PSI.
    2008.03   "cvadc.h/c" updated
    2009.07   Rewrite "cvadc.h/c" using APIs in "mvmestd.h".
    2009.07   Rename "cvadc.h/c" to "vadc_caen.h/c"
    2010.01   Add a batch of functions to handle BitSet2 register

  $Log:$
\******************************************************************/

#ifndef VADC_INCLUDE_H
#define VADC_INCLUDE_H 1

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "mvmestd.h"

#define  VADCN_MAX_CHANNELS   (DWORD) 16
#define  VADC_MAX_CHANNELS    (DWORD) 32
#define  VADC_SOFT_RESET      (DWORD) (0x1<<7) /* Software Reset */

/* Registers */
#define  VADC_REG_BASE        (DWORD) (0x1000)
#define  VADC_FIRM_REV_RO     (DWORD) (0x1000) /* Firmware revision */
#define  VADC_GEO_ADDR_RW     (DWORD) (0x1002) /* Geo Address */
#define  VADC_MCST_ADDR_RW    (DWORD) (0x1004) /* MCST/CBLT Address */
#define  VADC_BIT_SET1_RW     (DWORD) (0x1006) /* Bit Set 1 */
#define  VADC_BIT_CLEAR1_WO   (DWORD) (0x1008) /* Bit Clear 1 */
#define  VADC_INT_LEVEL_WO    (DWORD) (0x100A) /* Interrupt Level */
#define  VADC_INT_VECTOR_WO   (DWORD) (0x100C) /* Interrupt Vector */
#define  VADC_CSR1_RO         (DWORD) (0x100E) /* Status Register 1 */
#define  VADC_CR1_RW	      (DWORD) (0x1010) /* Control Register 1 */
#define  VADC_ADER_H_RW       (DWORD) (0x1012) /* ADER High */
#define  VADC_ADER_L_RW       (DWORD) (0x1012) /* ADER Low */
#define  VADC_SINGLE_RST_WO   (DWORD) (0x1016) /* Single Shot Reset */
#define  VADC_EVTRIG_REG_RW   (DWORD) (0x1020) /* Event Trigger Register */
#define  VADC_CSR2_RO         (DWORD) (0x1022) /* Status Register 2 */
#define  VADC_EVT_CNT_L_RO    (DWORD) (0x1024) /* Event Counter_L */
#define  VADC_EVT_CNT_H_RO    (DWORD) (0x1026) /* Event Counter_H */
#define  VADC_INCR_EVT_WO     (DWORD) (0x1028) /* Increment Event */
#define  VADC_INCR_OFFSET_WO  (DWORD) (0x102A) /* Increment Offset */
#define  VADC_LOAD_TEST_RW    (DWORD) (0x102C) /* Load Test Register */
#define  VADC_DELAY_CLEAR_RW  (DWORD) (0x102E) /* FCLR Windows */
#define  VADC_BIT_SET2_RW     (DWORD) (0x1032) /* Bit Set 2 */
#define  VADC_BIT_CLEAR2_WO   (DWORD) (0x1034) /* Bit Clear 2 */
#define  VADC_W_MEM_TEST_WO   (DWORD) (0x1036) /* W Memory Test Address */
#define  VADC_MEM_TEST_H_WO   (DWORD) (0x1038) /* Memory Test Word_High */
#define  VADC_MEM_TEST_L_WO   (DWORD) (0x103A) /* Memory Test Word_Low */
#define  VADC_CRATE_SEL_RW    (DWORD) (0x103C) /* Crate Select */
#define  VADC_TEST_EVENT_WO   (DWORD) (0x103E) /* Test Event Write */
#define  VADC_EVT_CNT_RST_WO  (DWORD) (0x1040) /* Event Counter Reset */
#define  VADC_R_TEST_WO       (DWORD) (0x1064) /* R Test Address */
#define  VADC_SW_COMM_WO      (DWORD) (0x1068) /* SW Comm */
#define  VADC_SLIDECONST_RW   (DWORD) (0x106A) /* Slide Constant */
#define  VADC_AAD_RO          (DWORD) (0x1070) /* AAD */
#define  VADC_BAD_RO          (DWORD) (0x1072) /* BAD */
#define  VADC_THRES_BASE      (DWORD) (0x1080) /* Thresholds */

#define  V775_FSR_RW          (DWORD) (0x1060) /* V775/V775N: Full Scale Range */
#define  V792_IPED_RW         (DWORD) (0x1060) /* V792/V792N: Iped */

/************** Generic Data Format *******************/
enum vadc_DataType {
  vadc_typeMeasurement=0,
  vadc_typeHeader     =2,
  vadc_typeFooter     =4,
  vadc_typeFiller     =6
};

typedef union { /* sub-addr: 0x0000~0x0FFC, D32/D64 */
  DWORD raw;
  struct Entry {
    unsigned adc:12; /* bit0 here */
    unsigned ov:1;
    unsigned un:1;
    unsigned _pad_1:2;
    unsigned channel:5; /* ch=channel/2, if V775N/V785N/V792N */
    unsigned _pad_2:3;
    unsigned type:3;
    unsigned geo:5;
  } data ;
  struct Header {
    unsigned _pad_1:8; /* bit0 here */
    unsigned cnt:6;
    unsigned _pad_2:2;
    unsigned crate:8;
    unsigned type:3;
    unsigned geo:5;
  } header;
  struct Footer {
    unsigned evtCnt:24; /* bit0 here */
    unsigned type:3;
    unsigned geo:5;
  } footer;
} vadc_Data;

typedef struct {
  unsigned short channel;
  unsigned short data;
} vadc_ChannelData;

vadc_ChannelData vadc_ReformData (vadc_Data *v);
vadc_ChannelData vadcn_ReformData(vadc_Data *v);

typedef union { /* sub-addr: VADC_CSR1_RO */
  DWORD raw;
  struct {
    unsigned DataReady:1; /* bit0 here */
    unsigned GlobalDataReady:1;
    unsigned Busy:1;
    unsigned GlobalBusy:1;
    unsigned Amnesia:1;
    unsigned Purge:1;
    unsigned TermOn:1;
    unsigned TermOff:1;
    unsigned EventReady:1; /* bit 8 here */
  } data;
} vadc_StatusRegister1;

typedef union { /* sub-addr: VADC_CSR2_RO */
  DWORD raw;
  struct {
    unsigned _pad_1:1; /* bit0 here */
    unsigned BufferEmpty:1;
    unsigned BufferFull:1;
    unsigned _pad_2:1;
    unsigned PB:4;
    /*
      unsigned DSEL0:1;
      unsigned DSEL1:1;
      unsigned CSEL0:1;
      unsigned CSEL1:1; */
  } data;
} vadc_StatusRegister2;

typedef union { /* sub-addr: VADC_CR1_RW */
  DWORD raw;
  struct {
    unsigned _pad_1:2;
    unsigned BlkEnd:1;
    unsigned _pad_2:1;
    unsigned ProgReset:1;
    unsigned BErr:1;
    unsigned Align64:1;
  } data;
} vadc_ControlRegister1;

typedef union { /* sub-addr: VADC_BIT_SET2_RW */
  DWORD raw;
  struct {
    unsigned MemTest:1;
    unsigned OffLine:1;
    unsigned ClearData:1;
    unsigned OverRange:1;
    unsigned LowThresh:1;
    unsigned _pad_1:1;    /* bit5 */
    unsigned TestAcq:1;
    unsigned SLDEnable:1;
    unsigned StepTH:1;
    unsigned _pad_2:2;    /* bits 9-10 */
    unsigned AutoIncr:1;
    unsigned EmptyProg:1;
    unsigned SlideSubEnable:1;
    unsigned AllTrg:1;
  } data;
} vadc_BitSet2Register;

void vadc_Write16(MVME_INTERFACE *mvme, DWORD base, int offset, WORD value);
WORD vadc_Read16 (MVME_INTERFACE *mvme, DWORD base, int offset);

/************** Registers and parameters *******************/
/* Interrupts */
void vadc_IntSet    (MVME_INTERFACE *mvme, DWORD base, int level, int vector);
void vadc_IntDisable(MVME_INTERFACE *mvme, DWORD base);
void vadc_IntEnable (MVME_INTERFACE *mvme, DWORD base, int level);

/* Status registers */
int  vadc_CSR1Read(MVME_INTERFACE *mvme, DWORD base);  /* Status Register 1 */
int  vadc_CSR2Read(MVME_INTERFACE *mvme, DWORD base);  /* Status Register 2 */
  
/* Bit Set 2 Options (VADC_BIT_SET2_RW and VADC_BIT_CLEAR2_WO) */
/* Bit Set 2 and Clear Set 2 are used together for certain option */
int  vadc_BitSet2Read (MVME_INTERFACE *mvme, DWORD base);
void vadc_BitSet2Set  (MVME_INTERFACE *mvme, DWORD base, WORD pat);
void vadc_BitSet2Clear(MVME_INTERFACE *mvme, DWORD base, WORD pat);

/*** Set options one by one ***/
/* Online/Offline: BitSet2--2 */
void vadc_MemTestEnable (MVME_INTERFACE *mvme, DWORD base);
void vadc_MemTestDisable(MVME_INTERFACE *mvme, DWORD base);
void vadc_SetOnline (MVME_INTERFACE *mvme, DWORD base);
void vadc_SetOffline(MVME_INTERFACE *mvme, DWORD base);
void vadc_OverCheckEnable (MVME_INTERFACE *mvme, DWORD base);
void vadc_OverCheckDisable(MVME_INTERFACE *mvme, DWORD base);
void vadc_LowCheckEnable (MVME_INTERFACE *mvme, DWORD base);
void vadc_LowCheckDisable(MVME_INTERFACE *mvme, DWORD base);
void v775_ValidControlEnable (MVME_INTERFACE *mvme, DWORD base);
void v775_ValidControlDisable(MVME_INTERFACE *mvme, DWORD base);
void vadc_SlideEnable (MVME_INTERFACE *mvme, DWORD base);
void vadc_SlideDisable(MVME_INTERFACE *mvme, DWORD base);
/* If step==2, then Thresholdx2; else Thresholdx16 */
void vadc_ThresholdStep(MVME_INTERFACE *mvme, DWORD base, WORD step);

void vadc_WriteEmptyEnable (MVME_INTERFACE *mvme, DWORD base);
void vadc_WriteEmptyDisable(MVME_INTERFACE *mvme, DWORD base);

/* void vadc_CrateSet(MVME_INTERFACE *mvme, DWORD base, DWORD *evtcnt); */
/* void vadc_DelayClearSet(MVME_INTERFACE *mvme, DWORD base, int delay); */

/* Event Trigger */
void vadc_EvtTriggerSet(MVME_INTERFACE *mvme, DWORD base, int count);

/*  Write Thresholds and read them back */
int  vadc_ThresholdWrite (MVME_INTERFACE *mvme, DWORD base, WORD *threshold);
int  vadcn_ThresholdWrite(MVME_INTERFACE *mvme, DWORD base, WORD *threshold);

/* Read Thresholds  */
int  vadc_ThresholdRead (MVME_INTERFACE *mvme, DWORD base, WORD *threshold);
int  vadcn_ThresholdRead(MVME_INTERFACE *mvme, DWORD base, WORD *threshold);

int  vadc_Setup(MVME_INTERFACE *mvme, DWORD base, int mode);

/************** Status checking and switching *******************/
/* mode: 0 -- 32 channels; 1 -- 16 channels */
void vadc_PrintStatus0(MVME_INTERFACE *mvme, DWORD base, int mode);
void vadcn_PrintStatus(MVME_INTERFACE *mvme, DWORD base);
void vadc_PrintStatus (MVME_INTERFACE *mvme, DWORD base);

/* Sets the module to a permanet RESET status. The RESET is released only via
   write access with the relevant Bit Set to 1 in the Bit Clear Register. */
void vadc_SoftReset(MVME_INTERFACE *mvme, DWORD base);

/* Generate a reset signal which clears the data, the write and read pointers,
   the event counter, the peak sections (V775/V775N, V785/V785N), the QAC
   sections (V792/V792N). */
void vadc_DataClear(MVME_INTERFACE *mvme, DWORD base);

/* Performs a module reset. Use it CAREFULLY and for DEBUGging purposes
   only. Recommended to use the SoftReset. */
void vadc_SingleShotReset(MVME_INTERFACE *mvme, DWORD base);

void vadc_GenSoftTrigger(MVME_INTERFACE *mvme, DWORD base);

/* Read Event Counter Value */
void vadc_EvtCntRead (MVME_INTERFACE *mvme, DWORD base, DWORD *evtcnt);
/* Reset event Counter */
void vadc_EvtCntReset(MVME_INTERFACE *mvme, DWORD base);
int  vadc_isBusy(MVME_INTERFACE *mvme, DWORD base);
int  vadc_isDataReady(MVME_INTERFACE *mvme, DWORD base);
int  vadc_isEvtReady(MVME_INTERFACE *mvme, DWORD base);

/************** Special modules *******************/
void v775_SetFullScaleRange(MVME_INTERFACE *mvme, DWORD base, WORD fsr);
void v775_CommonSet(MVME_INTERFACE *mvme, DWORD base, int mode);
void v792_SetI1Pedestal(MVME_INTERFACE *mvme, DWORD base, WORD i1ped);

/************** Data handling *******************/
/* Read nentry of data from the data buffer. Will use the DMA engine
   if size is larger then 127 bytes. */
int vadc_DataRead(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int *nentry);

/* Read single event, return event length (number of entries) */
int vadc_EventRead(MVME_INTERFACE *mvme, DWORD base, DWORD *pdest, int *nentry);

int vadc_GetEvent (MVME_INTERFACE *mvme, DWORD base, int *event);
int vadcn_GetEvent(MVME_INTERFACE *mvme, DWORD base, int *event);

void vadc_PrintEntry (const vadc_Data* v);
void vadcn_PrintEntry(const vadc_Data* v);

#endif  /* End of VADC_INCLUDE_H */
