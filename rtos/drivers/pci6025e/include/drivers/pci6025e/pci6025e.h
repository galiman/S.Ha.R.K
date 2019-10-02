/*****************************************************************************
* Filename:    pci6025e.h                                                    *
* Author:      Ziglioli Marco                                                *
* Date:        15/03/2001                                                    *
* Last update:                                                               *
* Description: Header file which contains declaration of structure variables *
*              and routines used to interface with PCI6025E                  *
*----------------------------------------------------------------------------*
* Notes:       Based on National C Routines                                  *
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

#ifndef _MY_PCI6025E_
#define _MY_PCI6025E_

#include <kernel/kern.h>
#include <ll/i386/hw-instr.h>

#include "regconst.h"

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

//#define  __REG_DEBUG__         //enable this in debug mode to see
                                 //configuration registers value

#define  NI_CODE                 0x1093         //NI Vendor_ID board code

#define  Board_Address           STC_Base_Address

#define  BAR0                    0xE8000L       //New address of MITE
#define  BAR1                    0xEA000L       //New address of STC

#define  INT_NO                   NIDevice_info[0].InterruptLevel

struct pci6025e_deviceinfo {
   WORD     DEVID;
   BYTE     DevFunction;
   BYTE     BusNumber;
   DWORD    BAR0Value;
   DWORD    IntLineRegValue;
   DWORD    RevisionID;
   BYTE     InterruptLevel;
};

extern struct pci6025e_deviceinfo NIDevice_info[10];
extern DWORD *IntLinestructptr;
extern DWORD *BAR0structptr;
extern DWORD *RevID;

extern DWORD    STC_Base_Address, MITE_Base_Address;

#define set(b,p)        b|=(0x01 << p)    //set p-th bit of byte b to 1
#define clr(b,p)        b&=~(0x01 << p)   //set p-th bit of byte b to 0

//scan PCI bus to find board and remap it on memory above 1MB
BYTE find_NI_Device(void);
BYTE reMap(void);

//IO Windowed access board registers
void DAQ_STC_Windowed_Mode_Write(WORD reg_addr, WORD value);
WORD DAQ_STC_Windowed_Mode_Read(WORD reg_addr);

//On board clock
BYTE  setIntClock(BYTE, BYTE, BYTE);

//PFI programming
void  PFIprogramming(WORD);

//Interrupt management
void  INT_personalize(BYTE);
void  INT_setup(BYTE, BYTE);

//needful macros
#define Immediate_Readb(addr)       *((BYTE *)(Board_Address + (addr)))
#define Immediate_Readw(addr)       *((WORD *)(Board_Address + (addr)))
#define Immediate_Writeb(addr, val) *((BYTE *)(Board_Address + (addr)))=(val)
#define Immediate_Writew(addr, val) *((WORD *)(Board_Address + (addr)))=(val)

void  bitfield(BYTE dim, DWORD value);
void  TEST_bitfield(BYTE dim, DWORD value, char *str);

//Software copy of STC general registers
extern WORD joint_reset, interrupt_a_enable, interrupt_a_ack,
            interrupt_b_enable, interrupt_b_ack, clock_and_fout;

__END_DECLS
#endif
/*End of File: Pci6025e.h*/
