/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
 * Copyright (C) 2003 Giacomo Guidi
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
 */

#include "kernel/kern.h"
#include "stdlib.h"
#include "unistd.h"

#include "drivers/shark_keyb26.h"
#include "drivers/shark_videodev26.h"
#include "drivers/shark_fb26.h"

#include "tracer.h"

#define WIDTH 640
#define HEIGHT 480
#define BYTES_PP 2

//#define COLOR
//#define MAX_SPEED

#define FRAME_GRABBER_NUMBER 0

#define FG_PERIOD 40000 //ONLY IF MAX_SPEED IS DISABLED
#define FG_WCET 30000
#define FG_W 320		
#define FG_H 240

unsigned int frame_number = 0;

void program_end(void *arg)
{

  exit(0);
  
}

void elaborate_image(void *imageptr);

void start_frame_grabber(PID elaborate_task_PID, int channel, struct video_buffer *fbuf) {

  struct video_window win;
  struct video_channel chan;
  struct video_tuner tuner;
  struct video_picture vpic;
  int res;

  /* Init videodev driver */
  VIDEODEV26_open(FRAME_GRABBER_NUMBER);

  /* Select the input channel */
  res = VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCGCHAN,(unsigned long)&chan);
  //cprintf("res = %d\n",res);

  chan.channel = channel;
  chan.type = VIDEO_VC_TUNER; 
  chan.norm = VIDEO_TYPE_CAMERA;
                                                                                                                             
  res = VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCSCHAN,(unsigned long)&chan);
  //cprintf("res = %d\n",res);

  /* Enable the tuner */

  tuner.tuner = 0;
  tuner.mode = VIDEO_MODE_PAL;

  res = VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCSTUNER,(unsigned long)&tuner);
  //cprintf("res = %d\n",res);

  /* Select palette and depth */
  res = VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCGPICT,(unsigned long)&vpic);
  //cprintf("res = %d\n",res);  

  #ifdef COLOR
	vpic.palette = VIDEO_PALETTE_RGB24;
  	vpic.depth = 24;
  #else
	vpic.palette = VIDEO_PALETTE_GREY;
        vpic.depth = 8;
  #endif

  vpic.brightness = 35000;
  vpic.hue = 32000;
  vpic.contrast = 32000;
  vpic.colour = 32000;                           
                                                                                                 
  res = VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCSPICT,(unsigned long)&vpic);
  //cprintf("res = %d\n",res);

  res = VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCGWIN,(unsigned long)&win);
  //cprintf("res = %d\n",res);
 
  win.x = 0;
  win.y = 0;
  win.width = FG_W;
  win.height = FG_H;

  res = VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCSWIN,(unsigned long)&win);
  //cprintf("res = %d\n",res);

  /* Set the buffer */
  res = VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCSFBUF,(unsigned long)(fbuf));
  //cprintf("res = %d\n",res);

  /* IMPORTANT: Set the aperiodic elaboration task
   * This is a SHARK change on VIDIOCSYNC. When the
   * new frame is ready, the task elaborate_task_PID
   * is activated. Elabortate_task must be aperiodic !!
   * To link the task to BTTV use this function: */

  VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCSYNC,(unsigned long)(elaborate_task_PID));

}

/* Check if the frame grabber is ready */
volatile int ready_to_grab = 1;

void grab_command(void *arg) {

  SYS_FLAGS f;

  f = kern_fsave();

  int on = 1;
  VIDEODEV26_ioctl(FRAME_GRABBER_NUMBER,VIDIOCCAPTURE,(unsigned long)&on);

  kern_frestore(f);

}

/* Elaboration task, it is called when the frame 
   grabber buffer is ready */
TASK elaborate_task(void *arg) {

  struct video_buffer *fbuf = (struct video_buffer *)(arg);
  char txt[100];

  while(1) {

    elaborate_image(fbuf->base);
    //cprintf("E");
 
    //Text version
    //printf_xy(1,20,WHITE,"%08x",
    //    *(unsigned int *)(fbuf->base+50*320*3+50*3));

    frame_number++;
    sprintf(txt,"Elapsed Time = %u us Frame = %u",(int)(sys_gettime(NULL)),frame_number);
    grx_text(txt, 10, 250, rgb16(255,255,255), 0);    

    ready_to_grab = 1;

    #ifdef MAX_SPEED
      grab_command(NULL);
    #endif

    task_testcancel();
    task_endcycle();

  }

  return NULL;

}

/* Send the grab command */
TASK grab_task(void *arg) {

  while(1) {

    if (ready_to_grab) {
      
      /* Grab */
      grab_command(NULL);

      //cprintf("G");

      ready_to_grab = 0;

    } else {

      //cprintf("S");
      /* Frame skipped */

    }

    task_testcancel();
    task_endcycle();
                                                                                                                             
  }
                                                                                                                             
  return NULL;

}

extern void *video_memory;

void elaborate_image(void *imageptr)
{

  WORD x,y;
  BYTE *col;

  #ifdef COLOR

    for(y = 0; y < FG_H; y++) 
      for(x = 0; x < FG_W; x++) {

	col = (BYTE *)(imageptr + y * FG_W * 3 + x * 3);
	*(WORD *)(video_memory + y*(WIDTH*2) + (x*2)) = (WORD)rgb16(*(BYTE *)(col+2),*(BYTE *)(col+1),*(BYTE *)(col+0));

    }

  #else

    for(y = 0; y < FG_H; y++)
      for(x = 0; x < FG_W; x++) {
                                                                                                                             
        col = (BYTE *)(imageptr + y * FG_W + x);
        *(WORD *)(video_memory + y*(WIDTH*2) + (x*2)) = (WORD)rgb16(*(BYTE *)(col),*(BYTE *)(col),*(BYTE *)(col));
                                                                                                                             
    }

  #endif

}

int main(int argc, char **argv)
{

  HARD_TASK_MODEL gt;
  SOFT_TASK_MODEL et;
  PID grab_task_pid,elaborate_task_pid;

  struct video_buffer fbuf;
  int channel = 0;

  if (argc < 2) {
	sys_shutdown_message("ERROR: Enter the input channel [ex> %s 0]\n",argv[0]);
	exit(1);
  }

  channel = atoi(argv[1]);

  soft_task_default_model(et);
  soft_task_def_period(et,FG_PERIOD);
  soft_task_def_arg(et,(void *)(&fbuf));
  soft_task_def_met(et,FG_WCET);
  soft_task_def_aperiodic(et);
  soft_task_def_ctrl_jet(et);
 
  hard_task_default_model(gt);
  hard_task_def_mit(gt,FG_PERIOD);
  hard_task_def_wcet(gt,5000);
  hard_task_def_ctrl_jet(gt);
 
  grab_task_pid = task_create("GrabTask",grab_task,&gt,NULL);
  if (grab_task_pid == NIL) {
	sys_shutdown_message("ERROR: Cannot create grab task\n");
	exit(1);
  }

  elaborate_task_pid = task_create("ElaborateTask",elaborate_task,&et,NULL);
  if (grab_task_pid == NIL) {
        sys_shutdown_message("ERROR: Cannot create elaborate task\n");
        exit(1);
  }

  #ifdef COLOR

	fbuf.base = malloc(FG_W*FG_H*3);
	fbuf.height = FG_H;
	fbuf.width = FG_W;
	fbuf.bytesperline = FG_W*3;
	fbuf.depth = 24;

  #else

	fbuf.base = malloc(FG_W*FG_H);
        fbuf.height = FG_H;
        fbuf.width = FG_W;
        fbuf.bytesperline = FG_W;
        fbuf.depth = 8;

  #endif

  start_frame_grabber(elaborate_task_pid,channel,&fbuf);

  #ifndef MAX_SPEED
    /* Activate periodic grabbing */
    task_activate(grab_task_pid);
  #else
    /* Grab */
    grab_command(NULL); 
  #endif

  while(keyb_getch(BLOCK) != ESC);

  #ifndef MAX_SPEED
    task_kill(grab_task_pid);
  #endif

  sleep(1);

  exit(1);

  return 0; 

}
