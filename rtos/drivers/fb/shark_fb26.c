/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
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

/* Glue Layer for Frame Buffer 2.6 Driver */
    
#include <kernel/kern.h>

extern void fbmem_init(void);
extern int fb_open_inode(int num);
extern int fb_close_inode(int num);
extern int fb_set_mode_inode(int num, unsigned char *modeopt);

int FB_installed = FALSE;

/* Init the Linux PCI 2.6 Driver */
int FB26_init() {

  fbmem_init();

  return 0;

}

int FB26_open(int num) {

  if (fb_open_inode(num)) {
    printk("FB ERROR: Cannot open the FrameBuffer driver\n");
    return -1;
  }

  FB_installed = TRUE;

  return 0;

}

int FB26_setmode(int num, unsigned char *modeopt) {

  if (FB_installed == FALSE) return -1;

  if (fb_set_mode_inode(num,modeopt)) {
    printk("FB ERROR: Cannot set mode for FrameBuffer driver\n");
    return -1;
  }

  return 0;

}

int FB26_close(int num) {

  if (FB_installed == FALSE) return -1;

  if (fb_close_inode(num)) {
    printk("FB ERROR: Cannot close the FrameBuffer driver\n");
    return -1;
  } 

  return 0;

}
