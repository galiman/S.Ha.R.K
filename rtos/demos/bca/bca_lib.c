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

/* CVS :        $Id: bca_lib.c,v 1.5 2006/07/03 15:34:08 tullio Exp $ */

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
 *  file:        BCA_LIB.c                          *
 *  libreria:    BCA_LIB                            *
 *  creato da:   Fabio CALABRESE                    *
 *                                                  *
 ****************************************************/

#ifndef __BCA_LIB_c__
#define __BCA_LIB_c__
//-------------------BEGIN---------------------------

// *** Librerie S.Ha.R.K ***
#include<kernel/kern.h>

#include<drivers/shark_fb26.h>

// *** Librerie Standard C ***
// *** Librerie FAB ***
#include"fab_lib/fab_msg.h"
// *** Librerie BCA ***
#include"bca.h"

char * titolo[10]={" UNIVERSITA' DEGLI \0",
                          "     STUDI DI      \0",
                          "       PAVIA       \0",
                          " S.Ha.R.K project  \0",
                          "  'BaseContrAerea' \0",
                          "-------------------\0",
                          "    PAVIA  2002    \0",
                          "by Fabio CALABRESE \0",
                          "<fabiocalabrese77  \0",
                          "         @yahoo.it>\0"};

void kern_alloc_images(); // richiede DISABLED_INTERUPTS
void kern_free_images();  // richiede DISABLED_INTERUPTS
void kern_load_images();  // solo in __kernel_register_levels__()

void kern_alloc_borders(); // richiede DISABLED_INTERUPTS
void kern_free_borders();  // richiede DISABLED_INTERUPTS
void kern_set_border();    // consigliato in __kernel_register-levels__()

void kern_alloc_frames(); // richiede DISABLED_INTERUPTS
void kern_free_frames();  // richiede DISABLED_INTERUPTS
void kern_set_frames();    // consigliato in __kernel_register-levels__()

void kern_alloc_ballframes(); // richiede DISABLED_INTERUPTS
void kern_free_ballframes();  // richiede DISABLED_INTERUPTS
void kern_set_ballframes();    // consigliato in __kernel_register-levels__()

void kern_alloc_task_tables(); // richiede DISABLED_INTERUPTS
void kern_free_task_tables();  // richiede DISABLED_INTERUPTS
void kern_set_task_tables();    // consigliato in __kernel_register-levels__()

void kern_alloc_task_tables()
{
  int test=0,j;

  FAB_print("TASKS'TABLES","allocazione...");
  for (j=0; j<AEREO_N_MAX; j++) {
     aereo_table[j].image=FAB_image_alloc(AEREO_LX,AEREO_LY);
     if(aereo_table[j].image==NULL) test=1;
     }
  FAB_msg(test,"fatto","almeno un elemento della tabella non allocato");
}
void kern_free_task_tables()
{ int j;

  for (j=0; j<AEREO_N_MAX; j++) {
     FAB_image_free(aereo_table[j].image);
     }
}
void kern_set_task_tables()
{ int j;

  FAB_print(NULL,"settaggio...");
  for (j=0; j<AEREO_N_MAX; j++) {
     aereo_table[j].status  = 0;
     aereo_table[j].killing = 0;
     aereo_table[j].image->trasparent     = 1;
     aereo_table[j].image->trasparent_col = AEREO_TRASPARENT_COL;
     }

  FAB_print(NULL,"fatto");
}


void kern_alloc_ballframes()
{
  int test=0,j;

  FAB_print("FAB_BALLFRAMEs","allocazione...");
  ballframe_radar = FAB_ballframe_alloc();
  if (ballframe_radar==NULL) test=1;
  for (j=0; j<3; j++) {
    ballframe_lucerossa[j] = FAB_ballframe_alloc();
    if (ballframe_lucerossa[j]==NULL) test=1;
    }
  FAB_msg(test,"fatto","almeno un ballframe non allocato");
}
void kern_free_ballframes()
{ int j;

  FAB_ballframe_free(ballframe_radar);
  for (j=0; j<3; j++)
    FAB_ballframe_free(ballframe_lucerossa[j]);
}
void kern_set_ballframes()
{ int j;

  FAB_print(NULL,"settaggio...");
  FAB_ballframe_border_set(ballframe_radar,border_verde7,7,
                           FRAME_MISURE_LINE_COLOR);
  FAB_ballframe_color_set(ballframe_radar,0,FAB_blue);
  for(j=0; j<3; j++) {
    FAB_ballframe_border_set(ballframe_lucerossa[j],border_verde7,7,
                             FRAME_MISURE_LINE_COLOR);
    FAB_ballframe_color_set(ballframe_lucerossa[j],0,FAB_red);
    }
  FAB_print(NULL,"fatto");
}

void kern_alloc_frames()
{
  int test=0;

  FAB_print("FAB_FRAMEs","allocazione...");
  frame_bca = FAB_frame_alloc();
  if (frame_bca==NULL) test=1;
  frame_misure_oriz = FAB_frame_alloc();
  if (frame_misure_oriz==NULL) test=1;
  frame_misure_vert = FAB_frame_alloc();
  if (frame_misure_vert==NULL) test=1;
  frame_titolo = FAB_frame_alloc();
  if (frame_titolo==NULL) test=1;
  frame_comandi = FAB_frame_alloc();
  if (frame_comandi==NULL) test=1;
  frame_control = FAB_frame_alloc();
  if (frame_control==NULL) test=1;

  FAB_msg(test,"fatto","almeno un frame non allocato");
}
void kern_free_frames()
{
  FAB_frame_free(frame_bca);
  FAB_frame_free(frame_misure_oriz);
  FAB_frame_free(frame_misure_vert);
  FAB_frame_free(frame_titolo);
  FAB_frame_free(frame_comandi);
  FAB_frame_free(frame_control);

}

void kern_set_frames()
{
  FAB_print(NULL,"settaggio...");
  FAB_frame_border_set(frame_bca,border_blu7,7,FAB_rgb(255,0,0));
  FAB_frame_color_set(frame_bca,0,image_bca,FAB_green);
  FAB_frame_border_set(frame_misure_oriz,border_verde7,7,FRAME_MISURE_LINE_COLOR);
  FAB_frame_color_set(frame_misure_oriz,0,NULL,FRAME_MISURE_BACK_COLOR);
  FAB_frame_border_set(frame_misure_vert,border_verde7,7,FRAME_MISURE_LINE_COLOR);
  FAB_frame_color_set(frame_misure_vert,0,NULL,FRAME_MISURE_BACK_COLOR);
  FAB_frame_border_set(frame_titolo,border_bianco7,7,FAB_white);
  FAB_frame_color_set(frame_titolo,0,NULL,FRAME_TITOLO_BACK_COLOR1);
  FAB_frame_border_set(frame_comandi,border_giallo7,7,FAB_yellow);
  FAB_frame_color_set(frame_comandi,0,NULL,FRAME_COMANDI_BACK_COLOR1);
  FAB_frame_border_set(frame_control,border_verde7,7,FAB_green);
  FAB_frame_color_set(frame_control,0,NULL,FRAME_CONTROL_BACK_COLOR);

  FAB_print(NULL,"fatto");
}

void kern_alloc_borders()
{
  int test=0;

  FAB_print("FAB_BORDERs","allocazione...");
  border_blu7 = FAB_border_alloc(7);
  if (border_blu7==NULL) test=1;
  border_verde7 = FAB_border_alloc(7);
  if (border_verde7==NULL) test=1;
  border_rosso7 = FAB_border_alloc(7);
  if (border_rosso7==NULL) test=1;
  border_bianco7 = FAB_border_alloc(7);
  if (border_bianco7==NULL) test=1;
  border_giallo7 = FAB_border_alloc(7);
  if (border_giallo7==NULL) test=1;

  FAB_msg(test,"fatto","almeno un bordo non allocato");
}
void kern_free_borders()
{
  FAB_border_free(border_blu7);
  FAB_border_free(border_verde7);
  FAB_border_free(border_rosso7);
  FAB_border_free(border_bianco7);
  FAB_border_free(border_giallo7);
}

void kern_set_borders()
{
  FAB_print(NULL,"settaggio...");
  border_blu7->buf[0]=FAB_rgb(  0,  0, 50);
  border_blu7->buf[1]=FAB_rgb(  0, 50,200);
  border_blu7->buf[2]=FAB_rgb( 50,100,255);
  border_blu7->buf[3]=FAB_rgb(100,150,255);
  border_blu7->buf[4]=FAB_rgb( 50,100,255);
  border_blu7->buf[5]=FAB_rgb(  0, 50,200);
  border_blu7->buf[6]=FAB_rgb(  0,  0, 50);

  border_verde7->buf[0]=FAB_rgb(  0, 50,  0);
  border_verde7->buf[1]=FAB_rgb(  0,100, 50);
  border_verde7->buf[2]=FAB_rgb( 50,255,100);
  border_verde7->buf[3]=FAB_rgb(100,255,150);
  border_verde7->buf[4]=FAB_rgb( 50,255,100);
  border_verde7->buf[5]=FAB_rgb(  0,100, 50);
  border_verde7->buf[6]=FAB_rgb(  0, 50,  0);

  border_rosso7->buf[0]=FAB_rgb( 50,  0,  0);
  border_rosso7->buf[1]=FAB_rgb(200, 25, 25);
  border_rosso7->buf[2]=FAB_rgb(255, 75, 75);
  border_rosso7->buf[3]=FAB_rgb(255,125,125);
  border_rosso7->buf[4]=FAB_rgb(255, 75, 75);
  border_rosso7->buf[5]=FAB_rgb(200, 25, 25);
  border_rosso7->buf[6]=FAB_rgb( 50,  0,  0);

  border_bianco7->buf[0]=FAB_rgb( 25, 25, 25);
  border_bianco7->buf[1]=FAB_rgb( 75, 75, 75);
  border_bianco7->buf[2]=FAB_rgb(125,125,125);
  border_bianco7->buf[3]=FAB_rgb(255,255,255);
  border_bianco7->buf[4]=FAB_rgb(125,125,125);
  border_bianco7->buf[5]=FAB_rgb( 75, 75, 75);
  border_bianco7->buf[6]=FAB_rgb( 25, 25, 25);

  border_giallo7->buf[0]=FAB_rgb( 25, 25,  0);
  border_giallo7->buf[1]=FAB_rgb(115,115, 25);
  border_giallo7->buf[2]=FAB_rgb(170,170, 75);
  border_giallo7->buf[3]=FAB_rgb(255,255,100);
  border_giallo7->buf[4]=FAB_rgb(170,170, 75);
  border_giallo7->buf[5]=FAB_rgb(115,115, 25);
  border_giallo7->buf[6]=FAB_rgb( 25, 25,  0);

  FAB_print(NULL,"fatto");
}

void kern_alloc_images()
{
  int j,test=0;

  FAB_print("FAB_IMAGEs","allocazione...");

  image_missile[0] = FAB_image_alloc(MISSILE_LX,MISSILE_LY);
  if (image_missile[0]==NULL) test=1;
  image_missile[1] = FAB_image_alloc(MISSILE_LX,MISSILE_LY);
  if (image_missile[1]==NULL) test=1;

  for (j=0; j<2; j++) {
     image_cannone[j] = FAB_image_alloc(CANNONE_LX,CANNONE_LY);
     if (image_cannone[j]==NULL) test=1;
     }

  for (j=0; j<4; j++) {
     image_aereo[j] = FAB_image_alloc(AEREO_LX,AEREO_LY);
     if (image_aereo[j]==NULL) test=1;
     }

  for (j=0; j<5; j++) {
     image_esplo[j] = FAB_image_alloc(ESPLO_LX,ESPLO_LY);
     if (image_esplo[j]==NULL) test=1;
     }

  image_sfondo = FAB_image_alloc(96,96);
  if (image_sfondo==NULL) test=1;

  image_bca = FAB_image_alloc(500,500);
  if (image_bca==NULL) test=1;

  FAB_msg(test,"fatto","almeno un'immagine non allocata");
}
void kern_free_images()
{
  int j;

  FAB_image_free(image_missile[0]);
  FAB_image_free(image_missile[1]);
  for (j=0; j<2; j++)  FAB_image_free(image_cannone[j]);
  for (j=0; j<4; j++)  FAB_image_free(image_aereo[j]);
  for (j=0; j<5; j++)  FAB_image_free(image_esplo[j]);
  FAB_image_free(image_sfondo);
  FAB_image_free(image_bca);
}

void kern_load_images()
{ int test=0;

  FAB_print(NULL,"caricamento dai file...");

  FAB_print(NULL,"  [missile1.raw]...");
  if(FAB_image_load(image_missile[0],"BCA_raw/missile1.raw")==-1) test=1;
  FAB_image_trasparent_set(image_missile[0],MISSILE_TRASPARENT_COL);
  FAB_print(NULL,"  [missile2.raw]...");
  if(FAB_image_load(image_missile[1],"BCA_raw/missile2.raw")==-1) test=1;
  FAB_image_trasparent_set(image_missile[1],MISSILE_TRASPARENT_COL);

  FAB_print(NULL,"  [aereo1.raw]...");
  if(FAB_image_load(image_aereo[0],"BCA_raw/aereo1.raw")==-1) test=1;
  FAB_image_trasparent_set(image_aereo[0],AEREO_TRASPARENT_COL);
  FAB_print(NULL,"  [aereo2.raw]...");
  if(FAB_image_load(image_aereo[1],"BCA_raw/aereo2.raw")==-1) test=1;
  FAB_image_trasparent_set(image_aereo[1],AEREO_TRASPARENT_COL);
  FAB_print(NULL,"  [aereo1_h.raw]...");
  if(FAB_image_load(image_aereo[2],"BCA_raw/aereo1_h.raw")==-1) test=1;
  FAB_image_trasparent_set(image_aereo[2],AEREO_TRASPARENT_COL);
  FAB_print(NULL,"  [aereo2_h.raw]...");
  if(FAB_image_load(image_aereo[3],"BCA_raw/aereo2_h.raw")==-1) test=1;
  FAB_image_trasparent_set(image_aereo[3],AEREO_TRASPARENT_COL);

  FAB_print(NULL,"  [cannon.raw]...");
  if(FAB_image_load(image_cannone[0],"BCA_raw/cannon.raw")==-1) test=1;
  FAB_image_trasparent_set(image_cannone[0],CANNONE_TRASPARENT_COL);
  FAB_print(NULL,"  [cannon_h.raw]...");
  if(FAB_image_load(image_cannone[1],"BCA_raw/cannon_h.raw")==-1) test=1;
  FAB_image_trasparent_set(image_cannone[1],CANNONE_TRASPARENT_COL);

  FAB_print(NULL,"  [esplo1.raw]...");
  if(FAB_image_load(image_esplo[0],"BCA_raw/esplo1.raw")==-1) test=1;
  FAB_image_trasparent_set(image_esplo[0],ESPLO_TRASPARENT_COL);
  FAB_print(NULL,"  [esplo2.raw]...");
  if(FAB_image_load(image_esplo[1],"BCA_raw/esplo2.raw")==-1) test=1;
  FAB_image_trasparent_set(image_esplo[1],ESPLO_TRASPARENT_COL);
  FAB_print(NULL,"  [esplo3.raw]...");
  if(FAB_image_load(image_esplo[2],"BCA_raw/esplo3.raw")==-1) test=1;
  FAB_image_trasparent_set(image_esplo[2],ESPLO_TRASPARENT_COL);
  FAB_print(NULL,"  [esplo4.raw]...");
  if(FAB_image_load(image_esplo[3],"BCA_raw/esplo4.raw")==-1) test=1;
  FAB_image_trasparent_set(image_esplo[3],ESPLO_TRASPARENT_COL);
  FAB_print(NULL,"  [esplo5.raw]...");
  if(FAB_image_load(image_esplo[4],"BCA_raw/esplo5.raw")==-1) test=1;
  FAB_image_trasparent_set(image_esplo[4],ESPLO_TRASPARENT_COL);

  FAB_print(NULL,"  [sfondo.raw]...");
  if(FAB_image_load(image_sfondo,"BCA_raw/sfondo.raw")==-1) test=1;
  FAB_image_no_trasparent_set(image_sfondo);

  FAB_print(NULL,"  [bca.raw]...");
  if(FAB_image_load(image_bca,"BCA_raw/bca.raw")==-1) test=1;
  FAB_image_no_trasparent_set(image_bca);

  FAB_msg(test,"fatto","almeno un file non esiste oppure ha un formato incompatibile");
}

// *** Corpo delle funzioni in CA.h *** //
void kern_init_bca()
{
  FAB_clr();
                                                                                                                             
  kern_alloc_task_tables();
  kern_set_task_tables();
                                                                                                                             
  kern_alloc_borders();
  kern_set_borders();
  kern_alloc_images();
  kern_load_images();
  kern_alloc_frames();
  kern_set_frames();
  kern_alloc_ballframes();
  kern_set_ballframes();
}

//----------------------END--------------------------
#endif
