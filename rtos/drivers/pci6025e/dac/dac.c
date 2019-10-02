/*****************************************************************************
* Filename:       DAC.C                                                      *      *
* Author:         Marco Ziglioli (Doctor Stein)                              *        *
* Date:           06/06/2001                                                 *          *
* Description:    Digital 2 analog conversion functions                      *
*----------------------------------------------------------------------------*
* Notes: Only module one of Analog Output Timing Module are programmed yet   *
*        No waveform staging and local buffer mode with pauses are           *
*        implemented yet                                                     *
*        Please refer to STC-DAC Technical Reference Manual and PCI E-Series *
*        Programmer Level Manual for further information                     *
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



#include <drivers/pci6025e/dac.h>

//Software copies of useful registers
static WORD ao_command_1, ao_command_2, ao_personal, ao_mode_1, ao_mode_2,
            ao_mode_3, ao_output_control, ao_start_select, ao_trigger_select;

static BYTE DAC_Initialized = 0;

/*****************************************************************************
*                 void DAC_reset(void)                                       *
*----------------------------------------------------------------------------*
* Reset both two DACs and the timing system                                  *
*****************************************************************************/
void  DAC_reset(void)
{
   set(joint_reset, 5);    //Enable AOTM configuration
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 5);

   //Disarm Timing module
   set(ao_command_1, 13);
   DAQ_STC_Windowed_Mode_Write(AO_COMMAND_1, ao_command_1);
   clr(ao_command_1, 13);

   //Clear all registers
   DAQ_STC_Windowed_Mode_Write(AO_PERSONAL, 0x0000);
   DAQ_STC_Windowed_Mode_Write(AO_COMMAND_1, 0x0000);
   DAQ_STC_Windowed_Mode_Write(AO_COMMAND_2, 0x0000);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_1, 0x0000);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_2, 0x0000);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_3, 0x0000);
   DAQ_STC_Windowed_Mode_Write(AO_OUTPUT_CONTROL, 0x0000);
   DAQ_STC_Windowed_Mode_Write(AO_START_SELECT, 0x0000);
   DAQ_STC_Windowed_Mode_Write(AO_TRIGGER_SELECT, 0x0000);

   //Clear software copies
   ao_personal = ao_command_1 = ao_command_2 = ao_mode_1 = ao_mode_2 =
   ao_mode_3 = ao_output_control = ao_start_select = ao_trigger_select = 0;

   //Disable related interrupt flags
   interrupt_b_enable &= 0xFE80;
   DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ENABLE, interrupt_b_enable);

   //Enable related interrupt request flags
   interrupt_b_ack |= 0x3F98;
   DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ACK, interrupt_b_ack);

   set(ao_personal, 4);
   DAQ_STC_Windowed_Mode_Write(AO_PERSONAL, ao_personal);

   set(joint_reset, 9);    //Disable AOTM configuration
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 9);
}

/*****************************************************************************
*                void DAC_boardInit(BYTE clock, WORD personal)               *
*----------------------------------------------------------------------------*
* Use this function to program software selectable options in the primary    *
* module. The options include source, polarity and pulsewidth of the most    *
* common used signals. This function must be executed after every invoke of  *
* DAC_reset and before executing any output operation.                       *
* Parameters:                                                                *
*        clock                                                               *
*           -bit 0:  if sets to 1 AO_IN_TIMEBASE is IN_TIMEBASE divided by 2 *
*           -bit 1:  if sets to 1 AO_OUT_TIMEBASE is IN_TIMEBASE divideb by 2*
*           -bit 2..9 NOT USED
*        personal                                                            *
*           -bit 0...1: update output signal select 0(high Z); 1(GND);       *
*                       2(enable active low); 3(enable active high)          *
*           -bit 2...4: not used (let this bit to 0)                         *
*           -bit 5:  update pulse width 0(3-3.5 AO_OUT_TIMEBASE period)      *
*                                       1(1-1.5 AO_OUT_TIMEBASE period)      *
*           -bit 6:  update pulse timabase 0(selected by AO_UPDATE pulse_Wi) *
*                                          1(selected by AO_UPDATE original) *
*           -bit 7:  not used                                                *
*           -bit 8:  DMA PIO control 0(FIFO Data interface mode)             *
*                                    1(unbuffered data interface mode)       *
*           -bit 9:  not used                                                *
*           -bit 10: enable TMRDACWR which control Analog Output FIFO        *
*           -bit 11: FIFO flags polarity 0(active low)                       *
*           -bit 12: TMRDACWR pulse width 0(3 AO_OUT_TIMEBASE period)        *
*                                         1(2 AO_OUT_TIMEBASE period)        *
*           -bit 13: not used                                                *
*           -bit 14: number of DAC packages 0(dual-DAC mode)                 *
*                                           1(single-DAC mode)               *
*           -bit 15: NOT USED                                                *
*****************************************************************************/
void  DAC_boardInit(BYTE clock, WORD personal)
{
   set(joint_reset, 5);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 5);

   //Setup AOTM clock module
   clock_and_fout = (clock_and_fout & 0xFFFC) | (clock & 0x03);
   DAQ_STC_Windowed_Mode_Write(CLOCK_AND_FOUT, clock_and_fout);

   //Setup Update signal
   ao_output_control = (ao_output_control & 0xFFFC) | (personal & 0x0003);
   DAQ_STC_Windowed_Mode_Write(AO_OUTPUT_CONTROL, ao_output_control);

   ao_personal = (ao_personal & 0x229F) | (personal & 0xDD6F);
   DAQ_STC_Windowed_Mode_Write(AO_PERSONAL, ao_personal);

   clr(ao_start_select, 12);
   DAQ_STC_Windowed_Mode_Write(AO_START_SELECT, ao_start_select);

   set(joint_reset, 9);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 9);
}

/*****************************************************************************
*                   void DAC_trigger(WORD trigger)                           *
*----------------------------------------------------------------------------*
* With this function you can select which is the signal which trigger the    *
* analog output operation                                                    *
* parameter:                                                                 *
*     bit 0...4:  start1 source select 0      -> bitfiled AO_START_1 pulse   *
*                                      1..10  -> PFI<0..9>                   *
*                                      11..17 -> RTSI<0..6>                  *
*     bit 5: edge detection 0(disable) 1(enable)                             *
*     bit 6: synchronization 0(disable) 1(enable)                            *
*     bit 13: start1 polarity 0(active hi) 1(active lo)                      *
*     bit 14: synchronized with BC source 0(disable) 1(enable)               *
*     bit 15: set this bit for single operation or clear for continuos mode  *
*     bit 7..12: NOT USED                                                    *
*****************************************************************************/
void  DAC_trigger(WORD trigger)
{
   set(joint_reset, 5);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 5);

   //Select continuos or one shoot mode
   if(trigger&0x8000)   set(ao_mode_1, 0);
   else                 clr(ao_mode_1, 0);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_1, ao_mode_1);

   //select source
   if( (trigger&0x001F) == 0 )
      ao_trigger_select = (ao_trigger_select & 0xDF80) | 0x0060;
   else
      ao_trigger_select = (ao_trigger_select & 0xDF80) | (trigger & 0x007F);

   //triger signal synchronization
   ao_trigger_select = (ao_trigger_select & 0x9FFF) | (trigger & 0x6000);
   DAQ_STC_Windowed_Mode_Write(AO_TRIGGER_SELECT, ao_trigger_select);

   clr(ao_mode_3, 11);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_3, ao_mode_3);

   set(joint_reset, 9);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 9);
}

/*****************************************************************************
*             void DAC_numSetup(DWORD n_updates, DWORD n_iter)               *
*----------------------------------------------------------------------------*
* Use this function to set up number of updates that must be performed and   *
* the number of buffer iterations. No Local buffered mode with pauses and    *
* waveform staging are implemented yet. Please refer to DAQ STC Technical    *
* Reference Manual for this features                                         *
* Parameters:                                                                *
*     n_updates:  number of updates in each buffer operation                 *
*     n_iter:     number of iterations that must be performed                *
*****************************************************************************/
void  DAC_numSetup(DWORD n_updates, DWORD n_iter)
{
   set(joint_reset, 5);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 5);

   set(ao_mode_1, 1);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_1, ao_mode_1);

   clr(ao_mode_2, 2);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_2, ao_mode_2);

   //Load numbre of updates for each iteration
   DAQ_STC_Windowed_Mode_Write(AO_BC_LOAD_A_LO, (WORD)n_updates);
   DAQ_STC_Windowed_Mode_Write(AO_BC_LOAD_A_HI, (WORD)(n_updates>>16));

   set(ao_command_1, 5);
   DAQ_STC_Windowed_Mode_Write(AO_COMMAND_1, ao_command_1);
   clr(ao_command_1, 5);

   clr(ao_mode_2, 11);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_2, ao_mode_2);

   //load number of iteration
   DAQ_STC_Windowed_Mode_Write(AO_UC_LOAD_A_LO, (WORD)n_iter);
   DAQ_STC_Windowed_Mode_Write(AO_UC_LOAD_A_HI, (WORD)(n_iter>>16));

   set(ao_command_1, 7);
   DAQ_STC_Windowed_Mode_Write(AO_COMMAND_1, ao_command_1);
   clr(ao_command_1, 7);

   set(joint_reset, 9);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 9);
}

/*****************************************************************************
*               void DAC_update(BYTE source, BYTE ui)                        *
*----------------------------------------------------------------------------*
* Use this function to select the update event. No waveform staging or local *
* buffer mode with pauses are implemented yet.                               *
* Parameters:                                                                *
*        source:  bit 0..4: update signal source                             *
*                 bit 5:    polarity of update signal                        *
*                 bit 6..7  NOT USED                                         *
*        ui    :  bit 0..4: UI source select                                 *
*                 bit 5:    polarity of UI signal                            *
*                 bit 6..7: NOT USED                                         *
*****************************************************************************/
void  DAC_update(BYTE source, BYTE ui)
{
   set(joint_reset, 5);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 5);

   if( (source&0x1F)==0 ){
      clr(ao_command_2, 11);

      ao_mode_1 &= 0x07FF;
      clr(ao_mode_1, 4);
      if( (ui&0x1F)==0 ){
         ao_mode_1 &= 0xF83F;
         clr(ao_mode_1, 3);
      } else {
         if( (ui&0x1F)==20 ){
            ao_mode_1 = (ao_mode_1 & 0xF83F) | 0x0500;
            clr(ao_mode_1, 3);
         } else {
            ao_mode_1 = (ao_mode_1 & 0xF83F) | (((WORD)ui & 0x001F)<<6);
            if(ui & 0x20) set(ao_mode_1, 3);
            else          clr(ao_mode_1, 3);
         }
      }
   } else {
      if( (source&0x1F)==19 ){
         set(ao_command_2, 11);
         ao_mode_1 = (ao_mode_1 & 0x07FF) | 0x9800;
         clr(ao_mode_1, 4);
      } else {
         set(ao_command_2, 11);
         ao_mode_1 = (ao_mode_1 & 0x07FF) | (((WORD)source&0x001F)<<11);
         if(source&0x20)   set(ao_mode_1, 4);
         else              clr(ao_mode_1, 4);
      }
   }

   DAQ_STC_Windowed_Mode_Write(AO_COMMAND_2, ao_command_2);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_1, ao_mode_1);

   set(joint_reset, 9);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 9);
}

/*****************************************************************************
*              void DAC_channel(BYTE num)                                    *
*----------------------------------------------------------------------------*
* Use this function to select hom many analog input channel must be updated  *
* every UPDATE event                                                         *
* Parameters:  multi:   if it's different from 0 multiple channel feature are*
*                       enabled                                              *
*              num:     number of channels that must be updated              *
* Note: With PCI6025E only two channel (DAC0 DAC1) are available             *
*****************************************************************************/
void DAC_channel(BYTE multi, BYTE num)
{
   set(joint_reset, 5);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 5);

   if(multi)   set(ao_mode_1, 5);
   else        clr(ao_mode_1, 5);

   ao_output_control = (ao_output_control & 0xFC3F) | (((WORD)num&0x0F)<<6);

   DAQ_STC_Windowed_Mode_Write(AO_MODE_1, ao_mode_1);
   DAQ_STC_Windowed_Mode_Write(AO_OUTPUT_CONTROL, ao_output_control);

   set(joint_reset, 9);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 9);
}

/*****************************************************************************
*              void DAC_LDACSourceUpdate(BYTE pref)                          *
*----------------------------------------------------------------------------*
* Use this function to set source and update mode for LDAC<0..1> signals     *
* Parameter:   pref: bit 0: LDAC0 source; if 0 LDAC will output on UPDATE    *
*                                         else will output on UPDATE2        *
*                    bit 1: DAC0 update mode; 0 DAC will update immediately  *
*                                             1 timed update mode            *
*                    bit 2: same of bit 0 but for LDAC1                      *
*                    bit 3: same of bit 1 but for DAC1                       *
*                    bit 4..7 NOT USED                                       *
*****************************************************************************/
void  DAC_LDACSourceUpdate(BYTE pref)
{
   set(joint_reset, 5);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 5);

   ao_command_1 = (ao_command_1 & 0xFFE1) | (((WORD)pref&0x001E)<<1);
   DAQ_STC_Windowed_Mode_Write(AO_COMMAND_1, ao_command_1);

   set(joint_reset, 9);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 9);
}

/*****************************************************************************
*              void DAC_stopError(BYTE error)                                *
*----------------------------------------------------------------------------*
* Use this function to set up the error conditions upon which the Timing     *
* Module of Analog Output will stop                                          *
* Parameter:   error:   bit 0 Stop on BC_TC error                            *
*                       bit 1 Stop on BC_TC Trigger error                    *
*                       bit 2 Stop on Overrun error                          *
*                       bit 3..7 NOT USED                                    *
* Set this bits means that Timing Module will stop. Clear this bits means    *
* Timing Module ignores error.                                               *
*****************************************************************************/
void  DAC_stopError(BYTE error)
{
   set(joint_reset, 5);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 5);

   ao_mode_3 = (ao_mode_3 & 0xFFC7) | (((WORD)error & 0x0038)<<3);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_3, ao_mode_3);

   set(joint_reset, 9);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 9);
}

/*****************************************************************************
*              void DAC_FIFOFlags(BYTE flags)                                *
*----------------------------------------------------------------------------*
* Use this function to select the data FIFO condition on which Interrupt or  *
* DMA request are asserted                                                   *
* Parameter:   flags:   bit 0..1 0 generate on empty FIFO                    *
*                                1 generate on less than half full FIFO      *
*                                2 generate on less than full FIFO           *
*                                3 generate on less than half full FIFO but  *
*                                  keep asserted until FIFO is full          *
*                       bit 2..7 NOT USED                                    *
*****************************************************************************/
void  DAC_FIFOFlags(BYTE flags)
{
   set(joint_reset, 5);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 5);

   ao_mode_2 = (ao_mode_2 & 0x1FFF) | (((WORD)flags & 0x0003) << 14);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_2, ao_mode_2);

   set(joint_reset, 9);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);
   clr(joint_reset, 9);
}

/*****************************************************************************
*              void DAC_enableInterrupts(WORD itr)                           *
*----------------------------------------------------------------------------*
* With this function it's possible to enable event on which Timing Module    *
* can generate Interrupt Request                                             *
* Parameter:   int:  bit 0: BC_TC Interrupt                                  *
*                    bit 1: START1 Interrupt                                 *
*                    bit 2: UPDATE Interrupt                                 *
*                    bit 3: START Interrupt                                  *
*                    bit 4: NOT USED                                         *
*                    bit 5: Error Interrupt                                  *
*                    bit 6: UC_TC Interrupt                                  *
*                    bit 7: NOT USED                                         *
*                    bit 8: FIFO Interrupt                                   *
*                    bit 9..15 NOT USED                                      *
* Set bits to enable interrupt generation or clear to avoid interrupt        *
* generation                                                                 *
*****************************************************************************/
void  DAC_enableInterrupts(WORD itr)
{
   interrupt_b_enable = (interrupt_b_enable & 0xFE90) | (itr & 0x016F);

   DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ENABLE, interrupt_b_enable);
}

/*****************************************************************************
*                       void DAC_arm(void)                                   *
*----------------------------------------------------------------------------*
* With this function Analog Output counters will be armed and the first value*
* is preloaded into DACs                                                     *
*****************************************************************************/
void  DAC_arm(void)
{
   set(ao_mode_3, 2);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_3, ao_mode_3);

   clr(ao_mode_3, 2);
   DAQ_STC_Windowed_Mode_Write(AO_MODE_3, ao_mode_3);

   while( (DAQ_STC_Windowed_Mode_Read(JOINT_STATUS_2) & 0x0020) != 0 );

   ao_command_1 |= 0x0540;
   DAQ_STC_Windowed_Mode_Write(AO_COMMAND_1, ao_command_1);
}

/*****************************************************************************
*                 void DAC_startOperation(void)                              *
*----------------------------------------------------------------------------*
* If software trigger was previously selected this function initiate an      *
* analog output operation. If trigger wasn't software driven this function   *
* has no effects                                                             *
*****************************************************************************/
void  DAC_startOperation(void)
{
   set(ao_command_2, 0);
   DAQ_STC_Windowed_Mode_Write(AO_COMMAND_2, ao_command_2);
   clr(ao_command_2, 0);
}

/*****************************************************************************
*                         void DAC_Init(void)                                *
*----------------------------------------------------------------------------*
* This function must be called previously any DAC settings or DAC operations *
* It preapers the envirorment to work with DACs in right way and calls the   *
* reset function (DAC_reset)                                                 *
*****************************************************************************/
void  DAC_Init(void)
{
   DAC_Initialized = 1;
   /*DAC0: Bipolar mode, Internal referenced, ReGlitch disabled*/
   Immediate_Writew(DAC_CONFIG, 0x0001);

   /*DAC1: Bipolar mode, Internal referenced, ReGlitch disabled*/
   Immediate_Writew(DAC_CONFIG, 0x0101);

   DAC_reset();
}

/*****************************************************************************
*                 void DAC_output(BYTE DAC_ID, WORD value)                   *
*----------------------------------------------------------------------------*
* Write "value" in "DAC_ID" register and put out the releted analog value    *
* Parameters:  DAC_ID:  This can be DAC0 or DAC1                             *
*              value:   first 12-bit output value                            *
*                       last 4 bits NOT USED                                 *
*****************************************************************************/
void  DAC_output(BYTE DAC_ID, WORD value)
{
   if(DAC_Initialized){
      if(DAC_ID)  Immediate_Writew(DAC1_DATA, (value & 0x0FFF));
      else        Immediate_Writew(DAC0_DATA, (value & 0x0FFF));
   }
}
/*End of file:    DAC.C*/
