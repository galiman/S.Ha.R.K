
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
#include "semaphore.h"
#include "ll/sys/cdefs.h"

__BEGIN_DECLS


typedef struct sb_device {
    DWORD BaseAddress;
    BYTE IntLine;
    BYTE DMA8Channel;
    BYTE DMA16Channel;
    BYTE DSPVersionM;
    BYTE DSPVersionm;
//    WORD period;
//    BYTE pwarning;
    TASK_MODEL *m;
} SB_DEVICE;

typedef struct sound_buffer {
    sem_t synchr;
    BYTE synch;
    struct dma_buff *sound_dma;
    int (*fun)(struct dma_buff *buff);
} SOUND_BUFFER;

typedef struct rawfuncs {
    int (*infun)(void *rawbuffer);
    BYTE infunpresent;
    int (*outfun)(void *rawbuffer);
    BYTE outfunpresent;
} RAWFUNCS;

void sb_spkoff (void);
void sb_spkon (void);
int sb_init (void); // return 0 if ok or ENODEV otherwise
void sbmixer_setmiclev(BYTE level);
void sbmixer_setoutput(BYTE in, BYTE onoff);
void sbmixer_setinput(BYTE in, BYTE onoff);
void sbmixer_reset(void);
void sbmixer_setingainlev(BYTE level);
void sbmixer_setAGC(BYTE onoff);
void sb_setrate (int sps, BYTE i_o);
void sb_dmaop(BYTE i_o);
void sb_dma16op(BYTE i_o);
void sb_dmabuffop(BYTE i_o);
void sb_dma16buffop(BYTE i_o);
void sb_stopdsp(BYTE b);
void sb_show(void);

__END_DECLS
