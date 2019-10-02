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

/* CVS :        $Id: fab_tool.h,v 1.1 2002/10/01 10:25:03 pj Exp $ */

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
 *  file:        FAB_tool.h                         *
 *  libreria:    FAB_LIB                            *
 *  version:     1.0 25/08/2002                     *
 *  creato da:   Fabio CALABRESE                    *
 *                                                  *
 ****************************************************
 *                                                  *
 *  descrizione: tool di funzioni d'utilita'        *
 *                                                  *
 ****************************************************/

#ifndef __FAB_LIB__FAB_tool_h__
#define __FAB_LIB__FAB_tool_h__
//-------------------BEGIN---------------------------

// *** Librerie S.Ha.R.K ***
// *** Librerie Standard C ***
#include <stdlib.h>
#include <math.h>
// *** Librerie FAB ***

// -----------
// Definizioni
//----------------------
//----------------------

// ----------------
// Funzioni & Macro
//-------------------------------------------------
#define FAB_PI                           3.141592654
#define FAB_grad(RADIANTI)               RADIANTI/FAB_PI*180
#define FAB_rad(GRADI)                   GRADI/180.0*FAB_PI
#define FAB_set_ang360(ANGOLO,X,Y)       if ((X)>0) {if((Y)>0) (ANGOLO)=atan((Y)/(double)(X));else if((Y)<0) (ANGOLO)=atan((Y)/(double)(X))+2*FAB_PI;else (ANGOLO)=0;}else if ((X)<0) (ANGOLO)=atan((Y)/(double)(X))+FAB_PI;else /* X==0 */if ((Y)>0) (ANGOLO)=FAB_PI/2;else if ((Y)<0) (ANGOLO)=FAB_PI/2+FAB_PI;else /* Y==0 */ (ANGOLO)=-1; /* X=Y=0 */
#define FAB_set_xy360(X,Y,MODULO,ANGOLO) (X)=(MODULO)*cos(ANGOLO);(Y)=(MODULO)*sin(ANGOLO);

#define FAB_ABS(X)               ((X) > 0 ? (X) : (X)*-1)

#define FAB_sign_rand()          (rand()%2 ? -1 : +1)
#define FAB_int_rand(MIN,MAX )   ((MIN) + rand()%((MAX)-(MIN)+1))
#define FAB_double_rand(MIN,MAX) ((MIN) + ((MAX)-(MIN))*((double)rand()/RAND_MAX))
//--------------------------------------------------


// Altro...

//----------------------END--------------------------
#endif
