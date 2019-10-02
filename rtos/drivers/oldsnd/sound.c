/* Project:     HARTIK 3.0 Sound Library                        */
/* Description: Hard Real TIme Kernel for 8086 compatible       */
/* Author:      Luca Abeni					*/
/* Date:        5/12/1997                                       */

/* File: 	Sound.C						*/
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

/* added by Paolo */
#define SOUND_WCET 200

/* Sound generic interface */
#include <kernel/kern.h>

#include "blaster.h"
#include <drivers/sound.h>
#include <drivers/dma.h>

//extern struct proc_des proc_table[MAX_PROC];

extern struct sb_device sb_dev;
extern TASK proc_play();
extern TASK proc_sample();
extern TASK sb_proc();
extern void sb_handler(int n);
extern struct sound_buffer buff_des;
struct rawfuncs f_des;
struct dma_buff dma_des;

/*int sound_exc(int err)
{
    switch (err) {
	case SOUND_CARD_NOT_FOUND:
	    cprintf("SOUND PANIC --> Sound card not found.\n");
	    return 0;
	case SOUND_RAWFUN_NOT_SET:
	    cprintf("Self-buffering OP error --> User function not set.\n");
	    return 0;
	case SOUND_TICK_TOO_LARGE:
	    cprintf("PIO mode OP error --> sys_tick too large.\n");
	    return 0;
	case SOUND_ASYNCH_WAIT:
	    cprintf("Asynchronous OP error --> Wait on asynch operation.\n");
	    return 1;
	default :
	    return 1;
    }
} */

/* Set the user's functions for self buffering */
void sound_setfun(int (*infun)(void *rawbuff), int (*outfun)(void *rawbuff))
{
    /* Should be NULL here! Luca: used -1 instead NULL */
    if (infun != NULL) {
	f_des.infun = infun;
	f_des.infunpresent = 1;
    }
    if(outfun != NULL) {
	f_des.outfun = outfun;
	f_des.outfunpresent = 1;
    }
}

/*
   This function is called by the sound card driver function when the card
   generate an interrupt during a self buffering input operation. The user's
   self buffering function is called by this function
*/
int selfinfun(struct dma_buff *b)
{
    int res;
    
    res = f_des.infun(&b->dma_buff[(b->dma_bufflen >> 1) * b->page]);
    b->page = !b->page;
    return res;
}

/*
   This function is called by the sound card driver function when the card
   generate an interrupt during a self buffering output operation. The user's
   self buffering function is called by this function
*/
int selfoutfun(struct dma_buff *b)
{
    int res;
    
    res = f_des.outfun(&b->dma_buff[(b->dma_bufflen >> 1) * b->page]);
    b->page = !b->page;
    return res;
}

/*
   Play a sample: this function calls the adeguate low-level playing function
   with the correct parameters

   the task model in the last parameter is used for the sb_player task.
   it must be a periodic task with a correct period. if not specified (NULL),
   a soft task is used instead.

   it returns 0 if all ok,
     EINVAL if the outfun isn't specified
     ESRCH if the sb_player can not be created
*/
int sound_play(BYTE *buff, DWORD sps, DWORD len, BYTE t, TASK_MODEL *m)
{
    double rate;
    PID p_pl;
//    WORD s1, p;

    /* Self Buffering?*/
    if (t & MYFUN) {
	/*Yes */
	if (f_des.outfunpresent != 0) {
	    buff_des.fun = selfoutfun;
	}
	else
	    return EINVAL;
    }
    else {
	/* No: set the standard double-buffering output function (module dma.c) */
	buff_des.fun = outfun;
    }
    /* set the operation parameters...*/
    buff_des.synch = !(t & SYNCH);
    dma_des.len = len;
    dma_des.p = buff;
    if (t & DMA_OP) {
	/* DMA op */
	sb_setrate(sps, OUT);
        /*
	if (t & DYNBUFF) {
	    s1 = (sps / 1000) * ((t & PCM16) ? 2 : 1);
	    p = ((dma_des.dma_bufflen >> 1) * 1000 ) / (sound_tick * s1);
	    if(p < 1) {
		sb_dev.pwarning = 1;
		p = 1;
	    } else sb_dev.pwarning = 0;
	    / * Adjust the driver task's minimum interarrival time */ /*
	    kern_cli();
	    proc_table[p_sb].period = p;
	    proc_table[p_sb].drel = p;
	    kern_sti();
	    sb_dev.period = p;
	} */
	if (!(t & NOBUFF)) {
	    if (t & PCM16) sb_dma16buffop(OUT);
	    else sb_dmabuffop(OUT);
	} else {
	    /* Double buffering operation */
	    buff_des.fun = dummyfun2;
	    if (t & PCM16) sb_dma16op(OUT);
	    else sb_dmaop(OUT);
	}
    } else {
	/* Non DMA op: create the playing process */
        SOFT_TASK_MODEL m_soft;
        if (!m) {
          rate = (999999 / sps) + 1;
          soft_task_default_model(m_soft);
          soft_task_def_system(m_soft);
          soft_task_def_nokill(m_soft);
          soft_task_def_period(m_soft,rate);
          soft_task_def_met(m_soft,rate);
          soft_task_def_wcet(m_soft,SOUND_WCET);
          m = (TASK_MODEL *)&m_soft;
        }
	p_pl = task_create("sb_Player",proc_play, &m, NULL);
	if (p_pl == NIL) {
	    cprintf("Sound.c: Cannot create sb_Player\n");
	    exit(1);
	    return ESRCH;
	}
	task_activate(p_pl);
    }

    return 0;
}

/*
   Sample: this function calls the adeguate low-level sampling function
   with the correct parameters. It is similar to sound_play (see it for the
   comments

   the task model in the last parameter is used for the sb_player task.
   it must be a periodic task with a correct period. if not specified (NULL),
   a soft task is used instead.

   it returns 0 if all ok,
     EINVAL if the outfun isn't specified
     ESRCH if the sb_player can not be created
*/
int sound_sample(BYTE *buff, DWORD sps, DWORD len, BYTE t, TASK_MODEL *m)
{
    double rate;
    PID p_sample;
//    WORD s1, p;
//    MODEL m = BASE_MODEL;

    if (t & MYFUN) {
	if (f_des.infunpresent != 0) {
	    buff_des.fun = selfinfun;
	}
	else
	    return EINVAL;
    }
    else
	buff_des.fun = infun;

    buff_des.synch = !(t & SYNCH);
    dma_des.len = len;
    dma_des.p = buff;
    if (t & DMA_OP) {
	sb_setrate(sps, IN);
        /*
	if (t & DYNBUFF) {
            p_sb = task_pid("sb_EndDMA");
	    s1 = (sps / 100) * ((t & PCM16) ? 2 : 1);
	    p = ((dma_des.dma_bufflen >> 1) * 1000 ) / (sound_tick * s1);
	    if(p < 1) {
		sb_dev.pwarning = 1;
		p = 1;
	    } else sb_dev.pwarning = 0;
	    kern_cli();
	    proc_table[p_sb].period = p;
	    proc_table[p_sb].drel = p;
	    kern_sti();
	    sb_dev.period = p;
	} */
	if (!(t & NOBUFF)) {
	    if (t & PCM16) sb_dma16buffop(IN);
	    else sb_dmabuffop(IN);
	} else {
	    buff_des.fun = dummyfun2;
	    if (t & PCM16) sb_dma16op(IN);
	    else sb_dmaop(IN);
	}
    } else {
	/* Non DMA op: create the playing process */
        SOFT_TASK_MODEL m_soft;
        if (!m) {
          rate = (999999 / sps) + 1;
          soft_task_default_model(m_soft);
          soft_task_def_system(m_soft);
          soft_task_def_nokill(m_soft);
          soft_task_def_period(m_soft,rate);
          soft_task_def_met(m_soft,rate);
          soft_task_def_wcet(m_soft,SOUND_WCET);
          m = (TASK_MODEL *)&m_soft;
        }
	p_sample = task_create("sb_Sampler",proc_sample, &m, NULL);
	if (p_sample == NIL) {
	    cprintf("Sound.c: Cannot create sb_Sampler\n");
	    exit(1);
	    return ESRCH;
	}
	task_activate(p_sample);
    }
    return 0;
}

/*
   Wait for the end of a synchronous sound op: it is implemented with a
   simple semaphore
   It returns  0 if all ok,
              -1 if no sinchronous operation was called before
*/
int sound_wait(void)
{
    if (buff_des.synch) {
	sem_wait(&buff_des.synchr);
	buff_des.synch = 0;
        return 0;
    }
    else
      return -1;
}


/* returns 0 if all ok,
   ENOSPC if a problem occurs when creating the semaphores structures
   ESRCH if the Enddma task cn not be created
*/
int sound_init(WORD rawbuffsize, TASK_MODEL *m)
{
    PID p_sb;
    int period;
    SOFT_TASK_MODEL m_soft;

    /* Semaphore for synchronous ops */
    if (sem_init(&buff_des.synchr,0,0))
      return ENOSPC;

    /* Init the card */
    sb_init();
    sbmixer_reset();
    sbmixer_setinput(0x01, ENABLE);
    sbmixer_setoutput(0x01, DISABLE);
    sbmixer_setmiclev(0x1F);
    sbmixer_setAGC(ENABLE);
    sbmixer_setingainlev(0);

    f_des.infun = 0;
    f_des.infunpresent = 0;
    f_des.outfun = 0;
    f_des.outfunpresent = 0;
    /* init the buffers for DMA ops */
    dma_getalignbuff(&dma_des, rawbuffsize);
    buff_des.sound_dma = &dma_des;

    if (!m) {
      period = (rawbuffsize * 1000000) / 48000;
      kern_printf("period=%d\n",period);
      soft_task_default_model(m_soft);
      soft_task_def_system(m_soft);
      soft_task_def_nokill(m_soft);
      soft_task_def_period(m_soft,period);
      soft_task_def_met(m_soft,SOUND_WCET);
      soft_task_def_wcet(m_soft,SOUND_WCET);
      soft_task_def_aperiodic(m_soft);
      m = (TASK_MODEL *)&m_soft;
    }

    /* create the driver process and set it and the Fast Handler */
    p_sb = task_create("sb_EndDMA", sb_proc, m, NULL);
    if (p_sb == NIL) {
	cprintf("Sound.c: Cannot create sb_EndDMA\n");
        cprintf("errno=%d\n",errno);
	exit(1);
	return ESRCH;
    }
    //sb_dev.period = period;
    handler_set(sb_dev.IntLine, sb_handler, FALSE, p_sb, NULL);

    return 0;
}

/* Obvious... */
void sound_info(void)
{
    cprintf("Hartik Sound lib [V 3.2]:\n");
    cprintf("Sound Blaster 16 or clone found:\n");
    sb_show();
}

/*
   This function is called by the driver process on the last transfert of an
   operation
*/
int dummyfun1(struct dma_buff *d)
{
    buff_des.fun = dummyfun2;
    return 0;
}

/*
   This function is called by the driver process when the next transfert
   will be the last of the current operation
*/
int dummyfun2(struct dma_buff *d)
{
    if (buff_des.synch) sem_post(&buff_des.synchr);
    sb_stopdsp(8);
    sb_stopdsp(16);
    dma_stop(sb_dev.DMA8Channel);
    dma16_stop(sb_dev.DMA16Channel);
    return 0;
}

/* Obvious... */
void sound_stop(void)
{
    if (!buff_des.synch) {
	sb_stopdsp(8);
	sb_stopdsp(16);
	dma_stop(sb_dev.DMA8Channel);
	dma16_stop(sb_dev.DMA16Channel);
    }
}
