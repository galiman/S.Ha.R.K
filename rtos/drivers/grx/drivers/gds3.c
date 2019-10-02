
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

#include <ll/i386/hw-data.h>
#include <ll/i386/mem.h>
/*
#include <ll/i386/x-dos.h>
#include <ll/i386/x-dosmem.h>
*/
#include <ll/i386/cons.h>
#include <ll/sys/ll/ll-func.h>

#include "s3.h"
#include "chips.h"


enum {
    S3_911, S3_924, S3_801, S3_805, S3_928, S3_864, S3_964, S3_TRIO32,
    S3_TRIO64, S3_866, S3_868, S3_968, S3_765, S3_VIRGE, S3_VIRGE_VX,
    S3_UNKNOWN
};

static const char *s3_chipname[] =
{"911", "924", "801", "805", "928",
 "864", "964", "Trio32", "Trio64",
 "866", "868", "968", "Trio64V+",
 "ViRGE", "ViRGE VX",
 "Unknown S3 chip"};




#define CRT_IC  0x3D4           /* CRT Controller Index - color emulation */
#define CRT_DC  0x3D5           /* CRT Controller Data Register - color emulation */

/* flags used by this driver */
#define S3_LOCALBUS		0x01
#define S3_CLUT8_8		0x02
#define S3_OLD_STEPPING		0x04

DWORD s3_linear_addr = 0;

int s3_chiptype;
int s3_flags;
int s3_memory;

/*	S3 specific Graphic drivers...	*/

static void s3_lock(void)
{
    outp(0x3d4, 0x39);
    outp(0x3d5, 0x00);
    outp(0x3d4, 0x38);
    outp(0x3d5, 0x00);
}

static void s3_lock_enh(void)
{
    BYTE tmp;
    
    if (s3_chiptype > S3_911) {
	outp(0x3d4, 40);
	tmp = inp(0x3d5);
	outp(0x3d5, tmp & ~0x01);
    }
    s3_lock();
}


static void s3_unlock(void)
{
    outp(0x3d4, 0x38);
    outp(0x3d5, 0x48);
    outp(0x3d4, 0x39);
    outp(0x3d5, 0xA5);
}

static void s3_unlock_enh(void)
{
    BYTE tmp;

    s3_unlock();
    if (s3_chiptype > S3_911) {
	outp(0x3d4, 0x40);
	tmp = inp(0x3d5);
	outp(0x3d5, tmp | 0x01);
    }
}

static int s3_init(int par1, int par2)
{
    int id, rev;
    BYTE config;

    s3_unlock();

    s3_flags = 0;               /* initialize */
    outp(0x3d4, 0x30);	/* Get chip id. */
    id = inp(0x3d5);
    rev = id & 0x0F;
    if (id >= 0xE0) {
	outp(0x3d4, 0x2D);
	id = inp(0x3d5) << 8;
	outp(0x3d4, 0x2E);
	id |= inp(0x3d5);
	outp(0x3d4, 0x2F);
	rev = inp(0x3d5);
    }
    s3_chiptype = -1;
    outp(0x3d4, 0x36);	/* Get config info */
    config = inp(0x3d5);
    switch (id & 0xf0) {
	case 0x80:
            if (rev == 1) {
		s3_chiptype = S3_911;
		break;
            }
            if (rev == 2) {
		s3_chiptype = S3_924;
		break;
            }
            break;
	case 0xa0:
	    switch (config & 0x03) {
		case 0x00:
		case 0x01:
		    /* EISA or VLB - 805 */
		    s3_chiptype = S3_805;
		    /* ARI: Test stepping: 0:B, 1:unknown, 2,3,4:C, 8:I, >=5:D */
		    if ((rev & 0x0F) < 2)
			s3_flags |= S3_OLD_STEPPING;        /* can't handle 1152 width */
                    break;
		case 0x03:
		    /* ISA - 801 */
		    s3_chiptype = S3_801;
		    /* Stepping same as 805, just ISA */
		    if ((rev & 0x0F) < 2)
			s3_flags |= S3_OLD_STEPPING;        /* can't handle 1152 width */
		    break;
            }
	    break;
	case 0x90:
	    s3_chiptype = S3_928;
	    if ((rev & 0x0F) < 4)       /* ARI: Stepping D or below */
		s3_flags |= S3_OLD_STEPPING;    /* can't handle 1152 width */
	    break;
	case 0xB0:
            /* 928P */
            s3_chiptype = S3_928;
            break;
        case 0xC0:
            s3_chiptype = S3_864;
            break;
        case 0xD0:
            s3_chiptype = S3_964;
            break;
	default:
	    switch (id) {
		case 0x8811:
		    switch (id & 0xFFF0) {
			case 0x10E0:
			    s3_chiptype = S3_TRIO32;
                	    break;
			case 0x01E0: /* S3Trio64V2/DX ... any others? */
            		case 0x11E0:
                	    if (rev & 0x0400)
                    		s3_chiptype = S3_765;
                	    else
                    		s3_chiptype = S3_TRIO64;
                	    break;
		    }
            	case 0x883d:  /*ME*/
		    s3_chiptype = S3_VIRGE_VX;
                    break;
            	case 0x5631: /* ViRGE ID */
		    s3_chiptype = S3_VIRGE;
                    break;
            	case 0x8901:  /*ME*/
		    s3_chiptype = S3_765;
                    break;
            	case 0x80E0:
                    s3_chiptype = S3_866;	/* Not Ok... */
                    break;
            	case 0x8880:
                    s3_chiptype = S3_868;
                break;
            	case 0x88f0:        /* XXXX From data book; XF86 says 0xB0E0? */
                    s3_chiptype = S3_968;
                    break;
            	default:
		    s3_chiptype = S3_UNKNOWN;
		    break;
            }
    }
/*	s3_lock();*/
    if (s3_chiptype == -1) {
	return -1;
    }
    if (s3_chiptype == S3_UNKNOWN) {
	cprintf("svgalib: S3: Unknown chip id %02x\n", id);
	return -1;
    }
    if (s3_chiptype <= S3_924) {
	if ((config & 0x20) != 0) {
	    s3_memory = 512;
	} else {
	    s3_memory = 1024;
	}
    } else {
	if (s3_chiptype == S3_VIRGE_VX) {
	    BYTE config2;
	    int m1;

	    outp(0x3d4, 0x37);
	    config2 = inp(0x3d5);
	    switch ((config & 0x60) >> 5) {
		case 0:
		    s3_memory = 2 * 1024;
		    break;
		case 1:
		    s3_memory = 4 * 1024;
		    break;
		case 2:
		    s3_memory = 6 * 1024;
		    break;
		case 3:
		    s3_memory = 8 * 1024;
		    break;
	    }
	    m1 = 0;
	    switch ((config2 & 0x60) >> 5) {
		case 0:
		    m1 = 4 * 1024;
		    break;
		case 2:
		    m1 = 2 * 1024;
		    break;
	    }
	    cprintf("M1: %d        M2: %d\n", s3_memory, m1);
	    s3_memory -= m1;
	} else {
	    /* look at bits 5, 6 and 7 */
	    switch ((config & 0xE0) >> 5) {
		case 0:
		    s3_memory = 4096;
		    break;
		case 2:
		    s3_memory = 3072;
		    break;
		case 3:
		    s3_memory = 8192;
		    break;
		case 4:
		    s3_memory = 2048;
		    break;
		case 5:
		    s3_memory = 6144;
		    break;
		case 6:
		    s3_memory = 1024;
		    break;
		case 7:
		    s3_memory = 512;
		    break;          /* Trio32 */
	    }
	}
    }
    if ((config & 0x03) < 3)        /* XXXX 928P is ignored */
    s3_flags |= S3_LOCALBUS;

    cprintf("svgalib: Using S3 driver (%s, %dK).\n", s3_chipname[s3_chiptype],
               s3_memory);
    if (s3_flags & S3_OLD_STEPPING)
	cprintf("svgalib: Chip revision cannot handle modes with width 1152.\n");
    if (s3_chiptype > S3_864) {
	cprintf("svgalib: s3: chipsets newer than S3-864 is not supported well yet.\n");
    }

#ifdef DAC
    cardspecs = malloc(sizeof(CardSpecs));
    cardspecs->videoMemory = s3_memory;
    cardspecs->nClocks = 0;
    /* cardspecs->maxHorizontalCrtc = 2040; SL: kills 800x600x32k and above */
    cardspecs->maxHorizontalCrtc = 4088;
    cardspecs->flags = INTERLACE_DIVIDE_VERT;

    /* Process S3-specific config file options. */
    __svgalib_read_options(s3_config_options, s3_process_option);

#ifdef INCLUDE_S3_TRIO64_DAC
    if ((s3_chiptype == S3_TRIO64 || s3_chiptype == S3_765) && dac_used == NULL)        dac_used = &__svgalib_Trio64_methods;
#endif

    if (dac_used == NULL)
        dac_used = __svgalib_probeDacs(dacs_to_probe);
    else
        dac_used->initialize();


    if (dac_used == NULL) {
        /* Not supported. */
        cprintf("svgalib: s3: Assuming normal VGA DAC.\n");

#ifdef INCLUDE_NORMAL_DAC
        dac_used = &__svgalib_normal_dac_methods;
        dac_used->initialize();
#else
        cprintf("svgalib: Alas, normal VGA DAC support is not compiled in, goodbye.\n");
        return 1;
#endif
    }
    if (clk_used)
        clk_used->initialize(cardspecs, dac_used);

    dac_used->qualifyCardSpecs(cardspecs, dac_speed);

    /* Initialize standard clocks for unknown DAC. */
    if ((!(cardspecs->flags & CLOCK_PROGRAMMABLE))
        && cardspecs->nClocks == 0) {
        /*
         * Almost all cards have 25 and 28 MHz on VGA clocks 0 and 1,
         * so use these for an unknown DAC, yielding 640x480x256.
         */
        cardspecs->nClocks = 2;
        cardspecs->clocks = malloc(sizeof(int) * 2);
        cardspecs->clocks[0] = 25175;

        cardspecs->clocks[1] = 28322;
    }
    /* Limit pixel clocks according to chip specifications. */
    if (s3_chiptype == S3_864 || s3_chiptype == S3_868) {
        /* Limit max clocks according to 95 MHz DCLK spec. */
        /* SL: might just be 95000 for 4/8bpp since no pixmux'ing */
        LIMIT(cardspecs->maxPixelClock4bpp, 95000 * 2);
        LIMIT(cardspecs->maxPixelClock8bpp, 95000 * 2);
        LIMIT(cardspecs->maxPixelClock16bpp, 95000);
        /* see explanation below */
        LIMIT(cardspecs->maxPixelClock24bpp, 36000);
        /*
         * The official 32bpp limit is 47500, but we allow
         * 50 MHz for VESA 800x600 timing (actually the
         * S3-864 doesn't have the horizontal timing range
         * to run unmodified VESA 800x600 72 Hz timings).
         */
        LIMIT(cardspecs->maxPixelClock32bpp, 50000);
    }
    __svgalib_driverspecs = &__svgalib_s3_driverspecs;
#endif		/*	DAC & c.	*/
    s3_lock();
    return 0;
}

int s3_test(void)
{
    BYTE new_val1, new_val2;

    s3_unlock();
    outp(0x3d4, 0x47);
    outp(0x3d5, 0xff);
    new_val1 = inp(0x3d5);
    outp(0x3d5, 0x00);
    new_val2 = inp(0x3d5);
    s3_lock();

    if ((new_val1 == 0xff) && (new_val2 == 0x00)) {
	if (s3_init(0, 0))       /* type not OK */
	    return 0;
	return 1;
    }
    cprintf("V1: %x         V2: %x\n", new_val1, new_val2);
    return 0;
}

DWORD s3_getmem(void)
{
    return s3_memory * 1024;
}

void s3_showinfo(void)
{
    cprintf("\t Using S3 driver (%s, %dK).\n", s3_chipname[s3_chiptype],
               s3_memory);
    if (s3_flags & S3_OLD_STEPPING) {
	cprintf("\t Chip revision cannot handle modes with width 1152.\n");
    }
    if (s3_chiptype > S3_864) {
	cprintf("\t s3: chipsets newer than S3-864 is not supported well yet.\n");
    }
    cprintf ("\t s3: Frame Linear Buffer @ %lx (%ld)\n", s3_linear_addr, s3_linear_addr);
}

/*	These are important!!!	*/
void s3_setpage(int page)
{
    s3_unlock();
    outp(0x3d4, 0x35);
    outp(0x3d5, (inp(0x3d5) & 0xF0) | (page & 0x0F));
/*	is it important???
    if (s3_chiptype >= S3_801) {
        outb(0x3d4, 0x51);
        outb(0x3d5, (inb(0x3d5) & ~0x0C) | ((page & 0x30) >> 2));
    }
*/
    inp(0x3d5);                 /* ARI: Ferraro says: required for first generation 911 only */
    s3_lock();
}

void s3_setpage864(int page)
{
    /* Let's try this... */
    s3_unlock();
    /* "Linear" mode banking. */
    outp(0x3d4, 0x6A);
    outp(0x3d5, (inp(0x3d5) & ~0x3F) | page);
    s3_lock();
}

void s3_linear(DWORD l_a)
{
    BYTE r59, r5A, tmp, linopt;
    BYTE i3;
    
    s3_setpage(0);
    r59 = l_a >> 24;
    r5A = (l_a >> 16) & 0x08;
    if (s3_memory <= 1024) {
	linopt = 0x15;
    } else if (s3_memory <= 2048) {
	linopt = 0x16;
    } else {
	linopt = 0x17;
    }

    s3_unlock();

/*#ifdef OLD*/
/*	4 805...	*/
      outp(0x3d4, 0x40);
      i3 = inp(0x3d5);
      i3 = (i3 & 0xf6) | 0x0a;/* enable fast write buffer and disable
				 * 8514/a mode */
      outp(0x3d4, i3);
/*      DISABLE_MMIO;*/
      { unsigned char tmp; 
      outp(0x3d4, 0x53); 
      tmp = inp(0x3d5);
      outp(0x3d5, tmp & 0xEF); }
/*      outb (vgaCRReg, s3LinApOpt | s3SAM256);
      if (!S3_x64_SERIES(s3ChipId)) {
         outb (vgaCRIndex, 0x54);
         outb (vgaCRReg, (s3Port54 + 07));
      }
*/
/*#endif*/
    
    outp(0x3d4, 0x58);
    tmp = inp(0x3d5) /*& 0x80*/;
/*    outp(0x3d5, linopt & ~0x04 | tmp);*/
    outp(0x3d5, linopt | tmp);

    outp(0x3d4, 0x59);
    outp(0x3d5, r59);
    outp(0x3d4, 0x5A);
    outp(0x3d5, r5A);

    s3_lock();
}
