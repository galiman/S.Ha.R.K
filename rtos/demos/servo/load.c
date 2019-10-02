/*
 * Project: S.Ha.R.K
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://hartik.sssup.it
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

#include "kernel/kern.h"
#include "drivers/shark_keyb26.h"
#include "unistd.h"

#include "servo.h"

#define N_SEND 10000

int main () {

  int res, i = 0, k;

  srand(sys_gettime(NULL));

  cprintf("(Open Com2)");
  res = servo_open(COM2, 19200);
  cprintf("(Res = %d)",res);

  cprintf("(Servo Turn On 0)");
  res = servo_turn_on(COM2, 0);
  cprintf("(Res = %d)",res);

  while(i < N_SEND) {

    cprintf("(Cycle = %d)",i);

    k = rand() % 180 - 90;

    cprintf("(Set Servo 0 %d)",k);
    res = servo_set_angle_sec(COM2, 0, ANGLE2SEC(k,0,0));
    cprintf("(Res = %d)",res);

    udelay(100000);

    i++;

  }

  servo_close(COM2);

  exit(0);

  return 0;

}

