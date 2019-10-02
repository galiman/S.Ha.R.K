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
 CVS :        $Id: dma.h,v 1.1 2004/05/11 15:14:48 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2004/05/11 15:14:48 $
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

#ifndef __DRIVERS_DMA_H__
#define __DRIVERS_DMA_H__

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

typedef struct dma_buff{
    BYTE *p;
    DWORD len;
    DWORD count;
    BYTE page;
    BYTE *dma_buff;
    WORD dma_bufflen;
} DMA_BUFF;

BYTE *dma_getpage(DWORD dim);

void dma_start(BYTE channel);
void dma_setmode(BYTE channel, BYTE mode);
void dma_setbuff(BYTE channel, BYTE *addr, WORD len);
void dma_reset(void);
void dma16_start(BYTE channel);
void dma16_setmode(BYTE channel, BYTE mode);
void dma16_setbuff(BYTE channel, BYTE *addr, WORD len);
void dma16_reset(void);

void dma_getalignbuff(struct dma_buff *buff, WORD len);
void dma_out(BYTE channel, struct dma_buff *b);
void dma_in(BYTE channel, struct dma_buff *b);
void dma_stop(BYTE channel);
void dma16_out(BYTE channel, struct dma_buff *b);
void dma16_in(BYTE channel, struct dma_buff *b);
void dma16_stop(BYTE channel);

int infun(struct dma_buff *b);
int outfun(struct dma_buff *b);

__END_DECLS
#endif
