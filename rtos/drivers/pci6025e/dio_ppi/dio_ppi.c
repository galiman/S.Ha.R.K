/*****************************************************************************
* Filename:    dio_ppi.c                                                     *    *
* Author:      Ziglioli Marco (Doctor Stein)                                 *
* Date:        16/03/2001                                                    *
* Description: Function to manage 8 digital lines provide from DAQ-STC and 24*
*              digital lines provide from 82C55A                             *
*----------------------------------------------------------------------------*
* Notes: No serial I/O available on DIO4 are implemented yet                 *
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

#include <drivers/pci6025e/dio_ppi.h>

/* PJ: Global data */
static BYTE  DIO_initialized;
static BYTE  PPI_initialized;

static BYTE  PL_Mask;                            //Configuration byte for DIO
static BYTE  PPI_A_Mask, PPI_B_Mask, PPI_C_Mask; /*configuration
                                                   bytes for PPI ports*/

static BYTE  PL_Configure;   //Will set true when DIO will be configured
static BYTE  PPI_Configure;   //Will set true when PPI will be configured

static WORD  dio_control, dio_output;
static BYTE  ppi_cfg_reg, ppi_port_a, ppi_port_b, ppi_port_c;


/*****************************************************************************
*  initializes digital lines                                                 *
*****************************************************************************/
void DIO_init(void)
{
   DIO_resetReg();
   DIO_initialized = 1;
}

/*****************************************************************************
*  Forces 0 all digital control lines                                        *
*****************************************************************************/
void DIO_resetReg(void)
{
   dio_control = 0x0000;
   dio_output  = 0x0000;
   DAQ_STC_Windowed_Mode_Write(DIO_CONTROL, dio_control);

}
/*****************************************************************************
*  Configure each 8 lines avaiable on board                                  *
*----------------------------------------------------------------------------*
*  In configuration byte:  0 stands for INPUT                                *
*                          1 stands for OUTPUT                               *
*****************************************************************************/
void config(BYTE cfg)
{
   if(DIO_initialized){
      dio_control |= cfg;

      DAQ_STC_Windowed_Mode_Write(DIO_CONTROL, dio_control);
      PL_Mask = cfg;
      PL_Configure = TRUE;   //Tells to world that DIO now are configured
   }
}

void DIO_config(BYTE cfg)
{
   config(cfg);
}

/*****************************************************************************
*  Reads a string made of 7 chars and returns configuration byte for DIO     *
*----------------------------------------------------------------------------*
*  String must be made of only eight chars which must be I or O to mean In   *
*  or Out. String starts with MSD and ends with LSD.                         *
*  Example: IIOOIIOO means D7 In D6 in D5 out ... D1 out D0 out              *
*  If most significal BYTE of value returned by function is different then 0 *
*  then configuration string is wrong and lowest significal byte is invalid  *
*****************************************************************************/
WORD DIO_setConfig(char *cfg)
{
   BYTE  i;
   WORD  config = 0x0000;

   for(i=0; i<8; i++){
      switch(cfg[i]){
         case 'I':
         case 'i':
            clr(config,(7-i));
            break;
         case 'O':
         case 'o':
            set(config,(7-i));
            break;
         default:
            set(config,16);   //Configuration string is wrong
            return config;
            break;
       }
    }

    return config;
}

/*****************************************************************************
*  Returns configuration byte for DIO lines. If most significal byte of word *
*  is different then 0 then DIO aren't yet configured so configuration byte  *
*  is invalid                                                                *
*****************************************************************************/
WORD DIO_getConfig(void)
{
   WORD config = 0x0000;
   if(PL_Configure == FALSE)
      set(config,16);
   else
      config = (WORD)PL_Mask;

   return config;
}

/*****************************************************************************
*  Read through Windowed mode values on digital lines                        *
*****************************************************************************/
BYTE DIO_read(void)
{
   WORD value;
   if(DIO_initialized){
      value = DAQ_STC_Windowed_Mode_Read(DIO_PARALLEL_INPUT);
      value &= 0x00FF;
      return (BYTE)value;
   }
   else   return 0;
}

/*****************************************************************************
*  Write through Windowed Mode values on digital IO lines                    *
*****************************************************************************/
BYTE DIO_write(BYTE out)
{
   if(DIO_initialized){
      dio_output = (out & PL_Mask);  /*Lines previously configured as Input
                                       are fixed to 0*/
      DAQ_STC_Windowed_Mode_Write(DIO_OUTPUT, dio_output);
      return 1;
   }
   else  return 0;
}

/*****************************************************************************
*  initializeds three PPI ports                                              *
*****************************************************************************/
void PPI_init(void)
{
   PPI_resetReg();
   PPI_initialized = 1;
}

/*****************************************************************************
*  Force all PPI configuration registers to 0                                *
*****************************************************************************/
void PPI_resetReg(void)
{
   ppi_cfg_reg =  0x80;
   ppi_port_a  =  0x00;
   ppi_port_b  =  0x00;
   ppi_port_c  =  0x00;
   Immediate_Writeb(PPI_CFG_REG, ppi_cfg_reg);
}

/*****************************************************************************
*  Setting up OKI MSM82C55A                                                  *
*  BYTE cfg can be write throgh this rules:                                  *
*     D7 -> Fixs to 1 (Chip understands in this way that you are sending a   *
*                       control word)                                        *
*     D6|                        MODE | 0 | 1 | 2 |                          *
*       |-> Set up port A mode   D6   | 0 | 0 | 1 | ( x=Don't care)          *
*     D5|                        D5   | 0 | 1 | x |                          *
*     D4 -> Sets direction of port A byte (0=Output; 1=Input)                *
*     D3 -> Sets direction of port C higher nibble (0=Output; 1=Input        *
*     D2 -> Sets Mode for port B (0=Mode 0; 1=Mode 1)                        *
*     D1 -> Sets direction of port B byte (0=Output; 1=Input)                *
*     D0 -> Sets direction of port C lower nibble (0=Output; 1=Input)        *
*----------------------------------------------------------------------------*
*  Examples:                                                                 *
*        0x80  means Mode0 PA, PB, PC in output mode                         *
*        0x99  means Mode0 PA, PC input mode and PB output mode              *
*        0x8A  means Mode0 PA, lower PC nibble in output and PB, higher PC   *
*                          nibble in input mode                              *
*****************************************************************************/
void PPI_config(BYTE cfg)
{
   if(PPI_initialized){
      //Setting up control word bit
      set(cfg,7);
      ppi_cfg_reg = cfg;
      Immediate_Writeb(PPI_CFG_REG, ppi_cfg_reg);
      PPI_A_Mask = PPI_B_Mask = PPI_C_Mask = 0;

      //Setting up configuration masks and enable configuration byte
      if(cfg&0x01)   PPI_C_Mask |= 0x0F;
      if(cfg&0x02)   PPI_B_Mask = 0xFF;
      if(cfg&0x08)   PPI_C_Mask |= 0xF0;
      if(cfg&0x10)   PPI_A_Mask = 0xFF;
      PPI_Configure = TRUE;
   }
}

/*****************************************************************************
*  WORD PPI_setConfig(BYTE PA, BYTE PA_MODE, BYTE PB, BYTE PB_MODE,          *
*                     BYTE HighPC, BYTE LowPC)                               *
*----------------------------------------------------------------------------*
*  This routine reads 6 arguments that describe configration for PPI and     *
*  return configuration byte to use with PPI_Config function.                *
*  Arguments have these means:                                               *
*  PA -> Direction of Port A (0 output, 1 input)                             *
*  PA_Mode -> 0 Basic Mode, 1 Strobe Input output, 2 bidirectional IOר       *
*  PB -> Direction of PORT B (0 output, 1 input)                             *
*  PB_Mode -> 0 Basic Mode, 1 Strobe IO                                      *
*  HighPC -> Direction of high nibble of Port C (0 output, 1 input)רר        *
*  LowPC -> Direction of low nibble of port C (0 output, 1 input)רר          *
*  If most significal byte of returned word is different than 0 than         *
*  configuration parameters is wrong and less significant byte is invalid    *
*----------------------------------------------------------------------------*
*  ר  Bidirectional IO is available only on port A                           *
*  רר Available only in mode 0 (basic IO). If other modes are selected PORT  *
*     C is used to perform handshake and HighPC and LowPC are ignored        *
*****************************************************************************/
WORD PPI_setConfig(BYTE PA, BYTE PA_Mode, BYTE PB, BYTE PB_Mode,
                   BYTE HighPC, BYTE LowPC)
{
   WORD config = 0x0000;

   switch(PA){
      case 0: clr(config,4); break;
      case 1: set(config,4); break;
      default: set(config,16); return config;
   }

   switch(PA_Mode){
      case 0: clr(config,5); clr(config,6); break;
      case 1: set(config,5); clr(config,6); break;
      case 2: set(config,6); break;
      default: set(config,16); return config;
   }

   switch(PB){
      case 0: clr(config,1); break;
      case 1: set(config,1); break;
      default: set(config,16); return config;
   }

   switch(PB_Mode){
      case 0: clr(config,2); break;
      case 1: set(config,2); break;
      default: set(config,16); return config;
   }

   switch(HighPC){
      case 0: clr(config,3); break;
      case 1: set(config,3); break;
      default: set(config,16); return config;
   }

   switch(HighPC){
      case 0: clr(config,0); break;
      case 1: set(config,0); break;
      default: set(config,16); return config;
   }

   set(config,7);    //Force meaning of control word

   return config;
}

/*****************************************************************************
*                    void PPI_getConfig(BYTE port)                           *
*----------------------------------------------------------------------------*
*  Returns configuration BYTE of specified PORT (0 = A, 1 = B, 2 = C)        *
*  If most significal byte of returned word is different than 0 than PPI     *
*  isn't yet configured and configuration low significal byte is invalid     *
*****************************************************************************/
WORD PPI_getConfig(BYTE port)
{
   WORD config = 0x00;
   if(PPI_Configure){
      switch(port){
         case 0: config = (WORD)PPI_A_Mask; break;
         case 1: config = (WORD)PPI_B_Mask; break;
         case 2: config = (WORD)PPI_C_Mask; break;
         default: set(config,16); break; //Parameter wrong
      }
   } else {
      set(config,16);   //Ports aren't configured
   }

   return config;
}

/*****************************************************************************
*  Read from specificated PPI port                                           *
*****************************************************************************/
BYTE PPI_read(BYTE off)
{
   if(PPI_initialized)     return Immediate_Readb(off);
   else                    return 0;
}

/*****************************************************************************
*  Write val into specificated PPI port                                      *
*****************************************************************************/
void PPI_write(BYTE off, BYTE val)
{
   if(PPI_initialized)  Immediate_Writeb(off, val);
}

/*****************************************************************************
*  Returns address of specified port (0 = A, 1 = B, 2 = C, other = CFG)      *
*****************************************************************************/
BYTE PPI_getAddress(BYTE port)
{
   switch(port){
      case 0:  return PPI_PORT_A;
      case 1:  return PPI_PORT_B;
      case 2:  return PPI_PORT_C;
      default: return PPI_CFG_REG;
   }
}
/*End of file: dio_ppi.c*/
