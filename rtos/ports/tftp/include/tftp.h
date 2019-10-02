
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

#ifndef	__TFTP_H
#define	__TFTP_H	1

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ll/sys/types.h>
#include <drivers/udpip.h>

#include "sem/sem/sem.h"

#define BASE_X   (40)

/* Real-time constraints for the sender and receiver tasks */
#define TFTP_UPLOAD_SENDER_PERIOD    (2000000)
#define TFTP_UPLOAD_SENDER_WCET      (1000000)
#define TFTP_UPLOAD_SENDER_MET        (500000)

/* opcodes options */
#define TFTP_READ_REQUEST   (1)
#define TFTP_WRITE_REQUEST  (2)
#define TFTP_DATA           (3)
#define TFTP_ACK            (4)
#define TFTP_ERROR          (5)

/* Default TFTP protocol error codes */
#define TFTP_ERR_NOT_DEFINED        (0)   /* Not defined, see error message (if any) */
#define TFTP_ERR_FILE_NOR_FOUND     (1)   /* File not found */
#define TFTP_ERR_ACCESS_VIOLATION   (2)   /* Access violation */
#define TFTP_ERR_DISK_FULL          (3)   /* Disk full or allocation exceeded */
#define TFTP_ERR_ILLEGAL_OPERATION  (4)   /* Illegal TFTP operation */
#define TFTP_ERR_UNKNOWN_ID         (5)   /* Unknown transfer ID */
#define TFTP_ERR_FILE_EXISTS        (6)   /* File already exists */
#define TFTP_ERR_NO_USER            (7)   /* No such user */

/* Custom TFTP protocol error codes */
#define TFTP_NO_ERROR     (-1)
#define TFTP_ERR_TIMEOUT  (-2)

/* TFTP connection modes */
#define TFTP_NETASCII_MODE  "netascii"
#define TFTP_OCTET_MODE     "octet"
#define TFTP_MAIL_MODE      "mail"

/* Status */
#define TFTP_NOT_CONNECTED          (-1)
#define TFTP_OPEN                   (-2)
#define TFTP_ACTIVE                 (-3)
#define TFTP_CONNECTION_REQUESTING  (-4)
#define TFTP_STREAMING              (-5)
#define TFTP_FLUSHING               (-6)
#define TFTP_ERR                    (-100)

/* Mode options */
//#define TFTP_UPLOAD		(1)
//#define TFTP_DOWNLOAD	(2)

/* Closing options */
#define TFTP_STOP_NOW			(1)
#define TFTP_FLUSH_BUFFER	(2)

/* Base port address for socket communication */
#define BASE_PORT			(2000)

/* The standard size of the tftp data packet */
#define TFTP_DATA_SIZE	(512)

typedef struct tftp_packet {
  WORD opcode;                        /* This is the opcode that represents operation to perform. */
  union {
    struct {
      BYTE filename[TFTP_DATA_SIZE];  /* This is the download data. */
    } request;
    struct {
      WORD block;                     /* The number of this block. */
      BYTE data[TFTP_DATA_SIZE];      /* This is the download data. */
    } data;
    struct {
      WORD block;                     /* This is the block number of the last data packet received. */
    } ack;
    struct {
      WORD errcode;                   /* This is the error code from the TFTP server. */
      BYTE errmsg[TFTP_DATA_SIZE];    /* This is the error message from the server. */
    } err;
  } u;
} TFTP_PACKET;

#define MAX_CONCURRENT_STREAM				(5)
#define MAX_BUFFER_SIZE							(1000000)
#define TFTP_DEFAULT_TIMEOUT        (6000000)
#define TFTP_DEFAULT_TIMEOUT_NUMBER (5)

typedef struct tftp_model {
  int status;
  int errcode;
  char errmsg[200];
  int handle;
  BYTE filename[80];
  PID sender_pid;
  PID receiver_pid;
  int socket;
  UDP_ADDR host;
  UDP_ADDR local;
  unsigned int nblock;
  TFTP_PACKET last_sent;
  BYTE waiting_ack;
  TIME timestamp;   /* the time we sent the last packet (data or ack) */
  TIME timeout;     /* in microseconds */
  int ntimeout;     /* number of timeouts (when 0 -> error!) */
} TFTP_MODEL;

typedef struct tftp_buffer {
  BYTE *data;
  unsigned long size;
  unsigned long nbytes;
} TFTP_BUFFER;

/* Mutex constant */
#define TFTP_PI   (0)
#define TFTP_SRP  (1)
#define TFTP_PC   (2)

/* We need to know what kind of mutex the user wants to use to share data with the reading (writing) queue
 */
typedef struct tftp_mutex {
  BYTE semtype;					/* Semaphore type. */
  union {
    struct {						/* SRP */
      SRP_RES_MODEL r;
			int pLevel;				/* Preemption level */
    } srp;
    struct {						/* Priority ceiling */
      PC_RES_MODEL r;
			int tPr;					/* Task priority */
    } pc;
    struct {						/* Priority inheritance */
      PI_mutexattr_t a;
    } pi;
  } sem;
} TFTP_MUTEX;

/***** Tftp packet related routines *****/
int tftp_get_opcode(TFTP_PACKET *pkt);
WORD tftp_get_data(TFTP_PACKET *pkt, BYTE *data, int n);
int tftp_get_ack_block(TFTP_PACKET *pkt);
int tftp_get_error(TFTP_PACKET *pkt, char *errmsg);
int tftp_fill_request(TFTP_PACKET *pkt, WORD opcode, const BYTE *filename, const BYTE *mode);
int tftp_fill_data(TFTP_PACKET *pkt, WORD nblock, BYTE *rawdata, WORD datasize);
int tftp_fill_ack(TFTP_PACKET *pkt, WORD nblock);

/***** Initialization routines *****/
int tftp_init();
int tftp_net_start(char *local_ip, char *host_ip, int init_net);

/***** Parameter setting routines *****/
int tftp_set_timeout(int h, int sec);
int tftp_set_timeout_numbers(int h, int n);

/***** Connection routines *****/
int tftp_open(char *fname);
int tftp_upload(int i, unsigned long buffsize, sem_t *mtx);
int tftp_download(int i, unsigned long buffsize, sem_t *mtx);
int tftp_close(int h, int hardness);

/* Buffer I/O routines */
int tftp_put(int h, BYTE *rawdata, WORD n);
int tftp_get(int h, BYTE *rawdata, WORD n);
int tftp_getbuffersize(int h);
int tftp_usedbuffer(int h);
int tftp_freebuffer(int h);

int tftp_status(int h);

/* Debugger routines */
int debug_setbuffer(int h, int size);
void debug_freebuffer(int h);

#endif	/* tftp.h */
