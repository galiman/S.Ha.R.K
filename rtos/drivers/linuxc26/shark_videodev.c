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

/* Glue Layer for VideoDev 2.6 Driver */
    
#include <kernel/kern.h>

extern int videodev_open_inode(int num);
extern int videodev_ioctl_inode(int num,unsigned int cmd,unsigned long arg);

/* Open Videodev */
int VIDEODEV26_open(int num) {

  if (videodev_open_inode(num)) {
    printk("VIDEODEV26 ERROR: Cannot open the VideoDev driver\n");
    return 1;
  }

  return 0;

}

extern int BTTV26_installed(void);

int VIDEODEV26_ioctl(int num,unsigned int cmd,unsigned long arg) {

  return videodev_ioctl_inode(num,cmd,arg);

}
