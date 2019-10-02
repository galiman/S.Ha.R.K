/*****************************************************************************
* Filename:    Regconst.h                                                    *
* Author:      Ziglioli Marco                                                *
* Date:        23/03/2001                                                    *
* Description: Symbolic constant names of registers used in STC and on board *
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


#ifndef  _MY_REGCONST_H_
#define  _MY_REGCONST_H_

//DAQ STC registers (see file REG-DES.txt for bitfield description)
#define  WIN_ADDR_REG            (0x00) * 2     //Write-only
#define  WIN_DATA_WR_REG         (0x01) * 2     //Write-only
#define  WIN_DATA_RD_REG         (0x01) * 2     //Read-only

#define  INTERRUPT_A_ACK         0x02           //Write-only
#define  AI_STATUS_1             0x02           //Read-only
#define  INTERRUPT_B_ACK         0x03           //Write-only
#define  AO_STATUS_1             0x03           //Read-only
#define  AI_COMMAND_2            0x04           //Write-only
#define  G_STATUS                0x04           //Read-only
#define  AI_STATUS_2             0x05           //Read-only
#define  AO_COMMAND_2            0x05           //Write-only
#define  AO_STATUS_2             0x06           //Read-only
#define  G0_COMMAND              0x06           //Write-only
#define  DIO_PARALLEL_INPUT      0x07           //Read-only
#define  G1_COMMAND              0x07           //Write-only
#define  AI_COMMAND_1            0x08           //Write-only
#define  G0_HW_SAVE_HI           0x08           //Read-only
#define  AO_COMMAND_1            0x09           //Write-only
#define  G0_HW_SAVE_LO           0x09           //Read-only
#define  DIO_OUTPUT              0x0A           //Write-only
#define  G1_HW_SAVE_HI           0x0A           //Read-only
#define  DIO_CONTROL             0x0B           //Write-only
#define  G1_HW_SAVE_LO           0x0B           //Read-only
#define  AI_MODE_1               0x0C           //Write-only
#define  G0_SAVE_HI              0x0C           //Read-only
#define  AI_MODE_2               0x0D           //Write-only
#define  G0_SAVE_LO              0x0D           //Read-only
#define  AI_SI_LOAD_A_HI         0x0E           //Write-only
#define  G1_SAVE_HI              0x0E           //Read-only
#define  AI_SI_LOAD_A_LO         0x0F           //Write-only
#define  G1_SAVE_LO              0x0F           //Read-only
#define  AI_SI_LOAD_B_HI         0x10           //Write-only
#define  AO_UI_SAVE_HI           0x10           //Read-only
#define  AI_SI_LOAD_B_LO         0x11           //Write-only
#define  AO_UI_SAVE_LO           0x11           //Read-only
#define  AI_SC_LOAD_A_HI         0x12           //Write-only
#define  AO_BC_SAVE_HI           0x12           //Read-only
#define  AI_SC_LOAD_A_LO         0x13           //Write-only
#define  AO_BC_SAVE_LO           0x13           //Read-only
#define  AI_SC_LOAD_B_HI         0x14           //Write-only
#define  AO_UC_SAVE_HI           0x14           //Read-only
#define  AI_SC_LOAD_B_LO         0x15           //Write-only
#define  AO_UC_SAVE_LO           0x15           //Read-only

#define  AI_SI2_LOAD_A           0x17           //Write-only
#define  AO_UI2_SAVE             0x17           //Read-only
#define  AI_SI2_LOAD_B           0x19           //Write-only
#define  AI_SI2_SAVE             0x19           //Read-only
#define  G0_MODE                 0x1A           //Write-only
#define  AI_DIV_SAVE             0x1A           //Read-only
#define  G1_MODE                 0x1B           //Write-only
#define  JOINT_STATUS_1          0x1B           //Read-only
#define  DIO_SERIAL_INPUT        0x1C           //Read-only
#define  G0_LOAD_A_HI            0x1C           //Write-only
#define  G0_LOAD_A_LO            0x1D           //Write-only
#define  JOINT_STATUS_2          0x1D           //Read-only
#define  G0_LOAD_B_HI            0x1E           //Write-only
#define  G0_LOAD_B_LO            0x1F           //Write-only
#define  G1_LOAD_A_HI            0x20           //Write-only
#define  G1_LOAD_A_LO            0x21           //Write-only
#define  G1_LOAD_B_HI            0x22           //Write-only
#define  G1_LOAD_B_LO            0x23           //Write-only
#define  G1_INPUT_SELECT         0x25           //Write-only
#define  G0_INPUT_SELECT         0x24           //Write-only
#define  AO_MODE_1               0x26           //Write-only
#define  AO_MODE_2               0x27           //Write-only
#define  AO_UI_LOAD_A_HI         0x28           //Write-only
#define  AO_UI_LOAD_A_LO         0x29           //Write-only
#define  AO_UI_LOAD_B_HI         0x2A           //Write-only
#define  AO_UI_LOAD_B_LO         0x2B           //Write-only
#define  AO_BC_LOAD_A_HI         0x2C           //Write-only
#define  AO_BC_LOAD_A_LO         0x2D           //Write-only
#define  AO_BC_LOAD_B_HI         0x2E           //Write-only
#define  AO_BC_LOAD_B_LO         0x2F           //Write-only
#define  AO_UC_LOAD_A_HI         0x30           //Write-only
#define  AO_UC_LOAD_A_LO         0x31           //Write-only
#define  AO_UC_LOAD_B_HI         0x32           //Write-only
#define  AO_UC_LOAD_B_LO         0x33           //Write-only
#define  AO_UI2_LOAD_A           0x35           //Write-only
#define  AO_UI2_LOAD_B           0x37           //Write-only
#define  CLOCK_AND_FOUT          0x38           //Write-only
#define  IO_BIDIRECTION_PIN      0x39           //Write-only
#define  RTSI_TRIG_DIRECTION     0x3A           //Write-only
#define  INTERRUPT_CONTROL       0x3B           //Write-only
#define  AI_OUTPUT_CONTROL       0x3C           //Write-only
#define  ANALOG_TRIGGER_ETC      0x3D           //Write-only
#define  AI_START_STOP_SELECT    0x3E           //Write-only
#define  AI_TRIGGER_SELECT       0x3F           //Write-only
#define  AI_DIV_LOAD_A           0x40           //Write-only
#define  AI_SI_SAVE_HI           0x40           //Read-only
#define  AI_SI_SAVE_LO           0x41           //Read-only
#define  AI_SC_SAVE_HI           0x42           //Read-only
#define  AO_START_SELECT         0x42           //Write-only
#define  AI_SC_SAVE_LO           0x43           //Read-only
#define  AO_TRIGGER_SELECT       0x43           //Write-only
#define  G0_AUTOINCREMENT        0x44           //Write-only
#define  G1_AUTOINCREMENT        0x45           //Write-only
#define  AO_MODE_3               0x46           //Write-only
#define  GENERIC_CONTROL         0x47           //Write-only
#define  JOINT_RESET             0x48           //Write-only
#define  INTERRUPT_A_ENABLE      0x49           //Write-only
#define  SECOND_IRQ_A_ENABLE     0x4A           //Write-only
#define  INTERRUPT_B_ENABLE      0x4B           //Write-only
#define  SECOND_IRQ_B_ENABLE     0x4C           //Write-only
#define  AI_PERSONAL             0x4D           //Write-only
#define  AO_PERSONAL             0x4E           //Write-only
#define  RTSI_TRIG_A_OUTPUT      0x4F           //Write-only
#define  RTSI_TRIG_B_OUTPUT      0x50           //Write-only
#define  RTSI_BOARD              0x51           //Write-only
#define  WRITE_STROBE_0          0x52           //Write-only
#define  WRITE_STROBE_1          0x53           //Write-only
#define  WRITE_STROBE_2          0x54           //Write-only
#define  WRITE_STROBE_3          0x55           //Write-only
#define  AO_OUTPUT_CONTROL       0x56           //Write-only
#define  AI_MODE_3               0x57           //Write-only

//Analog Input Registers Group
#define  ADC_CONFIG_HI           0x12           //Write-only
#define  ADC_CONFIG_LO           0x10           //Write-only
#define  ADC_DATA_READ           0x1C           //Read-only

//Analog Output Registers Group
#define  DAC_CONFIG              0x16           //Write-only
#define  DAC_FIFO                0x1E           //Write-only
#define  DAC0_DATA               0x18           //Write-only
#define  DAC1_DATA               0x1A           //Write-only

/*PPI OKI82C55A MSM registers:
   Finally I found this walues writing a cycle which scanned board registers
   and check for some test lines value. I don't know why but these aren't
   unique right values. Also 19 1B 1D 1F and 59 5B 5D 5F and 99 9B 9D 9F work
   so if you have collision problems with other address try to change these
   values.
*/
#define  PPI_PORT_A              0xD9     //PORT_A of 8255
#define  PPI_PORT_B              0xDB     //PORT_B of 8255
#define  PPI_PORT_C              0xDD     //PORT_C of 8255
#define  PPI_CFG_REG             0xDF     //Configuration Register


#endif
/*End of file: regconst.h*/
