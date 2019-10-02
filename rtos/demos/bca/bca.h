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

/* CVS :        $Id: bca.h,v 1.3 2006/07/03 15:10:15 tullio Exp $ */

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
 *  file:        bca.h                              *
 *  included by: bca.c & bca_lib di cui fa da header*
 *  data:        15/09/2002                         *
 *  creato da:   Fabio CALABRESE                    *
 *                                                  *
 ******************************************************
 *                                                    *
 *  descrizione: contiene la definizione di variabili *
 *               e strutture globali, con un corredo  *
 *               di macro e funzioni per la loro      *
 *               gestione, usate nel programma SHaRK  *
 *               "(B)ase(C)ontr(A)rea", il cui codice *
 *               e' scritto nel file bca.c;           *
 *               il file bca_lib.c contiene il corpo  *
 *               di tutte le funzioni qui dichiarate. *
 *                                                    *
 ******************************************************/

#ifndef __BCA_H__
#define __BCA_H__
//----------------BEGIN------------------

// *** Librerie S.Ha.R.K ***
// *** Librerie Standard C ***
// *** Librerie FAB ***
#include"fab_lib/fab_grx.h"
#include"fab_lib/fab_tool.h"
// *** Librerie BCA ***


// ---------------------------------
// *** Elementi Grafici Generici ***
// ---------------------------------

// IMMAGINI:
FAB_IMAGE* image_sfondo;
FAB_IMAGE* image_bca;
FAB_IMAGE* image_aereo[4];      // 1 aereo per ogni direzione(soft e hard)
FAB_IMAGE* image_esplo[5];      // 5 immagini in sequenza simulanti
FAB_IMAGE* image_cannone[2];    //  un esplosione
FAB_IMAGE* image_missile[2];

// BORDI:
FAB_BORDER* border_blu7;
FAB_BORDER* border_verde7;
FAB_BORDER* border_rosso7;
FAB_BORDER* border_bianco7;
FAB_BORDER* border_giallo7;

// FRAME:
FAB_FRAME* frame_bca;
FAB_FRAME* frame_misure_oriz;
FAB_FRAME* frame_misure_vert;
FAB_FRAME* frame_misure_vert;
FAB_FRAME* frame_titolo;
FAB_FRAME* frame_comandi;
FAB_FRAME* frame_control;

// BALLFRAME:
FAB_BALLFRAME* ballframe_radar;
FAB_BALLFRAME* ballframe_lucerossa[3];

// Base ContrAerea
#define BASE_L  1.0   // [Km]: lunghezza
#define BASE_H  2100  // [m]:  altezza max di visibilita'

// Finestra Virtuale sullo schermo
#define FIN_X_MIN  20      // [pixel]
#define FIN_X_MAX  520-1   // [pixel]
#define FIN_Y_MIN  20      // [pixel]
#define FIN_Y_MAX  520-1   // [pixel]
// Inoltre si possono usare le seguenti + comode define
// con gli stessi valori:
#define X0 FIN_X_MIN
#define X1 FIN_X_MAX
#define Y0 FIN_Y_MIN
#define Y1 FIN_Y_MAX
// NB: non e' necessaria la proporzionalita' tra finestra
//     reale(BASE_L*BASE_H) e virtuale((X1-X0)*(Y1-Y0))!

// Finestra Misure
#define FRAME_MISURE_LINE_COLOR FAB_green
#define FRAME_MISURE_BACK_COLOR FAB_black
#define FRAME_MISURE_ORIZ_LY    30
#define FRAME_MISURE_VERT_LX    52

// Finestra Titolo
#define FRAME_TITOLO_TEXT_COLOR1  FAB_yellow
#define FRAME_TITOLO_TEXT_COLOR2  FAB_white
#define FRAME_TITOLO_BACK_COLOR1  FAB_gray
#define FRAME_TITOLO_BACK_COLOR2  FAB_blue

// Finestra Comandi
#define FRAME_COMANDI_TEXT_COLOR1 FAB_yellow
#define FRAME_COMANDI_TEXT_COLOR2 FAB_white
#define FRAME_COMANDI_TEXT_COLOR3 FAB_red
#define FRAME_COMANDI_BACK_COLOR1 FAB_gray
#define FRAME_COMANDI_BACK_COLOR2 FAB_yellow

// Finestra Control
#define FRAME_CONTROL_TEXT_COLOR1 FAB_rgb( 50,255, 50)
#define FRAME_CONTROL_TEXT_COLOR2 FAB_rgb(150,255,150)
#define FRAME_CONTROL_BACK_COLOR  FAB_black

// ---------------------------------
// *** Elementi Task Dummy_radar ***
// ---------------------------------
#define    DUMMY_RADAR_GROUP  3
#define    DUMMY_RADAR_PERIOD 50000
#define    DUMMY_RADAR_WCET   400
#define    DUMMY_RADAR_MET    200

// ---------------------------------
// *** Elementi Task Control ***
// ---------------------------------
#define    CONTROL_PERIOD 50000
#define    CONTROL_WCET   400
#define    CONTROL_MET    300

// ---------------------------
// *** Elementi Task Esplo ***
// ---------------------------
#define    ESPLO_TRASPARENT_COL FAB_white
#define    ESPLO_LX             40
#define    ESPLO_LY             40

#define    ESPLO_PERIOD  80000
#define    ESPLO_WCET      300
#define    ESPLO_MET       100

// -----------------------------------
// *** Elementi Task Aereo_Creator ***
// -----------------------------------
#define    AEREO_CREATOR_PERIOD  200000
#define    AEREO_CREATOR_WCET       300
#define    AEREO_CREATOR_MET        100

// -----------------------------------
// *** Elementi Task Cannone_Creator ***
// -----------------------------------
#define    CANNONE_CREATOR_PERIOD 1000000
#define    CANNONE_CREATOR_WCET       150
#define    CANNONE_CREATOR_MET         50

// ---------------------------
// *** Elementi Task Aereo ***
// ---------------------------
#define    AEREO_N_MAX          10
#define    AEREO_N_MIN          1

#define    AEREO_TRASPARENT_COL FAB_white
#define    AEREO_BASE_COL       FAB_green
#define    AEREO_LX             32
#define    AEREO_LY             19
#define    AEREO_H_MIN  500    // [m]:    altezza min di volo
#define    AEREO_H_MAX  2000   // [m]:    altezza max di volo (<BASE_H)
#define    AEREO_L_MIN  0      // [km]:   posizione 0 nella base
#define    AEREO_L_MAX  BASE_L // [km]    posizione max nella base
#define    AEREO_V_MIN  300.0  // [Km/h]: velocit… min di volo
#define    AEREO_V_MAX  500.0  // [Km/h]: velocit… max di volo
#define    AEREO_X_MIN  X0 - AEREO_LX/2
#define    AEREO_X_MAX  X1 + AEREO_LX/2

#define    AEREO_PERIOD  30000
#define    AEREO_WCET     1250   //Verificato!
#define    AEREO_MET       800

struct { BYTE       status;  //[0=free/1=occupato]
         BYTE       killing; //richiesta uscita se Š 1 altrimenti 0
         PID        pid;
         COLOR      color;
         FAB_IMAGE* image;
         double     vel;     //[km/h]
         int        dir;     //[+1/-1]
         WORD       h;       //[m]
         double     l;       //[km]
         WORD       x, y;    //[pixel]
                         } aereo_table[AEREO_N_MAX];

int                        aereo_count;

// -----------------------------
// *** Elementi Task Cannone ***
// -----------------------------
#define    CANNONE_N_MAX          9
#define    CANNONE_N_MIN          1

#define    CANNONE_SENSIBILITA    200 //[m]

#define    CANNONE_TRASPARENT_COL FAB_white
#define    CANNONE_LX             33
#define    CANNONE_LY             15

#define    CANNONE_PERIOD 100000
#define    CANNONE_WCET     1000
#define    CANNONE_MET       100

struct { BYTE       status;  //[0=free/1=occupato]
         BYTE       killing; //richiesta uscita se Š 1 altrimenti 0
         PID        pid;
         BYTE       fire;
         WORD       x;
         double     missile_vel;
                         } cannone_table[CANNONE_N_MAX];

int                        cannone_count;

// -----------------------------
// *** Elementi Task Missile ***
// -----------------------------
#define    MISSILE_V_MIN            300.0  //[km/h]
#define    MISSILE_V_MAX            5000.0 //[km/h]
#define    MISSILE_ACC_MIN            100000.0    //[km/(h*h)]
#define    MISSILE_ACC_MAX          10000000.0    //[km/(h*h)]
#define    MISSILE_X_MAX            X1 + MISSILE_LX/2 //[pixel]
#define    MISSILE_X_MIN            X0 - MISSILE_LX/2 //[pixel]
#define    MISSILE_Y_MAX            Y1 + MISSILE_LY/2 //[pixel]
#define    MISSILE_Y_MIN            Y0 - MISSILE_LY/2 //[pixel]

#define    MISSILE_TRASPARENT_COL   FAB_white

#define    MISSILE_LX               7 //[pixel]
#define    MISSILE_LY               15 //[pixel]

#define    MISSILE_PERIOD  10000
#define    MISSILE_WCET      500
#define    MISSILE_MET       100





// MUTEX:
mutex_t grx_mutex; //per l'accesso alla grafica
int app_mutex_init(mutex_t *m);



// FUNZIONI GESTIONE RISORSE DI MEMORIA:
void kern_init_bca();

// implemented in esplo.c
TASK esplo(void *argv);

// implemented in missile.c
PID   crea_hard_missile();
PID   crea_soft_missile();
TASK  missile();

// implemented in cannone.c
PID   crea_soft_cannone_creator();
TASK  cannone_creator();
PID   crea_hard_cannone(int index);
PID   crea_soft_cannone(int index);
TASK  cannone(void * index);

// implemented in aereo.c
PID   crea_soft_aereo_creator();
TASK  aereo_creator();
PID   crea_hard_aereo(int index);
PID   crea_soft_aereo(int index);
TASK  aereo(void * index);

// implemented in control.c
PID   crea_soft_control();
TASK  control(void *);

// implemented in dummy.c
PID   crea_soft_dummy_radar ();
TASK  dummy_radar(void *);

// implemented in scenario.c
void   scenario();
void   info();


//------------------END------------------
#endif
