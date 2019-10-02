/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: pci.h,v 1.2 2004/05/11 16:03:40 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.2 $
 Last update: $Date: 2004/05/11 16:03:40 $
 ------------

**/

/*
 * Copyright (C) 2000 Luca Abeni
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

/* Project:     HARTIK 3.0 Network Library                      */
/* Description: Hard Real TIme Kernel for 8086 compatible       */
/* Author:      Luca Abeni					*/
/* Date:        14/4/1997                                       */
                           
/* File:        PCI.H                                           */
/* Revision:    1.02                                            */

#ifndef __PCI_H__
#define __PCI_H__

#include <linux/llpci.h>
#include <linux/linuxpci.h>

#ifdef __cplusplus
extern "C" {
#endif

#define N_MAX_DEVS 10

typedef struct pci_regs {
	WORD VendorId;
	WORD DeviceId;
	WORD PciCommand;
	WORD PciStatus;
	BYTE reserved1;
	BYTE dummy;
	WORD ClassCode;
	BYTE reserved2;
	BYTE LatencyTimer;
	BYTE HeaderType;
	BYTE reserved3;
	DWORD IoBaseAddress;
	DWORD reserved4;
	DWORD reserved5;
	DWORD reserved6;
	DWORD reserved7;
	DWORD reserved8;
	DWORD reserved9;
	DWORD reserved10;
	DWORD BiosRomControl;
	DWORD reserved11;
	DWORD reserver12;
	BYTE InterruptLine;
	BYTE InterruptPin;
	BYTE MinGnt;
	BYTE MaxLat;
	DWORD InternalConfig;
} PCI_REGS;

typedef struct pci_des {
		BYTE bus;
		BYTE dev;
		BYTE mem[256];
} PCI_DES;

/*
 * Error values that may be returned by the PCI bios.  Use
 * pcibios_strerror() to convert to a printable string.
 */
#define PCIBIOS_SUCCESSFUL		0x00
#define PCIBIOS_FUNC_NOT_SUPPORTED	0x81
#define PCIBIOS_BAD_VENDOR_ID		0x83
#define PCIBIOS_DEVICE_NOT_FOUND	0x86
#define PCIBIOS_BAD_REGISTER_NUMBER	0x87
#define PCIBIOS_SET_FAILED		0x88
#define PCIBIOS_BUFFER_TOO_SMALL	0x89


/*int pci_detect(struct pci_des *p_des);
void pci_show(struct pci_des *PCIdes, int n);*/
int pci_init(void);
void pci_show(void);
int scan_bus(struct pci_des *p_des);
struct pci_regs *pci_class(DWORD class_code, WORD index, BYTE *bus, BYTE *dev);
struct pci_regs *pci_device(WORD vendor, WORD device, WORD index, BYTE *bus, BYTE *dev);

#ifdef __cplusplus
};
#endif

#endif

