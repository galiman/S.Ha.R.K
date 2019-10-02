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

/* CVS :        $Id: fab_grx.h,v 1.3 2004/05/23 08:59:28 giacomo Exp $ */

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
 *  file:        FAB_grx.h                          *
 *  libreria:    FAB_LIB                            *
 *  version:     1.0 25/08/2002                     *
 *  creato da:   Fabio CALABRESE                    *
 *                                                  *
 ****************************************************
 *                                                  *
 *  descrizione: libreria di funzioni grafiche:     *
 *               permette di gestire con facilita'  *
 *               immagini, cornici, e veri e propri *
 *               pannelli colorati.                 *
 *               Il contesto di grafica va avviato  *
 *               con FAB_grx_open() e alla fine     *
 *               chiuso correttamente con la funz.  *
 *               FAB_grx_close(). L'ambiente di     *
 *               grafica e'impostato a 16bit per    *
 *               default, comunque e' sempre        *
 *               possibile scegliere la profondita' *
 *               di colori preferita tra 15/16/24/32*
 *               definendo semplicemente solo una   *
 *               delle seguenti costanti all'inizio *
 *               del proprio programma che include  *
 *               questa libreria grafica:           *
 *                FAB_GRX_15                        *
 *                FAB_GRX_16 (default)              *
 *                FAB_GRX_24                        *
 *                FAB_GRX_32                        *
 *                                                  *
 ****************************************************/

#ifndef __FAB_LIB_fab_grx_h__
#define __FAB_LIB_fab_grx_h__
//-------------------BEGIN---------------------------

// *** Librerie di S.Ha.R.K. ***
#include <kernel/kern.h>
#include <ll/sys/types.h>

#include <drivers/shark_fb26.h>

// *** Librerie Standard C ***
// *** Librerie FAB ***
#include"fab_grx.sys"

//#define FAB_GRX_15
//#define FAB_GRX_16
//#define FAB_GRX_24
//#define FAB_GRX_32

typedef __FAB_COLOR  COLOR;

typedef struct {
            WORD        lx,ly;
            COLOR     * buf;
            int         trasparent;
            COLOR       trasparent_col;
               } FAB_IMAGE;

typedef struct {
	        WORD    dim;
	        COLOR * buf;
                } FAB_BORDER;

typedef struct {
                 FAB_BORDER * border;     //default = NULL
                 WORD         border_dim; //default = 0
                 COLOR        border_col;

                 BYTE         trasparent;  //default = 0
                 FAB_IMAGE *  image;       //default = NULL
                 COLOR        color;

                 WORD         x0,y0,x1,y1; //spigoli pannello interno
                                           //cornice esclusa: vengono settati
                                           //automaticamente con FAB_frame_put().
                 } FAB_FRAME;
typedef struct {
                 FAB_BORDER * border;     //default = NULL
                 WORD         border_dim; //default = 0
                 COLOR        border_col;

                 BYTE         trasparent;  //default = 0
                 COLOR        color;

                 WORD         cx,cy,r;     //centro e raggio del pannello
                                           //circolare: vengono settati
                                           //automaticamente con FAB_ballframe_put().
                 } FAB_BALLFRAME;

// -----------
// Definizioni
//----------------------
#define FAB_BPP __FAB_BPP
#define FAB_rgb(R24,G24,B24) __FAB_rgb((R24),(G24),(B24))

// Colori base
#define FAB_white   FAB_rgb(255,255,255)
#define FAB_black   FAB_rgb(  0,  0,  0)
#define FAB_red     FAB_rgb(255,  0,  0)
#define FAB_green   FAB_rgb(  0,255,  0)
#define FAB_blue    FAB_rgb(  0,  0,255)
#define FAB_yellow  FAB_rgb(255,255,  0)
#define FAB_gray    FAB_rgb(100,100,100)

//----------------------

// ----------------
// Funzioni & Macro
//--------------------------------------------------
int  FAB_grx_open(WORD lx, WORD ly);
void FAB_grx_close();

FAB_IMAGE*  FAB_image_alloc(WORD lx, WORD ly);   //richiede DISABLED INTERRUPTS
void        FAB_image_free(FAB_IMAGE* image);    //richiede DISABLED INTERRUPTS
FAB_BORDER* FAB_border_alloc(WORD dim);          //richiede DISABLED INTERRUPTS
void        FAB_border_free(FAB_BORDER * border);//richiede DISABLED INTERRUPTS
FAB_FRAME*  FAB_frame_alloc();                   //richiede DISABLED INTERRUPTS
void        FAB_frame_free(FAB_FRAME * frame);   //richiede DISABLED INTERRUPTS
FAB_BALLFRAME*  FAB_ballframe_alloc();                    //richiede DISABLED INTERRUPTS
void            FAB_ballframe_free(FAB_BALLFRAME * frame);//richiede DISABLED INTERRUPTS

// Permette di riempire lo spazio allocato ad un FAB_IMAGE
// leggendo le sequenze rgb a 24bit da un file immagine binario.
// Un formato perfettamente compatibile Š quello
// "RAW"(con header=0 e sequenza ad ordinamento RGB-RGB-RGB...):
int FAB_image_load(FAB_IMAGE* image, char* file_name); // solo in __kernel_register_levels__()
// Per default un FAB_IMAGE non ha il colore di trasparenza:
#define FAB_image_no_trasparent_set(IMAGE)             (IMAGE)->trasparent=0
#define FAB_image_trasparent_set(IMAGE,TRASPARENT_COL) (IMAGE)->trasparent=1;(IMAGE)->trasparent_col=(TRASPARENT_COL)

//  Settaggio FAB_BORDER deve essere fatto manualmente!
//  esempio:
//     mio_border->buf[i] = FAB_rgb(0,100,0);
//     ...
//     mio_border->buf[N] = FAB_rgb(0,100,255);
//  (non esiste una funzione FAB_border_set)

#define FAB_frame_border_set(FRAME,BORDER,BORDER_DIM,BORDER_COL) (FRAME)->border=(BORDER);(FRAME)->border_dim=(BORDER_DIM);(FRAME)->border_col=(BORDER_COL);
#define FAB_frame_color_set(FRAME,TRASPARENT_FLAG,IMAGE,COLOR)   (FRAME)->trasparent=(TRASPARENT_FLAG);(FRAME)->image=(IMAGE);(FRAME)->color=(COLOR);
#define FAB_ballframe_border_set(FRAME,BORDER,BORDER_DIM,BORDER_COL) (FRAME)->border=(BORDER);(FRAME)->border_dim=(BORDER_DIM);(FRAME)->border_col=(BORDER_COL);
#define FAB_ballframe_color_set(FRAME,TRASPARENT_FLAG,COLOR)         (FRAME)->trasparent=(TRASPARENT_FLAG);(FRAME)->color=(COLOR);

void FAB_ball_put(WORD cx, WORD cy, WORD r, COLOR c);
void  FAB_image_get(FAB_IMAGE* image, WORD x, WORD y, WORD lx, WORD ly);
void  FAB_image_put(FAB_IMAGE* image, WORD x, WORD y);
void FAB_image_put_within(FAB_IMAGE* image, WORD x0, WORD y0,
                          WORD xx0, WORD yy0, WORD xx1, WORD yy1);
void  FAB_image_copy(FAB_IMAGE* orig_image, FAB_IMAGE* dest_image);
void  FAB_image_color_change(FAB_IMAGE* image, COLOR old_col, COLOR new_col);
void  FAB_image_fill(FAB_IMAGE* image, WORD x0, WORD y0, WORD lx, WORD ly);
void  FAB_border_put(FAB_BORDER* border, WORD x0, WORD y0, WORD x1, WORD y1);
void  FAB_ballborder_put(FAB_BORDER* border, WORD cx, WORD cy, WORD r_int);
void  FAB_frame_put(FAB_FRAME* frame, WORD x0, WORD y0, WORD lx, WORD ly);
void  FAB_ballframe_put(FAB_BALLFRAME* frame, WORD cx, WORD cy, WORD r);
//--------------------------------------------------

// Altro...

//---------------------END---------------------------
#endif
