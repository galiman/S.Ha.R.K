/*****************************************************************************
* Filename:       DAC.H                                                      *      *
* Author:         Marco Ziglioli (Doctor Stein)                              *        *
* Date:           06/06/2001                                                 *          *
* Description:    Digital 2 analog conversion package header file            *
*----------------------------------------------------------------------------*
* Notes:                                                                     *
*****************************************************************************/

/* This file is part of the S.Ha.R.K. Project - http://shark.sssup.it 
 *
 * Copyright (C) 2001 Marco Ziglioli
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef  _MY_DAC_H_
#define  _MY_DAC_H_

#include <drivers/pci6025e/pci6025e.h>
#include <drivers/pci6025e/regconst.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define  DAC0     0
#define  DAC1     1

//Analog output releted functions
void  DAC_reset(void);
void  DAC_boardInit(BYTE, WORD);
void  DAC_trigger(WORD);
void  DAC_numSetup(DWORD, DWORD);
void  DAC_update(BYTE, BYTE);
void  DAC_channel(BYTE, BYTE);
void  DAC_LDACSourceUpdate(BYTE);
void  DAC_stopError(BYTE);
void  DAC_FIFOFlags(BYTE);
void  DAC_enableInterrupts(WORD);
void  DAC_arm(void);
void  DAC_startOperation(void);

void  DAC_Init(void);
void  DAC_output(BYTE, WORD);

__END_DECLS
#endif
/*End of file:    DAC.H*/
