/*
 *
 * Project: 
 *   Parallel Port S.Ha.R.K. Project
 * 
 * Module: 
 *   ppPinDrv.c
 * 
 * Description: 
 *   file contents description 
 * 
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors: 
 *   Andrea Battistotti <btandrea@libero.it>
 *   Armando Leggio     <a_leggio@hotmail.com>
 *
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 *
 */
 
/*******************************************************************************************
* Module     : ppPinDrv.c
* Author     : Andrea Battistotti , Armando Leggio
* Description: Set On/Off single pin of LPT1...
* 2002 @ Pavia  - GNU Copyrights 
*******************************************************************************************/

/*
 * Copyright (C) 2002 Andrea Battistotti , Armando Leggio
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
 * CVS : $Id: pppindrv.c,v 1.1 2002/10/28 08:03:55 pj Exp $
 */


/*******************************************************************************************
* A standard PC provides for three printer ports, at the following base addresses:
*
*   LPT1 = 0x0378 or 0x03BC
*   LPT2 = 0x0278 or 0x0378
*   LPT3 = 0x0278
*
* This module assumes that LPT1 is at 0x0378. 
*
* The printer port has three 8-bit registers:
*
*   Data Register (base + 0) ........ outputs
*
*     7 6 5 4 3 2 1 0
*     . . . . . . . *  D0 ........... (pin 2), 1=High, 0=Low (true)
*     . . . . . . * .  D1 ........... (pin 3), 1=High, 0=Low (true)
*     . . . . . * . .  D2 ........... (pin 4), 1=High, 0=Low (true)
*     . . . . * . . .  D3 ........... (pin 5), 1=High, 0=Low (true)
*     . . . * . . . .  D4 ........... (pin 6), 1=High, 0=Low (true)
*     . . * . . . . .  D5 ........... (pin 7), 1=High, 0=Low (true)
*     . * . . . . . .  D6 ........... (pin 8), 1=High, 0=Low (true)
*     * . . . . . . .  D7 ........... (pin 9), 1=High, 0=Low (true)
*
*   Status Register (base + 1) ...... inputs
*
*     7 6 5 4 3 2 1 0
*     . . . . . * * *  Undefined
*     . . . . * . . .  Error ........ (pin 15), high=1, low=0 (true)
*     . . . * . . . .  Selected ..... (pin 13), high=1, low=0 (true)
*     . . * . . . . .  No paper ..... (pin 12), high=1, low=0 (true)
*     . * . . . . . .  Ack .......... (pin 10), high=1, low=0 (true)
*     * . . . . . . .  Busy ......... (pin 11), high=0, low=1 (inverted)
*
*   Control Register (base + 2) ..... outputs
*
*     7 6 5 4 3 2 1 0
*     . . . . . . . *  Strobe ....... (pin 1),  1=low, 0=high (inverted)
*     . . . . . . * .  Auto Feed .... (pin 14), 1=low, 0=high (inverted)
*     . . . . . * . .  Initialize ... (pin 16), 1=high, 0=low (true)
*     . . . . * . . .  Select ....... (pin 17), 1=low, 0=high (inverted)
*     * * * * . . . .  Unused
*
* Pins 18-25 are ground.  
********************************************************************************************/

#include <drivers/parport.h>


void ppSetDataPin(int state, PIN_MASK pin)
{
  BYTE port; 
  port=ppReadDataByte();
  switch (state)
    {
    case PIN_OFF: 	port &= ~pin;
    case PIN_ON:	port |=  pin;
    }
  ppSetDataByte(port); 
}


void ppSetCtrlPin(int state, PIN_MASK pin)
{
  BYTE port; 
  port=ppReadCtrlByte();
  switch (state)
    {
    case PIN_OFF: 	port &= ~pin;
    case PIN_ON:	port |=  pin;
    }
  ppSetCtrlByte(port); 
}

