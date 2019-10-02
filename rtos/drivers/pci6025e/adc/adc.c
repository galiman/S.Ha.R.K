/*****************************************************************************
* Filename:       ADC.C                                                      *      
* Author:         Ghiro Andrea,Franchino Gianluca                            *
* Date:           06/06/2001                                                 *
* Description:    Analog 2 digital conversion functions                      *
*----------------------------------------------------------------------------*
* Notes: 		Funcion for one sample from Ch0		             *
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

#include <drivers/pci6025e/adc.h>

/*
* Call this function to configure board options.
*/

void Configure_Board(BYTE numch)
{
WORD memhi;
  /*
  * Writing to register Write_Strobe_0_Register with address 82.
  *    Write_Strobe_0 <= 1
  * New pattern = 0x0001
  */
  DAQ_STC_Windowed_Mode_Write(WRITE_STROBE_0,0x0001);
 
  /*
  * Writing to register Write_Strobe_1_Register with address 83.
  *    Write_Strobe_1 <= 1
  * New pattern = 0x0001
  */
  DAQ_STC_Windowed_Mode_Write(WRITE_STROBE_1,0x0001);

  /*
  * Writing to register Config_Memory_High_Register with address 18.
  *    CM_Channel_Number <= channel_number (0)
  *    CM_Channel_Bank <= channel_bank (0)
  *    CM_Channel_Type <= channel_type_for_rev_b (3)
  * New pattern = 0x3000
  */
  memhi = 0x3000 | numch;
  Immediate_Writew(ADC_CONFIG_HI,memhi);

  /*
  * Writing to register Config_Memory_Low_Register with address 16.
  *    CM_Last_Channel <= last_channel (1)
  *    CM_Trigger <= trigger (0)
  *    CM_AI_Gain <= ai_gain (1)
  *    CM_AI_Polarity <= ai_polarity (0)
  *    CM_AI_Dither_Enable <= ai_dither_enable (0)
  * New pattern = 0x8001
  */
  Immediate_Writew(ADC_CONFIG_LO,0x8100);
  return;
 }

/*
* program the ADC_STC
*/
 
void ADC_Init()
{
/*
* configure the timebase options.
*/
  /*
  * Writing to register Clock_and_FOUT_Register with address 56.
  *    Slow_Internal_Timebase <= p->msc_slow_int_tb_enable (1)
  *    Slow_Internal_Time_Divide_By_2 <= p->msc_slow_int_tb_divide_by_2 (1)
  *    Clock_To_Board <= p->msc_clock_to_board_enable (1)
  *    Clock_To_Board_Divide_By_2 <= p->msc_clock_to_board_divide_by_2 (1)
  * New pattern = 0x1B00
  */
  DAQ_STC_Windowed_Mode_Write(CLOCK_AND_FOUT,0x1B00);

/*
* clear the AI FIFO.
*/
  /*
  * Writing to register Write_Strobe_1_Register with address 83.
  *    Write_Strobe_1 <= 1
  * New pattern = 0x0001
  */
  DAQ_STC_Windowed_Mode_Write(WRITE_STROBE_1,0x0001);

/*
* stop any activities in progress.
*/
  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Reset <= 1
  * New pattern = 0x0001
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0001);

  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 1
  * New pattern = 0x0010
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0010);

  /*
  * Writing to register Interrupt_A_Ack_Register with address 2.
  *    AI_SC_TC_Error_Confirm <= 1
  *    AI_SC_TC_Interrupt_Ack <= 1
  *    AI_START1_Interrupt_Ack <= 1
  *    AI_START2_Interrupt_Ack <= 1
  *    AI_START_Interrupt_Ack <= 1
  *    AI_STOP_Interrupt_Ack <= 1
  *    AI_Error_Interrupt_Ack <= 1
  * New pattern = 0x3F80
  */
  DAQ_STC_Windowed_Mode_Write(INTERRUPT_A_ACK,0x3F80);

  /*
  * Writing to register AI_Command_1_Register with address 8.
  *    AI_Command_1_Register <= 0
  * New pattern = 0x0000
  */
  DAQ_STC_Windowed_Mode_Write(AI_COMMAND_1,0x0000);

  /*
  * Writing to register AI_Mode_1_Register with address 12.
  *    Reserved_One <= 1
  *    AI_Start_Stop <= 1
  * New pattern = 0x000C
  */
  DAQ_STC_Windowed_Mode_Write(AI_MODE_1,0x000C);

  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 0
  *    AI_Configuration_End <= 1
  * New pattern = 0x0100
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0100);

/*
* setup the board.
*/
  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 1
  * New pattern = 0x0010
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0010);

  /*
  * Writing to register Clock_and_FOUT_Register with address 56.
  *    AI_Source_Divide_By_2 <= p->ai_source_divide_by_2 (0)
  *    AI_Output_Divide_By_2 <= p->ai_output_divide_by_2 (1)
  * New pattern = 0x1B80
  */
  DAQ_STC_Windowed_Mode_Write(CLOCK_AND_FOUT,0x1B80);

  /*
  * Writing to register AI_Personal_Register with address 77.
  *    AI_CONVERT_Pulse_Timebase <= p->ai_convert_pulse_timebase (0)
  *    AI_CONVERT_Pulse_Width <= p->ai_convert_pulse_width (1)
  *    AI_FIFO_Flags_Polarity <= p->ai_fifo_flags_polarity (0)
  *    AI_LOCALMUX_CLK_Pulse_Width <= p->ai_localmux_clk_pulse_width (1)
  *    AI_AIFREQ_Polarity <= p->ai_aifreq_polarity (0)
  *    AI_SHIFTIN_Polarity <= p->ai_shiftin_polarity (0)
  *    AI_SHIFTIN_Pulse_Width <= p->ai_shiftin_pulse_width (1)
  *    AI_EOC_Polarity <= p->ai_eoc_polarity (0)
  *    AI_SOC_Polarity <= p->ai_soc_polarity (1)
  *    AI_Overrun_Mode <= p->ai_overrun_mode (1)
  * New pattern = 0xA4A0
  */
  DAQ_STC_Windowed_Mode_Write(AI_PERSONAL,0xA4A0);

  /*
  * Writing to register AI_Output_Control_Register with address 60.
  *    AI_CONVERT_Output_Select <= p->ai_convert_output_select (2)
  *    AI_SC_TC_Output_Select <= p->ai_sc_tc_output_select (3)
  *    AI_SCAN_IN_PROG_Output_Select <= p->ai_scan_in_prog_output_select (3)
  *    AI_LOCALMUX_CLK_Output_Select <= p->ai_localmux_clk_output_select (2)
  * New pattern = 0x032E
  */
  DAQ_STC_Windowed_Mode_Write(AI_OUTPUT_CONTROL,0x032E);

  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 0
  *    AI_Configuration_End <= 1
  * New pattern = 0x0100
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0100);

/*
* access the first value in the configuration
* FIFO.
*/
  /*
  * Writing to register AI_Command_1_Register with address 8.
  *    AI_CONVERT_Pulse <= 1
  * New pattern = 0x0001
  */
  DAQ_STC_Windowed_Mode_Write(AI_COMMAND_1,0x0001); 
 
/* 
* setup for external hardware.
*/
  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 1
  * New pattern = 0x0010
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0010);

  /*
  * Writing to register AI_Mode_2_Register with address 13.
  *    AI_External_MUX_Present <= 0
  * New pattern = 0x0000
  */
  DAQ_STC_Windowed_Mode_Write(AI_MODE_2,0x0000);

  /*
  * Writing to register AI_Output_Control_Register with address 60.
  *    AI_EXTMUX_CLK_Output_Select <= p->ai_extmux_clk_output_select (0)
  * New pattern = 0x032E
  */
  DAQ_STC_Windowed_Mode_Write(AI_OUTPUT_CONTROL,0x032E);

  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 0
  *    AI_Configuration_End <= 1
  * New pattern = 0x0100
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0100);

/*
* enable or disable retriggering.
*/
  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 1
  * New pattern = 0x0010
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0010);

  /*
  * Writing to register AI_Mode_1_Register with address 12.
  *    AI_Trigger_Once <= 1
  * New pattern = 0x000D
  */
  DAQ_STC_Windowed_Mode_Write(AI_MODE_1,0x000D);

  /*
  * Writing to register AI_Trigger_Select_Register with address 63.
  *    AI_START1_Select <= 0
  *    AI_START1_Polarity <= 0
  *    AI_START1_Edge <= 1
  *    AI_START1_Sync <= 1
  * New pattern = 0x0060
  */
  DAQ_STC_Windowed_Mode_Write(AI_TRIGGER_SELECT,0x0060);

  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 0
  *    AI_Configuration_End <= 1
  * New pattern = 0x0100
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0100);
 
/*
* select the number of scans.
*/
/*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 1
  * New pattern = 0x0010
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0010);

  DAQ_STC_Windowed_Mode_Write(AI_MODE_1,0x000F);

  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 0
  *    AI_Configuration_End <= 1
  * New pattern = 0x0100
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0100);
 
/* 
* select the scan start event.
*/
  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 1
  * New pattern = 0x0010
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0010);

  /*
  * Writing to register AI_START_STOP_Select_Register with address 62.
  *    AI_START_Select <= 0
  *    AI_START_Edge <= 1
  *    AI_START_Sync <= 1
  *    AI_START_Polarity <= 0
  * New pattern = 0x0060
  */
  DAQ_STC_Windowed_Mode_Write(AI_START_STOP_SELECT,0x0060);


  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 0
  *    AI_Configuration_End <= 1
  * New pattern = 0x0100
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0100);
  
/*
* select the end of scan event.
*/
  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 1
  * New pattern = 0x0010
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0010);

  /*
  * Writing to register AI_START_STOP_Select_Register with address 62.
  *    AI_STOP_Select <= p->ai_stop_select (19)
  *    AI_STOP_Edge <= 0
  *    AI_STOP_Polarity <= p->ai_stop_polarity (0)
  *    AI_STOP_Sync <= 1
  * New pattern = 0x29E0
  */
  DAQ_STC_Windowed_Mode_Write(AI_START_STOP_SELECT,0x29E0);

  /*
  * Writing to register Joint_Reset_Register with address 72.
  *    AI_Configuration_Start <= 0
  *    AI_Configuration_End <= 1
  * New pattern = 0x0100
  */
  DAQ_STC_Windowed_Mode_Write(JOINT_RESET,0x0100);
 
/*
* clear the AI FIFO.
*/
  /*
  * Writing to register Write_Strobe_1_Register with address 83.
  *    Write_Strobe_1 <= 1
  * New pattern = 0x0001
  */
  DAQ_STC_Windowed_Mode_Write(WRITE_STROBE_1,0x0001);
return;
}

/* 
* Call this function to start the acquistion.
*/

void AI_Start_The_Acquisition()
{
  /*
  * Writing to register AI_Command_1_Register with address 8.
  *    AI_CONVERT_Pulse <= 1
  * New pattern = 0x0001
  */
  DAQ_STC_Windowed_Mode_Write(AI_COMMAND_1,0x0001);
  return;
 }
   
/*End of file:    adc.c*/
   
   
      
 




