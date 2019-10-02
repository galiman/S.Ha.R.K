
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

#include <kernel/func.h>
#include <kernel/kern.h>
#include <stdlib.h>
#include <stdio.h>
#include "string.h"

#include "drivers/udpip.h"

#include "tftp.h"
#include "endn.h"

/*  */
#include "sem/sem/sem.h"

char local_ip_addr[20];
char host_ip_addr[20];

/* The value is incremented when assigning a new port address to a new
 * connection.
 */
int port_counter;

/* The fixed IP/port (=69) to submit the connection requesting */
UDP_ADDR connection_request;

TFTP_MODEL model[MAX_CONCURRENT_STREAM];
sem_t *model_sem[MAX_CONCURRENT_STREAM];

TFTP_BUFFER buffer[MAX_CONCURRENT_STREAM];
//QUEUE queue[MAX_CONCURRENT_STREAM];
sem_t *buffer_sem[MAX_CONCURRENT_STREAM];

WORD tftp_get_data(TFTP_PACKET *pkt, BYTE *data, int n) {
	memcpy(data, pkt->u.data.data, n);
	return(pkt->u.data.block);
}

int tftp_get_ack_block(TFTP_PACKET *pkt) {
	return(pkt->u.ack.block);
}

int tftp_get_error(TFTP_PACKET *pkt, char *errmsg) {
	strcpy(errmsg, pkt->u.err.errmsg);
	return(pkt->u.err.errcode);
}

/* Returns the packet opcode.
 */
int tftp_get_opcode(TFTP_PACKET *pkt) {
  WORD tmp;
  tmp = pkt->opcode;
  SWAP_SHORT(tmp);    /* Swap endian!! */
  return(tmp);
}

int tftp_fill_request(TFTP_PACKET *pkt, WORD opcode, const BYTE *filename, const BYTE *mode) {
  int i, j;

  pkt->opcode = opcode;				/* Put the opcode in the right struct field */
  SWAP_SHORT(pkt->opcode);    /* Swap endian!! */

	/* Reset the filename field */
  memset(pkt->u.request.filename, 0, sizeof(pkt->u.request.filename));

	/* Concats the containing filename and mode NULL terminatd strings in the filename field */
  for (i = 0; i < strlen(filename); i++)
     pkt->u.request.filename[i] = filename[i];
  pkt->u.request.filename[i] = '\0';
  for (j = 0, i = i + 1; j < strlen(mode); i++, j++)
     pkt->u.request.filename[i] = mode[j];
  pkt->u.request.filename[i] = '\0';

  return(0);
}

int tftp_fill_data(TFTP_PACKET *pkt, WORD nblock, BYTE *rawdata, WORD datasize) {
	if (datasize > TFTP_DATA_SIZE) { return(1); }	/* Overflow checking */

	pkt->opcode = TFTP_DATA;	/* Put the DATA opcode in the opcode field */
	SWAP_SHORT(pkt->opcode);  /* Swap endian!! */

  pkt->u.data.block = nblock;
  SWAP_SHORT(pkt->u.data.block); /* Swap endian!! */

  memcpy(pkt->u.data.data, rawdata, datasize);  /* ??? Maybe some data manipulation required!!! */
  return(0);
}

int tftp_fill_ack(TFTP_PACKET *pkt, WORD nblock) {
	pkt->opcode = TFTP_ACK;		/* Put the ACK opcode in the opcode field */
	SWAP_SHORT(pkt->opcode);  /* Swap endian!! */

  pkt->u.ack.block = nblock;
	return(0);
}

void tftp_reset_handle(int h) {
	model[h].status = TFTP_NOT_CONNECTED;
  model[h].errcode = TFTP_NO_ERROR;
  model[h].handle = -1;
  model[h].sender_pid = -1;
  model[h].receiver_pid = -1;
	model[h].nblock = 0;
	model[h].waiting_ack = 0;
	model[h].timestamp = 0;
	model[h].timeout = TFTP_DEFAULT_TIMEOUT;
	model[h].ntimeout = TFTP_DEFAULT_TIMEOUT_NUMBER;

	buffer[h].data = NULL;
	buffer[h].size = 0;
	buffer[h].nbytes = 0;

	model_sem[h] = NULL;
	buffer_sem[h] = NULL;
}

int tftp_init() {
  int i;

  for (i = 0; i < MAX_CONCURRENT_STREAM; i++) {
    tftp_reset_handle(i);
  }

  port_counter = 0;

  return(0);
}

int tftp_net_start(char *local_ip, char *host_ip, int init_net) {
  struct net_model m = net_base;
  int netval;

	/* Save IPs locally */
  strcpy(local_ip_addr, local_ip);
  strcpy(host_ip_addr, host_ip);

  netval = 0;

  if (init_net) {
    net_setmode(m, TXTASK);			/* We want a task for TX mutual exclusion */
    net_setudpip(m, local_ip,"255.255.255.255");	/* We use UDP/IP stack */

    /* OK: let's start the NetLib! */
    netval = net_init(&m);
  }

  return(netval);
}

int tftp_setup_timeout(int h, int sec) {
  if (model[h].handle != TFTP_NOT_CONNECTED) return(-1);
  model[h].timeout = sec * 1000000;
  return(0);
}

int tftp_set_timeout_numbers(int h, int n) {
  if (model[h].handle != TFTP_NOT_CONNECTED) return(-1);
  model[h].ntimeout = n;
  return(0);
}

int tftp_open(char *fname) {
  int i;

  /* Finds the first free connection slot */
  for (i = 0; i < MAX_CONCURRENT_STREAM; i++)
     if (model[i].status == TFTP_NOT_CONNECTED) break;
  if (i >= MAX_CONCURRENT_STREAM) return(-1);   /* No connection slots available */

  model[i].handle = i;							/* Handle = index in the struct array */
  strcpy(model[i].filename, fname);	/* Save filename into struct */
	model[i].status = TFTP_OPEN;		  /* Connection opened */
  sem_init(model_sem[i], 0, 1);

  return(i);
}

TASK upload_sender(int id) {
  TFTP_PACKET pkt;
  char data[TFTP_DATA_SIZE];
  int mystatus;
  int i, n;

  i = 0;
  while(1) {
    sem_wait(model_sem[id]);

    if (model[id].waiting_ack) {   /* and status != error ??? */
      if (sys_gettime(NULL) - model[id].timestamp >= model[id].timeout) {   /* ??? check it!!! */
        if (!model[id].ntimeout) {
          model[id].status = TFTP_ERR;
          model[id].errcode = TFTP_ERR_TIMEOUT;
          sem_post(model_sem[id]);
        } else {
          model[id].ntimeout--;
          model[id].timestamp = sys_gettime(NULL);
          sem_post(model_sem[id]);
					udp_sendto(model[id].socket, (char*)(&model[id].last_sent), sizeof(TFTP_PACKET), &model[id].host);
        }
      } else {
        sem_post(model_sem[id]);
      }
    } else {
      mystatus = model[id].status;
      sem_post(model_sem[id]);

      switch (mystatus) {
        case TFTP_ACTIVE : {

          /* Doesn't use mutex 'cause uses "static" model fields */
          tftp_fill_request(&pkt, TFTP_WRITE_REQUEST, model[id].filename, TFTP_OCTET_MODE);
          udp_sendto(model[id].socket, (char*)(&pkt), sizeof(TFTP_PACKET), &connection_request);
  				memcpy(&model[id].last_sent, &pkt, sizeof(TFTP_PACKET));		/* Save the last sent packet for retransmission */

          sem_wait(model_sem[id]);
          if (model[id].status != TFTP_ERR)
            model[id].status = TFTP_CONNECTION_REQUESTING;
          else {
            sem_post(model_sem[id]);
            break;
          }
          model[id].waiting_ack = 1;
          model[id].timestamp = sys_gettime(NULL);
          sem_post(model_sem[id]);

          break;
        }
        case TFTP_CONNECTION_REQUESTING : {
        }
        case TFTP_STREAMING : {
          if (tftp_usedbuffer(id) >= TFTP_DATA_SIZE) {
            n = tftp_get(id, data, TFTP_DATA_SIZE);
            tftp_fill_data(&pkt, model[id].nblock, data, n);

            udp_sendto(model[id].socket, (char*)(&pkt), sizeof(TFTP_PACKET), &model[id].host);
    				memcpy(&model[id].last_sent, &pkt, sizeof(TFTP_PACKET));		/* Save the last sent packet for retransmission */

            sem_wait(model_sem[id]);
            model[id].waiting_ack = 1;
            model[id].timestamp = sys_gettime(NULL);
            sem_post(model_sem[id]);
          }
          break;
        }
        case TFTP_FLUSHING : {
        	n = tftp_usedbuffer(id);
          if (n >= TFTP_DATA_SIZE) {

            /* Get data for a full data packet */
            n = tftp_get(id, data, TFTP_DATA_SIZE);
            tftp_fill_data(&pkt, model[id].nblock, data, n);

            udp_sendto(model[id].socket, (char*)(&pkt), sizeof(TFTP_PACKET), &model[id].host);
    				memcpy(&model[id].last_sent, &pkt, sizeof(TFTP_PACKET));		/* Save the last sent packet for retransmission */

            sem_wait(model_sem[id]);
            model[id].waiting_ack = 1;
            model[id].timestamp = sys_gettime(NULL);
            sem_post(model_sem[id]);
          } else {

            /* Get remaining data from buffer */
            n = tftp_get(id, data, n);
            tftp_fill_data(&pkt, model[id].nblock, data, n);

            /* Sending 4 extra bytes for opcode and block number!! */
            udp_sendto(model[id].socket, (char*)(&pkt), sizeof(n + 4), &model[id].host);

						/* Don't wait for ack!! Maybe will be implemented later... */
            task_kill(model[id].receiver_pid);
            /* ..... */
            task_abort(NULL);
          }
          break;
        }

        case TFTP_ERROR : {
          break;
        }
      }
    }

    task_testcancel();
		task_endcycle();
  }
  return(0);
}

/* This non real-time task reads UDP packets with ACK from the network
 */
TASK upload_receiver(int id) {
  char msg[200];
  int mystatus;
  int n;
  int i;
  WORD opcode;
  TFTP_PACKET pkt;
  UDP_ADDR server;

  i = 0;
  while (1) {
    sem_wait(model_sem[id]);
    mystatus = model[id].status;
    sem_post(model_sem[id]);

    if (mystatus != TFTP_ERR) {
      n = udp_recvfrom(model[id].socket, &pkt, &server);
      opcode = tftp_get_opcode(&pkt);

      if (opcode == TFTP_ERROR) {
        n = tftp_get_error(&pkt, msg);   // re-use n: not too orthodox...

        sem_wait(model_sem[id]);
        model[id].status = TFTP_ERR;
        model[id].errcode = n;
        strcpy(model[id].errmsg, msg);
        sem_post(model_sem[id]);

      } else {
        switch (mystatus) {
          case TFTP_NOT_CONNECTED : {
            // discard the packet... set error??
            break;
          }
          case TFTP_CONNECTION_REQUESTING : {
            sem_wait(model_sem[id]);
            memcpy(&model[id].host, &server, sizeof(model[id].host));
            model[id].waiting_ack = 0;
            model[id].status = TFTP_STREAMING;
            model[id].nblock++;
            sem_post(model_sem[id]);

            break;
          }
          case TFTP_STREAMING : {
            // check the nblock on the arrived packet

            sem_wait(model_sem[id]);
            model[id].waiting_ack = 0;
            model[id].nblock++;
            sem_post(model_sem[id]);
            break;
          }
        }
      }
    }
    i++;
  }

  return(0);
}

int tftp_upload(int i, unsigned long buffsize, sem_t *mtx) {
  SOFT_TASK_MODEL soft_m;
  NRT_TASK_MODEL nrt_m;

  if ((buffer_sem[i] = mtx) == NULL) return(-3);									/* ??? check assignment!!! */

  if ((buffer[i].size = buffsize) > MAX_BUFFER_SIZE) return(-2);	/* Buffer size too large */
  if ((buffer[i].data = malloc(buffsize)) == NULL) return(-4);		/* Buffer allocation error */
  buffer[i].nbytes = 0;

  /* Create a socket for transmission */
  ip_str2addr(local_ip_addr, &(model[i].local.s_addr));
  model[i].local.s_port = BASE_PORT + port_counter;	/* Different port for each connection */
  port_counter++;

  ip_str2addr(host_ip_addr, &(connection_request.s_addr));
  connection_request.s_port = 69;   /* It is fixed for the connection request */

	model[i].socket = udp_bind(&model[i].local, NULL);

  /* First we set the sender's task properties... */
  soft_task_default_model(soft_m);
  soft_task_def_level(soft_m, 0);
  soft_task_def_arg(soft_m, (void *)(i));
  soft_task_def_group(soft_m, i);
  soft_task_def_periodic(soft_m);
  soft_task_def_wcet(soft_m, TFTP_UPLOAD_SENDER_WCET);
  soft_task_def_period(soft_m, TFTP_UPLOAD_SENDER_PERIOD);
  soft_task_def_met(soft_m, TFTP_UPLOAD_SENDER_MET);

  model[i].sender_pid = task_create("upload_sender", upload_sender, &soft_m, NULL);

  if (model[i].sender_pid == -1) {
     free(buffer[i].data);
     tftp_reset_handle(i);
     return(-5);
  }

  nrt_task_default_model(nrt_m);	/* Start the receiver task... */
  nrt_task_def_arg(nrt_m, (void *)(i));
  if ((model[i].receiver_pid = task_create("upload_receiver", upload_receiver, &nrt_m, NULL)) == NIL) {
     free(buffer[i].data);
     tftp_reset_handle(i);
     return(-6);
  }

	model[i].status = TFTP_ACTIVE;		/* Connection active */
  if (task_activate(model[i].sender_pid) == -1) {
     free(buffer[i].data);
     tftp_reset_handle(i);
     return(-7);
  }
  if (task_activate(model[i].receiver_pid) == -1) {
     free(buffer[i].data);   // Maybe not correct... sys_panic() may be better
     tftp_reset_handle(i);
     return(-8);
  }

  return(0);
}

int tftp_download(int i, unsigned long buffsize, sem_t *mtx) {
  return(0);
}

int tftp_close(int h, int hardness) {
  TFTP_PACKET pkt;

	if (hardness == TFTP_STOP_NOW) {
    task_kill(model[h].sender_pid);
  	task_kill(model[h].receiver_pid);
    tftp_fill_data(&pkt, model[h].nblock, NULL, 0);
    udp_sendto(model[h].socket, (char*)(&pkt), 4, &model[h].host);
    tftp_reset_handle(h);
    free(buffer[h].data);
    sem_destroy(buffer_sem[h]);
    sem_destroy(model_sem[h]);
  } else {
  	sem_wait(model_sem[h]);
    model[h].status = TFTP_FLUSHING;
  	sem_post(model_sem[h]);
  }

	return(0);
}

int tftp_put(int h, BYTE *rawdata, WORD n) {
	sem_wait(buffer_sem[h]);

	/* Buffer overflow checking */
  if (buffer[h].nbytes + n > buffer[h].size) {	/* Maybe ">"??? */
		sem_post(buffer_sem[h]);
		return(1);
  }

	/* Check this carefully!!! */
	memcpy(buffer[h].data + buffer[h].nbytes, rawdata, n);
	buffer[h].nbytes += n;

  sem_post(buffer_sem[h]);

  return(0);
}

int tftp_get(int h, BYTE *rawdata, WORD n) {
//  cprintf("get mutex %d - use %d\n", buffer_sem[h]->mutexlevel, buffer_sem[h]->use);
	sem_wait(buffer_sem[h]);

	if (buffer[h].nbytes < 1) return(0);
  if (buffer[h].nbytes < n) n = buffer[h].nbytes;

	/* Check this carefully!!! */
	memcpy(rawdata, buffer[h].data, n);							/* Export data to calling function */
	memcpy(buffer[h].data, buffer[h].data + n, n);	/* Shift data into buffer */
	buffer[h].nbytes -= n;

  sem_post(buffer_sem[h]);
	return(n);
}

int tftp_getbuffersize(int h) {
	return(buffer[h].size);		/* We on't use the mutex 'cause the size is read-only */
}

int tftp_usedbuffer(int h) {
	int n;

//  cprintf("used mutex %d - use %d\n", buffer_sem[h]->mutexlevel, buffer_sem[h]->use);
	sem_wait(buffer_sem[h]);
	n = buffer[h].nbytes;
	sem_post(buffer_sem[h]);
	return(n);
}

int tftp_freebuffer(int h) {
	int n;

	sem_wait(buffer_sem[h]);
	n = buffer[h].size - buffer[h].nbytes;
	sem_post(buffer_sem[h]);
	return(n);
}

int tftp_status(int h) {
	int n;

	sem_wait(model_sem[h]);
	n = model[h].status;
	sem_post(model_sem[h]);
	return(n);
}

///////////////////////////////////////////////////////////////////////////

int debug_setbuffer(int h, int size) {
  if ((buffer[h].data = malloc(size)) == NULL) return(-1);		/* Buffer allocation error */
  buffer[h].size = size;
  buffer[h].nbytes = 0;
  return(0);
}

void debug_freebuffer(int h) {
  free(buffer[h].data);		/* Buffer allocation error */
  buffer[h].size = 0;
  buffer[h].nbytes = 0;
}

