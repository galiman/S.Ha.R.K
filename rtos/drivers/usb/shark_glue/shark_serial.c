
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
#include <drivers/shark_usbserial26.h>

extern int usb_serial_init();
extern void usb_serial_exit(void);

extern int pl2303_init();
extern void pl2303_exit();

extern int serial_usbport_open(void *tty, int port_number);
extern int serial_usbport_write(void *tty, const unsigned char *buf, int count);
extern int serial_usbport_read(void *private, char* data_in);

int shark_usb_serial_init()
{
	usb_serial_init();
	pl2303_init();

	return 0;
}

void shark_usb_serial_close()
{
	usb_serial_exit();
	pl2303_exit();
}

int shark_usbcom_open(struct shark_tty_usbcom *tty_usb, int port_number)
{
	int retval;
	tty_usb->port_number = port_number;

	retval = serial_usbport_open(&tty_usb->tty, tty_usb->port_number);

	return retval;
}

int shark_usbcom_write(struct shark_tty_usbcom *tty_usb, const unsigned char *buf, int count)
{
	int retval;

	retval = serial_usbport_write(tty_usb->tty, buf, count);

	return retval;
}

char shark_usbcom_read(struct shark_tty_usbcom *tty_usb)
{
	int retval;
	char data_in;

	while ((retval = serial_usbport_read(tty_usb->tty, &data_in)) == 0);

	return data_in;
	/* do the hard stuff*/
}
