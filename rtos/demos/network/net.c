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

#include <kernel/kern.h>
#include <drivers/udpip.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Init the network stack */
int init_network(char *local_ip)
{

	struct net_model m = net_base;

	net_setudpip(m, local_ip, "255.255.255.255");
                                                                                                                             
        if (net_init(&m) != 1) {
                cprintf("Network: Init Error.\n");
		return -1;
        }

	return 0;

}

int main(int argc, char **argv)
{
	UDP_ADDR target, local;
	char local_ip[20], target_ip[20], buff[10];
	int socket, i;
	IP_ADDR bindlist[5];

	strcpy(local_ip,  "192.168.1.10");
	strcpy(target_ip, "192.168.1.1");

	if (init_network(local_ip)) exit(1);

	/* local IP string to addr */
	ip_str2addr(local_ip,&(local.s_addr));
	/* set the source port */
	local.s_port = 100;

	/* target IP string to addr */
	ip_str2addr(target_ip,&(bindlist[0]));
	memset(&(bindlist[1]), 0, sizeof(IP_ADDR));
	/* bind */
	socket = udp_bind(&local, NULL);

	/* target IP string to addr */
	ip_str2addr(target_ip,&(target.s_addr));
	/* target port */
	target.s_port = 100;

	for (i = 0; i < 5; i++) {
		strcpy(buff, "qwerty\n");
		cprintf("Send packet: %s\n", buff);
		udp_sendto(socket, buff, strlen(buff), &target);
		sleep(1);
	}

	cprintf("The End.\n");

	exit(1);

	return 0;
}
