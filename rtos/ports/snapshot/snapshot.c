/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
 *   (see the web pages for full authors list)
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
 */

#include "kernel/kern.h"
#include "string.h"
#include "drivers/shark_fb26.h"
#include "snapshot.h"

struct SNAP_SLOT{
	char used;
	int  wx;
	int  wy;
	int  bytesperpixel;
	void *buff;
} sslot[16] = {{0, 0, 0, 0, NULL},
	       {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
	       {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL},
               {0, 0, 0, 0, NULL}};

void *snapshot_getslot(int nbuff, int wx, int wy, int bytesperpixel)
{
	void* tmp;

	if ((wx<=0) || (wy<=0) || (nbuff<0) || (nbuff>15)){
		return NULL;
	}

	if (sslot[nbuff].used) 
		return NULL;

	/* Allocate memory. */
	tmp = (void *)kern_alloc(wx * wy * bytesperpixel);
	if (!tmp) return NULL;

	sslot[nbuff].used = 1;
	sslot[nbuff].wx = wx;
	sslot[nbuff].wy = wy;
        sslot[nbuff].bytesperpixel = bytesperpixel;
	sslot[nbuff].buff = tmp;
	return tmp;
}

void snapshot_freeslot(int nbuff)
{
        if (sslot[nbuff].used == 0) return;
	kern_free(sslot[nbuff].buff, sslot[nbuff].wx * sslot[nbuff].wy * sslot[nbuff].bytesperpixel);
	sslot[nbuff].used = 0;
}

void snapshot_grab(int nbuff)
{
	extern void *video_memory;

	memcpy(sslot[nbuff].buff,video_memory,sslot[nbuff].wx * sslot[nbuff].wy * sslot[nbuff].bytesperpixel);

}

int snapshot_save_pgm(int nbuff, char *fname)
{
	char out_st[40];
	register int i;
	unsigned char tmp[3];
	DOS_FILE *outf;

	if ((outf = DOS_fopen(fname, "w")) == NULL) {
		return -1;
	} else {
		sprintf(out_st, "P5\n%d %d\n%d\n", sslot[nbuff].wx, sslot[nbuff].wy, 255);
		DOS_fwrite(out_st, strlen(out_st), 1, outf);
		cprintf(out_st);

		for (i = 0; i < sslot[nbuff].wy * sslot[nbuff].wx; i++) {
			tmp[0] = *(unsigned char *)(sslot[nbuff].buff + i);
			tmp[1] = ' ';
			tmp[2] = 0;
			DOS_fwrite(&tmp, 1, 1, outf);
		}
		DOS_fclose(outf);
	}
	return 0;
}

int snapshot_save_ppm(int nbuff, char *fname)
{
	char out_st[40];
	register int i;
	int col_r, col_g, col_b;
	DOS_FILE *outf;

	if ((outf = DOS_fopen(fname, "w")) == NULL) {
		return -1;
	} else {
		sprintf(out_st, "P3\n%d %d\n%d\n", sslot[nbuff].wx, sslot[nbuff].wy, 255);
		DOS_fwrite(out_st, strlen(out_st), 1, outf);
		cprintf(out_st);

		for (i = 0; i < sslot[nbuff].wx * sslot[nbuff].wy; i++) {
			col_r = ((*(int *)(sslot[nbuff].buff + i*2) & 0xF800) >> 11) << 3;
			col_g = ((*(int *)(sslot[nbuff].buff + i*2) & 0x07E0) >>  5) << 2;
			col_b =  (*(int *)(sslot[nbuff].buff + i*2) & 0x001F) << 3;
			sprintf(out_st, "%d %d %d  ", col_r, col_g, col_b);
			DOS_fwrite(out_st, strlen(out_st), 1, outf);
		}
		DOS_fclose(outf);
	}
	return 0;
}
