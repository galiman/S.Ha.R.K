/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
 *
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
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

/* Game Header Linux Input Driver*/

#ifndef __SHARK_JOY26_H__
#define __SHARK_JOY26_H__

/* Flag Codes */
#define BTN1_BIT	0x01
#define BTN2_BIT	0x02
#define BTN3_BIT	0x04
#define BTN4_BIT	0x08

#define isButton1Pressed(b)	(b & BTN1_BIT)
#define isButton2Pressed(b)	(b & BTN2_BIT)
#define isButton3Pressed(b)	(b & BTN3_BIT)
#define isButton4Pressed(b)	(b & BTN4_BIT)
#define isButtonPressed(b, n)	(b & (1<<n))

void joy_getstatus(int *axe0, int *axe1, int *axe2, int *axe3, int *button);
void joy_setstatus(int axe0, int axe1, int axe2, int axe3, int button);

void joy_enable(void);
void joy_disable(void);

int JOY26_installed(void);
int JOY26_init(void);
int JOY26_close(void);
#endif

