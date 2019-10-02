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
 CVS :        $Id: mcurgrx.c,v 1.1 2003/03/24 10:54:17 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2003/03/24 10:54:17 $
 ------------

 Author:	Massimiliano Giorgi
 Date:	24/Giu/99

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

/*---------------*
 * GRX Management*
 *---------------*/

#include <kernel/kern.h>
//#include <sys/sys.h>
//#include <mem.h>
#include <drivers/mouse.h>
#include <drivers/glib.h>
#include "_mouse.h"

static BYTE left_ptr_bits[] = {
   0x00, 0x00, 0x08, 0x00, 0x18, 0x00, 0x38, 0x00, 0x78, 0x00, 0xf8, 0x00,
   0xf8, 0x01, 0xf8, 0x03, 0xf8, 0x07, 0xf8, 0x00, 0xd8, 0x00, 0x88, 0x01,
   0x80, 0x01, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00};


static BYTE left_ptrmsk_bits[] = {
   0x0c, 0x00, 0x1c, 0x00, 0x3c, 0x00, 0x7c, 0x00, 0xfc, 0x00, 0xfc, 0x01,
   0xfc, 0x03, 0xfc, 0x07, 0xfc, 0x0f, 0xfc, 0x0f, 0xfc, 0x01, 0xdc, 0x03,
   0xcc, 0x03, 0x80, 0x07, 0x80, 0x07, 0x00, 0x03};

/* if called with NULL -> retrieve next bit of a bits-stream
 * else                -> initialize the bit stream
 */
static int get_bit(BYTE *p)
{
  static BYTE *ptr;
  static BYTE val;
  static int  c;

  if (p!=NULL) {
    ptr=p;
    val=0;
    c=8;
    return 0;
  }

  if (c==8) {
    c=0;
    val=*ptr;
    ptr++;
  } else
    val>>=1;

  c++;
  return val&1;
}

/* dimensions (16x16) */
#define SHAPE_DX MOUSESHAPEDX
#define SHAPE_DY MOUSESHAPEDY

/* shape hot spot (3,1) */
#define HOTSPOT_X MOUSEHOTSPOTX
#define HOTSPOT_Y MOUSEHOTSPOTY

/* cursor shape */
static BYTE shape[SHAPE_DX*SHAPE_DY*4];
/* cursor mask */
static BYTE mask[SHAPE_DX*SHAPE_DY*4];

/* old memory buffer */
static BYTE saved_shape[SHAPE_DX*SHAPE_DY*4];
/* new memory buffer */
static BYTE new_shape[SHAPE_DX*SHAPE_DY*4];

static BYTE *saved_shapeptr=(void*)DEFAULT;
static BYTE *saved_maskptr=(void*)DEFAULT;

static int bpp; // bytes per pixel

/* show grx cursor */
static void show_grx_cursor(int x, int y)
{
  int i;
  x-=HOTSPOT_X-1;
  y-=HOTSPOT_Y-1;
  grx_getimage(x,y,x+SHAPE_DX-1,y+SHAPE_DY-1,saved_shape);
  for(i=0;i<SHAPE_DX*SHAPE_DY*bpp/4;i++)
    ((DWORD*)new_shape)[i]=(((DWORD*)saved_shape)[i]&((DWORD*)mask)[i])|((DWORD*)shape)[i];
  grx_putimage(x,y,x+SHAPE_DX-1,y+SHAPE_DY-1,new_shape);
}

/* restore grx cursor */
static void restore_grx_cursor(int x, int y)
{
  x-=HOTSPOT_X-1;
  y-=HOTSPOT_Y-1;
  grx_putimage(x,y,x+SHAPE_DX-1,y+SHAPE_DY-1,saved_shape);
}

int mouse_grxcursor(int cmd)
{
  mouse_grxshape(saved_shapeptr,saved_maskptr);
  return _mouse_cursor_init(cmd|GRXCURSOR,show_grx_cursor,restore_grx_cursor);
}

/* compute the shape and mask array */
int mouse_grxshape(BYTE *shapeptr, BYTE *maskptr)
{
  int          result;
  grx_vga_modeinfo info;
  int          nb;
  BYTE         b;
  int          pc;
  int          i,j;
  int          saved_x,saved_y;
  int          cond;
  
  result=gd_getmodeinfo(&info);
  if (result==-1) return -1;
  bpp=nb=info.bytesperpixel;

  cond=(
	((autocursormode&STATUSMASK)==ENABLE)&&
        ((autocursormode&GRXCURSOR)==GRXCURSOR)
       );

  if (cond) {
    _mouse_getsavedposition(&saved_x,&saved_y);
    restore_grx_cursor(saved_x,saved_y);
  }

  if (shapeptr==(void*)DEFAULT) {
    //int h;

    shapeptr=left_ptr_bits;
    pc=0;
    get_bit(left_ptr_bits);
    for (i=0;i<SHAPE_DX*SHAPE_DY;i++) {
      b=get_bit(NULL)?255:0;
      for (j=0;j<nb;j++) shape[pc++]=b;
      //grx_plot(h+180,i+80,b);
    }
  } else {
    memcpy(shape,shapeptr,SHAPE_DX*SHAPE_DY*nb);
    saved_shapeptr=shapeptr;
  }

  /*
  some test!
  
  pc=0;
  for (i=0;i<16;i++)
    for (j=0;j<16;j++) {
      char s[16];
      sprintf(s,"%c",shape[pc++]==0?'-':'*');
      grx_text(s,200+j*8,200+i*8,255,0);
    }
  grx_putimage(200,80,200+15,80+16,shape);
  
  grx_getimage(180,80,180+16,80+16,shape);

  pc=0;
  for (i=0;i<16;i++)
    for (j=0;j<17;j++) {
      char s[16];
      sprintf(s,"%c",shape[pc++]==0?'-':'*');
      grx_text(s,400+j*8,400+i*8,255,0);
    }
  */

  if (maskptr==(void*)DEFAULT) {
    maskptr=left_ptrmsk_bits;
    pc=0;
    get_bit(left_ptrmsk_bits);
    for (i=0;i<SHAPE_DX*SHAPE_DY;i++) {
      b=get_bit(NULL)?0:255;
      for (j=0;j<nb;j++) mask[pc++]=b;
    }
  }else {
    memcpy(mask,maskptr,SHAPE_DX*SHAPE_DY*nb);
    saved_maskptr=maskptr;
  }

  if (cond)
     show_grx_cursor(saved_x,saved_y);

  return 0;
}
