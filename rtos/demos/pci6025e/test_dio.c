/*****************************************************************************
* Filename:    Test_dio.c                                                    *
* Author:      Marco Ziglioli (Doctor Stein)                                 *
* Date:        22/03/2001                                                    *
* Last update: 22/03/2001                                                    *
* Description: Test STC digital lines (8 lines)                              *
*----------------------------------------------------------------------------*
* Notes:       Configure DIO 4 5 6 7 in input and DIO 0 1 2 3 in output      *
*              Two way to test this 8 lines:                                 *
*              1) Connect 4 LEDs to output lines and check LEDs lights       *
*                 themselves in counting order. Connect input lines to Vcc   *
*                 or GND and check on video that STC has readed the right    *
*                 nibble                                                     *
*              2) Connect 4 output lines with 4 input lines and check on     *
*                 video that the nibble readed by STC change in counting     *
*                 order                                                      *
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

#include <drivers/pci6025e/dio_ppi.h>

BYTE system = 0;

void close_event(void *);
TASK test_DIO(int dummy);
void exit_fun(KEY_EVT *);
void draw_screen(void);

int main(int argc, char **argv)
{
   HARD_TASK_MODEL   m;
   KEY_EVT k;
   PID pid_m;

   k.flag = CNTL_BIT;
   k.scan = KEY_C;
   k.ascii = 'c';
   k.status = KEY_PRESSED;
   keyb_hook(k, exit_fun, FALSE);
   k.flag = CNTR_BIT;
   k.scan = KEY_C;
   k.ascii = 'c';
   k.status = KEY_PRESSED;
   keyb_hook(k, exit_fun, FALSE);

   hard_task_default_model(m);
   hard_task_def_wcet(m, 90000);
   hard_task_def_mit(m, 250000);
   pid_m = task_create("TEST DIO", test_DIO, &m, NULL);
   if(pid_m == NIL){
      system = 10;
      exit(1);
   }

   //Look for a National board on PCI bus
   if(!reMap()){
      system = 21;
      exit(1);
   }

   //Start configuring DIO module
   DIO_init();

   draw_screen();

   task_activate(pid_m);

   return 0;
}

void draw_screen(void)
{
   grx_rect(5, 5, 405, 130, rgb16(255, 0, 0));
   grx_rect(4, 4, 406, 131, rgb16(0,255,255));
   grx_rect(3, 3, 407, 132, rgb16(255, 0, 0));
   grx_rect(15, 45, 195, 100, rgb16(255, 0, 0));
   grx_rect(215, 45, 395, 100, rgb16(0, 255, 0));
   grx_text("Test of PCI6025E DIO function",
            9, 7, rgb16(0, 255, 0), rgb16(0, 0, 0));
   grx_text("DIO 4-5-6-7 are configured in input mode",
            9, 20, rgb16(255, 70, 70), rgb16(0, 0, 0));
   grx_text("DIO 0-1-2-3 are configured in output mode",
            9, 28, rgb16(255, 70, 70), rgb16(0, 0, 0));
   grx_text("Output bits", 35, 50, rgb16(200, 0, 0), rgb16(0, 0, 0));
   grx_text("input bits", 235, 50, rgb16(30,255,30), rgb16(0, 0, 0));
   grx_text("CTRL + C to exit", 9, 115, rgb16(255,255,0), rgb16(0,0,0));
}

/*
* At each activation this task change output value of lowest digitals line
* and read value on highest digital lines, showing them at video
*/
TASK test_DIO(int dummy)
{
   BYTE  out_val = 0x00,
         in_val = 0;
   int i;
   char buf[10];

	//DIO 0..3 configured as output
	//DIO 4..7 configured as input
   DIO_setup(0x0F);

   while(1){
      DIO_write(out_val);	//sends out value
      in_val = DIO_read() >> 4;	//reads value

      for(i=3; i>=0; i--){
         if( (out_val>>i)%2 )
            grx_text("1", 25+10*(3-i), 75, rgb16(255,0,0), rgb16(0,0,0));
         else
            grx_text("0", 25+10*(3-i), 75, rgb16(255,0,0), rgb16(0,0,0));
         if( (in_val>>i)%2 )
            grx_text("1", 225+10*(3-i), 75, rgb16(0,255,0), rgb16(0,0,0));
         else
            grx_text("0", 225+10*(3-i), 75, rgb16(0,255,0), rgb16(0,0,0));
      }
      sprintf(buf, "%03d", out_val);
      grx_text(buf, 80, 75, rgb16(255,0,0), rgb16(0,0,0));
      sprintf(buf, "%03d", in_val);
      grx_text(buf, 280, 75, rgb16(0,255,0), rgb16(0,0,0));

      out_val = (out_val+1)%16;
      task_endcycle();
   }
}

void close_event(void *arg)
{
   switch(system){
      case 0: sys_shutdown_message("Regular end\n"); break;
      case 10:sys_shutdown_message("Cannot create task TEST DIO\n"); break;
      case 21:sys_shutdown_message("No National board on PC\n"); break;
      case 30:sys_shutdown_message("Cannot start graphic envirorment\n"); break;
      default: sys_shutdown_message("Unknown exit\n"); break;
   }
}

void exit_fun(KEY_EVT *k)
{
   system = 0;
   
   close_event(NULL);

   exit(1);
}

