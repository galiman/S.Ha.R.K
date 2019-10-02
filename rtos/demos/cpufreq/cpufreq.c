
/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: Giorgio Buttazzo <giorgio@sssup.it>
 *
 * Authors     : Mauro Marinoni
 *		 Giacomo Guidi    <giacomo@gandalf.sssup.it>
 * (see authors.txt for full list of hartik's authors)
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

#include <kernel/kern.h>

#include <drivers/shark_keyb26.h>
#include <drivers/shark_cpu26.h>

void endfunc(KEY_EVT *e)
{
  exit(0);
}

int main(int argc, char **argv)
{
	int val, num;
	char buf[1000];

	CPU26_showinfo();
	
	val = CPU26_get_min_frequency();
	cprintf("Min freq: %d\n", val);

	val = CPU26_get_max_frequency();
	cprintf("Max freq: %d\n", val);

	val = CPU26_get_cur_frequency();
	cprintf("Current freq: %d\n", val);

	num = CPU26_show_frequencies(buf);
	if (num>0)
		cprintf("Allowed frequencys: %s\n", buf);
	num = CPU26_get_frequencies(cpu26_freqs);

	if (num>0) {
		CPU26_set_frequency(cpu26_freqs[0], DVS_RELATION_L);
		val = CPU26_get_cur_frequency();
		cprintf("Actual freq: %d\n", val);
	}

	endfunc(NULL);

	return 0;
}
