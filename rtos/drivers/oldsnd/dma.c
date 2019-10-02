/* Project:     HARTIK 3.0 Sound Library                        */
/* Description: Hard Real TIme Kernel for 8086 compatible       */
/* Author:      Luca Abeni					*/
/* Date:        5/12/1997                                       */

/* File: 	DMA.C						*/
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

/*
   DMAC functions and structures. This module was developed for using some
   sound card's DMA operations, will become part of the HARTIK Kernel, for
   providing support to all applications that needs DMA
*/

#include <kernel/kern.h>
#include <drivers/dma.h>
#include "sbio.h"

#define appl2linear(x) (x)

/* This does not work at 16 bits!! I'm sorry */
/* Solution: Place them into a separate segment, perhaps it works... */
BYTE buff2[0xFFFF];
BYTE buff3[0xFFFF];

void dma_stop(BYTE channel)
{
    ll_out(0x0A, 0x04 | channel);
}

void dma16_stop(BYTE channel)
{
    ll_out(0xD4, 0x04 | (channel - 4));
}

void dma_reset(void)
{
    ll_out(0x0C,0x00);
}

void dma16_reset(void)
{
    ll_out(0xD8,0x00);
}

void dma_start(BYTE channel)
{
    ll_out(0x0A, channel);
}

void dma16_start(BYTE channel)
{
    ll_out(0xD4, channel- 4);
}

void dma_setmode(BYTE channel, BYTE mode)
{
    ll_out(0x0B,mode | channel);
}

void dma16_setmode(BYTE channel, BYTE mode)
{
    ll_out(0xD6,mode | (channel - 4));
}

/*
   Program the DMAC to transfert bytes to/from a buffer with logical
  address addr and lenght len using the specified DMA channel
*/
void dma_setbuff(BYTE channel, BYTE *addr, WORD len)
{
    DWORD ph_addr;
    WORD offset_port, page_port, len_port;

    switch (channel) {
	case 0: offset_port = 0;
		page_port = 0x87;
		len_port = 1;
		break;
	case 1: offset_port = 0x02;
		page_port = 0x83;
		len_port = 0x03;
		break;
	case 3: offset_port = 0x06;
		page_port = 0x82;
		len_port = 0x07;
		break;
	default: cprintf("dma_setbuff channel error!!!\n");
		 exit(1);
		 return;
    }
    ph_addr = appl2linear(addr);
    ll_out(offset_port, (ph_addr & 0xFF));
    ll_out(offset_port, (ph_addr >> 8) & 0xFF);
    ll_out(page_port, (ph_addr >> 16) & 0xFF);
    ll_out(len_port,(BYTE)(len&0xFF));
    ll_out(len_port,(BYTE)((len>>8)&0xFF));
}

/*
   Program the DMAC to transfert words to/from a buffer with logical
   address addr and lenght len using the specified DMA channel
*/
void dma16_setbuff(BYTE channel, BYTE *addr, WORD len)
{
    DWORD ph_addr;
    WORD offset_port, page_port, len_port;

    switch (channel) {
	case 5: offset_port = 0xC4;
		page_port = 0x8B;
		len_port = 0xC6;
		break;
	case 6: offset_port = 0xC8;
		page_port = 0x89;
		len_port = 0xCA;
		break;
	case 7: offset_port = 0xCC;
		page_port = 0x8A;
		len_port = 0xCE;
		break;
		/* It does not seem too much clean */
	default: cprintf("16 bit DMA?????\n");
		 exit(1);
		 return;
    }
    ph_addr = appl2linear(addr);
    ll_out(offset_port, (ph_addr >> 1) & 0xFF);
    ll_out(offset_port, (ph_addr >> 9) & 0xFF);
    ll_out(page_port, (ph_addr >> 16) & 0xFE);
    ll_out(len_port,(BYTE)((len >> 1) & 0xFF));
    ll_out(len_port,(BYTE)((len >> 9) & 0xFF));
}

/*
   Program the 8 bit DMAC to transer bytes from the buffer specified by
   dma_buff using double buffering
*/
void dma_out(BYTE channel, struct dma_buff *buff)
{
    DWORD len, i;

    buff->page = 0;
    len = buff->dma_bufflen -1;
    for(i = 0; i < buff->dma_bufflen; i++) {
	buff->dma_buff[i] = buff->p[i];
    }
    buff->count = buff->dma_bufflen;

    dma_stop(channel);
    dma_reset();
    dma_setmode(channel, 0x58);
    dma_setbuff(channel, buff->dma_buff, len);
    dma_start(channel);
}

/*
   Program the 8 bit DMAC to transer bytes to the buffer specified by
   dma_buff using double buffering
*/
void dma_in(BYTE channel, struct dma_buff *buff)
{
    DWORD len;

    buff->page = 0;
    len = buff->dma_bufflen - 1;
    buff->count = 0;

    dma_stop(channel);
    dma_reset();
    dma_setmode(channel, 0x54);
    dma_setbuff(channel, buff->dma_buff, len);
    dma_start(channel);
}

/*
   Program the 8 bit DMAC to transer bytes from the buffer specified by
   dma_buff using double buffering
*/
void dma16_out(BYTE channel, struct dma_buff *buff)
{
    DWORD len, i;

    buff->page = 0;
    len = buff->dma_bufflen - 1;
    for(i = 0; i < buff->dma_bufflen; i++) {
	buff->dma_buff[i] = buff->p[i];
    }
    buff->count = buff->dma_bufflen;

    dma16_stop(channel);
    dma16_reset();
    dma16_setmode(channel, 0x58);
    dma16_setbuff(channel, buff->dma_buff, len);
    dma16_start(channel);
}

/*
   Program the 8 bit DMAC to transer bytes to the buffer specified by
   dma_buff using double buffering
*/
void dma16_in(BYTE channel, struct dma_buff *buff)
{
    DWORD len;

    buff->page = 0;
    len = buff->dma_bufflen -1;
    buff->count = 0;

    dma16_stop(channel);
    dma16_reset();
    dma16_setmode(channel, 0x54);
    dma16_setbuff(channel, buff->dma_buff, len);
    dma16_start(channel);
}

/*
   The DMAC can use only buffers that don't cross a 64K boundary (the
   value (0xFFFF0000 & address) must be the same for every address in the
   buffer). We call this kind of buffers "aligned buffers": it can be a
   problem to allocate an aligned buffer, so we provide the dma_getalignbuff
   function
*/

/* Allocate an aligned buffer for DMA transfer */
void dma_getalignbuff(struct dma_buff *buff, WORD len)
{
//    BYTE *p;
//    DWORD phys;
//    BYTE done = 0;

    if (len > 0x8000) {
	cprintf("Don' t allocate too big buffers!!!!!\n");
/*	exc_raise(TOO_BIG_BUFFER);*/
    }
    buff->dma_bufflen = len;

//    while (!done)
//    {
	/* get a buffer */
//	p = VM_alloc(len);
	/* compute its phisical address */
//	phys = appl2linear(p);
	/* Is it aligned? */
//	if ((phys & 0x0F0000) != ((phys + len) & 0x0F0000))
	/* If no, try again */
//	    done = 0;
//	else done = 1;
//    }
//    buff->dma_buff = p;

    /* NB this function returns a page aligned on a 64k boundary
       ... this is not what it have to be, but it works */
    buff->dma_buff = kern_alloc_aligned(len, MEMORY_UNDER_16M, 16, 0);
}

/*
   Allocate a buffer starting from an address with the rightmost 16 bits equal
   to 0 (it's the simpler way to obtain an aligned buffer
*/
BYTE *dma_getpage(DWORD dim)
{
    /* Get a buffer of dimension dim+64K...*/
    return kern_alloc_aligned(dim, MEMORY_UNDER_16M, 16, 0);
}

/*
   Copy a part of the user buffer in half DMA buffer (used for
   double buffering)
*/
int outfun(struct dma_buff *b)
{
    int i;
    int result = 0;

    /* Is this the last cycle of the DMA output operation?*/
    if (b->len > (b->dma_bufflen >> 1) + b->count) {
	/*No */
	for(i = 0; i < (b->dma_bufflen >> 1); i++)
	    b->dma_buff[i+ ((b->dma_bufflen>>1) * b->page)] = b->p[b->count + i];
    } else {
	/* Yes */
	for(i = 0; i < (b->len - b->count); i++)
	    b->dma_buff[i + ((b->dma_bufflen>>1) * b->page)] = b->p[b->count + i];
	    /* return 1 to comunicate that the operation is finished */
	result = 1;
    }
    b->count += (b->dma_bufflen >> 1);
    b->page = !b->page;
    return result;
}

/* Copy half DMA buffer in the user buffer (used for double buffering) */
int infun(struct dma_buff *b)
{
    int i;
    int result = 0;

    /* Is this the last cycle of the DMA outpu operation? */
    if (b->len > (b->dma_bufflen >> 1) + b->count) {
	for(i = 0; i < (b->dma_bufflen >> 1); i++)
	    b->p[b->count+ i] = b->dma_buff[i + ((b->dma_bufflen>>1) * b->page)];
    } else {
	for(i = 0; i < (b->len - b->count); i++)
	    b->p[b->count+ i] = b->dma_buff[i+ ((b->dma_bufflen>>1) * b->page)];
	    /* return 2 to comunicate that the operation is finished */
	result = 2;
    }
    b->count += (b->dma_bufflen >> 1);
    b->page = !b->page;
    return result;
}
