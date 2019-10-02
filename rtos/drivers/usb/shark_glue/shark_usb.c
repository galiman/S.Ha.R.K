
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

extern int usb_init(void);
extern void usb_exit(void);

extern int ohci_hcd_pci_init (void);
extern void ohci_hcd_pci_cleanup (void);

extern int uhci_hcd_init(void);
extern void uhci_hcd_cleanup(void);

extern int ehci_hcd_init(void);
extern void ehci_hcd_cleanup(void);

extern int usb_mouse_init(void);
extern void usb_mouse_exit(void);

extern int usb_kbd_init(void);
extern void usb_kbd_exit(void);

extern int hid_init(void);
extern void hid_exit(void);


static int usb_installed = FALSE;

/* to do: return error code */
int USB26_init()
{
	if (usb_installed == TRUE)
	return 0;

	usb_init();
	ehci_hcd_init();
	ohci_hcd_pci_init();
	uhci_hcd_init();
	usb_mouse_init();
	usb_kbd_init();
	hid_init();

	usb_installed = TRUE;

	return 0;
}

/* to do : add all usb closing functions ?*/
int USB26_close()
{
	if (usb_installed == FALSE)
		return -1;
	
	ehci_hcd_cleanup();
 	ohci_hcd_pci_cleanup();
	uhci_hcd_cleanup(); 
	usb_mouse_exit();
	usb_kbd_exit();
	hid_exit();
	usb_exit(); 

	usb_installed = FALSE;

	return 0;
}
