/*********************************************************************

  Name:         v560.c
  Created by:   K.Olchanski

  Contents:     CAEN V560 16-channel 32-bit scaler

  $Id$
*********************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cv560.h"
#include "mvmestd.h"

/*****************************************************************/
/*
Read V560 register value
*/
static uint16_t cv560_RegRead(MVME_INTERFACE *mvme, DWORD base, int offset)
{
  mvme_set_am(mvme, MVME_AM_A32);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  return mvme_read_value(mvme, base + offset);
}

/*****************************************************************/
/*
Write V560 register value
*/
static void cv560_RegWrite(MVME_INTERFACE *mvme, DWORD base, int offset, uint16_t value)
{
  mvme_set_am(mvme, MVME_AM_A32);
  mvme_set_dmode(mvme, MVME_DMODE_D16);
  mvme_write_value(mvme, base + offset, value);
}

uint16_t cv560_RegisterRead(MVME_INTERFACE *mvme, DWORD base, int offset)
{
  return cv560_RegRead(mvme,base,offset);
}

void     cv560_RegisterWrite(MVME_INTERFACE *mvme, DWORD base, int offset, uint16_t value)
{
  cv560_RegWrite(mvme,base,offset,value);
}

void cv560_Read(MVME_INTERFACE *mvme, DWORD base, uint32_t data[16])
{
  mvme_set_am(mvme, MVME_AM_A32);
  mvme_set_dmode(mvme, MVME_DMODE_D32);
  mvme_read(mvme, data, base + 0x10, 16*4);
}

/*****************************************************************/
void cv560_Reset(MVME_INTERFACE *mvme, DWORD base)
{
  cv560_RegRead(mvme,base,0x54); // VME VETO reset
  cv560_RegRead(mvme,base,0x50); // scalers clear, vme interrupt clear and disable
}

/*****************************************************************/
void  cv560_Status(MVME_INTERFACE *mvme, DWORD base)
{
  printf("CAEN V560 at A32 0x%x: version 0x%x, type 0x%x, code 0x%x, scaler status: 0x%x\n",
	 (int)base,cv560_RegRead(mvme,base,0xFE),cv560_RegRead(mvme,base,0xFC),
	 cv560_RegRead(mvme,base,0xFA),cv560_RegRead(mvme,base,0x58));
}

//end
