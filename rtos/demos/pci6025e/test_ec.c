/*****************************************************************************
* Filename: test_ec.c                                                        *
* Author: Marco Ziglioli (Doctor Stein)                                      *
* Date: 20/06/2001                                                           *
* Description: Test program for gated event counting using PCI6025E board    *
*----------------------------------------------------------------------------*
* Notes: FOUT are enabled to provide a frequency of 6250 Hz. You could       *
*        connect PFI3 (pin 41) and PFI6 (pin 45) to this source for counting *
*        edges. Gated counting are enabled and PFI4 (pin 42) is gate pin for *
*        counter 0 and PFI5 (pin 44) is gate pin for counter 0. DIO 7 and 6  *
*        are als configured to switch between 0 and 5 V. Connect DIO 7 to    *
*        gate 0 and DIO 6 to gate 1. Use 'g' (counter 0) and 'h' (counter 1) *
*        to change DIO lines value. On left area of the screen you should    *
*        see counter while counting and on the right area you should lock    *
*        counter values by pressing 's' key.                                 *
*        Notice that line parameters are enabled and accept inital value     *
*        for the two counters. If they aren't specified or they are wrong    *
*        counters start from 0x00FFFFFF (counter 0 which counts down) and    *
*        0x00000000 (counter 1 which counts up).                             *
*        Last time addiction: TC Interrupts and Gate interrupts are enabled  *
*                             Bottom squares indicates when an interrupt is  *
*                             raised                                         *
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

#ifndef INT_NO
   #define INT_NO NIDevice_info[0].InterruptLevel
#endif

BYTE sys = 0;

PID show_aper_pid;
BYTE out = 0x00;

int black = rgb16(0, 0, 0),
    white = rgb16(255, 255, 255);

void endfun(KEY_EVT *);
void close_event(void *);
void show_evt(KEY_EVT *k);
void gate_change(KEY_EVT *k);

void drawInterface(void);

void int_evt(int intno);

TASK show_per(int);
TASK show_aper(int);

int main(int argc, char **argv)
{
   KEY_EVT k;
   SOFT_TASK_MODEL show_per_mod, show_aper_mod;
   PID show_per_pid;
   int result;
   DWORD init_val_c0, init_val_c1;

   if(argc >= 3){
      if( (result = sscanf(argv[1], "%ld", &init_val_c0)) != 1)
         init_val_c0 = 0x00FFFFFF;
      if( (result = sscanf(argv[2], "%ld", &init_val_c1)) != 1)
         init_val_c1 = 0x00000000;
   }
   if(argc == 2){
      if( (result = sscanf(argv[1], "%ld", &init_val_c0)) != 1)
         init_val_c0 = 0x00FFFFFF;
      init_val_c1 = 0x00000000;
   }
   if(argc == 1){
      init_val_c0 = 0x00FFFFFF;
      init_val_c1 = 0x00000000;
   }

   k.flag = CNTL_BIT;
   k.scan = KEY_C;
   k.ascii = 'c';
   k.status = KEY_PRESSED;
   keyb_hook(k, endfun, FALSE);

   k.flag = CNTR_BIT;
   k.status = KEY_PRESSED;
   keyb_hook(k, endfun, FALSE);

   soft_task_default_model(show_aper_mod);
   soft_task_def_aperiodic(show_aper_mod);
   soft_task_def_level(show_aper_mod, 2);
   soft_task_def_period(show_aper_mod, 250000);
   soft_task_def_met(show_aper_mod, 30000);
   soft_task_def_wcet(show_aper_mod, 60000);
   if( (show_aper_pid = task_create("Show aperiodic task", show_aper, &show_aper_mod, NULL)) == NIL ){
      sys = 10;
      exit(1);
   }

   k.flag = 0;
   k.scan = KEY_S;
   k.ascii = 's';
   k.status = KEY_PRESSED;
   keyb_hook(k, show_evt, FALSE);

   k.flag = 0;
   k.scan = KEY_G;
   k.ascii = 'g';
   k.status = KEY_PRESSED;
   keyb_hook(k, gate_change, FALSE);

   k.scan = KEY_H;
   k.ascii = 'h';
   k.status = KEY_PRESSED;
   keyb_hook(k, gate_change, FALSE);

   soft_task_default_model(show_per_mod);
   soft_task_def_level(show_per_mod, 2);
   soft_task_def_met(show_per_mod, 1000);
   soft_task_def_period(show_per_mod, 10000);
   if( (show_per_pid = task_create("Show periodic task", show_per, &show_per_mod, NULL)) == NIL){
      sys = 11;
      exit(1);
   }

   if(!reMap()){
      sys = 21;
      exit(1);
   }

   drawInterface();
	
	//Init DIO lines used to manage counters gates
   DIO_init();
   DIO_setup(0xFF);
   DIO_write(out);

	//All PFI are configured as input
   PFIprogramming(0x0000);
	//FOUT enable; Slow TIMEBASE, divided by two; divided by 16 on FOUT pin
   setIntClock(1, 1, 0);

   TIM_reset(2); //Reset both two counters

   //Source PFI3(41); Gate PFI 4(42); Down counting; counts rising edge;
   TIM_eventCounting(C0, 0x04, 0x45, 0x03, init_val_c0);

   //Source PFI6(45); Gate PFI 5(44); Up counting; counts rising edge;
   TIM_eventCounting(C1, 0x87, 0x46, 0x03, init_val_c1);

	//Set up interrupt group A and B enabling and programming to assert a request
	//both on line 2 and 3 respectively
   INT_setup(0x0A, 0x0B);
   INT_personalize(0x03);	//Interrupt request polarity low; IRQ driven on line 0 and 1 

   handler_set(INT_NO, int_evt, FALSE, show_aper_pid, NULL);

   TIM_arm(2); //Arm both two counters

   task_activate(show_per_pid);

   return 0;
}

void drawInterface(void)
{
   grx_rect(1, 1, 799, 99, rgb16(105, 0, 0));
   grx_rect(2, 2, 798, 98, rgb16(155, 0, 0));
   grx_rect(3, 3, 797, 97, rgb16(205, 0, 0));
   grx_rect(4, 4, 796, 96, rgb16(255, 0, 0));

   grx_text("Test program for Gated Event Counting capacity of PCI6025E timers",
            7, 10, rgb16(50, 255, 50), black);

   grx_text("This program counting rise edges on counters source (PFI3 & PFI6) when releted gates",
             7, 25, rgb16(0, 255, 255), black);
   grx_text("(PFI 42 & 44) are enabled. Frequency Out (FOUT) is enabled and provides a frequency of 6250 Hz",
             7, 33, rgb16(0, 255, 255), black);

   grx_text("Instruction:",7, 43, rgb16(255, 0, 0), black);
   grx_text("Use 's' to lock counters value in right squares",
            7, 51, rgb16(0, 255, 255), black);
   grx_text("Use 'g' to block or to release alternativly counter 0 (see top-left square)",
             7, 58, rgb16(0, 255, 255), black);

   grx_text("Use 'h' to block or to release alternativly counter 1 (see bottom-left square)",
             7, 65, rgb16(0, 255, 255), black);

   grx_text("Please connect DIO7 (pin 32) to PFI4 (pin 42) and DIO6 (pin 30) to PFI5 (pin 44)",
             7, 78, rgb16(0, 255, 0), black);
   grx_text("CTRL-C for Exit", 7, 88, rgb16(200, 200, 0), black);

   grx_rect(1, 110, 355, 170, rgb16(0, 105, 0));
   grx_rect(2, 111, 354, 169, rgb16(0, 155, 0));
   grx_rect(3, 112, 353, 168, rgb16(0, 205, 0));
   grx_rect(4, 113, 352, 167, rgb16(0, 255, 0));
   grx_text("Counter 0 evolution", 7, 120, rgb16(255, 255, 0), black);

   grx_rect(455, 110, 799, 170, rgb16(0, 105, 0));
   grx_rect(456, 111, 798, 169, rgb16(0, 155, 0));
   grx_rect(457, 112, 797, 168, rgb16(0, 205, 0));
   grx_rect(458, 113, 796, 167, rgb16(0, 255, 0));
   grx_text("Counter 0 locked value", 461, 120, rgb16(255, 0, 255), black);

   grx_rect(360, 110, 450, 170, rgb16(0, 105, 0));
   grx_rect(361, 111, 449, 169, rgb16(0, 155, 0));
   grx_rect(362, 112, 448, 168, rgb16(0, 205, 0));
   grx_rect(363, 113, 447, 167, rgb16(0, 255, 0));
   grx_text("Gate0", 367, 120, rgb16(200, 255, 200), black);
   grx_text("0 V", 367, 145, rgb16(255, 0, 0), black);

   grx_rect(1, 190, 355, 260, rgb16(85, 85, 255));
   grx_rect(2, 191, 354, 259, rgb16(135, 135, 255));
   grx_rect(3, 192, 353, 258, rgb16(190, 190, 255));
   grx_rect(4, 193, 352, 257, rgb16(230, 239, 255));
   grx_text("Counter 1 evolution", 7, 200, white, black);

   grx_rect(455, 190, 799, 260, rgb16(85, 85, 255));
   grx_rect(456, 191, 798, 259, rgb16(135, 135, 255));
   grx_rect(457, 192, 797, 258, rgb16(190, 190, 255));
   grx_rect(458, 193, 796, 257, rgb16(230, 230, 255));
   grx_text("Counter 1 locked value", 461, 200, white, black);

   grx_rect(360, 190, 450, 260, rgb16(85, 85, 255));
   grx_rect(361, 191, 449, 259, rgb16(135, 135, 255));
   grx_rect(362, 192, 448, 258, rgb16(190, 190, 255));
   grx_rect(363, 193, 447, 257, rgb16(230, 230, 255));
   grx_text("Gate1", 367, 200, rgb16(255, 200, 255), black);
   grx_text("0 V", 367, 225, rgb16(255, 0, 0), black);

   grx_text("Counter 0 Interrupt events", 7, 340, rgb16(255, 200, 100), black);
   grx_text("Counter 1 Interrupt events", 461, 340, rgb16(255, 200, 100), black);
   grx_rect(1, 350, 355, 400, rgb16(105, 0, 0));
   grx_rect(2, 351, 354, 399, rgb16(155, 0, 0));
   grx_rect(3, 352, 353, 398, rgb16(205, 0, 0));
   grx_rect(4, 353, 352, 397, rgb16(255, 0, 0));
   grx_rect(455, 350, 799, 400, rgb16(105, 0, 0));
   grx_rect(456, 351, 798, 399, rgb16(155, 0, 0));
   grx_rect(457, 352, 797, 398, rgb16(205, 0, 0));
   grx_rect(458, 353, 796, 397, rgb16(255, 0, 0));
}


TASK show_per(int none)
{
   DWORD val;
   char buf[30];

   while(1){
      val = TIM_readCounter(C0);	//Read counter 0 value
      sprintf(buf, "HEX: %08lx DEC: %08ld", val ,val);
      grx_text(buf, 7, 145, rgb16(255, 0, 0), black);

      val = TIM_readCounter(C1);	//Read counter 1 value
      sprintf(buf, "HEX: %08lx DEC: %08ld", val ,val);
      grx_text(buf, 7, 225, rgb16(255, 0, 0), black);

      task_endcycle();
   }
}

TASK show_aper(int dummy)
{
   DWORD val;
   char buf[30];

   while(1){
      val = TIM_readCounter(C0);
      sprintf(buf, "HEX: %08lx DEC: %08ld", val, val);
      grx_text(buf, 461, 145, rgb16(80, 80, 255), black);

      val = TIM_readCounter(C1);
      sprintf(buf, "HEX: %08lx DEC: %08ld", val, val);
      grx_text(buf, 461, 225, rgb16(80, 80, 255), black);

      task_endcycle();
   }
}

void endfun(KEY_EVT *k)
{
   close_event(NULL);

   exit(1);
}

void show_evt(KEY_EVT *k)
{
   task_activate(show_aper_pid);
}

void gate_change(KEY_EVT *k)
{
   if(k->ascii == 'g'){
      if( (out & 0x80) != 0){
         out &= 0x7F;
         grx_text("0 V", 367, 145, rgb16(255, 0, 0), black);
      } else {
         out |= 0x80;
         grx_text("5 V", 367, 145, rgb16(0, 255, 0), black);
      }
   } else {
      if( (out & 0x40) != 0){
         out &= 0xBF;
         grx_text("0 V", 367, 225, rgb16(255, 0, 0), black);
      } else {
         out |= 0x40;
         grx_text("5 V", 367, 225, rgb16(0, 255, 0), black);
      }
   }

   DIO_write(out);
}

void close_event(void *arg)
{
   TIM_disarm(2); //Disable both two counters
   handler_remove(INT_NO);

   switch(sys){
      case 0:     sys_shutdown_message("OK\n"); break;
      case 10:    sys_shutdown_message("Task <show aperiodic> down\n"); break;
      case 11:    sys_shutdown_message("Task <show periodic> down\n"); break;
      case 20:    sys_shutdown_message("No PCI bus\n"); break;
      case 21:    sys_shutdown_message("No National board on PCI bus\n"); break;
      case 30:    sys_shutdown_message("No graphic can be initialized\n"); break;
      case 31:    sys_shutdown_message("This graphic mode cannot be supported\n"); break;
      default:    sys_shutdown_message("???????????\n"); break;
   }
}

void int_evt(int intno)
{
   WORD status;

   status = DAQ_STC_Windowed_Mode_Read(AI_STATUS_1);
   if( (status & 0x8000) != 0){
      if( (status & 0x0008) != 0){
         grx_text("INT Group A raised! G0 Rolls over", 7, 360, rgb16(0, 255, 0), black);
         set(interrupt_a_ack, 14);
         DAQ_STC_Windowed_Mode_Write(INTERRUPT_A_ACK, interrupt_a_ack);
         clr(interrupt_a_ack, 14);
      }
      if( (status & 0x0004) != 0){
         grx_text("INT Group A raised! G0 gate pressed", 7, 380, rgb16(0, 255, 0), black);
         set(interrupt_a_ack, 15);
         DAQ_STC_Windowed_Mode_Write(INTERRUPT_A_ACK, interrupt_a_ack);
         clr(interrupt_a_ack, 15);
      }
      return;
   }

   status = DAQ_STC_Windowed_Mode_Read(AO_STATUS_1);
   if( (status & 0x8000) != 0){
      if( (status & 0x0008) != 0){
         grx_text("INT Group B raised! G1 Rolls over", 461, 360, rgb16(0, 255, 0), black);
         set(interrupt_b_ack, 14);
         DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ACK, interrupt_b_ack);
         clr(interrupt_b_ack, 14);
      }
      if( (status & 0x0004) != 0){
         grx_text("INT Group B raised! G1 gate pressed", 461, 380, rgb16(0, 255, 0), black);
         set(interrupt_b_ack, 15);
         DAQ_STC_Windowed_Mode_Write(INTERRUPT_B_ACK, interrupt_b_ack);
         clr(interrupt_b_ack, 15);
      }
      return;
   }
}
/* End of file: Test_ec.c */
