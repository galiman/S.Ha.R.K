/*****************************************************************************
* Filename:       test_dac.c                                                   * 
* Author:         Marco Ziglioli (Doctor Stein)                              *
* Date:           12/06/2001                                                 *
* Description:    Little test program for Analog Output section of PCI6025E  *
*----------------------------------------------------------------------------*
* Notes:          Connect a multimeter to DAC1 output (pin 21) and watch     *
*                 tension value. Use '+' and '-' on numeric pad to increase  *
*                 and decrise voltage                                        *
*                 With this program it's possible to point out possible      *
*                 offset errors. To correct them no software are written but *
*                 in National board package there's the program to calibrate *
*                 the board. Otherwise you can see how much is the offset    *
*                 and you can compensate it through software value           *
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

#include <drivers/shark_fb26.h>
#include <drivers/shark_keyb26.h>

#include <drivers/pci6025e/dac.h>

#define  TASK_VOLT_PERIOD     150E3
#define  TASK_VOLT_WCET       080E3
#define  TASK_DAC_PERIOD      050E3
#define  TASK_DAC_WCET        020E3

#define  CONV                 10/2048

#define  TASK_GROUP           1

WORD  dac0_value = 0,
      dac1_value = 0;

BYTE  sys = 0;
BYTE  dac = 0;

//some colors
int   black = rgb16(0, 0, 0),
      white = rgb16(255, 255, 255);

void drawInterface(void);
void endfun(KEY_EVT *);
void inc(KEY_EVT *);
void dec(KEY_EVT *);
void change_dac(KEY_EVT *);
void close_event(void *);
TASK Voltage_body(int);
TASK DAC_Check_body(int);

int main(int argc, char **argv)
{
   KEY_EVT k;
   HARD_TASK_MODEL m, d;
   PID pid_m, pid_d;

   k.flag  =  CNTR_BIT;
   k.scan  =  KEY_C;
   k.ascii =  'c';
   k.status = KEY_PRESSED;
   keyb_hook(k, endfun, FALSE);

   k.flag  =  CNTL_BIT;
   k.status = KEY_PRESSED;
   keyb_hook(k, endfun, FALSE);

   k.flag  =  0;
   k.scan  =  78;
   k.ascii =  43;
   k.status = KEY_PRESSED;
   keyb_hook(k, inc, FALSE);

   k.flag  =  0;
   k.scan  =  74;
   k.ascii =  45;
   k.status = KEY_PRESSED;
   keyb_hook(k, dec, FALSE);

   k.flag  =  0;
   k.scan  =  KEY_V;
   k.ascii =  'v';
   k.status = KEY_PRESSED;
   keyb_hook(k, change_dac, FALSE);

   hard_task_default_model(m);
   hard_task_def_wcet(m, TASK_VOLT_WCET);
   hard_task_def_mit(m, TASK_VOLT_PERIOD);
   hard_task_def_group(m, TASK_GROUP);
   pid_m = task_create("Voltage", Voltage_body, &m, NULL);
   if(pid_m == NIL){
      sys = 30;
      exit(1);
   }

   hard_task_default_model(d);
   hard_task_def_wcet(d, TASK_DAC_WCET);
   hard_task_def_mit(d, TASK_DAC_PERIOD);
   hard_task_def_group(d, TASK_GROUP);
   pid_d = task_create("DAC Check", DAC_Check_body, &d, NULL);
   if(pid_d == NIL){
      sys = 31;
      exit(1);
   }

   //Check if a NI board is on PCI bus
   if(!reMap()){
      sys = 11;
      exit(1);
   }

   drawInterface();

   //Analog output section set up
   DAC_Init();

	/*
	*AI_TIMEBASE div by 2; OUT_TIMEBASE div by 2;  single DAC mode
	*TMRDACWR = 3 OUT_TIMEBASE period; FIFO flags polarity active low
	*TMRDACWR disabled; DMA PIO control = FIFO DATA interface mode
	*UPDATE signal timebase = AO_UPDATE pulse width
	*UPDATE pulsewidth = 3-3.5 OUT_TIMEBASE period
	*UPDATE signal polarity = HIGH Z
	*/
   DAC_boardInit(0x02, 0x4000);
	/*
	*LDAC0 source = UPDATE
	*DAC0 update immediately
	*LDAC1 source = UPDATE
	*DAC1 update immediately
	*/
   DAC_LDACSourceUpdate(0x00);
	//End of Analog output section setup

   group_activate(TASK_GROUP);

   return 0;
}

/*
* Every time operator select a new value to send to a DAc this TASK makes
* actions needed to perform the operation
*/
TASK Voltage_body(int dac_work)
{
   WORD old0_value, old1_value;
   char buf[6];
   float volt;

   old0_value = dac0_value;
   old1_value = dac1_value;
   while(1){
      if(dac){
         if(dac1_value != old1_value){
            DAC_output(DAC1, dac1_value);
            old1_value = dac1_value;
            sprintf(buf, "%04d", dac1_value);
            grx_text(buf, 70, 120, rgb16(180, 0, 0), rgb16(255,255,140));
            sprintf(buf, "%04x", dac1_value);
            grx_text(buf, 300, 120, rgb16(180, 0, 0), rgb16(255,255,140));
            if( (dac1_value & 0x0800) == 0 )
               volt = (float)dac1_value * (float)CONV;
            else
               volt = (float)(dac1_value-0x0FFF) * (float)CONV;
            sprintf(buf, "%05.2f", volt);
            grx_text(buf, 70, 177, rgb16(180, 40, 180), black);
         }
      } else {
         if(dac0_value != old0_value){
            DAC_output(DAC0, dac0_value);
            old0_value = dac0_value;
            sprintf(buf, "%04d", dac0_value);
            grx_text(buf, 521, 120, rgb16(180, 0, 0), rgb16(255,255,140));
            sprintf(buf, "%04x", dac0_value);
            grx_text(buf, 754, 120, rgb16(180, 0, 0), rgb16(255,255,140));
            if( (dac0_value & 0x0800) == 0 )
               volt = (float)dac0_value * (float)CONV;
            else
               volt = (float)(dac0_value-0x0FFF) * (float)CONV;
            sprintf(buf, "%05.2f", volt);
            grx_text(buf, 521, 177, rgb16(180, 40, 180), black);
         }
      }
      task_endcycle();
   }
}

/*
* This TASK show which is the DAC active
*/
TASK DAC_Check_body(int dummy)
{
   BYTE old = dac;
   char buf[8];

   while(1){
      if(dac != old){
         old = dac;
         sprintf(buf, "DAC %d", dac);
         grx_text(buf, 385, 90, rgb16(255*dac, 255*(1-dac), 0), black);
      }
      task_endcycle();
   }
}

void drawInterface(void)
{
   grx_rect(1, 1, 799, 69, rgb16(105, 0, 0));
   grx_rect(2, 2, 798, 68, rgb16(155, 0, 0));
   grx_rect(3, 3, 797, 67, rgb16(205, 0, 0));
   grx_rect(4, 4, 796, 66, rgb16(255, 0, 0));

   grx_text("Test program for Analog output section of PCI6025E",
            7, 10, rgb16(50, 255, 50), black);
   grx_text("Use '+' and '-' on numeric pad to change tension",
            7, 25, rgb16(0, 255, 255), black);
   grx_text("Connect a tester to DAC1 output (pin21) or to DAC0 output (pin20)",
            7, 33, rgb16(0, 255, 255), black);
   grx_text("Use 'v' to alternate change active DAC",
            7, 40, rgb16(0, 255, 255), black);

   grx_text("CTRL-C for Exit", 7, 55, rgb16(200, 200, 0), black);

   grx_rect(1, 80, 355, 150, rgb16(0, 105, 0));
   grx_rect(2, 81, 354, 149, rgb16(0, 155, 0));
   grx_rect(3, 82, 353, 148, rgb16(0, 205, 0));
   grx_rect(4, 83, 352, 147, rgb16(0, 255, 0));

   grx_rect(1, 160, 355, 199, rgb16(0, 105, 0));
   grx_rect(2, 161, 354, 198, rgb16(0, 155, 0));
   grx_rect(3, 162, 353, 197, rgb16(0, 205, 0));
   grx_rect(4, 163, 352, 196, rgb16(0, 255, 0));

   grx_rect(455, 80, 799, 150, rgb16(105, 105, 0));
   grx_rect(456, 81, 798, 149, rgb16(155, 155, 0));
   grx_rect(457, 82, 797, 148, rgb16(205, 205, 0));
   grx_rect(458, 83, 796, 147, rgb16(255, 255, 0));

   grx_rect(455, 160, 799, 199, rgb16(105, 105, 0));
   grx_rect(456, 161, 798, 198, rgb16(155, 155, 0));
   grx_rect(457, 162, 797, 197, rgb16(205, 205, 0));
   grx_rect(458, 163, 796, 196, rgb16(255, 255, 0));

   grx_rect(360, 80, 450, 105, rgb16(85, 85, 255));
   grx_rect(361, 81, 449, 104, rgb16(125, 125, 255));
   grx_rect(362, 82, 448, 103, rgb16(175, 175, 255));
   grx_rect(363, 83, 447, 102, rgb16(225, 225, 255));

   grx_rect(153, 93, 195, 103, rgb16(255, 0, 0));
   grx_text("DAC 1", 155, 95, rgb16(255, 170, 170), black);

   grx_rect(607, 93, 649, 103, rgb16(255, 0, 0));
   grx_text("DAC 0", 609, 95, rgb16(255, 255, 210), black);

   grx_text("Decimal", 7, 120, rgb16(120, 120, 255), black);
   grx_text("Hexadecimal", 200, 120, rgb16(120, 120, 255), black);
   grx_text("Tension", 7, 177, rgb16(120, 120, 255), black);

   grx_text("Decimal", 461, 120, rgb16(255, 120, 120), black);
   grx_text("Hexadecimal", 654, 120, rgb16(255, 120, 120), black);
   grx_text("Tension", 461, 177, rgb16(255, 120, 120), black);
}

void close_event(void *arg)
{
   switch(sys){
      case 0 : sys_shutdown_message("Regular End\n"); break;
      case 1 : sys_shutdown_message("End fun invoked\n"); break;
      case 10: sys_shutdown_message("Pci bus not found\n"); break;
      case 11: sys_shutdown_message("No National board found\n"); break;
      case 20: sys_shutdown_message("Cannot initialize graphic envirorment\n"); break;
      case 21: sys_shutdown_message("Cannot start envirorment in 800x600x16\n"); break;
      case 30: sys_shutdown_message("Cannot create task <voltage>\n"); break;
      case 31: sys_shutdown_message("Canot create task <DAC Check>\n"); break;
      case 40: sys_shutdown_message("Break on clock end event\n"); break;
      default: sys_shutdown_message("Unkwon exit event\n"); break;
   }
}

void endfun(KEY_EVT *k)
{
   
   close_event(NULL);

   exit(1);
}

/*
* Capture correct key event and increase output tension of active DAC
*/
void inc(KEY_EVT *k)
{
   if(dac){
      if( (dac1_value & 0x0800) == 0 && dac1_value > 0x07FF )
         dac1_value = 0;
      else
         dac1_value++;
   } else {
      if( (dac0_value & 0x0800) == 0 && dac0_value > 0x07FF )
         dac0_value = 0;
      else
         dac0_value++;
   }
}

/*
* Same as above but decrease tension
*/
void dec(KEY_EVT *k)
{
   if(dac){
      if(dac1_value < 1)
         dac1_value = 0x0FFF;
      else
         dac1_value -= 1;
   } else {
      if(dac0_value < 1)
         dac0_value = 0x0FFF;
      else
         dac0_value -= 1;
   }
}

/*
* Capture correct key event and change active DAC
*/
void change_dac(KEY_EVT *k)
{
   if(dac)  dac = 0;
   else     dac = 1;
}

