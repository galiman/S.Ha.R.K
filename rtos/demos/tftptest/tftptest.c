
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

#include <string.h>
#include <kernel/func.h>
#include <kernel/kern.h>

#include <drivers/shark_keyb26.h>
#include <drivers/udpip.h>

#include "tftp.h"
#include "endian.h"

#include "sem/sem/sem.h"

#include <kernel/kern.h>
#include <ll/i386/hw-instr.h>


#define LOCAL_HOST_IP  "192.168.0.134"
#define REMOTE_HOST_IP "192.168.0.133"

sem_t m1, m2;
int handle, handle2;

int prog = 0;

/* This function is called when the user presses CTRL-C (stops the systems) */

void esci(KEY_EVT *k)
{
  sys_shutdown_message("Exit from the program...\n");
  sys_shutdown_message("Ctrl-C pressed!\n");
  exit(1);
}

void augprog(KEY_EVT *k)
{
  prog = 1;
}

void keyb_start(void) {
  KEY_EVT k;

  k.flag = CNTL_BIT;
  k.scan = KEY_C;
  k.status = KEY_PRESSED;
  k.ascii = 'c';
  keyb_hook(k, esci,FALSE);
  k.flag = CNTR_BIT;
  k.scan = KEY_C;
  k.ascii = 'c';
  k.status = KEY_PRESSED;
  keyb_hook(k, esci,FALSE);

  k.flag = CNTL_BIT;
  k.scan = KEY_A;
  k.ascii = 'a';
  k.status = KEY_PRESSED;
  keyb_hook(k, augprog,FALSE);
  k.flag = CNTR_BIT;
  k.scan = KEY_A;
  k.ascii = 'a';
  k.status = KEY_PRESSED;
  keyb_hook(k, augprog,FALSE);
}

TASK test_upload(void *arg) {
  int i;
  char msg[200];
  int bytes;

  i = 0;
  while (1) {

//    cprintf("uploader 1\n");

    sprintf(msg, "tftptest says: i = %5d\n", i);
    tftp_put(handle, msg, strlen(msg));

//    cprintf("uploader 2\n");

    bytes = tftp_usedbuffer(handle);
    /*!*/sprintf(msg, "buffer %5d   ", bytes);
    /*!*/puts_xy(BASE_X, 18, WHITE, msg);

//    cprintf("uploader 3\n");
    i++;

    task_endcycle();
  }
  return(0);
}

TASK test_upload2(void *arg) {
  int i;
  char msg[200];
  int bytes;

  i = 0;
  while (1) {

    sprintf(msg, "tftptest says: i = %5d\n", i);
    tftp_put(handle2, msg, strlen(msg));

    bytes = tftp_usedbuffer(handle2);
    /*!*/sprintf(msg, "buffer2 %5d   ", bytes);
    /*!*/puts_xy(BASE_X, 38, WHITE, msg);
    i++;

    task_endcycle();
  }
  return(0);
}

void wait() {
  char ch;
  do {
    ch = keyb_getch(NON_BLOCK);
  } while(prog == 0);
  prog = 0;
}

int main(void)
{
  int err;
  HARD_TASK_MODEL hard_m;
  PID p1, p2;

  keyb_start();
  cprintf("main: Keyboard handler started\n");

  tftp_init();
  cprintf("main: Tftp library initialized\n");

  err = tftp_net_start(LOCAL_HOST_IP, REMOTE_HOST_IP, 1);
  cprintf("netval = %d\n", err);
  if (err == 1) {
    cprintf("Net Init from %s to %s\n", LOCAL_HOST_IP, REMOTE_HOST_IP);
  } else {
    cprintf("Net Init Failed...\n");
    exit(1);
  }

  sem_init(&m1, 0, 1);


  if ((handle = tftp_open("test.txt")) == -1) {
     cprintf("No slots available. Program aborted...\n");
     exit(1);
  }

  cprintf("NOTE: if you use a Linux TFTP server, remember that the two files\n");
  cprintf("      to be written MUST exists on the server,\n");
  cprintf("      and must have the right permissions\n");
  cprintf("Ctrl-A to proceed *** Ctrl-C to stop\n");
  wait();
  clear();

  cprintf("Handle = %d\n", handle);

  if ((err = tftp_upload(handle, 4096, &m1)) != 0) {
     cprintf("Error %d calling tftp_upload(). Program aborted...\n", err);
     exit(1);
  }

  /* First we set the sender's task properties...*/
  hard_task_default_model(hard_m);
  hard_task_def_wcet(hard_m, 10000);
  hard_task_def_mit(hard_m, 300000);

  if ((p1 = task_create("test_upload", test_upload, &hard_m, NULL)) == NIL) {
     cprintf("Error creating test_upload task. Program aborted...\n");
     exit(1);
  }
  if (task_activate(p1) == -1) {
     cprintf("Error activating test_upload task. Program aborted...\n");
     exit(1);
  }


  sem_init(&m2, 0, 1);

  if ((handle2 = tftp_open("test2.txt")) == -1) {
     cprintf("No second slot available. Program aborted...\n");
     exit(1);
  }



  cprintf("Handle2 = %d\n", handle2);

  if ((err = tftp_upload(handle2, 4096, &m2)) != 0) {
     cprintf("Error %d calling tftp_upload(). Program aborted...\n", err);
     exit(1);
  }

  /* First we set the sender's task properties...*/
  hard_task_default_model(hard_m);
  hard_task_def_wcet(hard_m, 10000);
  hard_task_def_mit(hard_m, 300000);

  if ((p2 = task_create("test_upload2", test_upload2, &hard_m, NULL)) == NIL) {
     cprintf("Error creating test_upload2 task. Program aborted...\n");
     exit(1);
  }
  if (task_activate(p2) == -1) {
     cprintf("Error activating test_upload2 task. Program aborted...\n");
     exit(1);
  }


  wait();

  tftp_close(handle, TFTP_STOP_NOW);

  tftp_close(handle2, TFTP_STOP_NOW);

  cprintf("\nProgram terminated correctly.\n");

  exit(0);

  return(0);
}
