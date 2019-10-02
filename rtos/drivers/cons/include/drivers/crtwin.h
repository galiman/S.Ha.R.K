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
 CVS :        $Id: crtwin.h,v 1.1 2004/03/29 18:31:40 mauro Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2004/03/29 18:31:40 $
 ------------

 Author:      Giuseppe Lipari
 Date:        2/7/96

 File: 	CrtWin.H
 Revision:    1.0


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

#ifndef __CRTWIN_H__
#define __CRTWIN_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    /* Window location & size */
    int x,y,dx,dy;      
    /* Cursor position inside the window */
    int px,py;
} WIN;

/*
   The window layout is depicted here
   
      (x,y)                      (x+dx,y)
	  ************************
	  *(0,0)                 *
	  *                      *
	  *                      *
	  *           (dx-1,dy-1)*
	  ************************
				 (x+dx,y+dy)

    Inside the window relative coords are used!
*/

#define SINGLE_FRAME	1
#define DOUBLE_FRAME	2
#define NO_FRAME	0

void win_init(WIN *w,int x, int y, int dx, int dy);
void win_frame(WIN *w,BYTE bcol,BYTE fcol,char *title,BYTE type);
void win_scroll(WIN *w);
void win_clear(WIN *w);
void win_puts(WIN *w, char *s);
int win_gets(WIN *w, char *s,int l);

//void cons_init(void);

#ifdef __cplusplus
};
#endif

#endif
