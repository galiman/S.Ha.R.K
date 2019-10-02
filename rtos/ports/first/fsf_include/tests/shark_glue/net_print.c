
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
#include <drivers/udpip.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

UDP_ADDR target, local;
int      socket;
IP_ADDR  bindlist[5];
char local_ip[20], target_ip[20], buff[10];
static int pkt_number=0;

typedef struct tests_udp_header {
  char id[12];
  unsigned int pkt_number;
  unsigned int size;
} tests_udp_header;

#define HEADER "TEST"
#define LOCAL_ADDR "192.168.82.246"
#define REMOTE_ADDR "192.168.82.205"

/* Init the network stack */
int init_network(void)
{
  struct net_model m = net_base;
	
  strcpy(local_ip,  LOCAL_ADDR);
  strcpy(target_ip, REMOTE_ADDR);
  
  ip_str2addr(local_ip,&(local.s_addr));
  /* set the source port */
  local.s_port = 24000;
  
  /* target IP string to addr */
  ip_str2addr(target_ip,&(bindlist[0]));
  memset(&(bindlist[1]), 0, sizeof(IP_ADDR));
  /* bind */
  socket = udp_bind(&local, NULL);
  
  /* target IP string to addr */
  ip_str2addr(target_ip,&(target.s_addr));
  /* target port */
  target.s_port = 24000;

  net_setudpip(m, local_ip, "255.255.255.255");
                                                                                                                             
  if (net_init(&m) != 1) {
    cprintf("Network: Init Error.\n");
    return -1;
  }
  
  return 0;

}


int udp_print(char *str, ...)
{
  static char cbuf[1024];
  char *p=cbuf;
  va_list parms;
  int result;
  int err=0;
 
  p+=sizeof(tests_udp_header);
  va_start(parms,str);
  result = vsprintf(p,str,parms);
  va_end(parms);
  ((tests_udp_header *)cbuf)->pkt_number=pkt_number;
  strncpy(cbuf,HEADER, strlen(HEADER)+1);  
  pkt_number++;
  ((tests_udp_header *)cbuf)->size=result;
  
  err=udp_sendto(socket, cbuf, sizeof(tests_udp_header)+result,&target);
  sleep(1);
  return err;
}
  
