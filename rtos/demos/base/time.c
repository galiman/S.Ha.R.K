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
 *
 * CVS :        $Id: time.c,v 1.2 2005/01/08 14:31:38 pj Exp $
 *
 * Timer correctness test
 */

#include "kernel/kern.h"

#define NT 10

int main(int argc, char **argv)
{
  struct timespec t[NT];
  int i;

  cprintf("Timer correctness test (1 second).\n");

  for (i=0; i<NT; i++) NULL_TIMESPEC(&t[i]);

  do {
    for (i=0; i<NT-1; i++) t[i+1] = t[i];

    sys_gettime(&t[0]);

    if (TIMESPEC_A_LT_B(&t[0],&t[1])) {
      for (i=0; i<NT; i++)
	cprintf("%d %ld\n",i, t[i].tv_nsec);
      exit(0);
    }
  } while (t[0].tv_sec < 1);

  return 0;
}

