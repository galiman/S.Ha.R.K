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

/* CVS :        $Id: scenario.c,v 1.3 2006/07/03 15:10:15 tullio Exp $ */

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

#include "bca.h"

char * titolo[10];

void scenario_ca()
{   //SFONDO
    FAB_image_fill(image_sfondo,0,0,800,600);

    //FRAME_BCA
    FAB_frame_put(frame_bca,X0-7,Y0-7,500+7*2,500+7*2);
}
void scenario_misure()
{   int   x, y, cx, cy;
    char* str_km ="X.Xkm";
    char* str_m  ="XXXXm";
    int   h;

     //FRAME_MISURE orizzontale
    FAB_frame_put(frame_misure_oriz, X0-7, Y1+7+20-7,
                  500+7*2, FRAME_MISURE_ORIZ_LY+7*2);
    cy = frame_misure_oriz->y0 + 10;
    cx = frame_misure_oriz->x0 +
         (frame_misure_oriz->x1 - frame_misure_oriz->x0) / 2;
    for(x=frame_misure_oriz->x0 ;x<frame_misure_oriz->x1+1 ;x+=5)
        grx_line(x,cy,x+2,cy,FRAME_MISURE_LINE_COLOR);
    for(y=frame_misure_oriz->y0; y<cy-1; y+=5) {
      grx_line(frame_misure_oriz->x0,y,frame_misure_oriz->x0,y+2,FRAME_MISURE_LINE_COLOR);
      grx_line(cx,y,cx,y+2,FRAME_MISURE_LINE_COLOR);
      grx_line(frame_misure_oriz->x1,y,frame_misure_oriz->x1,y+2,FRAME_MISURE_LINE_COLOR);
      }
    sprintf(str_km,"%02.1fkm",0.0);
    grx_text(str_km,frame_misure_oriz->x0+10,cy+2,FRAME_MISURE_LINE_COLOR,frame_misure_oriz->color);
    sprintf(str_km,"%02.1fkm",BASE_L/2.0);
    grx_text(str_km,cx-10,cy+2,FRAME_MISURE_LINE_COLOR,frame_misure_oriz->color);
    sprintf(str_km,"%02.1fkm",BASE_L);
    grx_text(str_km,frame_misure_oriz->x1-FRAME_MISURE_ORIZ_LY,cy+2,FRAME_MISURE_LINE_COLOR,frame_misure_oriz->color);

   //FRAME_MISURE verticale
   FAB_frame_put(frame_misure_vert, X1+7+20-7, Y0-7,
                 FRAME_MISURE_VERT_LX+7*2, 500+7*2);
   cy = frame_misure_vert->y0 +
        (frame_misure_vert->y1 - frame_misure_vert->y0) / 2;
   cx = frame_misure_vert->x0 + 10;
   for(y=frame_misure_vert->y1; y>frame_misure_vert->y0-1; y-=5)
     grx_line(cx,y,cx,y-2,FRAME_MISURE_LINE_COLOR);
   h=0;
   for(y=frame_misure_vert->y1; y>frame_misure_vert->y0-1;
       y-=500.0*(X1-X0)/BASE_H, h+=500) {
     for(x=frame_misure_vert->x0; x<cx-1; x+=5)
        grx_line(x,y,x+2,y,FRAME_MISURE_LINE_COLOR);
     sprintf(str_m,"%dm",h);
     grx_text(str_m, cx+2, y-6,
              FRAME_MISURE_LINE_COLOR, frame_misure_vert->color);
     }
}
void scenario_radar()
{  FAB_ballframe_put(ballframe_radar,
                     frame_bca->x1+7+30, frame_bca->y1+7+30, 30);
   FAB_ballframe_put(ballframe_lucerossa[2],605,546,20);
   FAB_ballframe_put(ballframe_lucerossa[1],594,578,15);
   FAB_ballframe_put(ballframe_lucerossa[0],574,591,10);
}
void scenario_titolo()
{  int x0,y0;
   x0 = frame_misure_vert->x1+7+20-7;
   y0 = frame_bca->y0-7;
   FAB_frame_put(frame_titolo,x0,y0,800-x0-20+7,170);
   x0 = frame_titolo->x0;
   y0 = frame_titolo->y0;
   grx_text(titolo[0],x0+2,y0+10,FRAME_TITOLO_TEXT_COLOR2,FRAME_TITOLO_BACK_COLOR1);
   grx_text(titolo[1],x0+2,y0+20,FRAME_TITOLO_TEXT_COLOR2,FRAME_TITOLO_BACK_COLOR1);
   grx_text(titolo[2],x0+2,y0+30,FRAME_TITOLO_TEXT_COLOR2,FRAME_TITOLO_BACK_COLOR1);
   grx_text(titolo[3],x0+2,y0+46,FRAME_TITOLO_TEXT_COLOR2,FRAME_TITOLO_BACK_COLOR1);
   grx_text(titolo[4],x0+2,y0+66,FRAME_TITOLO_TEXT_COLOR1,FRAME_TITOLO_BACK_COLOR2);
   grx_text(titolo[5],x0+2,y0+81,FRAME_TITOLO_TEXT_COLOR2,FRAME_TITOLO_BACK_COLOR1);
   grx_text(titolo[6],x0+2,y0+96,FRAME_TITOLO_TEXT_COLOR2,FRAME_TITOLO_BACK_COLOR1);
   grx_text(titolo[7],x0+2,y0+116,FRAME_TITOLO_TEXT_COLOR2,FRAME_TITOLO_BACK_COLOR1);
   grx_text(titolo[8],x0+2,y0+133,FRAME_TITOLO_TEXT_COLOR2,FRAME_TITOLO_BACK_COLOR1);
   grx_text(titolo[9],x0+2,y0+141,FRAME_TITOLO_TEXT_COLOR2,FRAME_TITOLO_BACK_COLOR1);
}
void scenario_comandi()
{  int x0, y0;
   x0 = frame_misure_vert->x1+7+20-7;
   y0 = frame_titolo->y1+7+20-7;
   FAB_frame_put(frame_comandi,x0,y0,800-x0-20+7,100+7*2);
   x0 = frame_comandi->x0;
   y0 = frame_comandi->y0;
   grx_text("      COMMANDS     \0",x0+2,y0+10,FRAME_COMANDI_TEXT_COLOR1,FRAME_COMANDI_BACK_COLOR1);
   grx_text("-------------------\0",x0+2,y0+20,FRAME_COMANDI_TEXT_COLOR2,FRAME_COMANDI_BACK_COLOR1);
   grx_text("         exit      \0",x0+2,y0+40,FRAME_COMANDI_TEXT_COLOR2,FRAME_COMANDI_BACK_COLOR1);
   grx_text("[Ctrl+C]",x0+2,y0+40,FRAME_COMANDI_TEXT_COLOR3,FRAME_COMANDI_BACK_COLOR2);
   grx_text("        +/- plane  \0",x0+2,y0+60,FRAME_COMANDI_TEXT_COLOR2,FRAME_COMANDI_BACK_COLOR1);
   grx_text("[1/2]",x0+2,y0+60,FRAME_COMANDI_TEXT_COLOR3,FRAME_COMANDI_BACK_COLOR2);
   grx_text("        +/- cannon \0",x0+2,y0+80,FRAME_COMANDI_TEXT_COLOR2,FRAME_COMANDI_BACK_COLOR1);
   grx_text("[3/4]",x0+2,y0+80,FRAME_COMANDI_TEXT_COLOR3,FRAME_COMANDI_BACK_COLOR2);
}
void scenario_control()
{  int x0,y0;

   x0 = frame_misure_vert->x1+7+20-7;
   y0 = frame_comandi->y1+7+20-7;
   FAB_frame_put(frame_control,x0,y0,800-x0-20+7,frame_misure_vert->y1+7-y0);
   x0 = frame_control->x0;
   y0 = frame_control->y0;
   grx_text("      CONTROL      \0",x0+2, y0+5,FRAME_CONTROL_TEXT_COLOR2,FRAME_CONTROL_BACK_COLOR);
   grx_line(x0-3, y0+15,
            frame_control->x1+3, y0+15,
            FRAME_CONTROL_TEXT_COLOR1);

   grx_line(x0-3, frame_control->y1-81,
            frame_control->x1+3, frame_control->y1-81,
            FRAME_CONTROL_TEXT_COLOR1);
}
void scenario()
{   scenario_ca();
    scenario_misure();
    scenario_radar();
    scenario_titolo();
    scenario_comandi();
    scenario_control();
}

void info()
{ int i;
  for(i=0; i<10; i++) kern_printf("%s\n",titolo[i]);
}
