/*****************************************************************************
* Filename: test_mes.c                                                       *
* Author: Marco Ziglioli (Doctor Stein)                                      *
* Date: 25/06/2001                                                           *
* Description: Test file for single period and pulsewidth measurement feature*
*              of PCI6025E timers/counters                                   *
*----------------------------------------------------------------------------*
* Notes: board is configured to provide a frequency of 1 MHZ through its FOUT*
*        pin (50). PFI3 (41) and PFI6 (45) are configured like source pins   *
*        counter 0 and counter 1. PFI4 (42) and PFI 5(44) are the gates for  *
*        the two counters. Please connect DIO7(32) to PFI4 and DIO6(30) to   *
*        PFI 5.                                                              *
*        With 'g' key a task is started which generate a square wave on DIO7 *
*        with a period of 2 secs. With 'h' key the same square wave is       *
*        generated on DIO6.                                                  *
*        When measurement is performed data are stored into Hardware Save    *
*        Registers: use key 's' to show this values                          *
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

#define TASK_MET  20000
#define TASK_WCET 50000
#define TASK_PERIOD 100000

#define PERIOD 1000000

void endFun(KEY_EVT *);
void gateEvent(KEY_EVT *);

void drawInterface(void);
void closeEvent(void *);

TASK show_body(int);
TASK gate_body(int);

BYTE sys = 0;
PID c0_gate_pid, c1_gate_pid, show_pid;
int black = rgb16(0, 0, 0);

int main(int argc, char **argv)
{
   KEY_EVT k;
   SOFT_TASK_MODEL show_model;
   HARD_TASK_MODEL gating;

   k.flag = CNTL_BIT;
   k.scan = KEY_C;
   k.ascii = 'c';
   k.status = KEY_PRESSED;
   keyb_hook(k, endFun, FALSE);

   k.flag = CNTR_BIT;
   k.status = KEY_PRESSED;
   keyb_hook(k, endFun, FALSE);

   k.flag = 0;
   k.scan = KEY_G;
   k.ascii = 'g';
   k.status = KEY_PRESSED;
   keyb_hook(k, gateEvent, FALSE);

   k.scan = KEY_H;
   k.ascii = 'h';
   k.status = KEY_PRESSED;
   keyb_hook(k, gateEvent, FALSE);

   k.scan = KEY_S;
   k.ascii = 's';
   k.status = KEY_PRESSED;
   keyb_hook(k, gateEvent, FALSE);

   if(!reMap()){
      sys = 11;
      exit(1);
   }

   soft_task_default_model(show_model);
   soft_task_def_level(show_model, 2);
   soft_task_def_met(show_model, TASK_MET);
   soft_task_def_wcet(show_model, TASK_WCET);
   soft_task_def_aperiodic(show_model);
   soft_task_def_period(show_model, TASK_PERIOD);
   if( (show_pid = task_create("Show task", show_body, &show_model, NULL))
         == NIL ){
      sys = 20;
      exit(1);
   }

   hard_task_default_model(gating);
   hard_task_def_mit(gating, PERIOD);
   hard_task_def_wcet(gating, 20000);
   hard_task_def_arg(gating, C0);
   if( (c0_gate_pid = task_create("C0 Gate task", gate_body, &gating, NULL)) == NIL ){
      sys = 21;
      exit(1);
   }

   hard_task_def_arg(gating, (void *)C1);
   if( (c1_gate_pid = task_create("C1 Gate task", gate_body, &gating, NULL)) == NIL ){
      sys = 22;
      exit(1);
   }

   drawInterface();
	//Enable DIO to manage gates
   DIO_init();
   DIO_setup(0xFF);
   DIO_write(0x00);

	//All PFI configured as input
   PFIprogramming(0x0000);
	//Fout provide 1MHz: timebase = 20MHz; divided by 2; on FOUT pin also divided by 10
   setIntClock(0, 1, 10);

	//Reset both counters
   TIM_reset(2);
	
	//Source PFI3(41); Gate PFI 4(42); Measure period
   TIM_timeMeasurement(C0, 0x04, 0x45, 0, 0x00, 0);
   
	//Source PFI6(45); Gate PFI 5(44); Measure pulsewidth
   TIM_timeMeasurement(C1, 0x87, 0x46, 1, 0x00, 0);

	//Arm both counter
   TIM_arm(2);

   return 0;
}

TASK show_body(int dummy)
{
   DWORD val;
   char buf[20];

   while(1){
      val = TIM_readHWSaveReg(C0);
      sprintf(buf, "%ld", val);
      grx_text(buf, 600, 471, rgb16(0, 255, 0), black);
      val = TIM_readHWSaveReg(C1);
      sprintf(buf, "%ld", val);
      grx_text(buf, 200, 471, rgb16(255, 0, 0), black);

      task_endcycle();
   }
}

TASK gate_body(int counter)
{
   BYTE out;

   if(counter == C0)    out = 0x80;
   else                 out = 0x40;

   while(1){
      DIO_write(out);
      if(counter == C0){
         if(out)  out = 0x00;
         else     out = 0x80;
      } else {
         if(out)  out = 0x40;
         else     out = 0x00;
      }

      task_endcycle();
   }
}

void drawInterface(void)
{
   grx_rect(1, 1, 799, 120, rgb16(105, 0, 0));
   grx_rect(2, 2, 798, 119, rgb16(155, 0, 0));
   grx_rect(3, 3, 797, 118, rgb16(205, 0, 0));
   grx_rect(4, 4, 796, 117, rgb16(255, 0, 0));

   grx_text("Test program for single period and pulsewidth measurement features",
            7, 10, rgb16(200, 200, 0), black);

   grx_text("This program measures single period and pulsewidth of a square wave",
            7, 20, rgb16(0, 255, 0), black);
   grx_text("with frequency of 0.5 Hz", 7, 28, rgb16(0, 255, 0), black);
   grx_text("Please connect PFI3 & PFI6 (41 & 45) to FOUT pin (50) or to 1Mhz frequency source",
            7, 40, rgb16(255, 0, 0), black);
   grx_text("Connect also DIO7 (32) to PFI4 (42) and DIO6 to PFI5 (44)",
            7, 48, rgb16(255, 0, 0), black);
   grx_text("Commands:", 7, 60, rgb16(0, 120, 0), black);
   grx_text("Use 'g' to start wave generation on DIO7",
            7, 70, rgb16(0, 255, 255), black);
   grx_text("Use 'h' to start wave generation on DIO6",
            7, 78, rgb16(0, 255, 255), black);
   grx_text("Use 's' to show Hardware Save Registers content",
            7, 86, rgb16(0, 255, 255), black);
   grx_text("CTRL-C to exit", 7, 105, rgb16(255, 255, 0), black);

   grx_rect(197, 127, 603, 423, rgb16(0, 255, 0));
   grx_rect(198, 128, 602, 422, rgb16(0, 205, 0));
   grx_rect(199, 129, 601, 421, rgb16(0, 155, 0));
   grx_rect(200, 130, 600, 420, rgb16(0, 105, 0));
   grx_line(215, 405, 215, 150, rgb16(255, 255, 255));
   grx_line(210, 400, 580, 400, rgb16(255, 255, 255));
   grx_line(220, 395, 220, 170, rgb16(0, 0, 255));
   grx_line(220, 170, 390, 170, rgb16(0, 0, 255));
   grx_line(390, 170, 390, 395, rgb16(0, 0, 255));
   grx_line(390, 395, 560, 395, rgb16(0, 0, 255));
   grx_line(220, 150, 560, 150, rgb16(0, 255, 0));
   grx_line(220, 155, 390, 155, rgb16(255, 0, 0));

   grx_rect(1, 450, 390, 503, rgb16(105, 0, 0));
   grx_rect(2, 451, 389, 502, rgb16(155, 0, 0));
   grx_rect(3, 452, 388, 501, rgb16(205, 0, 0));
   grx_rect(4, 453, 387, 500, rgb16(255, 0, 0));
   grx_text("Pulsewidth: [us]", 7, 471, rgb16(255, 0, 0), black);

   grx_rect(410, 450, 799, 503, rgb16(105, 0, 0));
   grx_rect(411, 451, 798, 502, rgb16(155, 0, 0));
   grx_rect(412, 452, 797, 501, rgb16(205, 0, 0));
   grx_rect(413, 453, 796, 500, rgb16(255, 0, 0));
   grx_text("Period: [us]", 416, 471, rgb16(0, 255, 0), black);
}

void endFun(KEY_EVT *k)
{
   closeEvent(NULL);
  
   exit(1);
}

void gateEvent(KEY_EVT *k)
{
   if(k->scan == KEY_G)    task_activate(c0_gate_pid);
   if(k->scan == KEY_H)    task_activate(c1_gate_pid);
   if(k->scan == KEY_S)    task_activate(show_pid);
}

void closeEvent(void *arg)
{
   TIM_disarm(2);

   switch(sys){
      case 0:  sys_shutdown_message("OK\n"); break;
      case 10: sys_shutdown_message("No PCI bus found\n"); break;
      case 11: sys_shutdown_message("No NATIONAL board found\n"); break;
      case 20: sys_shutdown_message("task <show value> down!!\n"); break;
      case 21: sys_shutdown_message("task <C0 gate manage> down!!!\n"); break;
      case 22: sys_shutdown_message("task <C1 gate manage> down!!!\n"); break;
      case 30: sys_shutdown_message("Cannot init graphic envirorment\n"); break;
      case 31: sys_shutdown_message("graphic mode 800x600x16 not supported\n"); break;
      default: sys_shutdown_message("???????????????\n"); break;
   }
}
