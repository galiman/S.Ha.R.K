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

/**
 ------------
 CVS :        $Id: dosfs.c,v 1.1.1.1 2002/09/02 09:37:41 pj Exp $

 File:        $File$
 Revision:    $Revision: 1.1.1.1 $
 Last update: $Date: 2002/09/02 09:37:41 $
 ------------
**/

/*
 * Copyright (C) 2000 Paolo Gai and Gabriele Bolognini
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
 */

#include "kernel/kern.h"
#include "string.h"
#include "ll/i386/x-dos.h"


/* This is the buffer used by read_myfile */
char myfilebuf[1000];

/* This is the number of bytes read by read_myfile */
int myfilebuf_length;

/* This function write myfile.out (up to 30 chars) */
void write_myfile(void *arg);

int main(int argc, char **argv)
{
  int x;

  cprintf("\nBuffer contents:\n");

  for (x=0; x<myfilebuf_length; x++)
    cputc(myfilebuf[x]);

  /*
     WARNING: You can call these functions only when you are in
              real mode!!!

     The system is in real mode:
     - into __kernel_register_levels__()
     - into sys_atrunlevel functions posted with RUNLEVEL_AFTER_EXIT
  */

  if (myfilebuf_length)
    sys_atrunlevel(write_myfile, NULL, RUNLEVEL_AFTER_EXIT);

  return 0;
  /*
   * Since there is only one user task, the kernel will shut down after this
   * task finishes...
   */
}

/* This function read myfile.txt (up to 1000 chars) */
void read_myfile(void)
{
  /* DOS file descriptor */
  DOS_FILE *f;

  /* Error code */
  int err;

  /* open the DOS file for reading  (you can specify only "r" or "w") */
  f = DOS_fopen("myfile.txt","r");

  /* check for open errors */
  if (!f) {
    /* error!! */
    err = DOS_error();

    /* note that if you call DOS_error() here, it return 0!!! */
    cprintf("Error %d opening myfile.txt...\n", err);
    myfilebuf_length = 0;
    return;
  }

  /* read up to 1000 chars */
  myfilebuf_length = DOS_fread(&myfilebuf,1,1000,f);

  /* check for errors */
  err = DOS_error();

  cprintf("Read %d bytes from myfile.txt...\n", myfilebuf_length);

  if (err) {
    cprintf("Error %d reading myfile.txt...\n", err);
    myfilebuf_length = 0;
    /* there is not return because I want to close the file! */
  }

  /* Close the file */
  DOS_fclose(f);
}

/* This function write myfile.out (up to 30 chars) */
void write_myfile(void *arg)
{
  DOS_FILE *f;  /* DOS file descriptor */
  int err;  /* Error code */
  int maxbytes;
  int writtenbytes;  /* number of files written */

  /* open the DOS file for writing  (you can specify only "r" or "w") */
  f = DOS_fopen("myfile.out","w");

  /* check for open errors */
  if (!f) {
    /* error!! */
    err = DOS_error();

    /* note that if you call DOS_error() here, it return 0!!! */
    cprintf("Error %d opening myfile.out...\n", err);
    return;
  }

  /* write up to 30 bytes */
  if (myfilebuf_length > 30)
    maxbytes = 30;
  else
    maxbytes = myfilebuf_length;

  writtenbytes = DOS_fwrite(myfilebuf,1,maxbytes,f);

  /* check for errors */
  err = DOS_error();

  cprintf("Written %d bytes into myfile.out...\n", writtenbytes);

  if (err) {
    cprintf("Error %d writing myfile.txt...\n", err);
    /* there is not return because I want to close the file! */
  }

  /* Close the file */
  DOS_fclose(f);
}


