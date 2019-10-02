/*****************************************************************************
* Filename:    pci6025e.c                                                    *
* Author:      Ziglioli Marco                                                *
* Date:        15/03/2001                                                    *
* Description: Collection of routines to find on PCI bus and configure Natio-*
*              nal Instruments PCI6025E multifunction board. This board has  *
*              - One ADC 12 bit flash with 16 input lines (multiplexed)      *
*              - Two DACs 12 bit                                             *
*              - Eight Digital lines drived through NI DAQ-STC IC            *
*              - Twentyfour Digital lines drived through OKI MSM82C55A       *
*              - Two General purpose 24-bit counters/timers                  *
*              - Timing module for analog input (AITM)                       *
*              - Timing module for analog output (AOTM)                      *
*              - 10 Programmable Function pins (PFI)                         *
*              - RTSI bus for connection with other National Board           *
*              - Timing module which can provide frequency from 20MHz to     *
*                6,25 KHz                                                    *
*              - Two shared interrupt groups to manage up than 20 different  *
*                interrupt events                                            *
* For more information please refer to National Web site: www.ni.com         *
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

#include <drivers/pci6025e/pci6025e.h>

extern int pci20to26_find_class(unsigned int class_code, int index, BYTE *bus, BYTE *dev);
extern int pci20to26_read_config_byte(unsigned int bus, unsigned int dev, int where, BYTE *val);
extern int pci20to26_read_config_word(unsigned int bus, unsigned int dev, int where, WORD *val);
extern int pci20to26_read_config_dword(unsigned int bus, unsigned int dev, int where, DWORD *val);
extern int pci20to26_write_config_byte(unsigned int bus, unsigned int dev, int where, BYTE val);
extern int pci20to26_write_config_word(unsigned int bus, unsigned int dev, int where, WORD val);
extern int pci20to26_write_config_dword(unsigned int bus, unsigned int dev, int where, DWORD val);

//Software copy of importan register
static WORD interrupt_control;

// PJ: Global data
struct pci6025e_deviceinfo NIDevice_info[10];
DWORD *IntLinestructptr;
DWORD *BAR0structptr;
DWORD *RevID;
DWORD STC_Base_Address, MITE_Base_Address;

//Software copy of STC general registers
WORD joint_reset, interrupt_a_enable, interrupt_a_ack,
     interrupt_b_enable, interrupt_b_ack, clock_and_fout;

/*****************************************************************************
*                    Usefull(???) Functions                                  *
*****************************************************************************/

/*****************************************************************************
*  Print value of each bit contained in value                                *
*****************************************************************************/
void bitfield(BYTE dim, DWORD value)
{
   int   i;

   for(i=(dim*8)-1; i>=0; i--)
      cprintf("%ld",(value>>i)%2);
   cprintf("\n");
}

/*****************************************************************************
* Print custom string (str) before printing the bitfield                     *
*****************************************************************************/
void  TEST_bitfield(BYTE dim, DWORD value, char *str)
{
   cprintf("%s", str);
   bitfield(dim, value);
}

/*****************************************************************************
*  Low Level Functions to write in windowed mode into DAQ STC                *
*****************************************************************************/
void DAQ_STC_Windowed_Mode_Write(WORD addr, WORD Value)
{
   SYS_FLAGS f;

   f = kern_fsave();
   *((WORD *)(STC_Base_Address + WIN_ADDR_REG)) = addr;
   *((WORD *)(STC_Base_Address + WIN_DATA_WR_REG)) = Value;
   kern_frestore(f);
}

WORD DAQ_STC_Windowed_Mode_Read(WORD addr)
{
   WORD value = 0;
   SYS_FLAGS f;

   f = kern_fsave();
   *((WORD *)(STC_Base_Address + WIN_ADDR_REG)) = addr;
   value = *((WORD *)(STC_Base_Address + WIN_DATA_RD_REG));
   kern_frestore(f);
   return (value);
}

/*****************************************************************************
*  Detect all NI Devices on PCI bus                                          *
*****************************************************************************/
BYTE find_NI_Device(void)
{
   WORD     devNumFunNum = 0;
   WORD     busNum, i = 0;
   DWORD    value, temp = 0, Device_ID;
   BYTE     hdr, Device_Count;
   int      present;

   Device_Count = 0;
   BAR0structptr     = &NIDevice_info[0].BAR0Value;
   IntLinestructptr  = &NIDevice_info[0].IntLineRegValue;
   RevID             = &NIDevice_info[0].RevisionID;

   for(busNum = 0; busNum <= 0; busNum++){
      for(devNumFunNum = 0; devNumFunNum <= 0xFF; devNumFunNum += 0x08){
         present = 0;
         if((devNumFunNum & 0x07) == 0){
            present = pci20to26_read_config_byte((BYTE)busNum,
                                   (BYTE)devNumFunNum, 0x0E, &hdr);
            if(hdr & 0x80)
               present = 1;
            if(present){
               if(pci20to26_read_config_dword((BYTE)busNum, (BYTE)devNumFunNum,
                                  0x00, &value) && (value != 0xffffffff)){
                  if((value & 0xffffL) == NI_CODE){
                     Device_ID = value;
                     Device_ID &= 0xFFFF0000;
                     Device_ID = Device_ID >> 16;

                     NIDevice_info[i].DEVID = (WORD)temp;
                     NIDevice_info[i].DevFunction = (BYTE)devNumFunNum;
                     NIDevice_info[i].BusNumber = (BYTE)busNum;
                     pci20to26_read_config_dword((BYTE)busNum, (BYTE)devNumFunNum, 0x10, (DWORD *)BAR0structptr);
                     pci20to26_read_config_dword((BYTE)busNum, (BYTE)devNumFunNum, 0x3C, (DWORD *)IntLinestructptr);
                     pci20to26_read_config_dword((BYTE)busNum, (BYTE)devNumFunNum, 0x08, (DWORD *)RevID);
                     NIDevice_info[i].InterruptLevel = (BYTE)NIDevice_info[i].IntLineRegValue;
                     i++;
                     BAR0structptr++;
                     IntLinestructptr++;
                     RevID++;
                     Device_Count++;
                  }
               }
            }
         }
      }
   }
   return Device_Count;
}

/*****************************************************************************
*  Remap Base address of STC and MITE registers presents on NI boards        *
*  Note: If there's over than one NI Board on bus only the first device is   *
*        remapped                                                            *
*****************************************************************************/
BYTE reMap(void)
{
   //DWORD       Mite_Data;
   //DWORD       Value;

   if(!find_NI_Device()){
      return 0;
   }

 /*
   // ***************************************************************
   // This is usefull to remap board in another memory zone
   // Enable this and choose memory zone setting BAR0 and BAR1 values
   // on PCI6025E.H
   // ***************************************************************
   // Set Up MITE
   Value = BAR0;
   pcibios_write_config_dword((BYTE)NIDevice_info[0].BusNumber,
         NIDevice_info[0].DevFunction, 0x10, Value);
   Mite_Data = 0xAEAE;
   lmempoked(BAR0 + 0x0340, Mite_Data);

   // Set Up Board
   Value = BAR1;
   pcibios_write_config_dword((BYTE)NIDevice_info[0].BusNumber,
         NIDevice_info[0].DevFunction, 0x14, Value);
   Mite_Data = (BAR1 & 0xFFFFFF00) | 0x80;
   lmempoked(BAR0 + 0xC0, Mite_Data);
 */


   //Setting up Base Address
   pci20to26_read_config_dword((BYTE)NIDevice_info[0].BusNumber,
         (BYTE)NIDevice_info[0].DevFunction, 0x14, &STC_Base_Address);
   STC_Base_Address = (STC_Base_Address & 0xFFFFFF00) | 0x80;
   pci20to26_read_config_dword((BYTE)NIDevice_info[0].BusNumber,
         (BYTE)NIDevice_info[0].DevFunction, 0x10, &MITE_Base_Address);
   //MITE_Base_Address &= 0xFFFFFF00;

   *((DWORD *)MITE_Base_Address) = 0xAEAE;
   *((DWORD *)(MITE_Base_Address + 0xC0)) = STC_Base_Address;

   return 1;
}

/*****************************************************************************
*  Sets up the internal timebase clock and enable Frequency Out (FOUT).      *
*  Signal is available on pin 50                                             *
*----------------------------------------------------------------------------*
*  tbs      ->    if 0 then 20 Mhz freq; if 1 then 200 Khz freq              *
*  divBy2   ->    if 1 divides output frequency by two else if 0 doesn't     *
*                 divide it                                                  *
*  divider  ->    if 0 divides frequency by 16                               *
*                 if between 1 and 15 divides frequency by specified value   *
*****************************************************************************/
BYTE setIntClock(BYTE tbs, BYTE divBy2, BYTE divider)
{
   clr(clock_and_fout, 15);
   DAQ_STC_Windowed_Mode_Write(CLOCK_AND_FOUT, clock_and_fout);

   switch(tbs){
      case 0:
         clr(clock_and_fout, 14);   //IN_TIMEBASE
         break;
      case 1:
         set(clock_and_fout, 11);   //Enable IN_TIMEBASE2
         set(clock_and_fout, 14);   //Select IN_TIMEBASE2
         break;
      default:
         return 0;
         break;
   }

   switch(divBy2){
      case 0: clr(clock_and_fout, 12); break;   //Not divided by two
      case 1: set(clock_and_fout, 12); break;   //Divided by two
      default: return 0; break;
   }

   //if((divider < 0) || (divider > 15))
   if(divider > 15)  // PJ: byte has only positive values!!!
      return 0;      //Addictional frequency division wrong

   if(divider & 0x01)
      set(clock_and_fout, 0);
   else
      clr(clock_and_fout, 0);

   if(divider & 0x02)
      set(clock_and_fout, 1);
   else
      clr(clock_and_fout, 1);

   if(divider & 0x04)
      set(clock_and_fout, 2);
   else
      clr(clock_and_fout, 2);

   if(divider & 0x08)
      set(clock_and_fout, 3);
   else
      clr(clock_and_fout, 3);

   //Write in register
   DAQ_STC_Windowed_Mode_Write(CLOCK_AND_FOUT, clock_and_fout);

   //Enable internal clock
   set(clock_and_fout, 15);
   DAQ_STC_Windowed_Mode_Write(CLOCK_AND_FOUT, clock_and_fout);

   return 1;
}

/*****************************************************************************
*                 void PFIprogramming(WORD pfi)                              *
*----------------------------------------------------------------------------*
* Use this function to program the direction of the ten available PFI lines  *
* Parameter: pfi     bit 0 through 9: 0 releted PFI line configured as input *
*                                     1 releted PFI line configured as output*
*                    10..15   NOT USED                                       *
*****************************************************************************/
void PFIprogramming(WORD pfi)
{
   DAQ_STC_Windowed_Mode_Write(IO_BIDIRECTION_PIN, (pfi & 0x03FF));
}

/*****************************************************************************
*                 Interrupt manage module                                    *
*****************************************************************************/
/*****************************************************************************
*                 void INT_personalize(BYTE personal)                        *
*----------------------------------------------------------------------------*
* Use this function to select the logic level will indicate an interrupt     *
* condition on IRQ_OUT pins and to enable or disable the possibility of      *
* duplicate selected interrupt to IRQ_OUT<0..1> pins                         *
* Parameter:   personal:   bit 0: enable(1) or disable(0) signal duplication *
*                          bit 1: polarity: (0)active high (1)active low     *
*                          bit 2..7 NOT USED                                 *
*****************************************************************************/
void INT_personalize(BYTE personal)
{
   if( (personal & 0x02) )       set(interrupt_control, 0);
   else                          clr(interrupt_control, 0);

   if( (personal & 0x01) )       set(interrupt_control, 1);
   else                          clr(interrupt_control, 1);

   DAQ_STC_Windowed_Mode_Write(INTERRUPT_CONTROL, interrupt_control);
}

/*****************************************************************************
*                 void INT_setup(BYTE ga, BYTE gb)                           *
*----------------------------------------------------------------------------*
* Use this function to enable interrupt groups and to indicate which pin     *
* indicate interrupt condition.                                              *
* Parameter: ga: group A manages AITM, IRQ_IN0, Counter0                     *
*            gb: group B manages AOTM, IRQ_IN1, Counter1                     *
* Both parameters have first 3 bits (0..2) to indicate IRQ_OUT pin that will *
* indicate interrupt condition and the fourth bit (3) to enable the          *
* interrupts group.                                                          *
* Upper 4 bits of each BYTE are NOT USED                                     *
*****************************************************************************/
void INT_setup(BYTE ga, BYTE gb)
{
   if( (ga & 0x08) )
      interrupt_control = (interrupt_control & 0xF0FF) |
                          ((ga & 0x0F) << 8);
   else
      clr(interrupt_control, 11);

   if( (gb & 0x08) )
      interrupt_control = (interrupt_control & 0x0FFF) |
                          ((gb & 0x0F) << 12);
   else
      clr(interrupt_control, 15);

   DAQ_STC_Windowed_Mode_Write(INTERRUPT_CONTROL, interrupt_control);
}
/*End of file: pci6025e.c*/
