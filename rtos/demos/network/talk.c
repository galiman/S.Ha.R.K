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

/*
 * Copyright (C) 2000 Luca Abeni, Paolo Gai
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
 *
 * CVS :        $Id: aster1.c,v 1.1 2002/10/28 08:13:37 pj Exp 

 This is the talkdx.c Hartik's example.

 File:        Talk.C
 Revision:    1.00
 Author:      Luca Abeni


   Simple Netlib demo: nothing of seriously real-time, only another Unix
   Talk clone.
   Read it to see how the UDP/IP layers of the networ library work.

*/

#include <kernel/kern.h>
#include <string.h>

#include <drivers/crtwin.h>
#include <drivers/shark_keyb26.h>

#include <drivers/udpip.h>


WIN dbg;
BYTE esc = FALSE;

char talk_myipaddr[20];
char talk_toipaddr[20];

/*
   This non real-time task reads UDP packets from the network and writes
   them in a window
*/
TASK scrittore(void)
{
  char str[2000];
  UDP_ADDR from, local;
  WIN displ;
  int s,n;

  /* Connect on the local port #100 */
  local.s_port = 100;
  s = udp_bind(&local, NULL);

  /* Open the window */
  win_init(&displ,0,0,79,6);
  win_frame(&displ,BLACK,WHITE,"Remote",2);

  while (1) {
    /* Clear the buffer for receiving the packet...*/
    memset(str, 0, 1999);
    /*...and receive the packet (block until a packet arrives */
    n = udp_recvfrom(s, str, &from);
    win_puts(&displ, str);
  }
}

/*
   This non real-time task reads strings from the keyoard and sends them
   to the remote host
*/
TASK write(void)
{
  WIN wr;
  UDP_ADDR to,local;
  char str[80],tmp[5],ch;
  int s;
  IP_ADDR bindlist[5];

  win_init(&wr,0,7,79,6);
  win_frame(&wr,BLACK,WHITE,"Local",2);

  /* Create a socket for transitting */
  ip_str2addr(talk_myipaddr,&(local.s_addr));
  local.s_port = 101;

  /*
    If we want the address of the remote host in the ARP table before
    begginning the transmission (to eliminate a possible source of
    unpredictability), we can use the bindlist, otherwise we set the
    second parameter of udp_bind to NULL
  */
  ip_str2addr(talk_toipaddr,&(bindlist[0]));
  memset(&(bindlist[1]), 0, sizeof(IP_ADDR));
  s = udp_bind(&local, /*bindlist*/NULL);

  ip_str2addr(talk_toipaddr,&(to.s_addr));
  to.s_port = 100;
  sprintf(str,"Local IP address %d.%d.%d.%d\n", local.s_addr.ad[0],
	  local.s_addr.ad[1], local.s_addr.ad[2],
	  local.s_addr.ad[3]);
  win_puts(&dbg,str);
  sprintf(str,"Talk to %d.%d.%d.%d   ",to.s_addr.ad[0],to.s_addr.ad[1],
	  to.s_addr.ad[2],to.s_addr.ad[3]);
  win_puts(&dbg,str);
  while (1) {
    /* Get the string...*/
    while((ch = keyb_getch(BLOCK)) != 13) {
	sprintf(tmp,"%c",ch);
	strcat(str,tmp);
	win_puts(&wr,tmp);
    }
    strcat(str,"\n");
    win_puts(&wr,"\n");
    /*...and send it!!! */
    udp_sendto(s,str,strlen(str)+2,&to);
    str[0] = 0;
  }
}

/* This function is called when the user presses CTRL-C (stops the systems) */
void esci(KEY_EVT *k)
{
  esc = TRUE;
  exit(1);
}

int main(int argc, char **argv)
{
  KEY_EVT k;

  struct net_model m = net_base;

  NRT_TASK_MODEL m_nrt;

  k.flag = CNTL_BIT;
  k.scan = KEY_C;   
  k.ascii = 'c';
  k.status = KEY_PRESSED;
  keyb_hook(k,esci,FALSE);
  k.flag = CNTR_BIT;
  k.scan = KEY_C;   
  k.ascii = 'c';
  k.status = KEY_PRESSED;
  keyb_hook(k,esci,FALSE);

  clear();
  cprintf(" S.Ha.R.K. Talk! 			Ver. 1.10\n");

  if (argc != 3) {
    cprintf("S.Ha.R.K. Talk usage: talk fromIP toIP\n");
    return 0;
  }

  strcpy(talk_myipaddr, argv[1]);
  strcpy(talk_toipaddr, argv[2]);

  /* We want a task for TX mutual exclusion */
  net_setmode(m, TXTASK);
  /* We use UDP/IP stack */
  net_setudpip(m, talk_myipaddr, "255.255.255.255");
  /* OK: let's start the NetLib! */
  if (net_init(&m) == 1) {
    cprintf("Net Init OK...\n");
  } else {
    cprintf("Net Init Failed...\n");
    exit(300);
  }


  //dump_irq();
	
  cprintf("\n\n\n\tPress ENTER\n");
  while (!esc) {
    keyb_getcode(&k,BLOCK);
    if (k.ascii == 13) esc = TRUE;
  }

  esc = FALSE; 
  clear();
  win_init(&dbg,0,20,60,3);
  win_frame(&dbg,BLACK,WHITE,"Debug",2);
    
  /* Start the sender and receiver tasks...*/
  nrt_task_default_model(m_nrt);
  task_activate(task_create("aaa",scrittore,&m_nrt,NULL));
  task_activate(task_create("bbb",write,&m_nrt,NULL));
    
  return 0;
}
