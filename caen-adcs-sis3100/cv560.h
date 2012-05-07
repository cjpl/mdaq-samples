/*********************************************************************

  Name:         v560.h
  Created by:   K.Olchanski

  Contents:     CAEN V560 16-channel 32-bit scaler

  $Id$
*********************************************************************/

#ifndef  CV560_INCLUDE_H
#define  CV560_INCLUDE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mvmestd.h"

uint16_t cv560_RegisterRead(MVME_INTERFACE *mvme, DWORD base, int offset);
void     cv560_RegisterWrite(MVME_INTERFACE *mvme, DWORD base, int offset, uint16_t value);
void     cv560_Read(MVME_INTERFACE *mvme, DWORD base, uint32_t data[16]);
void     cv560_Reset(MVME_INTERFACE *mvme, DWORD base);
void     cv560_Status(MVME_INTERFACE *mvme, DWORD base);

#endif // CV560_INCLUDE_H
