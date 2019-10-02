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

/* Glue Layer for Linux PCI 2.6 Driver */
    
#include <kernel/kern.h>

extern int pci_direct_init(void);
extern int pcibios_init(void);
extern int pcibios_irq_init(void);
extern int pci_driver_init(void); 
extern int pci_legacy_init(void);
extern int pci_init(void);

static int pci_installed = FALSE;

/* Init the Linux PCI 2.6 Driver */
int PCI26_installed(void)
{
	return pci_installed;
}

int PCI26_init() {

	if (pci_installed == TRUE)
		return 0;

	pci_direct_init();

	pcibios_init();

	pci_driver_init(); 

	pci_legacy_init();
  
	pcibios_irq_init();

	pci_init();

	return 0;

}
