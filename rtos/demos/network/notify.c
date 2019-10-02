/*
* Project: S.Ha.R.K.
*
* Coordinators: 
*   Giorgio Buttazzo    <giorgio@sssup.it>
*   Paolo Gai           <pj@gandalf.sssup.it>
*
* Authors     :
*	 Tullio Facchinetti  <tullio.facchinetti@unipv.it>
*	 Luca M. Capisani    <luca.capisani@unipv.it> 
*
* Robotic Lab (Dipartimento di ingegneria informatica - Pavia - Italy)
*
* http://robot.unipv.it
* http://www.sssup.it
* http://retis.sssup.it
* http://shark.sssup.it
*/

/*
* udp_notify demo: this simple demo shows how to set up a simple UDP/IP
* packet handler function in order to avoid busy waits or task creation
* for the packet receiver function.
* usage: after compiling this demo, with the "make" command, start it with:
* x net <loc_ip> <loc_port>
* where <loc_ip> stands for the local ip of the receiver host and
* <loc_port> stands for the local UDP port where the receiver listens.
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
#include <string.h> //for strcpy()
#include <unistd.h> //for sleep()
#include "drivers/shark_keyb26.h" //for keyboard


//Received Packet Counter
long recvcount=0;

//The Internet Address of this Host 
char local_ip[20];

//A index for the receiver socket
int socket;
	
//Declaring a structure containing the receiver UDP port
UDP_ADDR local;

//UDP receiver handler: this function is called upon UDP packet received.
//No Busy waits are present.
int recvFun(int len, unsigned char *buff, void *p){
	
	recvcount++;

	//Printing on screen the data contained, assuming that is text data
	cprintf("Packet # %ld, len %d: %s.\n",recvcount, len,buff);

	return 1;
}
	
/* Init the network stack */
int init_network(char *local_ip)
{
	//Declaring a network model and asigning the UDP/IP model;
	struct net_model m = net_base;
	

	//Setting up the UDP/IP stack, assigning a UDP/IP/Ethernet Network Model,
	//a host and a network broadcast address.
	net_setudpip(m,local_ip,"255.255.255.0");
	
	//Initializing the network stack
	if (net_init(&m) != 1) {
		cprintf("Network: Init Error.\n");
		return -1;
	}


	//Binding of the UDP port in the system network stack
	//without assigning a peer IP-list in the second parameter
	socket = udp_bind(&local, NULL);
	
	//Assigning a receiver notify handler called on packed received
	udp_notify(socket, &recvFun,NULL);
	
	return 0;

}

void program_key_end(KEY_EVT* e)
{
	exit(1);
}


int main(int argc, char **argv)
{
	//Assigning a CTRL+C exit key shortcut
	KEY_EVT	    k;
	k.flag = CNTL_BIT;
	k.scan = KEY_C;
	k.ascii = 'c';
	k.status = KEY_PRESSED;
	keyb_hook(k,program_key_end,FALSE);
	
	if (argc==3){
		//Copy the input parameters values into the UDP/IP stack
		strcpy(local_ip,  argv[1]);
		
		//A local UDP port number where the receiver listens
		local.s_port=atoi(argv[2]);
	}else{
		//The user has given a wrong number of parameters:
		cprintf("S.Ha.R.K. udp_notify usage: x net <localIP> <localUDPport>\n");
		return(1);
	}
	
	cprintf("Initializing the network stack... \n");
	
	//Initializing the network settings
	if (init_network(local_ip)) exit(1);
	
	cprintf("Waiting for packets on port %d... \n",local.s_port);

	//Waiting for packets
	while(1){
		sleep(1);
	}
	
	
	return 0;
}
