/*****************************************************************************
* Filename:       wave.c                                                     * 
* Author:         Marco Ziglioli (Doctor Stein)                              *
* Date:           12/06/2001                                                 *
* Description:    Little test program for Analog Output section of PCI6025E  *
*----------------------------------------------------------------------------*
* Notes:          Connect an oscilloscope to DACs output pins (20 & 21) and  *
*                 watch the waveforms.                                       *
*                 and decrise voltage                                        *
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

#include <sem/sem/sem.h>

#include <drivers/pci6025e/dac.h>

#define  MAX_VAL         500

#define  WAVE_PERIOD    1000
#define  WAVE_WCET       200
#define  GRAPH_PERIOD   1000
#define  GRAPH_WCET      550

#define  TASK_GROUP        1

#define  DAC0_CONV       0.1
#define  DAC1_CONV      0.05

#define  INC              40

void createWaves(void);
void drawInterface(void);

void endfun(KEY_EVT *);
void close_event(void *);

TASK wave_body(int);
TASK video_body(int);

WORD wave0[MAX_VAL], wave1[MAX_VAL];
int black = rgb16(0,0,0),
    white = rgb16(255, 255, 255);

BYTE sys = 0;

int main(int argc, char **argv)
{
   KEY_EVT  k;
   HARD_TASK_MODEL   wave0, wave1;
   HARD_TASK_MODEL   video;
   PID               wave0_pid, wave1_pid, video_pid;

   k.flag  =  CNTR_BIT;
   k.scan  =  KEY_C;
   k.ascii =  'c';
   k.status = KEY_PRESSED;
   keyb_hook(k, endfun, FALSE);

   k.flag  =  CNTL_BIT;
   k.status = KEY_PRESSED;
   keyb_hook(k, endfun, FALSE);

   hard_task_default_model(wave0);
   hard_task_def_wcet(wave0, WAVE_WCET);
   hard_task_def_mit(wave0, WAVE_PERIOD);
   hard_task_def_arg(wave0, 0);
   hard_task_def_group(wave0, TASK_GROUP);
   if( (wave0_pid = task_create("Wave 0", wave_body, &wave0, NULL)) == NIL ){
      sys = 10;
      exit(1);
   }

   hard_task_default_model(wave1);
   hard_task_def_wcet(wave1, WAVE_WCET);
   hard_task_def_mit(wave1, WAVE_PERIOD);
   hard_task_def_arg(wave1, (void *)1);
   hard_task_def_group(wave1, TASK_GROUP);
   if( (wave1_pid = task_create("Wave 1", wave_body, &wave1, NULL)) == NIL ){
      sys = 11;
      exit(1);
   }

   hard_task_default_model(video);
   hard_task_def_wcet(video, GRAPH_WCET);
   hard_task_def_mit(video, GRAPH_PERIOD);
   hard_task_def_group(video, TASK_GROUP);
   if( (video_pid = task_create("Video task", video_body, &video, NULL))
           == NIL ){
      sys = 12;
      exit(1);
   }

   if(!reMap()){
      sys = 21;
      exit(1);
   }

   createWaves();
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

void endfun(KEY_EVT *k)
{
   close_event(NULL);

   exit(1);
}

void close_event(void *arg)
{
   switch(sys){
      case 0:  sys_shutdown_message("Regular End!\n"); break;
      case 10: sys_shutdown_message("Cannot create <wave 0> task!\n"); break;
      case 11: sys_shutdown_message("Cannot create <wave 1> task!\n"); break;
      case 12: sys_shutdown_message("Cannot create <video> task!\n"); break;
      case 20: sys_shutdown_message("No PCI bus found!\n"); break;
      case 21: sys_shutdown_message("No NATIONAL PCI E-Series board found on PCI bus!\n");
               break;
      case 30: sys_shutdown_message("Cannot start graphic envirorment!\n"); break;
      case 31: sys_shutdown_message("800x600x16 video mode not supported!\n");
      default: sys_shutdown_message("Unknown exit event!\n"); break;
   }
}

/*
* Wave's samples generation
*/
void createWaves(void)
{
   int i;
   int value0, value1;
   BYTE direction;

   /* Wave0
         *  *   *  *  *  *  *  *
        ** **  ** ** ** ** ** **
       * ** * * ** ** ** ** ** *
      *  *  **  *  *  *  *  *  *
      --------------------------...

      Wave 1
         *               *
        * *             * *
       *   *           *   *
      *     *         *     *
      -------*-------*-------*--...
              *     *         *
               *   *           *
                * *
                 *                 */

   value0 = 0;
   value1 = 0;
   direction = 0;
   for(i=0; i<MAX_VAL; i++){
      wave0[i] = (value0 & 0x0FFF);
      wave1[i] = (value1 & 0x0FFF);

      value0 = (value0 + INC) % 2000;
      if(!direction)   value1 += INC;
      else             value1 -= INC;

      if(value1 >= 2000)    direction = 1;
      if(value1 <= -2000)   direction = 0;
   }
}

void drawInterface(void)
{
   int i;

   grx_rect(1, 1, 799, 69, rgb16(105, 0, 105));
   grx_rect(2, 2, 798, 68, rgb16(155, 0, 155));
   grx_rect(3, 3, 797, 67, rgb16(205, 0, 205));
   grx_rect(4, 4, 796, 66, rgb16(255, 0, 255));

   grx_text("Test program for Analog output section of PCI6025E",
            7, 10, rgb16(50, 255, 50), black);
   grx_text("DAC0 and DAC1 should generate saw-toothed wave and triangular wave",
            7, 33, rgb16(0, 255, 255), black);
   grx_text("Use an oscilloscope to test this software",
            7, 40, rgb16(0, 255, 255), black);

   grx_text("CTRL-C for Exit", 7, 55, rgb16(200, 200, 0), black);

   grx_text("DAC 0", 100, 92, rgb16(200, 200, 0), black);
   grx_rect(1, 100, 799, 325, rgb16(0, 105, 0));
   grx_rect(2, 101, 798, 324, rgb16(0, 155, 0));
   grx_rect(3, 102, 797, 323, rgb16(0, 205, 0));
   grx_rect(4, 103, 796, 322, rgb16(0, 255, 0));
   grx_line(19, 115, 19, 320, white);
   grx_line(14, 315, 530, 315, white);

   grx_text("DAC 1", 100, 362, rgb16(200, 200, 0), black);
   grx_rect(1, 370, 799, 595, rgb16(105, 0, 0));
   grx_rect(2, 371, 798, 594, rgb16(155, 0, 0));
   grx_rect(3, 372, 797, 593, rgb16(205, 0, 0));
   grx_rect(4, 373, 796, 592, rgb16(255, 0, 0));
   grx_line(19, 385, 19, 585, white);
   grx_line(14, 485, 530, 485, white);

   for(i=22; i<530; i+=2){
      //DAC0
      grx_plot(i, 115, white);
      grx_plot(i, 215, white);
      //DAC1
      grx_plot(i, 385, white);
      grx_plot(i, 435, white);
      grx_plot(i, 535, white);
      grx_plot(i, 585, white);
   }

   grx_text("5 V", 540, 211, rgb16(0, 255, 0), black);
   grx_text("10 V", 540, 111, rgb16(0, 255, 0), black);
   grx_text("+5 V", 540, 431, rgb16(0, 255, 0), black);
   grx_text("+10 V", 540, 381, rgb16(0, 255, 0), black);
   grx_text("-5 V", 540, 531, rgb16(255, 0, 0), black);
   grx_text("-10 V", 540, 581, rgb16(255, 0 , 0), black);
}

/*
* Sends out waves' samples
*/
TASK wave_body(int wv)
{
   int i = 0;
   while(1){
      if(wv)
         DAC_output(DAC1, wave1[i]);
      else
         DAC_output(DAC0, wave0[i]);

      i = (i + 1) % 500;
      task_endcycle();
   }
}

/*
* Shows wave on screen
*/
TASK video_body(int dummy)
{
   int i = 0;
   int n_tmp = 0, o_tmp;
   //char buf[10];

   while(1){
      o_tmp = n_tmp;
      if( (wave1[i] & 0x0800) != 0 )   n_tmp = wave1[i]-0x0FFF;
      else                             n_tmp = wave1[i];

      if(i>0){
        grx_line(19+i, 314-wave0[i-1]*DAC0_CONV,
                 20+i, 314-wave0[i]*DAC0_CONV, rgb16(255, 255, 0));
        grx_line(19+i, 485-o_tmp*DAC1_CONV,
                 20+i, 485-n_tmp*DAC1_CONV, rgb16(0, 255, 255));
      }

      i = (i + 1) % 500;
      task_endcycle();
   }
}
