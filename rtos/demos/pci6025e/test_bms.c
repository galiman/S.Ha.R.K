/*****************************************************************************
* Filename: test_bms.c                                                       *
* Author: Marco Ziglioli (Doctor Stein)                                      *
* Date: 27/06/2001                                                           *
* Description: Test program for buffered period and semiperiod measurement   *
*              capacity of National PCI6025E board                           *
*----------------------------------------------------------------------------*
* Notes: FOUT are enable and avaiable on pin 50 to provide 1 Mhz frequency   *
*        You should connect source 0 (PIN 41) and source 1 (pin 45) to this  *
*        freq source. Gate 0 (pin 42) and gate 1 (pin 44) must be connected  *
*        rispectivly to DIO7 (pin 32) and DIO6 (pin 30).                     *
*        Use 'g' button to activate gate_action_task which generate a square *
*        wave with a freq of 0.5 Hz and Duty cycle of 75%.                   *
*        's' button should show counters countent but gate_action_task       *
*        when active shows counters content automatically so 's' button isn't*
*        useful.                                                             *
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

#include <kernel/kern.h>

#include <drivers/shark_keyb26.h>
#include <drivers/shark_fb26.h>

#include <drivers/pci6025e/timer.h>
#include <drivers/pci6025e/dio_ppi.h>

#define  SHOW_MET         30000
#define  SHOW_WCET        50000
#define  SHOW_PERIOD    1000000

#define  GATE_MET          2000
#define  GATE_WCET         5000
#define  GATE_PERIOD    2000000

#define  GATE_ACTION_P   500000
#define  GATE_ACTION_W    10000

#define  black          rgb16(0, 0, 0)

void endFun(KEY_EVT *);
void gateEvent(KEY_EVT *);
void showEvent(KEY_EVT *);

void closeEvent(void *);

void drawInterface(void);

TASK show_val_body(int);
TASK gate_action_body(int);

BYTE     sys = 0;
PID      show_val_pid, gate_action_pid;

int main(int argc, char **argv)
{
   KEY_EVT k;
   SOFT_TASK_MODEL show_val_mod;
   HARD_TASK_MODEL gate_action_model;

   k.flag  =  CNTL_BIT;
   k.scan  =  KEY_C;
   k.ascii =  'c';
   k.status = KEY_PRESSED;
   keyb_hook(k, endFun, FALSE);

   k.flag  =  CNTR_BIT;
   k.status = KEY_PRESSED;
   keyb_hook(k, endFun, FALSE);

   k.flag  =  0;
   k.scan  =  KEY_G;
   k.ascii =  'g';
   k.status = KEY_PRESSED;
   keyb_hook(k, gateEvent, FALSE);

   k.scan  =  KEY_S;
   k.ascii =  's';
   k.status = KEY_PRESSED;
   keyb_hook(k, showEvent, FALSE);

   if(!reMap()){
      sys = 11;
      exit(1);
   }

   soft_task_default_model(show_val_mod);
   soft_task_def_aperiodic(show_val_mod);
   soft_task_def_level(show_val_mod, 2);
   soft_task_def_met(show_val_mod, SHOW_MET);
   soft_task_def_wcet(show_val_mod, SHOW_WCET);
   soft_task_def_period(show_val_mod, SHOW_PERIOD);
   if( (show_val_pid = task_create("Show task", show_val_body, &show_val_mod, NULL))
            == NIL ){
      sys = 20;
      exit(1);
   }

   hard_task_default_model(gate_action_model);
   hard_task_def_mit(gate_action_model, GATE_ACTION_P);
   hard_task_def_wcet(gate_action_model, GATE_ACTION_W);
   if( (gate_action_pid = task_create("Gate Action", gate_action_body, &gate_action_model, NULL))
            == NIL ){
      sys = 22;
      exit(1);
   }

   drawInterface();

   DIO_init();
   DIO_setup(0xFF);
   DIO_write(0x00);

	//All PFI configured as input
   PFIprogramming(0x0000);

	//Fout provide 1MHz: timebase = 20MHz; divided by 2; on FOUT pin also divided by 10
   setIntClock(0, 1, 10); 

	//Reset both counters
   TIM_reset(2);

   //C0 measures period on PFI3 with gate from PFI 4 driven by DIO7
   TIM_bufferedTimeMeasurement(C0, 0x04, 0x45, 0, 0);

   //C1 measures semiperiod on PFI 6 with gate from PFI 5 driven by FIO6
   TIM_bufferedTimeMeasurement(C1, 0x87, 0x46, 1, 0);

	//arm both counters
   TIM_arm(2);

   return 0;
}

void endFun(KEY_EVT *k)
{
   closeEvent(NULL);

   exit(1);
}

void showEvent(KEY_EVT *k)
{
   task_activate(show_val_pid);
}

void gateEvent(KEY_EVT *k)
{
   task_activate(gate_action_pid);
}

void drawInterface(void)
{
   grx_rect(1, 1, 799, 129, rgb16(105, 0, 0));
   grx_rect(2, 2, 798, 128, rgb16(155, 0, 0));
   grx_rect(3, 3, 797, 127, rgb16(205, 0, 0));
   grx_rect(4, 4, 796, 126, rgb16(255, 0, 0));

   grx_text("Test program for Buffered Period and Semiperiod measure through PCI6025E timers",
            7, 10, rgb16(50, 255, 50), black);

   grx_text("This program counting rise edges on counters source (PFI3 & PFI6) between two rising",
             7, 25, rgb16(0, 255, 255), black);
   grx_text("edges on gate (PFI 42) and beetwen each gate edge (PFI44).FOUT is enabled and",
             7, 33, rgb16(0, 255, 255), black);
   grx_text("provides a frequency of 1 MHz", 7, 41, rgb16(0, 255, 255), black);
   grx_text("Instruction:",7, 53, rgb16(255, 0, 0), black);
   grx_text("Use 's' to see counters value",
            7, 61, rgb16(0, 255, 255), black);
   grx_text("Use 'g' to enbale automatic tasks which generate square wave with freq of 0.5Hz",
             7, 68, rgb16(0, 255, 255), black);

   grx_text("And duty cycle of 75%. Counter 0 must be loaded with about 2E6 ticks and counter 1",
             7, 75, rgb16(0, 255, 255), black);
   grx_text("must be loaded alternativly with about 1.5E6 and 0.5E6 ticks",
             7, 83, rgb16(0, 255, 255), black);

   grx_text("Please connect DIO7 (pin 32) to PFI4 (pin 42) and DIO6 (pin 30) to PFI5 (pin 44)",
             7, 95, rgb16(0, 255, 0), black);
   grx_text("CTRL-C for Exit", 7, 110, rgb16(200, 200, 0), black);

   grx_rect(1, 147, 355, 183, rgb16(0, 105, 0));
   grx_rect(2, 148, 354, 182, rgb16(0, 155, 0));
   grx_rect(3, 149, 353, 181, rgb16(0, 205, 0));
   grx_rect(4, 150, 352, 180, rgb16(0, 255, 0));
   grx_text("Period", 7, 155, rgb16(255, 255, 0), black);

   grx_rect(455, 147, 799, 183, rgb16(0, 105, 0));
   grx_rect(456, 148, 798, 182, rgb16(0, 155, 0));
   grx_rect(457, 149, 797, 181, rgb16(0, 205, 0));
   grx_rect(458, 150, 796, 180, rgb16(0, 255, 0));
   grx_text("Semiperiod", 461, 155, rgb16(255, 0, 255), black);
}

TASK show_val_body(int dummy)
{
   DWORD val;
   char buf[40];

   while(1){
      val = TIM_readHWSaveReg(C0);
      sprintf(buf,"C0 %07ld", val);
      grx_text(buf, 7, 165, rgb16(255, 0, 0), rgb16(0, 0, 0));
      val = TIM_readHWSaveReg(C1);
      sprintf(buf,"C1 %07ld", val);
      grx_text(buf, 461, 165, rgb16(0, 255, 0), rgb16(0, 0, 0));

      task_endcycle();
   }
}

TASK gate_action_body(int dummy)
{
   int i;
   i = 0;

   while(1){
      if( (i%4)==0 )    DIO_write(0xC0);
      if( (i%4)==3 )    DIO_write(0x00);

      i++;

      task_activate(show_val_pid);

      task_endcycle();
   }
}

void closeEvent(void *arg)
{
   TIM_disarm(2);
   switch(sys){
      case  0:  sys_shutdown_message("Ok\n"); break;
      case 10:  sys_shutdown_message("No PCI\n"); break;
      case 11:  sys_shutdown_message("No National Board\n"); break;
      case 20:  sys_shutdown_message("task <show val> down\n"); break;
      case 22:  sys_shutdown_message("task <gate action> down\n"); break;
      case 30:  sys_shutdown_message("Cannot initialize grx\n"); break;
      case 31:  sys_shutdown_message("Resolution 800x600x16 not supported\n"); break;
      case 32:  sys_shutdown_message("Cannot sets up graphic envirorment\n"); break;
      default:  sys_shutdown_message("????????????\n"); break;
   }
}
