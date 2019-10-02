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

/* CVS :        $Id: fab_lib.c,v 1.5 2005/01/08 14:34:28 pj Exp $ */

/*
 * Copyright (C) 2000 Fabio Calabrese <fabiocalabrese77@yahoo.it>
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

/****************************************************
 *                                                  *
 *  file:        FAB_lib.c                          *
 *  libreria:    FAB_LIB                            *
 *  version:     1.0 25/08/2002                     *
 *  creato da:   Fabio CALABRESE                    *
 *                                                  *
 ****************************************************/

#ifndef __FAB_LIB_c__
#define __FAB_LIB_c__
//-------------------BEGIN---------------------------

// *** Librerie di S.Ha.R.K. ***
#include<kernel/kern.h>
#include<kernel/mem.h>

#include<drivers/shark_fb26.h>

#include<ll/sys/types.h>
#include<ll/i386/x-dos.h>
// *** Librerie Standard C ***
// *** Librerie FAB ***
#include"fab_lib/fab_msg.h"
#include"fab_lib/fab_tool.h"
#include"fab_lib/fab_grx.h"

// *** Corpo delle funzioni in FAB_msg.h *** //

void FAB_print(char * InitMsg, char * msg)
{ if (InitMsg!=NULL) {kern_printf("\n[%s]",(InitMsg));}
  if (msg!=NULL)     {kern_printf("\n  %s.",msg);}
}
void __FAB_go(int test, char * OkMsg, char * ErrorMsg,int EXITflag)
{
  if (OkMsg==NULL)    OkMsg=FAB_EMPTY_MSG;
  if (ErrorMsg==NULL) ErrorMsg=FAB_EMPTY_MSG;

  if (!test) {kern_printf("\n  OK: %s.",OkMsg);}
  else       {kern_printf("\n  ERROR: %s!",ErrorMsg);
              if (EXITflag==FAB_EXIT_YES) exit(0);
             }
}

// *** Corpo delle funzioni in FAB_tool.h *** //

// nessun corpo!

// *** Corpo delle funzioni in FAB_show.h *** //

void FAB_show_sched_modules()
{ //int i;

  FAB_print("MODULI DI SCHEDULAZIONE",NULL);
  kern_printf("\n  (livello) (nome)");
  kern_printf("Names non more available!");
    //for (i=0; i<sched_levels; i++)
    //     {kern_printf("\n   %-9d %s",i,(level_table[i])->level_name);
    //     }
  FAB_newline();
}  

// *** Corpo delle funzioni in FAB_grx.h ***

FAB_BALLFRAME* FAB_ballframe_alloc()
{ FAB_BALLFRAME* frame;

  frame=(FAB_BALLFRAME*)kern_alloc(sizeof(FAB_BALLFRAME));
  if (frame==NULL) {
      FAB_print(NULL,"BALLFRAME CREATING... ERROR #1");
      return NULL;
      }
  return frame;
}
void FAB_ballframe_free(FAB_BALLFRAME * frame)
{ if (frame!=NULL)
    { kern_free(frame,sizeof(FAB_BALLFRAME));
    }
}

FAB_FRAME* FAB_frame_alloc()
{ FAB_FRAME* frame;

  frame=(FAB_FRAME*)kern_alloc(sizeof(FAB_FRAME));
  if (frame==NULL) {
      FAB_print(NULL,"FRAME CREATING... ERROR #1");
      return NULL;
      }

  return frame;
}
void FAB_frame_free(FAB_FRAME * frame)
{ if (frame!=NULL)
    { kern_free(frame,sizeof(FAB_FRAME));
    }
}
void  FAB_ballborder_put(FAB_BORDER* border, WORD cx, WORD cy, WORD r_int)
{ int i;

  for (i=0; i<border->dim; i++)
    grx_circle(cx, cy, r_int+i, border->buf[i]);
}
void FAB_border_put(FAB_BORDER* border, WORD x0, WORD y0, WORD x1, WORD y1)
{ int i;

  for (i=0; i<border->dim; i++)
    grx_rect(x0-i, y0-i, x1+i, y1+i, border->buf[i]);
}
FAB_BORDER* FAB_border_alloc(WORD dim)
{ FAB_BORDER* border;

  border=(FAB_BORDER*)kern_alloc(sizeof(FAB_BORDER));
  if (border==NULL) {
      FAB_print(NULL,"BORDER CREATING... ERROR #1");
      return NULL;
      }

  border->buf=(COLOR*)kern_alloc(dim*sizeof(COLOR));
  if (border->buf==NULL) {
      FAB_print(NULL,"BORDER CREATING... ERROR #2");
      return NULL;
      }

  border->dim = dim;

  return border;
}
void FAB_border_free(FAB_BORDER * border)
{ if (border!=NULL)
    { kern_free(border->buf,border->dim*sizeof(COLOR));
      kern_free(border,sizeof(FAB_BORDER));
    }
}
void FAB_image_free(FAB_IMAGE* image)
{ if (image!=NULL)
    { kern_free(image->buf,image->lx*image->ly*sizeof(COLOR));
      kern_free(image,sizeof(FAB_IMAGE));
    }
}
FAB_IMAGE* FAB_image_alloc(WORD lx, WORD ly)
{ FAB_IMAGE* image;

  image=(FAB_IMAGE*)kern_alloc(sizeof(FAB_IMAGE));
  if (image==NULL) {
      FAB_print(NULL,"IMAGE CREATING... ERROR #1");
      return NULL;
      }

  image->buf=(COLOR*)kern_alloc(lx*ly*sizeof(COLOR));
  if (image->buf==NULL) {
      FAB_print(NULL,"IMAGE CREATING... ERROR #2");
      return NULL;
      }
  FAB_image_no_trasparent_set(image);
  image->lx = lx;
  image->ly = ly;

  return image;
}
int FAB_image_load(FAB_IMAGE* image, char* file_name)
{
  DOS_FILE* file;
  BYTE      rgb[3];
  COLOR   * buf;
  int i;

  file=DOS_fopen(file_name,"r");
  if (file==NULL) return -1;

  buf = image->buf;
  for (i=0; i<image->lx*image->ly; i++)
    { DOS_fread(&rgb[0],sizeof(BYTE),3,file);
      *buf = FAB_rgb(rgb[0],rgb[1],rgb[2]);
      buf++;
    }

  DOS_fclose(file);
  return(0);
}
void FAB_image_get(FAB_IMAGE* image, WORD x0, WORD y0, WORD lx, WORD ly)
{ WORD x, y;
  COLOR* buf;

  buf = image->buf;
  for (y=0; y<image->ly; y++)
    for (x=0; x<image->lx; x++) {
      *buf = (COLOR) grx_getpixel(x0+x,y0+y);
      buf++;
      }
  FAB_image_no_trasparent_set(image);
}
void FAB_image_put(FAB_IMAGE* image, WORD x0, WORD y0)
{ WORD x, y;
  COLOR* buf;

  buf = image->buf;
  if (!image->trasparent) {
    for (y=0; y<image->ly; y++)
      for (x=0; x<image->lx; x++)
        grx_plot(x0+x,y0+y,*buf++);
    }
  else {
    for (y=0; y<image->ly; y++)
      for (x=0; x<image->lx; x++) {
        if (*buf!=image->trasparent_col)
           grx_plot(x0+x,y0+y,*buf);
        buf++;
        }
    }
}
void FAB_image_put_within(FAB_IMAGE* image, WORD x0, WORD y0,
                          WORD xx0, WORD yy0, WORD xx1, WORD yy1)
{ int step_x0_xx0;
  int step_y0_yy0;
  int step_xx1_x1;
  int step_yy1_y1;
  WORD xx, yy;
  COLOR* buf;

  step_x0_xx0 = xx0 - x0;
  step_y0_yy0 = yy0 - y0;
  step_xx1_x1 = (x0 + image->lx - 1) - xx1;
  step_yy1_y1 = (y0 + image->ly - 1) - yy1;

  if (step_x0_xx0 < 0) step_x0_xx0 = 0;
  if (step_y0_yy0 < 0) step_y0_yy0 = 0;
  if (step_xx1_x1 < 0) step_xx1_x1 = 0;
  if (step_yy1_y1 < 0) step_yy1_y1 = 0;

  buf = image->buf + step_y0_yy0*image->lx;
  if (!image->trasparent) {
    for(yy=step_y0_yy0; yy<image->ly-step_yy1_y1; yy++) {
      buf += step_x0_xx0;
      for(xx=step_x0_xx0; xx<image->lx-step_xx1_x1; xx++) 
          grx_plot(x0+xx,y0+yy,*buf++);
      buf += step_xx1_x1;
      }
    }
  else { 
    for(yy=step_y0_yy0; yy<image->ly-step_yy1_y1; yy++) {
      buf += step_x0_xx0;
      for(xx=step_x0_xx0; xx<image->lx-step_xx1_x1; xx++) {
        if (*buf!=image->trasparent_col)
           grx_plot(x0+xx,y0+yy,*buf);
        buf++;
        }
      buf += step_xx1_x1;
      }
    }
}
//...funzione usata da FAB_image_fill()...
void __FAB_image_put_into(FAB_IMAGE* image, WORD x0, WORD y0, WORD lx, WORD ly)
{ WORD x, y;
  WORD skipped_pixels;
  COLOR* buf;

  if (lx < image->lx) skipped_pixels = image->lx - lx;
  else                skipped_pixels = 0; 
  buf = image->buf;

  if (!image->trasparent) {
    for (y=0; y<ly; y++, buf+=skipped_pixels) {
      if (y > image->ly -1) break;
      for (x=0; x<lx; x++, buf++) {
        if (x < image->lx) grx_plot(x0+x,y0+y,*buf);
        else break;
        }
      }
    }
  else {
    for (y=0; y<ly; y++, buf+=skipped_pixels) {
      if (y > image->ly -1) break;
      for (x=0; x<lx; x++, buf++) {
        if (x < image->lx) {
           if (*buf!=image->trasparent_col)
              grx_plot(x0+x,y0+y,*buf);
           }
        else break;
        }
      }
    }
}
void FAB_image_fill(FAB_IMAGE* image, WORD x0, WORD y0, WORD lx, WORD ly)
{ WORD x, y;
  WORD sub_lx, sub_ly;

  for (y=0; y<ly; y+=image->ly) {
    sub_ly = ly - y; // spazio verticale disponibile
    if (sub_ly > image->ly) sub_ly = image->ly;
    for (x=0; x<lx; x+=image->lx) {
       sub_lx = lx - x; // spazio orizzontale disponibile
       if (sub_lx > image->lx) sub_lx = image->lx;
       __FAB_image_put_into(image,x0+x,y0+y,sub_lx,sub_ly);
       }
    }
}
void FAB_frame_put(FAB_FRAME* frame, WORD x0, WORD y0, WORD lx, WORD ly)
{
  WORD eff_dim;
  int j;

  //BORDO DEL FRAME
  if (frame->border!=NULL) eff_dim = frame->border->dim;
  else                     eff_dim = frame->border_dim;
  frame->x0 = x0 + eff_dim;     
  frame->y0 = y0 + eff_dim;
  frame->x1 = x0 + lx - eff_dim - 1;
  frame->y1 = y0 + ly - eff_dim - 1;
  if (frame->border!=NULL) {
     FAB_border_put(frame->border, frame->x0-1, frame->y0-1,
                                   frame->x1+1, frame->y1+1);
     }
  else {
     for (j=0; j<eff_dim; j++)
       grx_rect(frame->x0-1-j, frame->y0-1-j,
                frame->x1+1+j, frame->y1+1+j, frame->border_col);
     }

  //PANNELLO DEL FRAME
  if (!frame->trasparent) {
    if (frame->image==NULL) grx_box(frame->x0, frame->y0,
                                    frame->x1, frame->y1, frame->color);
    else                    FAB_image_fill(frame->image,
                                           frame->x0,    frame->y0,
                                           lx-eff_dim*2, ly-eff_dim*2);
    }
}

void FAB_ballframe_put(FAB_BALLFRAME* frame, WORD cx, WORD cy, WORD r)
{
  int j;
  WORD eff_dim;

  //BORDO DEL FRAME
  if (frame->border!=NULL) eff_dim = frame->border->dim;
  else                     eff_dim = frame->border_dim;
  frame->cx = cx;     
  frame->cy = cy;
  frame->r = r - eff_dim;
  
  if (frame->border!=NULL) {
     FAB_ballborder_put(frame->border,frame->cx,frame->cy,frame->r+1);
     }
  else {
     for (j=0; j<eff_dim; j++)
       grx_circle(frame->cx,frame->cy,frame->r+1+j,frame->border_col);
     }

  //PANNELLO DEL FRAME
    if (!frame->trasparent) {
    grx_disc(frame->cx,frame->cy,frame->r,frame->color);
    }
}

void FAB_image_copy(FAB_IMAGE* orig_image, FAB_IMAGE* dest_image)
{
  COLOR* orig_buf;
  COLOR* dest_buf;
  int j,n;

  n = orig_image->lx * orig_image->ly;
  if (n==(dest_image->lx * dest_image->ly)) {
    orig_buf = orig_image->buf;
    dest_buf = dest_image->buf;
    for(j=0; j<n; j++) {
      *dest_buf = *orig_buf;
      orig_buf++;
      dest_buf++;
      }
    } 
}
void   FAB_image_color_change(FAB_IMAGE* image, COLOR old_col, COLOR new_col)
{ COLOR* buf;
  int j,n;

  buf = image->buf;
  n = image->lx * image->ly;
  for(j=0; j<n; j++, buf++)
    if (*buf==old_col)
       *buf=new_col;   
}
//----------------------END--------------------------
#endif
