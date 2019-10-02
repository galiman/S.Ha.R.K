/* Project:     HARTIK 3.0 Sound Library                        */
/* Description: Hard Real TIme Kernel for 8086 compatible       */
/* Author:      Luca Abeni					*/
/* Date:        5/12/1997                                       */

/* File: 	Blaster.C					*/
/* Revision:    3.0						*/

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

/* Sound Blaster 16 specific functions and structures */

#include <kernel/kern.h>

#include <drivers/dma.h>
#include "sbio.h"
#include "blaster.h"
#include <drivers/sound.h>
#include <semaphore.h>

struct sb_device sb_dev;
struct sound_buffer buff_des;

void sbmixer_reset(void)
{
    sbmixer_write(sb_dev.BaseAddress, MIXER_RESET, 0);
}

/* Enable or disable specified mixer inputs */
void sbmixer_setinput(BYTE in, BYTE onoff)
{
    BYTE inreg;

    inreg = sbmixer_read(sb_dev.BaseAddress, MIXER_INCTRLEFT);
    if (onoff == ENABLE)
    	inreg = inreg | in;
    else inreg = inreg & (!in);
    sbmixer_write(sb_dev.BaseAddress, MIXER_INCTRLEFT, inreg);
    inreg = sbmixer_read(sb_dev.BaseAddress, MIXER_INCTRRIGHT);
    if (onoff == ENABLE)
    	inreg = inreg | in;
    else inreg = inreg & (!in);
    sbmixer_write(sb_dev.BaseAddress, MIXER_INCTRRIGHT, inreg);
}

/* Enable or disable specified mixer outputs */
void sbmixer_setoutput(BYTE in, BYTE onoff)
{
    BYTE inreg;

    inreg = sbmixer_read(sb_dev.BaseAddress, MIXER_OUTCTR);
    if (onoff == ENABLE)
    	inreg = inreg | in;
    else inreg = inreg & (~in);
    sbmixer_write(sb_dev.BaseAddress, MIXER_OUTCTR, inreg);
}

/* Set the output level */
void sbmixer_setmiclev(BYTE level)
{
    BYTE levreg;

    levreg = sbmixer_read(sb_dev.BaseAddress, MIXER_MICLEV);
    levreg = (levreg & 0x07) | (level << 3);
    sbmixer_write(sb_dev.BaseAddress, MIXER_MICLEV, levreg);
}

/* Set the mixer input level */
void sbmixer_setingainlev(BYTE level)
{
    BYTE gainreg;

    gainreg = sbmixer_read(sb_dev.BaseAddress, MIXER_IGCRIGHT);
    gainreg = (gainreg & 0x3F) | (level << 6);
    sbmixer_write(sb_dev.BaseAddress, MIXER_IGCRIGHT, gainreg);
    gainreg = sbmixer_read(sb_dev.BaseAddress, MIXER_IGCLEFT);
    gainreg = (gainreg & 0x3F) | (level << 6);
    sbmixer_write(sb_dev.BaseAddress, MIXER_IGCLEFT, gainreg);
}

/* Enable or disable the mixer input Automatic Gain Control */
void sbmixer_setAGC(BYTE onoff)
{
    BYTE reg;

    reg = sbmixer_read(sb_dev.BaseAddress, MIXER_AGC);
    if (onoff == ENABLE)
    	reg = reg & 0xFE;
    else reg = reg | 0x01;
    sbmixer_write(sb_dev.BaseAddress, MIXER_AGC, reg);
}

int sb_init(void)
{
    BYTE cfgreg;

    /* Is there a SB16 in the system? */
    if((sb_dev.BaseAddress = sb_probe()) == 0)
      return ENODEV;

    /* Yes: get the DSP version and the used interrupt and DMA Channels */
    sbdsp_write(sb_dev.BaseAddress, DSPCMD_GETVER);
    sb_dev.DSPVersionM = sbdsp_read(sb_dev.BaseAddress);
    sb_dev.DSPVersionm = sbdsp_read(sb_dev.BaseAddress);
    cfgreg = sbmixer_read(sb_dev.BaseAddress, MIXER_IRQREG);
    if (cfgreg & 1)
	sb_dev.IntLine = 2;
    if (cfgreg & 2)
	sb_dev.IntLine = 5;
    if (cfgreg & 4)
	sb_dev.IntLine = 7;
    if (cfgreg & 8)
	sb_dev.IntLine = 10;
    
    // force irq line if you use a SB PNP!!!	
    // sb_dev.IntLine = 9;
    
    cfgreg = sbmixer_read(sb_dev.BaseAddress, MIXER_DMAREG);
    if (cfgreg & 1)
	sb_dev.DMA8Channel= 0;
    if (cfgreg & 2)
	sb_dev.DMA8Channel = 1;
    if (cfgreg & 8)
	sb_dev.DMA8Channel = 3;
    if (cfgreg & 0x20)
	sb_dev.DMA16Channel = 5;
    if (cfgreg & 0x40)
	sb_dev.DMA16Channel = 6;
    if (cfgreg & 0x80)
	sb_dev.DMA16Channel = 7;

    return 0;
}

/* Show the card informations */
void sb_show(void)
{
    cprintf("        BaseAddress: %x\n", (unsigned int)sb_dev.BaseAddress);
    cprintf("        Interrupt Line: %d\n", sb_dev.IntLine);
    cprintf("        DSP Version: %d.%d\n", sb_dev.DSPVersionM, sb_dev.DSPVersionm);
    cprintf("        8 bit DMA channel: %d\n", sb_dev.DMA8Channel);
    cprintf("        16 bit DMA channel: %d\n", sb_dev.DMA16Channel);
}

void sb_spkon (void)
{
    sbdsp_write(sb_dev.BaseAddress, DSPCMD_SPKON);
}

void sb_spkoff (void)
{
    sbdsp_write(sb_dev.BaseAddress, DSPCMD_SPKOFF);
}

/* Set the DMA sampling/playing rate */
void sb_setrate (int sps, BYTE i_o)
{
    BYTE cmd;

    if (i_o == IN) cmd = DSPCMD_SETINRATE;
    else cmd = DSPCMD_SETOUTRATE;
    sbdsp_write(sb_dev.BaseAddress, cmd);
    /* MSB...*/
    sbdsp_write(sb_dev.BaseAddress, (sps >> 8) & 0xFF);
    /*...and then LSB */
    sbdsp_write(sb_dev.BaseAddress, sps & 0xFF);
}

/* Start an 8 bit sampling/playing operation */
void sb_dmaop(BYTE i_o)
{
    DWORD len;
    BYTE cmd;
    BYTE *buff;

    buff = buff_des.sound_dma->p;
    len = buff_des.sound_dma->len;
    cmd = DSPCMD_8BITIO;

    /* Stop any previous operation and reset the DMAC */
    dma_stop(sb_dev.DMA8Channel);
    dma_reset();
    /* Prepare the DMAC for the operation */
    if (i_o == OUT) {
	cmd |= SBIO_OUT;
	dma_setmode(sb_dev.DMA8Channel, 0x48);
    } else {
	cmd |= SBIO_IN;
	dma_setmode(sb_dev.DMA8Channel, 0x44);
    }
    dma_setbuff(sb_dev.DMA8Channel, buff, len);
    dma_start(sb_dev.DMA8Channel);
    /*...and start it!!! */
    sbdsp_write(sb_dev.BaseAddress, cmd);
    sbdsp_write(sb_dev.BaseAddress, IOMODE_UNSIGNED | IOMODE_MONO);
    /* LSB...*/
    sbdsp_write(sb_dev.BaseAddress, (BYTE)(len & 0xFF));
    /*...and then MSB */
    sbdsp_write(sb_dev.BaseAddress, (BYTE)((len >> 8) & 0xFF));
}

/* Start a 16 bit sampling/playing operation */
void sb_dma16op(BYTE i_o)
{
    DWORD len;
    BYTE cmd;
    BYTE *buff;

    buff = buff_des.sound_dma->p;
    len = buff_des.sound_dma->len;
    cmd = DSPCMD_16BITIO;

    /* Stop any previous operation and reset the DMAC */
    dma16_stop(sb_dev.DMA16Channel);
    dma16_reset();
    if (i_o == OUT) {
	cmd |= SBIO_OUT;
	dma16_setmode(sb_dev.DMA16Channel, 0x48);
    } else {
	cmd |= SBIO_IN;
	dma16_setmode(sb_dev.DMA16Channel, 0x44);
    }
    dma16_setbuff(sb_dev.DMA16Channel, buff, len);
    dma16_start(sb_dev.DMA16Channel);
    /*...and start it!!! */
    sbdsp_write(sb_dev.BaseAddress, cmd);
    sbdsp_write(sb_dev.BaseAddress, IOMODE_SIGNED | IOMODE_MONO);
    /* LSB...*/
    sbdsp_write(sb_dev.BaseAddress, (BYTE)(len & 0xFF));
    /*...and then MSB */
    sbdsp_write(sb_dev.BaseAddress, (BYTE)((len >> 8) & 0xFF));
}

/* Start an 8 bit sampling/playing operation using double buffer */
void sb_dmabuffop(BYTE i_o)
{
    DWORD len;
    BYTE cmd;
    struct dma_buff *buff;

    buff = buff_des.sound_dma;
    cmd = DSPCMD_8BITIO | SBIO_AUTOINIT;
    len = buff->len;
    if (i_o == OUT) {
	cmd |= SBIO_OUT;
	dma_out(sb_dev.DMA8Channel, buff);
    } else {
	cmd |= SBIO_IN;
	dma_in(sb_dev.DMA8Channel, buff);
    }
    sbdsp_write(sb_dev.BaseAddress, cmd);
    sbdsp_write(sb_dev.BaseAddress, IOMODE_UNSIGNED | IOMODE_MONO);
    sbdsp_write(sb_dev.BaseAddress, (BYTE)(((buff->dma_bufflen >> 1) - 1) & 0xFF));
    sbdsp_write(sb_dev.BaseAddress, (BYTE)((((buff->dma_bufflen >> 1) - 1) >> 8) & 0xFF));
}

/* Start a 16 bit sampling/playing operation using double buffer */
void sb_dma16buffop(BYTE i_o)
{
    DWORD len;
    BYTE cmd;
    struct dma_buff *buff;

    buff = buff_des.sound_dma;
    cmd = DSPCMD_16BITIO | SBIO_AUTOINIT;
    len = buff->len;
    if (i_o == OUT) {
	cmd |= SBIO_OUT;
	dma16_out(sb_dev.DMA16Channel, buff);
    } else {
	cmd |= SBIO_IN;
	dma16_in(sb_dev.DMA16Channel, buff);
    }
    sbdsp_write(sb_dev.BaseAddress, cmd);
    sbdsp_write(sb_dev.BaseAddress, IOMODE_SIGNED | IOMODE_MONO);
    sbdsp_write(sb_dev.BaseAddress, (BYTE)(((buff->dma_bufflen >> 2) - 1) & 0xFF));
    sbdsp_write(sb_dev.BaseAddress, (BYTE)((((buff->dma_bufflen >> 2) - 1) >> 8) & 0xFF));
}

void sb_stopdsp(BYTE bit)
{
    if (bit == 8) 
    	sbdsp_write(sb_dev.BaseAddress, DSPCMD_EXIT8);
    else if (bit == 16)
	sbdsp_write(sb_dev.BaseAddress, DSPCMD_EXIT16);
    else cprintf("How many bits?\n");
}

/* Sound Blaster 16 interrupt's Fast Handler */
void sb_handler(int n)
{
    switch(sbmixer_read(sb_dev.BaseAddress, MIXER_INTSTATUS) & 3) {
    	case 1: ll_in(sb_dev.BaseAddress + ACK8);
		break;
    	case 2: ll_in(sb_dev.BaseAddress + ACK16);
		break;
	default: cprintf("Unknown int: %x\n", sbmixer_read(sb_dev.BaseAddress, 0x82));
    }
}

/*
   This process is activated when the sound card generates an interrupt:
   it calls the function to copy data from/to the sound buffer (this function
   can be the double buffering-function or the user's self-buffering function
*/
TASK sb_proc(void)
{
    for(;;) {
	/* call the copy function */
	switch(buff_des.fun(buff_des.sound_dma)) {
	    /* the operation will finish with the next interrupt */
	    case 1: buff_des.fun = dummyfun1;
	    	    break;
	    /* operation finished */
	    case 2: dummyfun2(buff_des.sound_dma);
	    	    buff_des.fun = dummyfun2;
		    break;
	    default: break;
	}
	task_endcycle();
    }
    return 0;
}

/* PIO mode sampling process */
TASK proc_sample()
{
    DWORD i;

    sbdsp_reset(sb_dev.BaseAddress);
    for(i = 0; i < buff_des.sound_dma->len; i++) {
	sbdsp_write(sb_dev.BaseAddress, DSPCMD_DIRECTIN); 
	buff_des.sound_dma->p[i] = sbdsp_read(sb_dev.BaseAddress);
	task_endcycle();
    }
    /* Operation finished: if it was blocking signal */
    if (buff_des.synch) sem_post(&buff_des.synchr);
    //task_abort();
    return 0;
}

/* PIO mode playing process */
TASK proc_play()
{
    DWORD i;
    
    sbdsp_reset(sb_dev.BaseAddress);
    for(i = 0; i < buff_des.sound_dma->len; i++) {
	sbdsp_write(sb_dev.BaseAddress, DSPCMD_DIRECTOUT);
	sbdsp_write(sb_dev.BaseAddress, buff_des.sound_dma->p[i]);
	task_endcycle();
    }
    /* Operation finished: if it was blocking signal */
    if (buff_des.synch) sem_post(&buff_des.synchr);
    //task_abort();
    return 0;
}
