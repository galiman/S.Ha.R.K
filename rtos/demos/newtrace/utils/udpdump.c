/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *    Giacomo Guidi      <giacomo@gandalf.sssup.it>
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

#include <netinet/in.h>
 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>/* close() */
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define SERVER_PORT 20000
#define MAX_MSG 10000

struct tracer_udp_header {
  char id[12];
  unsigned int pkt_number;
  unsigned int events;
  unsigned int size;
};

FILE *output_file;

int miss;

void close_and_exit()
{

  printf("Closing...\n");

  if (miss == 1) printf("Possible error receiving packets !\n");

  fclose(output_file);

  exit(0);

}

int main(int argc, char *argv[])
{
  int sd, rc, n, cliLen,count;
  struct sockaddr_in cliAddr, servAddr;
  char msg[MAX_MSG];

  struct tracer_udp_header *pkt_head = (struct tracer_udp_header *)(msg);

  if (argc < 2) {
    printf("%s: Enter the output file name [%s filename]\n",argv[0],argv[0]);
    exit(1);
  }

  // socket creation
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sd < 0) {
    printf("%s: cannot open socket \n",argv[0]);
    exit(1);
  }

  output_file = fopen(argv[1],"w+b");
  if (output_file == NULL) {
    printf("%s: Cannot open the file\n",argv[0]);
    exit(1);
  }

  // bind local server port
  servAddr.sin_family = AF_INET;

  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(SERVER_PORT);

  rc = bind (sd, (struct sockaddr *)&servAddr,sizeof(servAddr));
  if(rc < 0) {
    printf("%s: cannot bind port number %d \n", 
	   argv[0], SERVER_PORT);
    exit(1);
  }

  signal(SIGINT, close_and_exit);

  count = 1;
  miss = 0;

  while(1) {

    printf("Wait packet...\n");

    // receive message
    cliLen = sizeof(cliAddr);
    n = recvfrom(sd, msg, MAX_MSG, 0,(struct sockaddr *)&cliAddr, &cliLen);

    if (strncmp(pkt_head->id,"TRACER",6)) continue;

    printf("Received %d, length %d(%d), %d tracer events.\n", 
		pkt_head->pkt_number, n, pkt_head->size, pkt_head->events);
    
    if (pkt_head->pkt_number != count) { 
      printf("Miss Packet !!!\n");
      miss = 1;
    }

    if (n > 0) {

      count++;

      fwrite((void *)(msg+sizeof(struct tracer_udp_header)),n-sizeof(struct tracer_udp_header),1,output_file);
     
    }

  }

  fclose(output_file);

  return 0;

}

