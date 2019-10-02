/*
 *
 * Project: 
 *   Parallel Port S.Ha.R.K. Project
 * 
 * Module: 
 *   ppNRTDrv.c
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
 
/*************************************************************************
* Module     : ppNRTDrv.c
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
 * CVS : $Id: ppnrtdrv.c,v 1.1 2002/10/28 08:03:54 pj Exp $
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


/*************************************************************************/
BOOL ppNRTWaitRTS(void)
{
  BYTE port;
  TIME start = clock();       
  do
    {
      port = ~inp(RX_PORT);       /* port status */
      port = port & RX_CTR;       /* test cntr read bit*/
      if(clock() > start + CLK_TIMEOUT) return TIMEOUT;
    } while(port == 0);
  return TRUE;
}

/*************************************************************************/
BOOL ppNRTWaitDR(void)
{
  BYTE port;
  TIME start = clock();       /* start */
  do {
    port = (~inp(RX_PORT)) | (~RX_CTR);    /* test cntr read bit*/
    if(clock() > start + CLK_TIMEOUT) return TIMEOUT;
  } while(port != 0x7F);         /* 0111 1111 */

  return TRUE;
}

/*************************************************************************/
BOOL ppNRTWaitOTS(void)
{
  BYTE port;
  TIME start = clock();      		 /* start time */
  do
    {
      port = ~inp(RX_PORT);        /* read port status */
      port = port & RX_CTR;        /* test cntr read bit*/
      if(clock() > start + CLK_TIMEOUT) return TIMEOUT;
    } while(port == 0);
  return TRUE;
}

/*************************************************************************/
BOOL ppNRTWaitER(void)
{
  BYTE port;
  TIME start = clock();       /* start */
  do
    {
      port = ~inp(RX_PORT);       /* read port status */
      port = port | (~RX_CTR);    /* test  cntr read bit*/
      if(clock() > start + CLK_TIMEOUT) return TIMEOUT;
    } while(port != 0x7F);        /* 0111 1111  */
  return TRUE;
}


/**************************************************************************/
BOOL ppNRTTxOneByte(BYTE c)
{
  BYTE port;
  /*------------------------- Least Significative Nibble */
  ppSendRTS();                             /* Request To Send */

  port = inp(TX_PORT);             /* read port status   */
  port = port & (~TX_DATA);        /* set tx bits == 0 */
  port = port | (c & LSN);         /* set bit 0..3 with LSN   */

#if PP_DEBUG == 1
  kern_printf("ppNRTTxOneByte: SendRTS, before WaitOTS\n");
#endif

  if(ppNRTWaitOTS()!=TRUE) return TIMEOUT;
  outp(TX_PORT,port);              
  ppSendDR();                              /* Data Ready */

#if PP_DEBUG == 1
  kern_printf("ppNRTTxOneByte: SendDR, before WaitER\n");
#endif


  if(ppNRTWaitER() !=TRUE) return TIMEOUT;

  /*-------------------------  More Significative Nibble */
  ppSendRTS();                         /* Request To Send */

  port = inp(TX_PORT);             /* read port status  */
  port = port & (~TX_DATA);        /* set off trasmission bits... */
  port = port | (c >> 4);          /* set bit 0..3 with MSN   */

#if PP_DEBUG == 1  
  kern_printf("ppNRTTxOneByte: SendRTS, before WaitOTS\n");
#endif

  if(ppNRTWaitOTS()!=TRUE) return TIMEOUT;
  outp(TX_PORT,port);              		 /* send data nibble...  */
  ppSendDR();                              /* Data Ready */

#if PP_DEBUG == 1       
  kern_printf("ppNRTTxOneByte: SendDR, before WaitER\n");
#endif

  if(ppNRTWaitER() !=TRUE) return TIMEOUT;

  return TRUE;
}

/**************************************************************************/
BOOL ppNRTRxOneByte(BYTE *c)
{
  BYTE port;

  if(ppIsRTS() == FALSE) return FALSE;


  ppSendOTS();                               /* Ok To Send */

#if PP_DEBUG == 1   
  kern_printf("ppNRTRxOneByte: IsRTS, SendOTS, before WaitDR\n");
#endif

  if(ppNRTWaitDR() != TRUE) return TIMEOUT;
  port = inp(RX_PORT);                	/* read nibble    */
  ppSendER();                         /*  End Read */
  *c = (port >> 3);                    	/* read LSN */

#if PP_DEBUG == 1      
  kern_printf("ppNRTRxOneByte: SendER, before WaitRTS\n");
#endif

  if(ppNRTWaitRTS() != TRUE) return TIMEOUT;
  ppSendOTS();

#if PP_DEBUG == 1       
  kern_printf("ppNRTRxOneByte: SendOTS, before WaitDR\n");
#endif

  if(ppNRTWaitDR() != TRUE) return TIMEOUT;

#if PP_DEBUG == 1      
  kern_printf("ppNRTRxOneByte: DR received, send ER\n");
#endif

  port = inp(RX_PORT);                 /* read nibble    */
  ppSendER();                      /* End Read */
  *c = (*c & ~MSN);                    /* set 0 c MSN nibble... */
  *c = (*c | ((port >> 3) << 4));      /* read MSN */

  return TRUE;
}


/*************************************************************************
------------------------------------------------------------------------*/
BOOL ppNRTOpenComm(void)
{
  BYTE c,port,rx,tx;
  TIME start = clock();

  outp(TX_PORT,0);
  outp(TX_CTR,0);

  do
    {
      port = inp(RX_PORT);                 /* read nibble    */
      /* x4321x xx */
      c = (port >> 3);                     /* xxxx 4321 */
      c = c & 0xF;                         /* 0000 4321 */
      if(clock() > start + CLK_TIMEOUT*20) return TIMEOUT;
              
    } while (c != 0);     /* test nibble 4321 == 0000 */

  /* the other also is laying down his TX_PORT (==my RX_PORT...) */

  rx = FALSE;	tx = FALSE; start = clock();
  do /* try if it also on line... */
    {

      if((rx == FALSE) && (ppNRTRxOneByte(&c)==TRUE) && (c == BYTE_CTR))	rx = TRUE;
      if((tx == FALSE) && (ppNRTTxOneByte(BYTE_CTR) == TRUE))	tx = TRUE;

      if(clock() > start + CLK_TIMEOUT*20) return TIMEOUT;

    } while(tx==FALSE || rx==FALSE);

  ppSendER();
  ppNRTWaitER();        

  return TRUE;

}

 
