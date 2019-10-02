
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

#ifndef __SHARK_USBSERIAL26_H__
#define __SHARK_USBSERIAL26_H__

struct shark_tty_usbcom {
	void *tty;
	int port_number;
};

int shark_usb_serial_init(void);
void shark_usb_serial_close(void);

int 
shark_usbcom_open(struct shark_tty_usbcom *tty_usb, int port_number);

char
shark_usbcom_read(struct shark_tty_usbcom *tty_usb);

#endif
