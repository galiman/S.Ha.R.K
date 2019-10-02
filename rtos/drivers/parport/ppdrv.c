/*
 * Project: 
 *   Parallel Port S.Ha.R.K. Project
 * 
 * Module: 
 *   ppDrv.c
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

/**********************************************************************************************************
 * Module : ppDrv.c Author : Andrea Battistotti , Armando Leggio
 * Description: Tranfer Byte via LPT1 laplink cable...  2002 @ Pavia -
 * GNU Copyrights */

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
 * CVS : $Id: ppdrv.c,v 1.1 2002/10/28 08:03:54 pj Exp $
 */


#include <drivers/parport.h>


/* internal */
BYTE RxBuf[PP_BUF_LEN];  			/* Received bytes buffer */
BYTE TxBuf[PP_BUF_LEN]; 			/* To transmit bytes buffer */

unsigned int nextByteReadyToRx;   /* pointer to first byte to read by ppOneByteRx(BYTE *c) ... */
unsigned int nextByteReadyToTx;   /* when ppbTransmit had to send a byte, send this...*/
unsigned int nextByteFreeRx;      /* when polling complete reading a byte, will save it in this loc...*/
unsigned int nextByteFreeTx;      /* when ppbTransmit had to write a byte, write it here...*/

/* define inline macro */
/* these are for cicle buffer mamagement */
#define RxPointerGap		(nextByteFreeRx-nextByteReadyToRx)
#define bytesReadyToRx      ((RxPointerGap)>=0? (RxPointerGap):PP_BUF_LEN+(RxPointerGap))
#define TxPointerGap		(nextByteFreeTx-nextByteReadyToTx)
#define bytesReadyToTx      ((TxPointerGap)>=0? (TxPointerGap):PP_BUF_LEN+(TxPointerGap))
#define freeBytesInRxBuffer (PP_BUF_LEN-bytesReadyToRx)
#define freeBytesInTxBuffer (PP_BUF_LEN-bytesReadyToTx)


/* for pp sys msg */
char SysMsg[SYS_MSG_COLS+1][SYS_MSG_LINS+1];    /* space for sys msgs... */
char bufMsg[SYS_MSG_COLS+1];					/* to build msgs... */

unsigned int nextMsgToRead;
unsigned int nextMsgFreeToWrite;

/* define inline macro */
/* these are for cicle buffer mamagement */
#define msgPointerGap		(nextMsgFreeToWrite-nextMsgToRead)
#define msgReadyToRead      ((msgPointerGap)>=0? (msgPointerGap):SYS_MSG_LINS+(msgPointerGap))
#define freeMsgInBuffer     (SYS_MSG_LINS-msgReadyToRead)




/* status ... */
enum ppReadingAvailableStates
{
  ppNoAllowReading,      /* writing is on...*/	
  ppNoReading,
  ppWaitingDR_Nibble1, 
  ppWaitingRTS_Nibble2,
  ppWaitingDR_Nibble2,
};

enum ppWritingAvailableStates
{
  ppNoAllowWriting,      /* reading is on...*/
  ppNoWriting,
  ppWaitingOTS_Nibble1,
  ppWaitingER_Nibble1,
  ppWaitingOTS_Nibble2,
  ppWaitingER_Nibble2
};


int ppStatusReading;
int ppStatusWriting;
BYTE ppReceivingByte;
BYTE ppTransmittingByte;


#if PP_STATS == 1
/* for internal statistic ...if activate...*/
long statReading[ppWaitingDR_Nibble2+1];
long statWriting[ppWaitingER_Nibble2+1];
#endif



/*********************************************/
/* sys msg managment */
/*********************************************/

int ppReadSysMsg(char * buf)
{
  if (!msgReadyToRead)             			/* there is nothing to read...*/
    {
      return (PP_NOSYSMSG_EXC);
    }
  else
    {int i=0;
    while (i<SYS_MSG_COLS && SysMsg[nextMsgToRead][i])   /* !='\0'...*/
      *buf++=SysMsg[nextMsgToRead][i++]; 	      		/* read char */
		
    *buf='\0';	
    nextMsgToRead=++nextMsgToRead%SYS_MSG_LINS; 	/* circular buffer increment */
    return (PP_SYSMSG_OK);
    }
}



int ppWriteSysMsg(char * buf, ...)
{
  char * pbufMsg=bufMsg;
  va_list args;
  bufMsg[0]='\0';
  va_start(args, buf);
  vsprintf(bufMsg,buf,args);    /* Not garatee msg len... */  
  va_end(args);

        
  if (freeMsgInBuffer < 1)
    {
      return (PP_NOFREEMSG_EXC);
    }
  else
    {int i=0;
    while ((i<SYS_MSG_COLS) && (*pbufMsg) )   /* !='\0'...*/         
      SysMsg[nextMsgFreeToWrite][i++]=*pbufMsg++;
		
    SysMsg[nextMsgFreeToWrite][i-1]='\n'; 
    SysMsg[nextMsgFreeToWrite][i]='\0';	
    nextMsgFreeToWrite=++nextMsgFreeToWrite%SYS_MSG_LINS; /* circular buffer pointer increment */
    return (PP_SYSMSG_OK);
    }
}


/******************************************/
/*   Inizialization: this is NRT task...  */
/******************************************/

void ppInitDrv(void (*pf)(char *))
{
  /* set to zero all pointer & buffer & status... */


  while(ppNRTOpenComm()!=TRUE)   // not real-time....
    {
      if (pf!=NULL) (*pf)("Waiting Open Communcation...\n");
      ppSendER();
    }
  if (pf!=NULL) (*pf)("Open Communcation OK!\n");
}


/******************************************/
/* TX RX funtions... */
/******************************************/

int ppRxOneByte(BYTE *c)
{                   
  if (!bytesReadyToRx)             			/* there is nothing to read...*/
    {
      return (PP_COMM_NOREADYBYTES_EXC);
    }
  else
    {
      *c=RxBuf[nextByteReadyToRx]; 	      			/* read byte */
      nextByteReadyToRx=++nextByteReadyToRx%PP_BUF_LEN; 	/* circular buffer increment */
      return (PP_COMM_OK);
    }
}


int ppTxOneByte(BYTE c)
{
  if (freeBytesInTxBuffer < 1)
    {
      return (PP_COMM_NOFREEBYTES_EXC);
    }
  else
    {
      TxBuf[nextByteFreeTx]=c;
      nextByteFreeTx=++nextByteFreeTx%PP_BUF_LEN; /* circular buffer pointer increment */

      return (PP_COMM_OK);
    }
}

int ppTxBytes(BYTE * c, unsigned int  nbyte)
{
  if (freeBytesInTxBuffer<nbyte) 					/* if there are less than nbyte return nothing...*/
    {
      return (PP_COMM_NOFREEBYTES_EXC);
    }
  else
    { unsigned int i;
    for (i=0;i<nbyte;i++)
      {
	TxBuf[nextByteFreeTx]=*c++;
	nextByteFreeTx=++nextByteFreeTx%PP_BUF_LEN; /* circular buffer pointer increment */
      }

    return (PP_COMM_OK);
    }
}



int ppRxBytes(BYTE * c, unsigned int nbyte)
{
                  
  if (bytesReadyToRx<nbyte)           /* if there are less than nbyte return nothing...*/
    {
      return (PP_COMM_NOREADYBYTES_EXC);
    }
  else
    { unsigned int i;

    for (i=0;i<nbyte;i++)
      {
	*c++=RxBuf[nextByteReadyToRx]; 	      		     	/* read byte */
					
#if PP_DEBUG == 1
	ppWriteSysMsg("Received value: %i %i of %i \n",RxBuf[nextByteReadyToRx],i,nbyte);
#endif
                    
	nextByteReadyToRx=++nextByteReadyToRx%PP_BUF_LEN;       /* circular buffer increment */
      }

    return (PP_COMM_OK);
    }
}




/* polling server ... */
TASK ppPollingSvr(void *arg)
{
  BYTE port;
  nextByteReadyToRx=0; 
  nextByteReadyToTx=0;           
  nextByteFreeRx=0;   
  nextByteFreeTx=0; 

  nextMsgToRead=0;
  nextMsgFreeToWrite=0;


  ppStatusReading=ppNoReading;
  ppStatusWriting=ppNoWriting;

  ppWriteSysMsg("Polling Server started...\n"); 

  task_endcycle();

  while (1) 
    {
            
      /* case ppReading: read ... */
      switch (ppStatusReading)
	{
	case ppNoAllowReading: break;
	
	case ppNoReading:

	  ppStatusWriting=ppNoWriting;
                         
	  if(!ppIsRTS())   break;
	  ppSendOTS();     /* Set Ok To Send - the other one can send... */
       			       			
#if PP_DEBUG == 1
	  ppWriteSysMsg(" %i : Received RTS...\n", ppStatusReading);
#endif

	  ppStatusWriting=ppNoAllowWriting;
	  ppStatusReading=ppWaitingDR_Nibble1;
	
	case ppWaitingDR_Nibble1:
	
#if PP_STATS == 1                              /* for internal statistic ...*/
	  statReading[ppStatusReading]++;
#endif
	
	

#if PP_DEBUG == 1 
	  ppWriteSysMsg("Send OTS\n");
	  ppWriteSysMsg("Waiting DR Nibble1\n");
#endif

	  if(!ppIsDR()) break;                /* data no ready: read it next period...*/
	  port = inp(RX_PORT);                       /* read  nibble    */
	  ppSendER();   							   /* send a End Read */

	  ppReceivingByte = (port >> 3);             /* read LSN */
	  ppStatusReading=ppWaitingRTS_Nibble2; 
				
	case ppWaitingRTS_Nibble2:
#if PP_STATS == 1					/* for internal statistic ...*/
	  statReading[ppStatusReading]++;
#endif
	
#if PP_DEBUG == 1

	  ppWriteSysMsg("Received DR Nibble1\n");
	  ppWriteSysMsg("Send ER Nibble1\n");
	  ppWriteSysMsg("Waiting RTS Nibble2\n");
#endif
	
	  if(!ppIsRTS()) break;           /* */
	  ppSendOTS();                       /* Ok To Send - the other one can send... */

	  ppStatusReading=ppWaitingDR_Nibble2;
	
	case ppWaitingDR_Nibble2:
#if PP_STATS == 1					/* for internal statistic ...*/
	  statReading[ppStatusReading]++;
#endif
#if PP_DEBUG == 1
	  ppWriteSysMsg("Received RTS Nibble2\n");
	  ppWriteSysMsg("Send OTS Nibble2\n");
	  ppWriteSysMsg("Waiting DR Nibble2\n");
#endif

	  if(!ppIsDR()) break;
	  port = inp(RX_PORT);                 /* read nibble    */
	  ppSendER();                          /* send a End Read */


#if PP_DEBUG == 1
	  ppWriteSysMsg("Received DR Nibble2\n");
	  ppWriteSysMsg("Read Nibble2\n");
	  ppWriteSysMsg("Send ER Nibble2\n");
#endif



	  ppReceivingByte = (ppReceivingByte & ~MSN);               /* set to zero c MSN */
	  ppReceivingByte = (ppReceivingByte | ((port >> 3) << 4)); /* read MSN */


				/* here is possible insert some ctrl ... */
				
				
				/* byte is ok, so now make it available to ppRxOneByte() */
				
	  RxBuf[nextByteFreeRx]=ppReceivingByte;
	  nextByteFreeRx=++nextByteFreeRx%PP_BUF_LEN; /* circular buffer pointer increment */
	
#if PP_STATS == 1
	  ppWriteSysMsg("Trasmission :\n");
	  ppWriteSysMsg("W_DR_1  : %d\n",statReading[ppWaitingDR_Nibble1]);
	  ppWriteSysMsg("W_RTS_2 : %d\n",statReading[ppWaitingRTS_Nibble2]);
	  ppWriteSysMsg("W_DR_2  : %d\n",statReading[ppWaitingDR_Nibble2]);
	  ppWriteSysMsg("Received byte : %i\n",ppReceivingByte);
	  statReading[ppWaitingDR_Nibble1]=0;
	  statReading[ppWaitingRTS_Nibble2]=0;
	  statReading[ppWaitingDR_Nibble2]=0;
#endif
	  /* end reading so reset status... */
	
	  ppStatusReading=ppNoReading; 
	  //ppStatusWriting=ppNoWriting;
	  break;								
	}
			
				
      /* case Writing: can only if this cycle not is reading... */
      switch (ppStatusWriting)
	{
			
	case ppNoAllowWriting: break;
	
	case ppNoWriting:

	  ppStatusReading=ppNoReading;

	  if(!bytesReadyToTx)
	    {
	      break;
#if PP_DEBUG == 1
	      ppWriteSysMsg("Writin break\n");
#endif
	    }
	  else  
	    {
#if PP_DEBUG == 1
	      ppWriteSysMsg("NO Writin break\n");
	      ppWriteSysMsg("TX Gap: %i \n",TxPointerGap);
	      ppWriteSysMsg("nextByteFreeTx: %i\n",nextByteFreeTx);
	      ppWriteSysMsg("nextByteReadyToTx: %i\n",nextByteReadyToTx);
#endif
	    }

	  ppSendRTS();                                 /* Set On RequestToSend bit */

	  ppTransmittingByte=TxBuf[nextByteReadyToTx];
	
#if PP_DEBUG == 1
	  ppWriteSysMsg("pllsvr: ppTransmittingByte : %i %c \n",ppTransmittingByte,ppTransmittingByte);
#endif
							
				
	  port = inp(TX_PORT) & (~TX_DATA);            /* set to zero trasmission bits */
	  port = port | (ppTransmittingByte & LSN);    /* set  bits 0..3 with LSN   */
	
	  ppStatusWriting=ppWaitingOTS_Nibble1;
	  ppStatusReading=ppNoAllowWriting;
	
	case ppWaitingOTS_Nibble1:
#if PP_STATS == 1                          
	  statWriting[ppStatusWriting]++;
#endif
	
#if PP_DEBUG == 1                               
	  ppWriteSysMsg(" Send RTS Nibble1\n");
	  ppWriteSysMsg(" Waiting OTS Nibble1\n");
#endif
	  if(!ppIsOTS()) break;
	  outp(TX_PORT,port);                   /* send nibble 1   */
	  ppSendDR();       		              /* set on Data Ready bit  */


	  ppStatusWriting=ppWaitingER_Nibble1;


	case ppWaitingER_Nibble1:
#if PP_STATS == 1        
	  statWriting[ppStatusWriting]++;
#endif
	
#if PP_DEBUG == 1
	  ppWriteSysMsg(" Send Nibble1\n"); 
	  ppWriteSysMsg(" Send DR Nibble1\n"); 
	  ppWriteSysMsg(" Waiting ER Nibble1\n"); 
#endif

	  if(!ppIsER()) break;

	  ppSendRTS();                                 /* send trasmission request */

	  port = inp(TX_PORT) & (~TX_DATA);            /* set to zero bit trasmission bits */
	  port = port | (ppTransmittingByte >> 4);     /* set  bits 0..3 with MSN   */
				
	  ppStatusWriting=ppWaitingOTS_Nibble2;
				
	case ppWaitingOTS_Nibble2:

#if PP_STATS == 1
	  statWriting[ppStatusWriting]++;
#endif

#if PP_DEBUG == 1
	  ppWriteSysMsg(" Received ER Nibble1\n"); 
	  ppWriteSysMsg(" Send RTS Nibble2\n"); 
	  ppWriteSysMsg(" Waiting OTS Nibble2\n"); 
#endif

			
	  if(!ppIsOTS()) break;
	  outp(TX_PORT,port);                           /* send nibble 2          */
	  ppSendDR();                                   /* set on Data Ready bit  */

	  ppStatusWriting=ppWaitingER_Nibble2;

	case ppWaitingER_Nibble2:
#if PP_STATS == 1                               
	  statWriting[ppStatusWriting]++;
#endif
	
#if PP_DEBUG == 1
	  ppWriteSysMsg(" Received OTS Nibble2\n"); 
	  ppWriteSysMsg(" Write Nibble2\n");
	  ppWriteSysMsg(" Send DR Nibble2\n");
	  ppWriteSysMsg(" Waiting ER Nibble2\n");
#endif

	  if(!ppIsER()) break;
	
				/* byte is ok, so move pointer to next byte to be send... */
	  nextByteReadyToTx=++nextByteReadyToTx%PP_BUF_LEN; /* circular buffer pointer increment */
				
#if PP_STATS == 1 
	  ppWriteSysMsg("Reception :\n");
	  ppWriteSysMsg("W_OTS_1  : %ld\n",statWriting[ppWaitingOTS_Nibble1]);
	  ppWriteSysMsg("W_ER_2 : %ld\n",statWriting[ppWaitingER_Nibble2]);
	  ppWriteSysMsg("W_OTS_2  : %ld\n",statWriting[ppWaitingOTS_Nibble2]);
	  statWriting[ppWaitingOTS_Nibble1]=0;
	  statWriting[ppWaitingER_Nibble2]=0;
	  statWriting[ppWaitingOTS_Nibble2]=0;
#endif

	  /* end writing. so reset status... */
	  ppStatusReading=ppNoReading; 
	  ppStatusWriting=ppNoWriting;
	}	 		
		
      task_endcycle();
    }                                                
  return (0);
}



