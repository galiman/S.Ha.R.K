
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

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*	DSP Registers	*/
#define RESET			0x06
#define READDATA		0x0A
#define WRITEBUFFERSTATUS	0x0C
#define DATAAVAILABLE		0x0E
#define ACK8			0x0E
#define	ACK16			0x0F

/*	DSP Commands	*/
#define DSPCMD_DIRECTOUT	0x10
#define DSPCMD_DIRECTIN		0x20
#define DSPCMD_SETOUTRATE	0x41
#define DSPCMD_SETINRATE	0x42
#define DSPCMD_16BITIO		0xb0
#define DSPCMD_8BITIO		0xC0
	#define SBIO_FIFOON	0x02
	#define SBIO_AUTOINIT	0x04
	#define SBIO_OUT	0x00
	#define SBIO_IN		0x08
	#define IOMODE_UNSIGNED 0x00
	#define IOMODE_SIGNED	0x10
	#define IOMODE_MONO	0x00
	#define IOMODE_STEREO	0x20
#define DSPCMD_GETVER		0xE1
#define DSPCMD_SPKON		0xD1
#define DSPCMD_SPKOFF		0xD3
#define DSPCMD_EXIT8		0xDA
#define DSPCMD_EXIT16		0xD9

/*	Mixer I/O registers	*/
#define MIXERREGISTER		0x04
#define MIXERDATA		0x05

/*	Mixer Registers		*/
#define MIXER_RESET		0x00
#define MIXER_OUTCTR		0x3c
#define MIXER_INCTRLEFT		0x3d
#define MIXER_INCTRRIGHT	0x3e
#define MIXER_MICLEV		0x3a
#define MIXER_AGC		0x43
#define MIXER_IGCLEFT		0x3F
#define MIXER_IGCRIGHT		0x40

#define MIXER_IRQREG		0x80
#define MIXER_DMAREG		0x81
#define MIXER_INTSTATUS		0x82

WORD sb_probe (void);
BYTE sbdsp_read (WORD base);
void sbdsp_write (WORD base, BYTE c );
BYTE sbmixer_read(WORD base, BYTE index);
void sbmixer_write(WORD base, BYTE index, BYTE c);
BYTE sbdsp_reset(WORD base);

/* OKKIO!!!! vanno tolti da qui!!!! */
BYTE inpb(WORD addr);
void outpb(WORD addr, BYTE val);

__END_DECLS

