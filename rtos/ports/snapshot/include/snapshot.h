
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
#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

void* snapshot_getslot(int nbuff, int wx, int wy, int bytesperpixel);
void  snapshot_freeslot(int nbuff);
void  snapshot_grab(int nbuff);
int   snapshot_save_pgm(int nbuff, char *fname);
int   snapshot_save_ppm(int nbuff, char *fname);

#endif

