/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
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
 CVS :        $Id: demo.c,v 1.8 2005/01/08 14:35:17 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.8 $
 Last update: $Date: 2005/01/08 14:35:17 $
 ------------
**/

/*
 * Copyright (C) 2000-2003 Paolo Gai
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

#include "demo.h"
#include <kernel/func.h>
#include <string.h>
#include <stdlib.h>

/* useful colors... */
int white;
int black;
int red;
int gray;

void app_mutex_init(mutex_t *m);

static void version( void )
{
  cprintf( "S.Ha.R.K. Jumpball Demo 1.0\n" );
  cprintf( "---------------------------\n" );
  cprintf( "by Paolo Gai 1999-2001\n"   );
  cprintf( "   <pj@sssup.it>\n"         );
  cprintf( "---------------------------\n" );
}

int myrand(int x)
{
  return rand()%x;
}

void reverse(char s[])
{
  int c, i, j;

  for (i = 0, j = strlen(s)-1; i<j; i++, j--)
  {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

char * itoa(int n, char *s)
{
  int i, sign;

  if ((sign = n) < 0)
    n = -n;

  i = 0;

  do
  {
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  if (sign < 0)
    s[i++] = '-';

  s[i] = 0;

  reverse(s);

  return s;
}


void scenario()
{
  grx_text("S.Ha.R.K. Jumpball Demo 1.0", 0, 0, rgb16(0,255,0), black );
  grx_text("  by Paolo Gai 1999-2001"   , 0, 8, rgb16(0,255,0), black );
  grx_text("       pj@sssup.it"         , 0,16, rgb16(0,255,0), black );

  grx_text("Ctrl-C, Ctrr-C, Enter: exit"             ,320, 0, gray, black );
  grx_text("Alt-C                : void statistics"  ,320, 8, gray, black );
  grx_text("Space                : create noise ball",320,16, gray, black );
  grx_text("Backspace            : kill noise balls" ,320,24, gray, black );


  #ifdef JET_ON
  scenario_jetcontrol();
  #endif

  #ifdef BALL_ON
  scenario_ball();
  #endif
}

void endfun(KEY_EVT *k)
{
  exit(0);
}

void zerofun(KEY_EVT *k)
{
  int i;
  for (i=0; i<MAX_PROC; i++) jet_delstat(i);
}

int main(int argc, char **argv)
{

    KEY_EVT k;

    version();

    srand(4);

    k.flag = CNTR_BIT;
    k.scan = KEY_C;
    k.ascii = 'c';
    k.status = KEY_PRESSED;
    keyb_hook(k,endfun,FALSE);
    k.flag = CNTL_BIT;
    k.scan = KEY_C;
    k.ascii = 'c';
    k.status = KEY_PRESSED;
    keyb_hook(k,endfun,FALSE);
    k.flag = ALTL_BIT;
    k.scan = KEY_C;
    k.ascii = 'c';
    k.status = KEY_PRESSED;
    keyb_hook(k,zerofun,FALSE);
    k.flag = 0;
    k.scan = KEY_ENT;
    k.ascii = 13;
    k.status = KEY_PRESSED;
    keyb_hook(k,endfun,FALSE);

    /* init the graphic mutex */
    app_mutex_init(&mutex);

    /* useful colors ... */
    white = rgb16(255,255,255);
    black = rgb16(0,0,0);
    red   = rgb16(255,0,0);
    gray  = rgb16(128,128,128);

    scenario();

    #ifdef JET_ON
    init_jetcontrol();
    #endif

    #ifdef BALL_ON
    init_ball();
    #endif

    group_activate(1);

    return 0;
}


