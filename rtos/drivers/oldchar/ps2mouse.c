/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: ps2mouse.c,v 1.2 2005/02/25 11:04:03 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2005/02/25 11:04:03 $
 ------------

 Author:	Massimiliano Giorgi
 Date:	2/2/98

 File: 	PS2MOUSE.C
 Revision:	1.0
 Last update: 22/mar/1999

 PS2 mouse driver

**/

/*
 * Copyright (C) 2000 Paolo Gai
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

/*#define __DEBUG_PS2MOUSE__*/

//#include <string.h>
//#include <stdlib.h>
//#include <cons.h>

//#include "vm.h"
#include <kernel/kern.h>
//#include "exc.h"
#include <hartport/hartport/hartport.h>

#include "8042.h"
#include <drivers/mouse.h>

#include "_mouse.h"
#include "ps2mouse.h"

#define WAITING 0
#define WAITX   1
#define WAITY   2
//static int status;

#ifdef __DEBUG_PS2MOUSE__

/* to print some debug information on line YDEB of the screen */
#define YDEB 2

#define change_status(x) {                            \
  status=(x);                                         \
  printf_xy(0,YDEB,WHITE,"[PS2mouse server] status: %s      ",#x); \
}

#define debug_show_data(b1,b2,b3) { \
  printf_xy(40,YDEB,WHITE,"B1: 0x%-2x ",b1);  \
  printf_xy(49,YDEB,WHITE,"B2: 0x%-2x ",b2);  \
  printf_xy(58,YDEB,WHITE,"B3: 0x%-2x ",b3);  \
}

#else

#define change_status(x) status=(x)
#define debug_show_data(b1,b2,b3)

#endif

/*
 * PS2 Mouse Server TASK
 *
 * This is the format of the incoming data.
 *
 * Data packet format:
 *
 *     |   D7      D6      D5      D4      D3      D2      D1      D0
 * -------------------------------------------------------------------
 * B1  |   XV      XV      YS      XS      1       0       R       L
 * B2  |   X7      X6      X5      X4      X3      X2      X1      X0
 * B3  |   Y7      Y6      Y5      Y4      Y3      Y2      Y1      Y0
 *
 * L       Left button state (1 = pressed down)
 * R       Right button state (1 = pressed down)
 * X0-X7   Movement in X direction
 * Y0-Y7   Movement in Y direction
 * XS,YS   Movement data sign bits (1 = negative)
 * XV,YV   Movement data overflow bits (1 = overflow has occured)
 *
 * Note:
 * -- X0-X7 and Y0-Y7 seems to be signed numbers
 * -- bit D3 in B1 seems to be the 3rd button (0 = pressed down)
 */

/*
 * the old ps/2 mouse server!
 * now the genaral mouse serve into mouse.c is used
 */

//TASK ps2mouse_server(void)
//{
//  int b1,b2,b3;
//  int mickey;
//
//  b3=b1=b2=0;
//  change_status(WAITING);
//  for (;;) {
//    wait_mouse();
//    
//    b3=get_mouse();
//
//    switch(status) {
//    case WAITING:
//      b1=b3;
//      debug_show_data(b1,b2,b3);
//      /* can only be tested if bit D3 is one ...*/
//      if ((b1&0x08)!=0x08) break;
//      change_status(WAITX);
//      break;
//    case WAITX:
//      b2=b3;
//      debug_show_data(b1,b2,b3);
//      change_status(WAITY);
//      break;
//    case WAITY:
//      debug_show_data(b1,b2,b3);
//
//      /* buttons */
//      mouse_buttons=((b1&0x1)<<1)|((b1&0x02)>>1)|(b1&0x04);
//
//      /* axes X */
//      mickey=((b1&0x10)?(b2-256):b2);
//      /* this part come from "sermouse.c" */
//      mouse_x_mick+=mickey;
//      while(mouse_x_mick>mouse_thresholdlim) {
//	mouse_x++;
//	mouse_x_mick-=mouse_thresholdlim;
//      }
//      while (mouse_x_mick<-mouse_thresholdlim) {
//	mouse_x--;
//	mouse_x_mick+=mouse_thresholdlim;
//      }
//      if (mouse_x>mouse_lim_x2) mouse_x=mouse_lim_x2;
//      else if (mouse_x<mouse_lim_x1) mouse_x=mouse_lim_x1;
//
//      /* axes Y */
//      mickey=((b1&0x20)?256-b3:-b3);
//      /* this part come from "sermouse.c" */
//      mouse_y_mick+=mickey;
//      while(mouse_y_mick>mouse_thresholdlim) {
//	mouse_y++;
//	mouse_y_mick-=mouse_thresholdlim;
//      }
//      while (mouse_y_mick<-mouse_thresholdlim) {
//	mouse_y--;
//	mouse_y_mick+=mouse_thresholdlim;
//      }
//      if (mouse_y>mouse_lim_y2) mouse_y=mouse_lim_y2;
//      else if (mouse_y<mouse_lim_y1) mouse_y=mouse_lim_y1;
//
//      /* mouse handler */
//      if (mouse_handler != NULL)
//	mouse_handler(mouse_x,mouse_y,mouse_buttons);
//
//      change_status(WAITING);
//      break;
//    }
//  }  
//}
//

TASK (*ps2mouse_getserveraddr(void))(void)
{
  return generalmouse_server;
}

/*
 * this is from gpm (see sermouse.c for comments)
 */

#include "gpmcomp.h"

int opt_glidepoint_tap=0;

/* mice.c START */

int M_ps2(Gpm_Event *state,  unsigned char *data)
{
  static int tap_active=0; /* there exist glidepoint ps2 mice */

  state->buttons=
    !!(data[0]&1) * GPM_B_LEFT +
    !!(data[0]&2) * GPM_B_RIGHT +
    !!(data[0]&4) * GPM_B_MIDDLE;

  if (data[0]==0 && opt_glidepoint_tap) /* by default this is false */
    state->buttons = tap_active = opt_glidepoint_tap;
  else if (tap_active) {
    if (data[0]==8)
      state->buttons = tap_active = 0;
    else state->buttons = tap_active;
}

 /* Some PS/2 mice send reports with negative bit set in data[0]
  * and zero for movement.  I think this is a bug in the mouse, but
  * working around it only causes artifacts when the actual report is -256;
  * they'll be treated as zero. This should be rare if the mouse sampling
  * rate is set to a reasonable value; the default of 100 Hz is plenty.
  * (Stephen Tell)
  */
  if(data[1] != 0)
    state->dx=   (data[0] & 0x10) ? data[1]-256 : data[1];
  else
    state->dx = 0;
  if(data[2] != 0)
    state->dy= -((data[0] & 0x20) ? data[2]-256 : data[2]);
  else
    state->dy = 0;
  return 0;
}

/* mice.c END */

/*
 * Virtual operation on a ps/2 mouse
 */

int ps2mouse_open(void *ptrPID)
{
  /* some modification are made into 8042.c to allow mouse reinitialization! */
  //static int first=1;

  //if (first) {
    C8042_auxinit(*(PID*)ptrPID);
    /* probably not needed! */
    task_activate(*(PID*)ptrPID);
  // first=0;
  //}
  C8042_auxportenable();

  return 0;
}

void ps2mouse_close(void)
{
  C8042_auxportdisable();
  C8042_auxend();
}

void ps2mouse_disable(void)
{
  C8042_auxportdisable();
}

void ps2mouse_enable(void)
{
  C8042_auxportenable();
}

void ps2mouse_wait(void)
{
  task_endcycle();
}

int ps2mouse_get(BYTE *data)
{
  return C8042_auxget(data,NON_BLOCK);
}





