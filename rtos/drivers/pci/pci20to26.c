/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Mauro Marinoni <mauro.marinoni@unipv.it>
 *   (see the web pages for full authors list)
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
 */

#include <linuxcomp.h>
#include <linux/pci.h>

//#define __DEBUG_20TO26__

void linuxpci_init(void)
{
			// PCI26_init MUST be called first ( TODO - Check )
}

int pci20to26_find_class(unsigned int class_code, int index, BYTE *bus, BYTE *dev)
{
	struct pci_dev *ptmp = NULL;
	int curr = 0;

	ptmp = pci_find_class(class_code, ptmp);
	while (ptmp) {
#ifdef __DEBUG_20TO26__
		printk(KERN_DEBUG "pci20to26_find_class %x: found at bus %d, dev %d.\n", ptmp->class, ptmp->bus->number, ptmp->devfn);
#endif

		if (curr == index) {
			*bus = ptmp->bus->number;
			*dev = ptmp->devfn;
			
#ifdef __DEBUG_20TO26__
			printk(KERN_DEBUG "pci20to26_find_class: found at bus %d, dev %d\n", *bus, *dev);
#endif

			return PCIBIOS_SUCCESSFUL;
		}
		++curr;
		ptmp = pci_find_class(class_code, ptmp);
	} 

	return PCIBIOS_DEVICE_NOT_FOUND;
}

int pcibios_present(void)
{
	return 1;	// PCI26 initialized? ( TODO - Check )
}

int pci20to26_read_config_byte(unsigned int bus, unsigned int dev, int where, u8 *val)
{
	struct pci_dev tmp;
	struct pci_dev *ptmp = &tmp;

	ptmp = pci_find_slot(bus, dev);
	if (!ptmp)
		return 0;
	pci_read_config_byte(ptmp, where, val);

#ifdef __DEBUG_20TO26__
	printk(KERN_DEBUG "pci20to26_read_config_byte: (bus %d, dev %d) -> %d\n", bus, dev, *val);
#endif

	return 1;
}

int pci20to26_read_config_word(unsigned int bus, unsigned int dev, int where, u16  *val)
{
	struct pci_dev tmp;
	struct pci_dev *ptmp = &tmp;

	ptmp = pci_find_slot(bus, dev);
	if (!ptmp)
		return 0;
	pci_read_config_word(ptmp, where, val);

#ifdef __DEBUG_20TO26__
	printk(KERN_DEBUG "pci20to26_read_config_word: (bus %d, dev %d) -> %d\n", bus, dev, *val);
#endif

	return 1;
}

int pci20to26_read_config_dword(unsigned int bus, unsigned int dev, int where, u32 *val)
{
	struct pci_dev tmp;
	struct pci_dev *ptmp = &tmp;

	ptmp = pci_find_slot(bus, dev);
	if (!ptmp)
		return 0;
	pci_read_config_dword(ptmp, where, val);

#ifdef __DEBUG_20TO26__
	printk(KERN_DEBUG "pci20to26_read_config_dword: (bus %d, dev %d) -> %uld\n", bus, dev, *val);
#endif

	return 1;
}

int pci20to26_write_config_byte(unsigned int bus, unsigned int dev, int where, u8 val)
{
	struct pci_dev tmp;
	struct pci_dev *ptmp = &tmp;

	ptmp = pci_find_slot(bus, dev);
	if (!ptmp)
		return 0;
	pci_write_config_byte(ptmp, where, val);

	return 1;
}

int pci20to26_write_config_word(unsigned int bus, unsigned int dev, int where, u16 val)
{
	struct pci_dev tmp;
	struct pci_dev *ptmp = &tmp;

	ptmp = pci_find_slot(bus, dev);
	if (!ptmp)
		return 0;
	pci_write_config_word(ptmp, where, val);

	return 1;
}

int pci20to26_write_config_dword(unsigned int bus, unsigned int dev, int where, u32 val)
{
	struct pci_dev tmp;
	struct pci_dev *ptmp = &tmp;

	ptmp = pci_find_slot(bus, dev);
	if (!ptmp)
		return 0;
	pci_write_config_dword(ptmp, where, val);

	return 1;
}
