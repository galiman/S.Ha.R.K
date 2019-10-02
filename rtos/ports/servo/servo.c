/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
 * Copyright (C) 2002 Paolo Gai
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

#include "drivers/scom.h"
#include "drivers/scomirq.h"

#include "servo.h"

//#define SERVO_DEBUG

#define SERVO_SPEED 19200
#define SERVO_PARITY EVEN
#define SERVO_LEN 8
#define SERVO_STOP 1

#define SERVO_CLOCK 20000000 /* 20MHz */

#define TICK_LEN 1600 /* ns */
#define TICK_LEN_PERIOD 51200 /* ns */

struct servo_data {
  int min_angle_sec;
  int max_angle_sec;
  int delta_tick;
  int zero_tick;
};

struct servo_data servo_table[4][16] = {
  {{-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600}},
  {{-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600}},
  {{-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600}},
  {{-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600},
  {-324000, 324000, 1200, 1600}}};

#define SERVO_TIMEOUT 100000

#define RXTX_BUFF_MAX 100
volatile static BYTE RXTX_buff[4][RXTX_BUFF_MAX];
volatile static BYTE *RXTX_addr[4][RXTX_BUFF_MAX];
volatile unsigned int RX_position[4] = {0,0,0,0};
volatile unsigned int TX_position[4] = {0,0,0,0};
volatile unsigned int RX_cycle[4] =  {0,0,0,0};
volatile unsigned int TX_cycle[4] =  {0,0,0,0};
volatile unsigned int lock_write[4] = {0,0,0,0};

#define SERVO_LOCK 0
#define SERVO_UNLOCK 1
#define SERVO_SKIP (BYTE *)(0xFFFFFFFF)

const int BaudTable[] = {
	1200,
	2400,
	4800,
	9600,
	14400,
	19200,
	38400,
	57600,
	115200,
        -1};

void servo_rx_error(unsigned port, unsigned type, unsigned data) {

  #ifdef SERVO_DEBUG
    kern_printf("(SERVO: BUFFER ERROR PORT:%d ERR:%d DATA:%02x)",port,type,data);
  #endif

}

void servo_indication(unsigned port, BYTE data) {

 if (RXTX_addr[port][RX_position[port]] == NULL) {
   if (data != 0) {
     servo_rx_error(port,1,data);
     while(RXTX_addr[port][RX_position[port]] != SERVO_SKIP) {
       RXTX_addr[port][RX_position[port]] = SERVO_SKIP;
       RX_position[port]++;

       if (RX_position[port] >= RXTX_BUFF_MAX) {
         RX_cycle[port]++;
         RX_position[port] = 0;
       }
       if ((RX_cycle[port] == TX_cycle[port] && RX_position[port] > TX_position[port]) ||
            (RX_cycle[port] > TX_cycle[port]))
         servo_rx_error(port,2,data);

     }
   }

   RXTX_addr[port][RX_position[port]] = SERVO_SKIP;

   RX_position[port]++;

   if (RX_position[port] >= RXTX_BUFF_MAX) {
	RX_cycle[port]++;
	RX_position[port] = 0;
   }
   if ((RX_cycle[port] == TX_cycle[port] && RX_position[port] > TX_position[port]) ||
	(RX_cycle[port] > TX_cycle[port]))
     servo_rx_error(port,2,data);

 } else {

   if (RXTX_addr[port][RX_position[port]] != SERVO_SKIP) {

     *RXTX_addr[port][RX_position[port]] = data;
     RXTX_buff[port][RX_position[port]]  = SERVO_UNLOCK; //Unlock the data

     RXTX_addr[port][RX_position[port]] = SERVO_SKIP;

     RX_position[port]++;

     if (RX_position[port] >= RXTX_BUFF_MAX) {
	RX_cycle[port]++;
	RX_position[port] = 0;
     }
     if ((RX_cycle[port] == TX_cycle[port] && RX_position[port] > TX_position[port]) ||
          (RX_cycle[port] > TX_cycle[port]))
       servo_rx_error(port,3,data);

   } else {

     RX_position[port]++;
                                                                                                                             
     if (RX_position[port] >= RXTX_BUFF_MAX) {
        RX_cycle[port]++;
        RX_position[port] = 0;
     }
     if ((RX_cycle[port] == TX_cycle[port] && RX_position[port] > TX_position[port]) ||
          (RX_cycle[port] > TX_cycle[port]))
       servo_rx_error(port,3,data);

     servo_indication(port, data);

   }

 }

}

extern unsigned SCom_Error[4];

void servo_confirm(unsigned port, BYTE msg_status) {

  #ifdef SERVO_DEBUG
    if (msg_status == COM_ERROR) {
      kern_printf("(SCOM: PORT:%d ERROR:%d)",port,SCom_Error[port]);
    }
  #endif

  lock_write[port] = 0;

} 

int servo_send_msg(unsigned port, BYTE *msg, unsigned len_msg, BYTE *res, unsigned len_res) {

  int i = 0, old, oldcycle, RX = 0;
  volatile int start = kern_gettime(NULL), timeout = 0;
  SYS_FLAGS f; 

  while(lock_write[port] == 1 && timeout == 0)
     if ((start + SERVO_TIMEOUT) < kern_gettime(NULL)) timeout = 1;

  if (timeout == 1) {

    #ifdef SERVO_DEBUG
      kern_printf("(SERVO: TIMEOUT SEND)");
    #endif

    return -1;
  }

  lock_write[port] = 1;

  f = kern_fsave(); 

  old = TX_position[port];
  oldcycle = TX_cycle[port]; 

  while(i < len_msg) {
    RXTX_buff[port][TX_position[port]] = msg[i];
    RXTX_addr[port][TX_position[port]] = NULL;
    TX_position[port]++;
    if (TX_position[port] >= RXTX_BUFF_MAX) {
	TX_cycle[port]++;
	TX_position[port] = 0;
    }
    if (TX_position[port] == RX_position[port]) {
        TX_position[port] = old;
        TX_cycle[port] = oldcycle;
        lock_write[port] = 0;

	#ifdef SERVO_DEBUG
	  kern_printf("(SERVO: BUFFER FULL)");
	#endif

        kern_frestore(f);
        return -1;
    }
    i++;
  }

  i = 0;
  while(i < len_res) {
    RXTX_buff[port][TX_position[port]] = SERVO_LOCK; //Lock the data
    RXTX_addr[port][TX_position[port]] = res+i;
    RX = TX_position[port];
    TX_position[port]++;
    if (TX_position[port] >= RXTX_BUFF_MAX) {
	TX_cycle[port]++;
	TX_position[port] = 0;
    }
    if (TX_position[port] == RX_position[port]) {
        TX_position[port] = old;
        TX_cycle[port] = oldcycle;
        lock_write[port] = 0;

	#ifdef SERVO_DEBUG
	  kern_printf("(SERVO: BUFFER FULL)");
	#endif

        kern_frestore(f);
        return -1;
    }
    i++;
  }

  /* SERVO_SKIP close the command sequence */
  RXTX_addr[port][TX_position[port]] = SERVO_SKIP;
  TX_position[port]++;
  if (TX_position[port] >= RXTX_BUFF_MAX) {
    TX_cycle[port]++;
    TX_position[port] = 0;
  }
  if (TX_position[port] == RX_position[port]) {
    TX_position[port] = old;
    TX_cycle[port] = oldcycle;
    lock_write[port] = 0;
                                                                                                                             
    #ifdef SERVO_DEBUG
      kern_printf("(SERVO: BUFFER FULL)");
    #endif
                                                                                                                             
    kern_frestore(f);
    return -1;
  }

  kern_frestore(f);

  com_irq_send(port, len_msg, msg);

  return RX;

}

int servo_wait(unsigned port, int RX) {

  volatile int start = kern_gettime(NULL), timeout = 0, temp;

  /* Active wait until number bytes received */
  while(RXTX_buff[port][RX] == SERVO_LOCK && timeout == 0)
     if ((start + SERVO_TIMEOUT) < kern_gettime(NULL)) timeout = 1;  

  if (timeout == 1) {
    SYS_FLAGS f;

    #ifdef SERVO_DEBUG
      kern_printf("(SERVO: TIMEOUT GET)");
    #endif

    f = kern_fsave();

    temp = RX;
    
    while(RXTX_addr[port][temp] != SERVO_SKIP) {
      RXTX_addr[port][temp] = SERVO_SKIP;
      RXTX_buff[port][temp] = SERVO_UNLOCK;
      temp--;
      if (temp < 0) temp = RXTX_BUFF_MAX-1;
    }
    
    kern_frestore(f);

  }

  return timeout;

}

int servo_open(int port,int speed)
{
  int err;

  err = com_open((unsigned)(port), (DWORD)speed, SERVO_PARITY, SERVO_LEN, SERVO_STOP);

  com_init_irq((unsigned)(port));

  com_set_functions((unsigned)(port),servo_confirm,servo_indication);

  com_irq_enable((unsigned)(port),ALL_IRQ);

  return err;

}

int servo_close(int port)
{
  int err;

  com_irq_disable((unsigned)(port),ALL_IRQ);

  com_close_irq((unsigned)(port));

  err = com_close((unsigned)(port));

  return err;

}

/* 1000.011w:bbbb.bbbb */
int servo_set_RS232_baudrate(int port, int baud)
{
  unsigned char b[2];
  int spbrg_temp, i;
  unsigned char spbrg, w;
  int servo_port = (unsigned)(port);

  i = 0;
  while(BaudTable[i] != baud && BaudTable[i] != -1) i++;
  if (BaudTable[i] == -1) {
    kern_printf("SERVO:Error wrong baud rate\n");
    return -1;
  }

  w = 1;
  spbrg_temp = (SERVO_CLOCK*10 / (16*baud)) - 10;
  if (spbrg_temp>2550) {
    w = 0;
    spbrg_temp = (SERVO_CLOCK*10 / (64*baud)) - 10;
  }
  spbrg = spbrg_temp / 10;
  if (spbrg_temp % 10 > 5) spbrg++;
  
  #ifdef SERVO_DEBUG
    kern_printf("(SERVO:SBPRG %d W %d)",spbrg,w);
  #endif

  b[0] = 0x86 | (w & 0x01);
  b[1] = spbrg;
  servo_send_msg(servo_port, b, 2, NULL, 0);

  return 0;

}

/* 1000.0101 */
int servo_get_RS232_baudrate(int port)
{
  unsigned char b[1],r[2];
  int res, res_w, res_b, t;
  int servo_port = (unsigned)(port);
  int RX;

  b[0] = 0x85;
  RX = servo_send_msg(servo_port, b, 1, r, 2); 

  if (RX != -1) {
                                                                                                                             
    t = servo_wait(servo_port, RX);
                                                                                                                             
    if (t == 0) {

      res_w = r[0]; /* bit W */
      res_b = r[1]; /* byte SPBRG */
  
      if (res_w != -1 && res_b != -1) {
        if (res_w)
          res = SERVO_CLOCK / ( 16 * (res_b + 1) );
        else
          res = SERVO_CLOCK / ( 64 * (res_b + 1) );
      } else {
        return -1;
      }

      return res;

    } else return -1;

  } else return -1;

}

/* 1000.0100 */
int servo_store_RS232_baudrate(int port)
{
  unsigned char b[1];
  int servo_port = (unsigned)(port);

  b[0] = 0x84;
  servo_send_msg(servo_port, b, 1, NULL, 0);
    
  return 0;

}

/* 1000.1010:llll.llll */
int servo_set_period(int port, int period)
{
  unsigned char b[2];
  int servo_port = (unsigned)(port);

  b[0] = 0x8A;
  b[1] = (period*1000)/TICK_LEN_PERIOD/8 & 0xFF;
  servo_send_msg(servo_port, b, 2, NULL, 0);  
  
  return 0;

}

/* 1000.1001 */
int servo_get_period(int port)
{
  unsigned char b[1],r[1];
  int res, RX, t;
  int servo_port = (unsigned)(port);

  b[0] = 0x89;
  RX = servo_send_msg(servo_port, b, 1, r, 1);

  if (RX != -1) {
                                                                                                                             
    t = servo_wait(servo_port, RX);
                                                                                                                             
    if (t == 0) {

      res = r[0];

      return (((unsigned char)(res))*TICK_LEN_PERIOD/1000*8);

    } else return -1;

  } else return -1;

}

/* 1000.1000 */
int servo_store_period(int port)
{
  unsigned char b[1];
  int servo_port = (unsigned)(port);

  b[0] = 0x88;
  servo_send_msg(servo_port, b, 1, NULL, 0);

  return 0;

}

/* 1000.1100 */
int servo_get_setup_switch(int port)
{
  unsigned char b[1],r[1];
  int res, RX, t;
  int servo_port = (unsigned)(port);

  b[0] = 0x8C;
  RX = servo_send_msg(servo_port, b, 1, r, 1);  

  if (RX != -1) {
                                                                                                                             
    t = servo_wait(servo_port, RX);
                                                                                                                             
    if (t == 0) {

       res = r[0];

       return res;

    } else return -1;

  } else return -1;

}

/* 1000.111s */
int servo_set_RC5_switch(int port, int data)
{
  unsigned char b[1];
  int servo_port = (unsigned)(port);

  b[0] = 0x8E | (data & 0x01);
  servo_send_msg(servo_port, b, 1, NULL, 0);

  return 0;

}

/* 1000.0000:0000.Mmmm */
int servo_turn_off(int port, int servo)
{
  unsigned char b[2];
  int servo_port = (unsigned)(port);

  if (servo > 15) return -1;

  b[0] = 0x80;
  b[1] = 0x00 | (servo & 0x0F);
  servo_send_msg(servo_port, b, 2, NULL, 0);
  
  return 0;

}

/* 1000.0000:0001.Mmmm */
int servo_turn_on(int port, int servo)
{
  unsigned char b[2];
  int servo_port = (unsigned)(port);

  if (servo > 15) return -1;

  b[0] = 0x80;
  b[1] = 0x10 | (servo & 0x0F);
  servo_send_msg(servo_port, b, 2, NULL, 0);

  return 0;

}

/* 1000.0000:0010.0000 */
int servo_turn_off_all(int port)
{
  unsigned char b[2];
  int servo_port = (unsigned)(port);

  b[0] = 0x80;
  b[1] = 0x20;
  servo_send_msg(servo_port, b, 2, NULL, 0);

  return 0;

}

/* 1000.0000:0010.0001 */
int servo_turn_on_all(int port)
{
  unsigned char b[2];
  int servo_port = (unsigned)(port);

  b[0] = 0x80;
  b[1] = 0x21;
  servo_send_msg(servo_port, b, 2, NULL, 0);
    
  return 0;

}

/* 1000.0000:0101.000M:mmmm.mmmm */
int servo_set_levels(int port, int bank, int mask)
{
  unsigned char b[3];
  int servo_port = (unsigned)(port);

  b[0] = 0x80;
  b[1] = 0x50 | (0x01 & bank);
  b[2] = (unsigned char)(mask & 0xFF);
  servo_send_msg(servo_port, b, 3, NULL, 0);

  return 0;

}

/* 1000.0000:0100.000M */
int servo_get_levels(int port, int bank)
{
  unsigned char b[2],r[1];
  int res, RX, t;
  int servo_port = (unsigned)(port);

  b[0] = 0x80;
  b[1] = 0x40 | (0x01 & bank);
  RX = servo_send_msg(servo_port, b, 2, r, 1);

  if (RX != -1) {
                                                                                                                             
    t = servo_wait(servo_port, RX);
                                                                                                                             
    if (t == 0) {

      res = r[0];

      return res;

    } else return -1;

  } else return -1;

}

/* 1000.0000:1000.0000 */
int servo_store_levels(int port)
{
  unsigned char b[2];
  int servo_port = (unsigned)(port);

  b[0] = 0x80;
  b[1] = 0x80;
  servo_send_msg(servo_port, b, 2, NULL, 0);
  
  return 0;

}

int servo_set_max_angle_sec(int port, int servo, int angle_sec)
{

  servo_table[port][servo].max_angle_sec = angle_sec;
  return 0;

}

int servo_set_min_angle_sec(int port, int servo, int angle_sec)
{

  servo_table[port][servo].min_angle_sec = angle_sec;
  return 0;

}

int servo_set_servo_tick(int port, int servo, int zero_tick, int delta_tick)
{

  if (zero_tick != -1) servo_table[port][servo].zero_tick = zero_tick;
  if (delta_tick != -1) servo_table[port][servo].delta_tick = delta_tick;
  return 0;

} 

/* 0000.Pppp:0000.vvvv:vvvv.vvvv */
int servo_set_angle_sec(int port, int servo, int angle_sec)
{
  unsigned char b[3];
  int angle_tick, res = 0;
  int servo_port = (unsigned)(port);

  if (servo > 15) return -1;

  angle_tick = (servo_table[port][servo].zero_tick + angle_sec * 
		servo_table[port][servo].delta_tick /
                (servo_table[port][servo].max_angle_sec - servo_table[port][servo].min_angle_sec)) * 1000 / TICK_LEN;

  b[0] = 0x00 | (servo & 0x0F);
  b[1] = 0x00 | ((angle_tick >> 8) & 0x0F);
  b[2] = 0x00 | (angle_tick & 0xFF);
  res = servo_send_msg(servo_port, b, 3, NULL, 0);

  return res;

}

/* 0010.Pppp */
int servo_store_default_position(int port, int servo)
{
  unsigned char b[1];
  int res = 0;
  int servo_port = (unsigned)(port);

  if (servo > 15) return -1;

  b[0] = 0x20 | (servo & 0x0F);
  res = servo_send_msg(servo_port, b, 1, NULL, 0);  

  return res;

}

/* 0001.Pppp */
int servo_get_angle_sec(int port, int servo)
{
  unsigned char b[1],r[2];
  int res,data,RX,t;
  int servo_port = (unsigned)(port);

  if (servo > 15) return -1;

  b[0] = 0x10 | (servo & 0x0F);
  RX = servo_send_msg(servo_port, b, 1, r, 2);

  if (RX != -1) {

    t = servo_wait(servo_port, RX);

    if (t == 0) {

      res = (int)(r[0]) << 8;
      res |= r[1];

      data = ((res*TICK_LEN/1000) - servo_table[port][servo].zero_tick) *
              (servo_table[port][servo].max_angle_sec - servo_table[port][servo].min_angle_sec) /
               servo_table[port][servo].delta_tick;

      return data;

    } else return -1;

  } else return -1;

}

/* 0100:baaa */
int servo_get_analog(int port, int adport, int nbit)
{
  unsigned char b[1],r[2];
  int res, RX, t, nb;
  int servo_port = (unsigned)(port);

  if (adport > 7) return -1;
  nb = (nbit==8) ? 0 : 1;

  b[0] = 0x40 | (adport & 0x07);
  if (nb) {
    RX = servo_send_msg(servo_port, b, 1, r, 2);
  } else {
    b[0] |= 0x08;
    RX = servo_send_msg(servo_port, b, 1, r, 1);
  } 

  if (RX != -1) {

    t = servo_wait(servo_port, RX);

    if (t == 0) {

      if (nb) {
        res = (int)r[0] << 8;
        res |= r[1];
      } else {
        res = r[0];
      }

      return res;

    } else return -1;
                                                                                                                             
  } else return -1;

}

