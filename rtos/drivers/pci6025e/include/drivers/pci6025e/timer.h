/*****************************************************************************
* Filename:    Timer.h                                                       *
* Author:      Ziglioli Marco (Doctor Stein)                                 *
* Date:        23/03/2001                                                    *
* Last update:                                                               *
* Description: Header file for routines used to configure and manage General *
*              Purpouse Timer Conter on PCI6025E board                       *
*----------------------------------------------------------------------------*
* Notes:       I only need time measurament, other features like events      *
*              count position sensing or signal generation are not           *
*              implemented yet                                               *
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


#ifndef  _PCI6025_TIMER_H_
#define  _PCI6025_TIMER_H_

#include <drivers/pci6025e/pci6025e.h>
#include <drivers/pci6025e/regconst.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define C0     0
#define C1     1

WORD  g0_command, g0_mode, g0_input_select, g0_autoincrement,
      g1_command, g1_mode, g1_input_select, g1_autoincrement;

void  reset_counter_0(void);
void  reset_counter_1(void);
void  arm_counter_0(void);
void  arm_counter_1(void);
void  disarm_counter_0(void);
void  disarm_counter_1(void);

void  TIM_reset(BYTE);
void  TIM_arm(BYTE);
void  TIM_disarm(BYTE);

DWORD TIM_readCounter(BYTE);
DWORD TIM_readHWSaveReg(BYTE);

void  TIM_eventCounting(BYTE, BYTE, BYTE, BYTE, DWORD);
void  TIM_bufferedEventCounting(BYTE, BYTE, BYTE, BYTE, DWORD);
void  TIM_timeMeasurement(BYTE, BYTE, BYTE, BYTE, BYTE, DWORD);
void  TIM_bufferedTimeMeasurement(BYTE, BYTE, BYTE, BYTE, DWORD);

__END_DECLS
#endif
/*--------------------------------------------------------------------------*/
