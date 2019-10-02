/*
 * Project: 
 *   Parallel Port S.Ha.R.K. Project
 * 
 * Module: 
 *   ppDrv.h
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

/* PPDrv.h 
	header file for par port communication...
*/

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
 * CVS : $Id: parport.h,v 1.1 2003/03/24 10:51:06 pj Exp $
 */

#include <kernel/kern.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/func.h>
#include <string.h>
#include <ll/ll.h>
#include <kernel/types.h>
#include <kernel/descr.h>
#include <math.h>
#include "ll/sys/cdefs.h"

__BEGIN_DECLS


/* general defs... */
#define PP_DEBUG   1      /* 1/0  Activate/Disactive internal debugs...     */
#define PP_STATS   1      /* 1/0  Activate/Disactive internal statistics... */

/* return value... */
#define FALSE   0
#define TRUE    1
#define TIMEOUT 2

#define PP_BASE_ADR    		0x0378 						/* std addr for LPT1 */

#define BYTE unsigned char
#define BOOL unsigned char
#define BUF_IDX unsigned int
#define BUF_PNTR unsigned int
#define PIN_MASK unsigned int

#define clock()              sys_gettime(NULL)

enum PIN_STATUS	{ PIN_OFF ,	PIN_ON } ;             /* positive logic: off==0, on==1 */

/*********************************************************************************/
/* PART 1 : LOW LIVEL FUNC */

/* defs used in ppPinDrv....*/
/* for std & pin use of pp... */
#define PP_DATA_REG     (PP_BASE_ADR+0)             //  address of data register
#define PP_STATUS_REG   (PP_BASE_ADR+1)             //  address of status register
#define PP_CONTR_REG    (PP_BASE_ADR+2)             //  address of control regist

/* out data pins... */
#define PP_PIN_D0 			0x01   /* pin 2 */
#define PP_PIN_D1 			0x02   /* pin 3 */
#define PP_PIN_D2 			0x04   /* pin 4 */
#define PP_PIN_D3 			0x08   /* pin 5 */
#define PP_PIN_D4 			0x10   /* pin 6 */
#define PP_PIN_D5 			0x20   /* pin 7 */
#define PP_PIN_D6 			0x40   /* pin 8 */
#define PP_PIN_D7 			0x80   /* pin 9 */
 
/* status pins... */
#define PP_PIN_ERROR 		0x08   /* pin 15 */
#define PP_PIN_SELECTED 	0x10   /* pin 13 */
#define PP_PIN_PAPEROUT 	0x20   /* pin 12 */
#define PP_PIN_ACK 			0x40   /* pin 10 */
#define PP_PIN_BUSY 		0x80   /* pin 11 */

/* control pins... */
#define PP_PIN_DATASTROBE 	0x01   /* pin 1  */
#define PP_PIN_AUTOFEED    	0x02   /* pin 14 */
#define PP_PIN_INITOUT     	0x04   /* pin 16 */
#define PP_PIN_SELECT      	0x08   /* pin 17 */
 
/* Data i/o */
#define ppSetDataByte(a)             outp(PP_DATA_REG,a)
#define ppReadDataByte()             inp(PP_DATA_REG)
/* this is NOT bidirectional actually: simply read value that I myself wrote on parport early...*/
/* in std lpt you cannot lay down electric pin D0,D1,..D7 from extern and read value in pc...*/
/* this (in std lpt) will broke down lpt port!... */
/* there are obviously also bi-dir port on 8 bit (ECC & ECP) but ctrl & status pins have */
/* different meaning so we don't manage them... See docs. */

/* Data pins */
void ppSetDataPin(int state, PIN_MASK pin);						/* in ppPinDrv.c */
#define ppSetPin_D0(a)				ppSetDataPin(a,PP_PIN_D0)   /* On ==1 , Off == 0 */
#define ppSetPin_D1(a)				ppSetDataPin(a,PP_PIN_D1)
#define ppSetPin_D2(a)				ppSetDataPin(a,PP_PIN_D2)
#define ppSetPin_D3(a)				ppSetDataPin(a,PP_PIN_D3)
#define ppSetPin_D4(a)				ppSetDataPin(a,PP_PIN_D4)
#define ppSetPin_D5(a)				ppSetDataPin(a,PP_PIN_D5)
#define ppSetPin_D6(a)				ppSetDataPin(a,PP_PIN_D6)
#define ppSetPin_D7(a)				ppSetDataPin(a,PP_PIN_D7)

/* Status pins */
#define ppCheckPin_Error()      	(inp(PP_STATUS_REG & PP_PIN_ERROR)!=0?1:0)
#define ppCheckPin_Selected()   	(inp(PP_STATUS_REG & PP_PIN_SELECTED)!=0?1:0)
#define ppCheckPin_PaperOut()   	(inp(PP_STATUS_REG & PP_PIN_PAPEROUT)!=0?1:0)
#define ppCheckPin_Acknowledge()  	(inp(PP_STATUS_REG & PP_PIN_ACK)!=0?1:0)
#define ppCheckPin_Busy()  			(inp(PP_STATUS_REG & PP_PIN_BUSY)!=0?0:1)       /* act low...*/

/* Control pins */
/* Control i/o */
#define ppSetCtrlByte(a)             outp(PP_CONTR_REG,a)
#define ppReadCtrlByte()             inp(PP_CONTR_REG) 
/* idem...*/
void ppSetCtrlPin(int state, PIN_MASK pin);								/* in ppPinDrv.c */
#define ppSetPin_DataStrobe(a)		ppSetCtrlPin(!a,PP_PIN_DATASTROBE)  /* low active...*/
#define ppSetPin_Autofeed(a)		ppSetCtrlPin(!a,PP_PIN_AUTOFEED)    /* low active...*/
#define ppSetPin_InitOut(a)			ppSetCtrlPin(a,PP_PIN_INITOUT)    
#define ppSetPin_Select(a)			ppSetCtrlPin(!a,PP_PIN_SELECT)    	/* low active...*/




/*********************************************************************************/
/* PART 2 : DATA TRANSFER BETWEEN PC */
/* defs used in ppDrv & ppNRTDrv... */
#define PPDRV_PERIOD          1000                      /* 300000 ok for debug...*/
#define PPDRV_WCET             150                      /* lower bound: 120; more if debug & stats are on...*/
#define PP_BUF_LEN            1024                      /* between 2^2 and 2^16 (64k) */
#define CLK_TIMEOUT          55000                      /* timeout for sync pc-pc...*/

/* for laplink use of std pp... */
#define TX_PORT 			PP_BASE_ADR     		/* transmit port */
#define RX_PORT 			TX_PORT+1      			/* receive port  */

/*  laplink bit mask */
#define TX_DATA 			0x0F    				/* 0000 1111 pin 2,3,4,5 */
#define TX_CTR  			0x10    				/* 0001 0000 bit 4 port TX pin 6*/
#define RX_DATA 			0x78    				/* 0111 1000 pin 15,13,12,10 */
#define RX_CTR  			0x80    				/* 1000 0000 bit 7 port RX pin 11*/
#define LSN     			0x0F    				/* 0000 1111 low significative nibble */
#define MSN     			0xF0    				/* 1111 0000 most significative nibble */
#define BYTE_CTR 			0xAF    				/* 1010 1111 control char */


/* comm protocol */
#define ppSendRTS()          ppSetOnPinTX_CTR()
#define ppIsRTS()            ppReadIfPinRX_CTRIsOn()
#define ppSendOTS()          ppSetOnPinTX_CTR()
#define ppIsOTS()            ppReadIfPinRX_CTRIsOn()
#define ppSendDR()           ppSetOffPinTX_CTR()
#define ppIsDR()             ppReadIfPinRX_CTRIsOff()
#define ppSendER()           ppSetOffPinTX_CTR()
#define ppIsER()             ppReadIfPinRX_CTRIsOff()


#define ppSetOnPinTX_CTR()          outp(TX_PORT,(inp(TX_PORT)|TX_CTR))          /* used by: ppSendRTS ppSendOTS */
#define ppSetOffPinTX_CTR()         outp(TX_PORT,(inp(TX_PORT)&(~TX_CTR)))       /* used by: ppSendDR ppSendER */
#define ppReadIfPinRX_CTRIsOn()     ((((~inp(RX_PORT))&RX_CTR)==0)?FALSE:TRUE)
#define ppReadIfPinRX_CTRIsOff()    (((BYTE)((~RX_CTR)|(~inp(RX_PORT)))==0x7F)?TRUE:FALSE)

/* Funct Return Code */
enum PP_COMM_RTR_CODE {
  PP_COMM_OK,
  PP_COMM_NOREADYBYTES_EXC,
  PP_COMM_NOFREEBYTES_EXC
};

/* Funct Return Code */
enum PP_SYSMSG_RTR_CODE {
  PP_SYSMSG_OK,
  PP_NOSYSMSG_EXC,
  PP_NOFREEMSG_EXC
};

/* NON REAL TIME (== BLOCK) functions...*/
/* from ppNRTDrv.c...*/
BOOL ppNRTOpenComm(void);
BOOL ppNRTWaitRTS(void);
BOOL ppNRTWaitDR(void);
BOOL ppNRTWaitOTS(void);
BOOL ppNRTWaitER(void);
BOOL ppNRTTxOneByte(BYTE c);
BOOL ppNRTRxOneByte(BYTE *c);


/* REAL TIME (== NON BLOCK) POLLING SERVER */
/* from ppDrv.c... */
void ppInitDrv(void (*pf)(char *));                   /* NRT: to be called before start ppPollingSrv...*/
TASK ppPollingSvr(void *arg);   /* periodic task to be started before any call to Rx/Tx...*/

/* input output function */
int ppRxOneByte(BYTE *c);                    /* retrive 1 byte */
int ppTxOneByte(BYTE c);                     /* send 1 byte */

int ppRxBytes(BYTE *c, unsigned int nbyte);  /* retrive n byte... */
int ppTxBytes(BYTE *c, unsigned int nbyte);  /* send n byte... */


/* System msg */
#define SYS_MSG_COLS 33
#define SYS_MSG_LINS 15


int ppReadSysMsg(char * buf);
int ppWriteSysMsg(char * buf, ...);

__END_DECLS
