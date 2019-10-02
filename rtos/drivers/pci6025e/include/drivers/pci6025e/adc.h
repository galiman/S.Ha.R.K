/*****************************************************************************
* Filename:       ADC.H                                                      *
* Author:         Ghiro Andrea,Franchino Gianluca                            *
* Date:           09/2003                                                    *          
* Description:    Analog 2 digital conversion package header file            *
*----------------------------------------------------------------------------*
* Notes:                                                                     *
*****************************************************************************/

/* This file is part of the S.Ha.R.K. Project - http://shark.sssup.it 
 *
 * Copyright (C) 2003 Ghiro Andrea,Franchino Gianluca
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


#ifndef  _MY_ADC_H_
#define  _MY_ADC_H_

#include <drivers/pci6025e/pci6025e.h>
#include <drivers/pci6025e/regconst.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

//Analog input releted functions
void Configure_Board(BYTE);
void ADC_Init(void);
void AI_Start_The_Acquisition(void);

__END_DECLS
#endif
/*End of file:    adc.h*/

