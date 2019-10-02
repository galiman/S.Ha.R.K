
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
#ifndef __SERVO_H__
#define __SERVO_H__

#ifndef COM1
#define COM1 0
#define COM2 1
#define COM3 2
#define COM4 3
#endif

int servo_open(int port, int speed);
int servo_close(int port);

/* Setup */

int servo_set_RS232_baudrate(int port, int baud); /* BaudRate */
int servo_get_RS232_baudrate(int port);
int servo_store_RS232_baudrate(int port);

int servo_set_period(int port, int period); /* Servo period in us */
int servo_get_period(int port);
int servo_store_period(int port);

int servo_get_setup_switch(int port); /* RC0 RC1 RC2 */
int servo_set_RC5_switch(int port, int data);

/* Servo control */

/* Convert angle (degree, minute, second -> second) */
#define ANGLE2SEC(deg,min,sec) ((deg)*3600 + (min)*60 + (sec))

int servo_turn_off(int port, int servo);
int servo_turn_on(int port, int servo);
int servo_turn_off_all(int port);
int servo_turn_on_all(int port);

int servo_set_levels(int port, int bank, int mask);
int servo_get_levels(int port, int bank);
int servo_store_levels(int port);

int servo_set_max_angle_sec(int port, int servo, int angle_sec);
int servo_set_min_angle_sec(int port, int servo, int angle_sec);
int servo_set_servo_tick(int port, int servo, int zero_tick, int delta_tick);

int servo_set_angle_sec(int port, int servo, int angle_sec);
int servo_get_angle_sec(int port, int servo);

int servo_store_default_position(int port, int servo);

/* Analog control */

#define MAX_ANALOG_10 0x03FF
#define MAX_ANALOG_8  0x00FF

int servo_get_analog(int port, int adport, int nbit);

#endif

