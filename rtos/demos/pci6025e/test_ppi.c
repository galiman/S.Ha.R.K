/*****************************************************************************
* Filename:    Test_ppi.c                                                    *
* Author:      Marco Ziglioli (Doctor Stein)                                 *
* Date:        22/05/2001                                                    *
* Description: Test PPI82C55MSM on NI DAQ PCI6025E                           *
*----------------------------------------------------------------------------*
* Notes:       Configures port A and port C in input and port B in output    *
*              Test 1:                                                       *
*              Now connect port B to port A and watch on video value changes *
*              in counting order. After connect port B to port C and watch   *
*              the same events on port C                                     *
*              Test 2:                                                       *
*              Connect 8 LEDs on port B and port A and port C lines to Vcc or*
*              GND. Now run this test and watch 8 diode changing in counting *
*              mode and on screen port A and port C values displayed         *
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



#include <drivers/shark_keyb26.h>
#include <drivers/shark_fb26.h>

#include <drivers/pci6025e/dio_ppi.h>

void exit_fun(KEY_EVT *);
TASK test_ppi(int);
void close_event(void *);
void draw_screen(void);

BYTE system = 0;

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
   pid_m = task_create("TEST PPI", test_ppi, &m, NULL);
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
   PPI_init();

   draw_screen();

   task_activate(pid_m);

   return 0;
}

void draw_screen(void)
{
   grx_rect(5, 5, 610, 130, rgb16(255, 0, 0));
   grx_rect(4, 4, 611, 131, rgb16(0,255,255));
   grx_rect(3, 3, 612, 132, rgb16(255, 0, 0));
   grx_rect(15, 45, 195, 100, rgb16(255, 0, 0));
   grx_rect(215, 45, 395, 100, rgb16(0, 255, 0));
   grx_rect(415, 45, 595, 100, rgb16(200, 200, 255));
   grx_text("Test of PPI82C55MSM function (Hosted on PCI6025E)",
            9, 7, rgb16(0, 255, 0), rgb16(0, 0, 0));
   grx_text("PORT A and PORT C are configured in input mode",
            9, 20, rgb16(255, 70, 70), rgb16(0, 0, 0));
   grx_text("PORT B is configured in output mode",
            9, 28, rgb16(255, 70, 70), rgb16(0, 0, 0));
   grx_text("PORT B (Output)", 35, 50, rgb16(200, 0, 0), rgb16(0, 0, 0));
   grx_text("PORT A (Input)", 235, 50, rgb16(30,255,30), rgb16(0, 0, 0));
   grx_text("PORT C (Input)", 435, 50, rgb16(200,200,255), rgb16(0, 0, 0));
   grx_text("CTRL + C to exit", 9, 115, rgb16(255,255,0), rgb16(0,0,0));
}

/*
* At each activation this task sends out value on port B and reads values from
* port A and C
*/
TASK test_ppi(int dummy)
{
   BYTE val, pA, pC;
   int i;
   char buf[10];

   PPI_config(0x99);    //Mode 0 for all; Port A, Port C input; Port B output
   val = 0;
   while(1){
      PPI_write(PPI_PORT_B, val);	//sends out value
      pA = PPI_read(PPI_PORT_A);	//reads from port A
      pC = PPI_read(PPI_PORT_C);	//reads from port C

      for(i=7; i>=0; i--){
         if( (val>>i)%2 )
            grx_text("1", 25+10*(7-i), 75, rgb16(255,0,0), rgb16(0,0,0));
         else
            grx_text("0", 25+10*(7-i), 75, rgb16(255,0,0), rgb16(0,0,0));
         if( (pA>>i)%2 )
            grx_text("1", 225+10*(7-i), 75, rgb16(0,255,0), rgb16(0,0,0));
         else
            grx_text("0", 225+10*(7-i), 75, rgb16(0,255,0), rgb16(0,0,0));
         if( (pC>>i)%2 )
            grx_text("1", 425+10*(7-i), 75, rgb16(200,200,255), rgb16(0,0,0));
         else
            grx_text("0", 425+10*(7-i), 75, rgb16(200,200,255), rgb16(0,0,0));
      }
      sprintf(buf, "%03d", val);
      grx_text(buf, 140, 75, rgb16(255,0,0), rgb16(0,0,0));
      sprintf(buf, "%03d", pA);
      grx_text(buf, 340, 75, rgb16(0,255,0), rgb16(0,0,0));
      sprintf(buf, "%03d", pC);
      grx_text(buf, 540, 75, rgb16(200,200,255), rgb16(0,0,0));
      val++;
      task_endcycle();
   }
}

void close_event(void *arg)
{
   switch(system){
      case 0: sys_shutdown_message("Regular end\n"); break;
      case 10:sys_shutdown_message("Cannot create task 'TEST PPI'\n"); break;
      case 20:sys_shutdown_message("Pci bus don't find\n"); break;
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

/*end of file: test_ppi.c*/
