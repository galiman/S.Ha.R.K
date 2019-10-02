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
 CVS :        $Id: crtwin.c,v 1.1 2004/03/29 18:31:40 mauro Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2004/03/29 18:31:40 $
 ------------

 Author:	Gerardo Lamastra
 Date:	9/5/96

 File: 	CrtWin.C
 Revision:	1.1g

 Text windowing functions for S.Ha.R.K.

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

//#include <string.h>
//#include <cons.h>

#include <kernel/kern.h>
#include <ll/string.h>
#include <drivers/crtwin.h>

#define CUPS1	201        /* Definizione caratteri per */
#define LOR1	205        /* La cornice delle varie    */
#define	CUPD1	187        /* Finestre                  */
#define	LVR1	186
#define	CDWS1	200
#define	CDWD1	188

#define	CUPS2	218
#define	LOR2	196
#define	CUPD2	191
#define	LVR2	179
#define	CDWS2	192
#define	CDWD2	217

void win_init(WIN *w, int x, int y, int dx, int dy)
{
    w->x = x;
    w->y = y;
    w->dx = dx;
    w->dy = dy;

    w->px = 0;
    w->py = 0;
}

void win_clear(WIN *w)
{
    _clear(' ',WHITE,w->x+1,w->y+1,w->x+w->dx-1,w->y+w->dy-1);
}

void win_scroll(WIN *w)
{
    _scroll(WHITE,w->x+1,w->y+1,w->x+w->dx-1,w->y+w->dy-1);
}

void win_frame(WIN *w,BYTE bcol,BYTE fcol,char *title,BYTE type)
{
    unsigned char var,var2,len;
    register int i;
    char cus,lv,cud,lo,cds,cdd,cl = ((bcol << 4) | fcol);
    len = strlen(title);
    switch(type) {
	       case 1 : cus = CUPS1; lv = LVR1;   cud = CUPD1;
		        lo = LOR1;   cds = CDWS1; cdd = CDWD1;
		        break;
	       case 2 : cus = CUPS2; lv = LVR2;   cud = CUPD2;
	  		lo = LOR2;   cds = CDWS2; cdd = CDWD2;
		        break;
	       default : cus = lv = cud = lo = cds = cdd = 32;
		         break;
    }
    for (i = w->x+1; i < w->x+w->dx; i++) {
	putc_xy(i,w->y,cl,lo);
	putc_xy(i,w->y+w->dy,cl,lo);
    }
    putc_xy(w->x+w->dx,w->y,cl,cud);
    putc_xy(w->x,w->y,cl,cus);
    
    if (title != NULL) {
	var2 = w->x+(w->dx/2) - len/2 - 1;
	var = 0;
	putc_xy(var2,w->y,cl,'[');
	puts_xy(var2+1,w->y,cl,title);
	putc_xy(var2+len+1,w->y,cl,']');
    }
    
    for (i = w->y+1; i < w->y+w->dy; i++) {
	putc_xy(w->x,i,cl,lv);
	putc_xy(w->x+w->dx,i,cl,lv);
    }
    putc_xy(w->x,w->y+w->dy,cl,cds);
    putc_xy(w->x+w->dx,w->y+w->dy,cl,cdd);
}

void win_puts(WIN *w1, char *s)
{
    unsigned pos;
    char c;
    static unsigned short scan_x,x,y;
    WIN w;

    w = *w1;
    x = w.px + w.x + 1;
    y = w.py + w.y + 1;
    
    pos = 2*(y*80 + x);
    while (*s != '\0') {
	c = *s++;
	switch (c) {
	    case '\t' : x += 8;
    		        if (x >= w.x+w.dx) {
			    x = w.x+1;
			    if (y == w.y+w.dy-1) win_scroll(&w);
			    else y++;
    		        } else {
			    scan_x = w.x+1;
			    while ((scan_x+8) < x) scan_x += 8;
			    x = scan_x;
			}
			pos = 2*(x + 80*y);
		        break;
	    case '\n' : y += (x - w.x - 1) / w.dx + 1;
			x = w.x + 1;
			while (y > (w.y+w.dy-1)) {
			    win_scroll(&w);
	    		    y--;
			}
			pos = 2*(x + 80*y);
	    		break;
	    case '\b' : putc_xy(x-1,y,WHITE,' '); 
	    		break;
	    default   : putc_xy(x,y,WHITE,c);
			pos += 2;
			x++;
			if (x >= w.x + w.dx) {
			    x = w.x+1;
			    if (y >= (w.y + w.dy - 1)) win_scroll(&w);
			    else y++;
			    pos = 2*(x + 80*y);
			}
	}
    }
    if (x > (w.x+w.dx-1)) {
	y += (x - w.x - 1) / w.dx;
	x = (x - w.x - 1) % w.dx + w.x + 1;
	while (y >= (w.y+w.dy)) {
	    win_scroll(&w);
	    y--;
	}
    }
    w1->px = x - w.x - 1;
    w1->py = y - w.y - 1;
}

/*int win_gets(WIN *w, char *s,int len)
{
    int strpos = 0,lungh = 0;
    int x,y,tx,ty,i;
    BYTE flag = FALSE;
    KEY_EVT c;
    
    x = w->px + w->x + 1;
    y = w->py + w->y + 1;
    s[0] = 0;
    place(x,y);

    while (!flag) {
	keyb_getcode(&c,BLOCK);
	if (isScanCode(c) && (c.scan == LEFT_KEY) && (strpos > 0)) {
	    strpos--;
	    x--;
	    if (x <= w->x) {
		x = w->x + w->dx - 1;
		y--;
		if (y <= w->y) y = w->y + 1;
	    }
	    place(x,y);
	}
	else if (isScanCode(c) && (c.scan == RIGHT_KEY) && s[strpos] != 0) {
	    strpos++;
	    x++;
	    if (x >= (w->x + w->dx)) {
		x = w->x + 1;
	        y++;
		if (y >= (w->y + w->dy)) {
		    y--;
		    win_scroll(w);
		}
	    }
	    place(x,y);
	}
	switch (c.ascii) {
	    case ENTER :
	    	if (!isScanCode(c)) flag = TRUE;
	    	break;
	    case BACKSPACE :
	        if (!isScanCode(c) && (strpos > 0)) {
		    strpos--;
		    x--;
		    if (x <= w->x) {
			x = w->x + w->dx - 1;
			y --;
			if (y <= w->y) y = w->y + 1;
		    }
		    tx = x; ty = y;
		    i = strpos;
		    while (s[i] != 0) {
			s[i] = s[i+1];
			putc_xy(tx,ty,WHITE,s[i]);
			i++;
			tx++;
			if (tx >= w->x + w->dx) {
			    tx = w->x + 1;
			    ty++;
			}
		    }
		    lungh--;
		}
		place(x,y);
		break;
	    case DELETE :
	        if (s[strpos] != 0) lungh--;
		tx = x; ty = y;
		i = strpos;
		while (s[i] != 0) {
		    s[i] = s[i+1];
		    putc_xy(tx,ty,WHITE,s[i]);
		    i++;
		    tx++;
		    if (tx >= w->x + w->dx) {
			tx = w->x;
			ty++;
		    }
		}
		place(x,y);
		break;
	    default :
	        if (!isScanCode(c)) {
		    i = lungh;
		    while (i > strpos) {
			s[i] = s[i-1];
			i--;
		    }
		    s[lungh+1] = 0;
		    s[strpos] = c.ascii;
		    strpos++;
		    lungh++;
		    if (lungh == (len - 1)) flag = TRUE;
		    tx = x;
		    ty = y;
		    i = strpos - 1;
		    while (s[i] != 0) {
			putc_xy(tx,ty,WHITE,s[i]);
			tx++;
			i++;
		    	if (tx >= (w->x + w->dx)) {
			    tx = w->x + 1;
	            	    ty++;
		    	    if (ty >= (w->y + w->dy)) {
			        ty--;
				y--;
			        win_scroll(w);
		            }
		        }
		    }
		    x++;
		    if (x >= (w->x + w->dx)) {
		    	x = w->x + 1;
	            	y++;
		    }
		}
		place(x,y);
		break;
	}

    }
    w->px = 0;
    w->py = y - w->y;
    if (w->py >= w->dy - 1) {
	w->py--;
	win_scroll(w);
    }
    return(lungh);
}

static void cons_end(void *arg)
{
    set_active_page(0);
    set_visual_page(0);
}

static void cons_change(KEY_EVT *k)
{
  //k.scan -= KEY_1;
  set_visual_page(k->scan-KEY_1);
}
	    
void screen_init(void)
{
    KEY_EVT k;
    int i;

    k.flag = ALTR_BIT;
    for (i = 0; i < 8; i++) {
	k.ascii = '1' + i;
    	k.scan = KEY_1 + i;
	keyb_hook(k,cons_change);
    }
    sys_atrunlevel(cons_end,NULL,RUNLEVEL_AFTER_EXIT);
}*/
