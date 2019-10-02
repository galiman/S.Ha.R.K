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
 CVS :        $Id: sound.h,v 1.1 2004/05/11 15:14:48 giacomo Exp $

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

/* Project:     HARTIK 3.0 Sound Library                        */
/* Description: Hard Real TIme Kernel for 8086 compatible       */
/* Author:      Luca Abeni                                      */
/* Date:        5/12/1997                                       */

/* File:        Sound.H                                         */
/* Revision:    3.0                                             */


#include <drivers/dma.h>

#ifndef __SOUND_H__
#define __SOUND_H__

#ifdef __cplusplus
extern "C" {
#endif
#define ENABLE 1
#define DISABLE 0

#define PIO_OP		0x00
#define DMA_OP		0x01

#define PCM8		0x00
#define PCM16		0x02

#define SYNCH		0x00
#define ASYNCH		0x04

#define MYFUN		0x08
#define NOBUFF		0x10

#define STATBUFF	0x00
#define DYNBUFF		0x20

#define IN		0
#define OUT		1


#define BUFFSIZE 0xFFFF /* esattamente un segmento */
#define TICK /*91*/ 100

void sound_setfun(int (*infun)(void *rawbuff), int (*outfun)(void *rawbuff));
int sound_sample(BYTE *buff, DWORD sps, DWORD len, BYTE t, TASK_MODEL *m);
int sound_play(BYTE *buff, DWORD sps, DWORD len, BYTE t, TASK_MODEL *m);
int sound_wait(void);
void sound_stop(void);
int sound_init(WORD rawbuffsize, TASK_MODEL *m);
void sound_info(void);
int dummyfun1(struct dma_buff *d);
int dummyfun2(struct dma_buff *d);

#ifdef __cplusplus

};

#endif



#endif

