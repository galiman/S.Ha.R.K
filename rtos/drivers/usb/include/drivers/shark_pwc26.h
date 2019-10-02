
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

#ifndef __SHARK_PWC26_H__
#define __SHARK_PWC26_H__

struct PWC26_DEVICE
{
	void* private_data;
	int width;
	int height;
	BYTE* imgptr;
};

void shark_PWC_init();
int shark_PWC_read(struct PWC26_DEVICE *pwc26);
int shark_PWC_open(struct PWC26_DEVICE *pwc26, int width, int height, int fps, int quality, int webcamnr);
void shark_PWC_close(struct PWC26_DEVICE *pwc26);

#endif
