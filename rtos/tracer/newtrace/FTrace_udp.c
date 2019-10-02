/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *   Tullio Facchinetti  <tullio.facchinetti@unipv.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 *
 * Robotib Lab (University of Pavia)
 * http://robot.unipv.it
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

#include <ll/sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <tracer.h>
#include <unistd.h>

#include <FTrace_udp.h>
#include <FTrace_OSD.h>
#include <FTrace_chunk.h>

#include <kernel/kern.h>

#include <drivers/udpip.h>

//#define TRACER_UDP_DEBUG

#define UDP_MAXSIZE 1000
#define TRACER_PORT 20000

#define DELAY_USEC 5000

BYTE pkt[UDP_MAXSIZE];
int total_pkt_size = 0;

int TracerUDPInit = 0;
UDP_ADDR target,local;
char local_ip[20],target_ip[20];
int socket;

PID udp_task;

int sending_flag = 0;

struct FTrace_udp_header {
  char id[12];
  unsigned int pkt_number;
  unsigned int number_of_events;
  unsigned int total_size;
};

/** 
 * Initialize the Tracer chunk sender through the network using the UDP
 * protocol supported by S.Ha.R.K. 
 * If flag = 1 initializes the network driver, otherwise it considers
 * that the network layer has already been initialized.
 * It also sets the internal chunk sender to the function that initializes
 * the task for sending the chunk.
 */
int FTrace_OSD_init_udp(int flag, char *l_ip, char *t_ip) {

  struct net_model m = net_base;
  
  FTrace_set_internal_chunk_sender(FTrace_OSD_create_udp_task);  // Tool

  strcpy(local_ip,l_ip);
  strcpy(target_ip,t_ip);

  if (flag) {

    net_setudpip(m, local_ip, "255.255.255.255");

    if (net_init(&m) != 1) {
      return -1;
    }

  }

  ip_str2addr(local_ip,&(local.s_addr));
  local.s_port = TRACER_PORT;
  socket = udp_bind(&local, NULL);
  
  ip_str2addr(target_ip,&(target.s_addr));
  target.s_port = TRACER_PORT;

  sleep(1);
  
  TracerUDPInit = 1;

  return 0;

}

int send_udp_event(void *p, int size) {

  static BYTE *current = pkt;
  static int events_number = 0;
  static int packet_number = 0;
  
  struct FTrace_udp_header head = {"TRACER-V1.0",0,0,0};
  struct FTrace_udp_header *phead = (struct FTrace_udp_header *)(pkt);

  if ((total_pkt_size + size < UDP_MAXSIZE) && p != NULL ) {

    if (total_pkt_size == 0) {

      memcpy(pkt,&head,sizeof(struct FTrace_udp_header));

      current = pkt;
      current += sizeof(struct FTrace_udp_header);

      total_pkt_size += sizeof(struct FTrace_udp_header);
      phead->total_size = total_pkt_size;

      packet_number++;
      phead->pkt_number = packet_number;

    }

    events_number++;
    phead->number_of_events = events_number;
    
    total_pkt_size += size;
    phead->total_size = total_pkt_size;
    
    //Copy the event
    memcpy(current,p,size);
    current += size;

    return 0;

  } else {

    #ifdef TRACER_UDP_DEBUG
      cprintf(".");
    #endif

    if (TracerUDPInit == 1) {
      udp_sendto(socket, pkt, total_pkt_size, &target);
      usleep(DELAY_USEC);
    }

    events_number = 0;
    total_pkt_size = 0;
    current = pkt;
    
    if (p != NULL) send_udp_event(p,size);
    
    return 0;    

  }

  return -1;

}

//Sender Task
TASK udp_sender(void *arg)
{
  FTrace_Chunk_Ptr c = (FTrace_Chunk_Ptr)(arg);

  DWORD total_size = (DWORD)(c->size) + (DWORD)(c->emergency_size);
  DWORD start = (DWORD)(c->osd + FTRACE_OSD_CHUNK_HEAD);
  DWORD current = start;
  DWORD count = 0;  
                                                                                                                           
  while (current+16 <= start+total_size) {
                                                                                                                             
        if (*(WORD *)(current) != 0) {
                send_udp_event((void *)(current), 16);
        	count++;
	}

	current += 16;
                                                                                                                            
  }

  // Flush the buffer out
  send_udp_event(NULL, 0);
  
  FTrace_chunck_output_end();  // Tool

  #ifdef TRACER_UDP_DEBUG
    cprintf("\nTotal Chunk Event Sent: %d\n",(int)count);
  #endif  

  return 0;  

}

int FTrace_OSD_create_udp_task(FTrace_Chunk_Ptr c)
{

  NRT_TASK_MODEL st;

  FTrace_chunck_output_start();   // Tool

  nrt_task_default_model(st);
  nrt_task_def_arg(st,(void *)c);
    
  udp_task = task_create("UDP_Sender",udp_sender,&st,NULL);
  if (udp_task == NIL) {
    cprintf("Error creating UDP Sender\n");
    exit(1);
  } 
    
  task_activate(udp_task);

  return 0;

} 
