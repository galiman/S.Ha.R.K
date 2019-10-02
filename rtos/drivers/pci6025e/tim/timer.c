/*****************************************************************************
* Filename:    timer.c                                                       *
* Author:      Ziglioli Marco (Doctor Stein)                                 *
* Date:        25/03/2001                                                    *
* Last update:                                                               *
* Description: Contains routines used to interface with two 24 bits general  *
*              Purpouse Timer Conter on PCI6025E board                       *
*----------------------------------------------------------------------------*
* Notes:       Pulse generation and position sensing isn't implemented yet   *
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

#include <drivers/pci6025e/timer.h>

/*****************************************************************************
*  Reset the specified counter. If argument is grater than 1 both counters   *
*  will be reset                                                             *
*****************************************************************************/
void TIM_reset(BYTE ct)
{
   switch(ct){
      case C0: reset_counter_0(); break;
      case C1: reset_counter_1(); break;
      default: reset_counter_0(); reset_counter_1(); break;
   }
}

void reset_counter_0(void)
{
   set(joint_reset, 2);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset); //Reset flag raised
   clr(joint_reset, 2);

   //clears some registers
   g0_mode = g0_input_select = g0_command = g0_autoincrement = 0;
   DAQ_STC_Windowed_Mode_Write(G0_MODE, g0_mode);
   DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
   DAQ_STC_Windowed_Mode_Write(G0_INPUT_SELECT, g0_input_select);
   DAQ_STC_Windowed_Mode_Write(G0_AUTOINCREMENT, g0_autoincrement);

   interrupt_a_enable &= 0xFEBF;
   DAQ_STC_Windowed_Mode_Write(INTERRUPT_A_ENABLE, interrupt_a_enable);


   //Set synchronized gate flag
   set(g0_command, 8);
   DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);


   interrupt_a_ack |= 0xC060;
   DAQ_STC_Windowed_Mode_Write(INTERRUPT_A_ACK, interrupt_a_ack);

   DAQ_STC_Windowed_Mode_Write(G0_AUTOINCREMENT, g0_autoincrement);
}


void reset_counter_1(void)
{
   set(joint_reset, 3);
   DAQ_STC_Windowed_Mode_Write(JOINT_RESET, joint_reset);  //Reset flag raised
   clr(joint_reset, 3);

   //clears some registers

   g1_mode = g1_input_select = g1_command = g1_autoincrement = 0;
   DAQ_STC_Windowed_Mode_Write(G1_MODE, g1_mode);
   DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
   DAQ_STC_Windowed_Mode_Write(G1_INPUT_SELECT, g1_input_select);
   DAQ_STC_Windowed_Mode_Write(G1_AUTOINCREMENT, g1_autoincrement);

   interrupt_b_enable &= 0xF9FF;
   DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ENABLE, interrupt_b_enable);

   //Set synchronized gate flag
   set(g1_command, 8);
   DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);


   interrupt_b_ack |= 0xC006;
   DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ACK, interrupt_b_ack);

   DAQ_STC_Windowed_Mode_Write(G1_AUTOINCREMENT, g0_autoincrement);
}

/*****************************************************************************
*  Arming the specified counter. If argument is grater than 1 both counters  *
*  will be armed                                                             *
*****************************************************************************/
void TIM_arm(BYTE counter)
{
   switch(counter){
      case C0:  arm_counter_0(); break;
      case C1:  arm_counter_1(); break;
      default: arm_counter_0(); arm_counter_1(); break;
   }
}

void arm_counter_0(void)
{
   set(g0_command, 0);
   set(g1_command, 13);
   DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
   DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
   clr(g0_command, 0);
   clr(g1_command, 13);
}

void arm_counter_1(void)
{
   set(g1_command, 0);
   set(g0_command, 13);
   DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
   DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
   clr(g1_command, 0);
   clr(g0_command, 13);
}

/*****************************************************************************
*  Disarming the specified counter. If argument is grater than 1 both        *
*  counters will be disarmed                                                 *
*****************************************************************************/
void TIM_disarm(BYTE counter)
{
   switch(counter){
      case C0:  disarm_counter_0(); break;
      case C1:  disarm_counter_1(); break;
      default: disarm_counter_0(); disarm_counter_1(); break;
   }
}

void disarm_counter_0(void)
{
   set(g0_command, 4);
   set(g1_command, 15);
   DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
   DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
   clr(g0_command, 4);
   clr(g1_command, 15);
}

void disarm_counter_1(void)
{
   set(g1_command, 4);
   set(g0_command, 15);
   DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
   DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
   clr(g1_command, 4);
   clr(g0_command, 15);
}

/*****************************************************************************
*              DWORD TIM_readCounter(BYTE counter)                           *
*----------------------------------------------------------------------------*
* Use this function to read counter value while it's armed and without       *
* disturbing the counting process.                                           *
* Counter specifies which counter will be readed                             *
*****************************************************************************/
DWORD TIM_readCounter(BYTE counter)
{
   DWORD s1 = 0, s2;
   if(counter == C0){
      clr(g0_command, 1);

      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      set(g0_command, 1);

      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      clr(g0_command, 1);
      s1 = (DWORD)DAQ_STC_Windowed_Mode_Read(G0_SAVE_LO) +
           ((DWORD)DAQ_STC_Windowed_Mode_Read(G0_SAVE_HI) << 16);

      s2 = (DWORD)DAQ_STC_Windowed_Mode_Read(G0_SAVE_LO) +
           ((DWORD)DAQ_STC_Windowed_Mode_Read(G0_SAVE_HI) << 16);

   if(s1 != s2)
      s1 = (DWORD)DAQ_STC_Windowed_Mode_Read(G0_SAVE_LO) +
           ((DWORD)DAQ_STC_Windowed_Mode_Read(G0_SAVE_HI) << 16);

   } else {
      clr(g1_command, 1);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      set(g1_command, 1);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      clr(g1_command, 1);
      s1 = (DWORD)DAQ_STC_Windowed_Mode_Read(G1_SAVE_LO) +
        ((DWORD)DAQ_STC_Windowed_Mode_Read(G1_SAVE_HI) << 16);

      s2 = (DWORD)DAQ_STC_Windowed_Mode_Read(G1_SAVE_LO) +
        ((DWORD)DAQ_STC_Windowed_Mode_Read(G1_SAVE_HI) << 16);

      if(s1 != s2)
         s1 = (DWORD)DAQ_STC_Windowed_Mode_Read(G1_SAVE_LO) +
              ((DWORD)DAQ_STC_Windowed_Mode_Read(G1_SAVE_HI) << 16);
   }

   return s1;
}

/*****************************************************************************
*              DWORD TIM_readHWSaveReg(BYTE counter)                         *
*----------------------------------------------------------------------------*
* Use this function to read the value in Hardware save registers.            *
* Counter specifies which counter will be readed                             *
*****************************************************************************/
DWORD TIM_readHWSaveReg(BYTE counter)
{
   DWORD s1 = 0;

   if(counter == C0){
      if( (DAQ_STC_Windowed_Mode_Read(AI_STATUS_1) & 0x0004) != 0 )
         s1 = (DWORD)DAQ_STC_Windowed_Mode_Read(G0_HW_SAVE_LO) +
              ((DWORD)DAQ_STC_Windowed_Mode_Read(G0_HW_SAVE_HI) << 16);
   } else {
      if( (DAQ_STC_Windowed_Mode_Read(AO_STATUS_1) & 0x0004) != 0 )
         s1 = (DWORD)DAQ_STC_Windowed_Mode_Read(G1_HW_SAVE_LO) +
              ((DWORD)DAQ_STC_Windowed_Mode_Read(G1_HW_SAVE_HI) << 16);
   }

   return s1;
}

/*****************************************************************************
*void TIM_eventCounting(BYTE counter, BYTE source, BYTE gate,                *
*                      BYTE interrupts, DWORD in)                            *
*----------------------------------------------------------------------------*
*  Use this function to perform an event counting through source line        *
*  selected and, in the case of gating event counting, when gate enable      *
*  counting operation                                                        *
*  Parameters:                                                               *
*           counter: select which counter must perform counting              *
*           source: bit 0..4: source select: 0 G_IN_TIMEBASE1                *
*                                            1 through 10 PFI 0..9           *
*                                            11 through 17 RTSI 0..6         *
*                                            18 IN_TIMEBASE_2                *
*                                            19 other G_TC                   *
*                   bit 5: source polarity: 0 counting rising edge           *
*                                           1 counting falling edge          *
*                   bit 6: output polarity of G_OUT 0 active hi              *
*                                                   1 active lo              *
*                   bit 7: counting direction 0 down counting                *
*                                             1 up counting                  *
*           gate:   bit 0..4: gate select:   1 through 10 PFI 0..9           *
*                                            11 through 17 RTSI 0..6         *
*                                            18 IN_TIMEBASE2                 *
*                                            19 UI2_TC                       *
*                                            20 other G_TC                   *
*                                            21 AI_START1                    *
*                                            31 Logic low (not gated count)  *
*                   bit 5: gate polarity 0 active high                       *
*                                        1 active low                        *
*                   bit 6..7 output mode for G_OUT: 1 one clock cycle output *
*                                                   2 toggle on TC           *
*                                                   3 toggle on TC or gate   *
*           interrupts: bit 0: Terminal Count Interrupt enable               *
*                       bit 1: Gate Interrupt enable                         *
*           in: Initial value loaded into the counter                        *
*****************************************************************************/
void TIM_eventCounting(BYTE counter, BYTE source, BYTE gate,
                       BYTE interrupts, DWORD in)
{
   in &= 0x00FFFFFF;
   if(counter == C0){
      clr(g0_mode, 7);
      DAQ_STC_Windowed_Mode_Write(G0_MODE, g0_mode);
      DAQ_STC_Windowed_Mode_Write(G0_LOAD_A_LO, (WORD)in);
      DAQ_STC_Windowed_Mode_Write(G0_LOAD_A_HI, (WORD)(in >> 16));

      set(g0_command, 2);
      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      clr(g0_command, 2);

      g0_input_select &= 0x0003; //reset bits that I'm going to manage

      g0_input_select |= (source & 0x1F) << 2;        //Source selected
      if(source & 0x20)    set(g0_input_select, 15);  //Source polarity
      else                 clr(g0_input_select, 15);

      g0_input_select |= (gate & 0x1F) << 7;    //Gate selected

      /* GI_OR_GATE = 0
         GI_GATE_SELECT_LOAD_SOURCE = 0
         this bit is 0*/

      if(source & 0x40)    set(g0_input_select, 14);
      else                 clr(g0_input_select, 14);

      g0_mode &= 0x8080;
      if(gate & 0x20)      set(g0_mode, 13);
      else                 clr(g0_mode, 13);

      g0_mode |= (gate & 0xC0) << 2;
      g0_mode |= 0x0011;

      g0_command &= 0xE79F;
      if(source & 0x80)       set(g0_command, 5);

      interrupt_a_enable &= 0xFEBF;

      if(interrupts & 0x01)      set(interrupt_a_enable, 6);
      if(interrupts & 0x02)      set(interrupt_a_enable, 8);

      DAQ_STC_Windowed_Mode_Write(G0_INPUT_SELECT, g0_input_select);
      DAQ_STC_Windowed_Mode_Write(G0_MODE, g0_mode);
      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      DAQ_STC_Windowed_Mode_Write(INTERRUPT_A_ENABLE, interrupt_a_enable);
   } else {
      clr(g1_mode, 7);
      DAQ_STC_Windowed_Mode_Write(G1_MODE, g1_mode);
      DAQ_STC_Windowed_Mode_Write(G1_LOAD_A_LO, (WORD)in);
      DAQ_STC_Windowed_Mode_Write(G1_LOAD_A_HI, (WORD)(in >> 16));

      set(g1_command, 2);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      clr(g1_command, 2);

      g1_input_select &= 0x0003; //reset bits that I'm going to manage

      g1_input_select |= (source & 0x1F) << 2;        //Source selected
      if(source & 0x20)    set(g1_input_select, 15);  //Source polarity
      else                 clr(g1_input_select, 15);

      g1_input_select |= (gate & 0x1F) << 7;    //Gate selected

      /* Gi_OR_GATE = 0
         Gi_GATE_SELECT_LOAD_SOURCE = 0
         this bit is 0*/

      if(source & 0x40)    set(g1_input_select, 14);
      else                 clr(g1_input_select, 14);

      g1_mode &= 0x8080;
      if(gate & 0x20)      set(g1_mode, 13);
      else                 clr(g1_mode, 13);

      g1_mode |= (gate & 0xC0) << 2;
      g1_mode |= 0x0011;

      g1_command &= 0xE79F;
      if(source & 0x80)       set(g1_command, 5);

      interrupt_b_enable &= 0xF9FF;

      if(interrupts & 0x01)      set(interrupt_b_enable, 9);
      if(interrupts & 0x02)      set(interrupt_b_enable, 10);

      DAQ_STC_Windowed_Mode_Write(G1_INPUT_SELECT, g1_input_select);
      DAQ_STC_Windowed_Mode_Write(G1_MODE, g1_mode);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ENABLE, interrupt_b_enable);
   }
}

/*****************************************************************************
* void TIM_bufferedEventCounting(BYTE counter, BYTE source, BYTE gate,       *
*                                BYTE cumulative, DWORD in)                  *
*----------------------------------------------------------------------------*
* Like previous function performs an event counting but saves into Hardware  *
* save registers counter value when gate is enabled.                         *
* if parameter cumulative is 0 every activation of gate makes counter        *
* resetting and starting from 0 the new count. If parameter is 1 the counting*
* is cumulative and counter doesn't reset.                                   *
*****************************************************************************/
void TIM_bufferedEventCounting(BYTE counter, BYTE source, BYTE gate,
                               BYTE cumulative, DWORD in)
{
   in &= 0x00FFFFFF;
   if(counter == C0){
      clr(g0_mode, 7);
      DAQ_STC_Windowed_Mode_Write(G0_MODE, g0_mode);
      DAQ_STC_Windowed_Mode_Write(G0_LOAD_A_LO, (WORD)in);
      DAQ_STC_Windowed_Mode_Write(G0_LOAD_A_HI, (WORD)(in >> 16));

      set(g0_command, 2);
      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      clr(g0_command, 2);

      g0_input_select &= 0x0003; //reset bits that I'm going to manage

      g0_input_select |= (source & 0x1F) << 2;        //Source selected
      if(source & 0x20)    set(g0_input_select, 15);  //Source polarity
      else                 clr(g0_input_select, 15);

      g0_input_select |= (gate & 0x1F) << 7;    //Gate selected

      /* GI_OR_GATE = 0
         GI_GATE_SELECT_LOAD_SOURCE = 0
         this bit is 0*/

      if(source & 0x40)    set(g0_input_select, 14);
      else                 clr(g0_input_select, 14);

      g0_mode &= 0x0080;
      if(gate & 0x20)      set(g0_mode, 13);
      else                 clr(g0_mode, 13);

      g0_mode |= (gate & 0xC0) << 2;
      g0_mode |= 0x0011;

      set(g0_mode, 15);

      if(!cumulative)
         set(g0_mode, 14);

      g0_mode |= 0x001A;

      g0_command &= 0xE79F;
      if(source & 0x80)       set(g0_command, 5);

      interrupt_a_enable &= 0xFEBF;
      set(interrupt_a_enable, 8);

      DAQ_STC_Windowed_Mode_Write(G0_INPUT_SELECT, g0_input_select);
      DAQ_STC_Windowed_Mode_Write(G0_MODE, g0_mode);
      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      DAQ_STC_Windowed_Mode_Write(INTERRUPT_A_ENABLE, interrupt_a_enable);
   } else {
      clr(g1_mode, 7);
      DAQ_STC_Windowed_Mode_Write(G1_MODE, g1_mode);
      DAQ_STC_Windowed_Mode_Write(G1_LOAD_A_LO, (WORD)in);
      DAQ_STC_Windowed_Mode_Write(G1_LOAD_A_HI, (WORD)(in >> 16));

      set(g1_command, 2);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      clr(g1_command, 2);

      g1_input_select &= 0x0003; //reset bits that I'm going to manage

      g1_input_select |= (source & 0x1F) << 2;        //Source selected
      if(source & 0x20)    set(g1_input_select, 15);  //Source polarity
      else                 clr(g1_input_select, 15);

      g1_input_select |= (gate & 0x1F) << 7;    //Gate selected

      /* GI_OR_GATE = 0
         GI_GATE_SELECT_LOAD_SOURCE = 0
         this bit is 0*/

      if(source & 0x40)    set(g1_input_select, 14);
      else                 clr(g1_input_select, 14);

      g1_mode &= 0x0080;
      if(gate & 0x20)      set(g1_mode, 13);
      else                 clr(g1_mode, 13);

      g1_mode |= (gate & 0xC0) << 2;
      g1_mode |= 0x0011;

      set(g1_mode, 15);

      if(!cumulative)
         set(g1_mode, 14);

      g1_mode |= 0x001A;

      g1_command &= 0xE79F;
      if(source & 0x80)       set(g1_command, 5);

      interrupt_b_enable &= 0xFEBF;
      set(interrupt_b_enable, 8);

      DAQ_STC_Windowed_Mode_Write(G1_INPUT_SELECT, g1_input_select);
      DAQ_STC_Windowed_Mode_Write(G1_MODE, g1_mode);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ENABLE, interrupt_b_enable);
   }
}

/*****************************************************************************
* void TIM_timeMeasurement(BYTE counter, BYTE source, BYTE, gate, BYTE type  *
*                          BYTE itr, DWORD in)                               *
*----------------------------------------------------------------------------*
* Use this function to perform a single period or a single pulsewidth        *
* measurement. Parameters "counter source gate and in" are equal to previous *
* function so watch above.                                                   *
* Other parameters:  type: specifies type of measure: (0)single period       *
*                                                     (1)pulsewidth          *
*                    itr:  specifies which interrupt events enable:          *
*                                            bit 0 enable TC event           *
*                                            bit 1 enable gate event         *
*****************************************************************************/
void TIM_timeMeasurement(BYTE counter, BYTE source, BYTE gate, BYTE type,
                         BYTE itr, DWORD in)
{
   if(counter == C0){
      clr(g0_mode, 7);
      DAQ_STC_Windowed_Mode_Write(G0_MODE, g0_mode);
      DAQ_STC_Windowed_Mode_Write(G0_LOAD_A_LO, (WORD)in);
      DAQ_STC_Windowed_Mode_Write(G0_LOAD_A_HI, (WORD)(in >> 16));

      set(g0_command, 2);
      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      clr(g0_command, 2);

      g0_input_select &= 0x0003;

      g0_input_select |= (source & 0x1F) << 2;

      if(source & 0x20)    set(g0_input_select, 15);

      g0_input_select |= (gate & 0x1F) << 7;

      if(source & 0x40)    set(g0_input_select, 14);

      g0_mode &= 0x0080;

      if(gate & 0x20)   set(g0_mode, 13);

      g0_mode |= ((WORD)gate & 0x00C0) << 2;

      if(!type)         g0_mode |= 0x8802;
      else              g0_mode |= 0x4811;

      g0_command &= 0xE79F;
      set(g0_command, 5);

      interrupt_a_enable &= 0xFEBF;
      if(itr & 0x01)    set(interrupt_a_enable, 6);
      if(itr & 0x02)    set(interrupt_a_enable, 8);

      DAQ_STC_Windowed_Mode_Write(G0_INPUT_SELECT, g0_input_select);
      DAQ_STC_Windowed_Mode_Write(G0_MODE, g0_mode);
      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      DAQ_STC_Windowed_Mode_Write(INTERRUPT_A_ENABLE, interrupt_a_enable);
   } else {
      clr(g1_mode, 7);
      DAQ_STC_Windowed_Mode_Write(G1_MODE, g1_mode);
      DAQ_STC_Windowed_Mode_Write(G1_LOAD_A_LO, (WORD)in);
      DAQ_STC_Windowed_Mode_Write(G1_LOAD_A_HI, (WORD)(in >> 16));

      set(g1_command, 2);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      clr(g1_command, 2);

      g1_input_select &= 0x0003;

      g1_input_select |= (source & 0x1F) << 2;

      if(source & 0x20)    set(g1_input_select, 15);

      g1_input_select |= (gate & 0x1F) << 7;

      if(source & 0x40)    set(g1_input_select, 14);

      g1_mode &= 0x0080;

      if(gate & 0x20)   set(g1_mode, 13);

      g1_mode |= ((WORD)gate & 0x00C0) << 2;

      if(!type)         g1_mode |= 0x8802;
      else              g1_mode |= 0x4811;

      g1_command &= 0xE79F;
      set(g1_command, 5);

      interrupt_b_enable &= 0xF9FF;
      if(itr & 0x01)    set(interrupt_a_enable, 9);
      if(itr & 0x02)    set(interrupt_a_enable, 10);

      DAQ_STC_Windowed_Mode_Write(G1_INPUT_SELECT, g1_input_select);
      DAQ_STC_Windowed_Mode_Write(G1_MODE, g1_mode);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ENABLE, interrupt_b_enable);

   }
}

/*****************************************************************************
* void TIM_bufferedTimeMeasurement(BYTE counter, BYTE source, BYTE gate,     *
*                                  BYTE type, DWORD in)                      *
*----------------------------------------------------------------------------*
* Use this function to perform a period or semiperiod or pulsewidth multiple *
* measurement. Refer to DAQ-STC technical reference manual to understand what*
* are this kind of measurement. See above TIM_eventCounting for parameters   *
* counter, source, gate, in.                                                 *
* Parameter type: type of buffered measurement: (0)period                    *
*                                               (1)semiperiod                *
*                                               (2)pulsewidth                *
*****************************************************************************/
void TIM_bufferedTimeMeasurement(BYTE counter, BYTE source, BYTE gate,
                                 BYTE type, DWORD in)
{
   if(counter == C0){
      clr(g0_mode, 7);
      DAQ_STC_Windowed_Mode_Write(G0_MODE, g0_mode);
      DAQ_STC_Windowed_Mode_Write(G0_LOAD_A_LO, (WORD)in);
      DAQ_STC_Windowed_Mode_Write(G0_LOAD_A_HI, (WORD)(in >> 16));

      set(g0_command, 2);
      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      clr(g0_command, 2);

      g0_input_select &= 0x0003;

      g0_input_select |= (source & 0x1F) << 2;

      if(source & 0x20)    set(g0_input_select, 15);

      g0_input_select |= (gate & 0x1F) << 7;

      if(source & 0x40)    set(g0_input_select, 14);

      g0_mode &= 0x0080;

      if(gate & 0x20)   set(g0_mode, 13);

      g0_mode |= ((WORD)gate & 0x00C0) << 2;

      switch(type){
         case 0: g0_mode |= 0x401A; break;
         case 1: g0_mode |= 0x401F; break;
         case 2: g0_mode |= 0x4019; break;
         default: g0_mode |= 0x401A; break;
      }

      g0_command &= 0xE79F;
      set(g0_command, 5);

      interrupt_a_enable &= 0xFEBF;
      set(interrupt_a_enable, 8);

      DAQ_STC_Windowed_Mode_Write(G0_INPUT_SELECT, g0_input_select);
      DAQ_STC_Windowed_Mode_Write(G0_MODE, g0_mode);
      DAQ_STC_Windowed_Mode_Write(G0_COMMAND, g0_command);
      DAQ_STC_Windowed_Mode_Write(INTERRUPT_A_ENABLE, interrupt_a_enable);
   } else {
      clr(g1_mode, 7);
      DAQ_STC_Windowed_Mode_Write(G1_MODE, g1_mode);
      DAQ_STC_Windowed_Mode_Write(G1_LOAD_A_LO, (WORD)in);
      DAQ_STC_Windowed_Mode_Write(G1_LOAD_A_HI, (WORD)(in >> 16));

      set(g1_command, 2);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      clr(g1_command, 2);

      g1_input_select &= 0x0003;

      g1_input_select |= (source & 0x1F) << 2;

      if(source & 0x20)    set(g1_input_select, 15);

      g1_input_select |= (gate & 0x1F) << 7;

      if(source & 0x40)    set(g1_input_select, 14);

      g1_mode &= 0x0080;

      if(gate & 0x20)   set(g1_mode, 13);

      g1_mode |= ((WORD)gate & 0x00C0) << 2;

      switch(type){
         case 0: g1_mode |= 0x401A; break;
         case 1: g1_mode |= 0x401F; break;
         case 2: g1_mode |= 0x4019; break;
         default: g1_mode |= 0x401A; break;
      }

      g1_command &= 0xE79F;
      set(g1_command, 5);

      interrupt_b_enable &= 0xF9FF;
      set(interrupt_a_enable, 10);

      DAQ_STC_Windowed_Mode_Write(G1_INPUT_SELECT, g1_input_select);
      DAQ_STC_Windowed_Mode_Write(G1_MODE, g1_mode);
      DAQ_STC_Windowed_Mode_Write(G1_COMMAND, g1_command);
      DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ENABLE, interrupt_b_enable);
   }
}
/* End of file: Timer.c */
