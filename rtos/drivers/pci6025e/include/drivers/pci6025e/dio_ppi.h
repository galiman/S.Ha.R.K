/*****************************************************************************
* Filename:    Dio.h                                                         *
* Author:      Ziglioli Marco                                                *
* Date:        16/03/2001                                                    *
* Last update:                                                               *
* Description: Header file for routines used to configure and use the digital*
*              IO lines of PCI6025E NI board                                 *
*----------------------------------------------------------------------------*
* Notes:       At this time only the eight digital IO lines are programmed   *
*              In the future also the 24 lines of 8255 will be programmed    *
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

#ifndef _MY_DIO_H_
#define _MY_DIO_H_

#include <drivers/pci6025e/pci6025e.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#define ALL_IN          0x0000      //All 8 STC line in input
#define ALL_OUT         0x00FF      //All 8 STC line in output

#define DIO_setup(b)    config(b)   //ALIAS

#ifndef FALSE
#define FALSE           0
#endif

#ifndef TRUE
#define TRUE            1
#endif

void  config(BYTE b);

void  DIO_init(void);
void  DIO_resetReg(void);
void  DIO_config(BYTE);
BYTE  DIO_read(void);
BYTE  DIO_write(BYTE b);
WORD  DIO_setConfig(char *);
WORD  DIO_getConfig(void);

void  PPI_init(void);
void  PPI_resetReg(void);
void  PPI_config(BYTE);
BYTE  PPI_read(BYTE);
void  PPI_write(BYTE, BYTE);
WORD  PPI_setConfig(BYTE, BYTE, BYTE, BYTE, BYTE, BYTE);
WORD  PPI_getConfig(BYTE);
BYTE  PPI_getAddress(BYTE);

__END_DECLS
#endif
/*--------------------------------------------------------------------------*/
