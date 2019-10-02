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


/***************************************

  CVS :        $Id: idelow.c,v 1.1.1.1 2002/03/29 14:12:49 pj Exp $
 
  Revision:    $Revision: 1.1.1.1 $

  Last update: $Date: 2002/03/29 14:12:49 $

  This module is responsable of the protocol between the IDE device driver
  interface and the host (the computer).

***************************************/

/*
 * Copyright (C) 1999,2000 Massimiliano Giorgi
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

#include "glue.h"
#include <fs/bdevinit.h>
#include <fs/magic.h>
#include <fs/assert.h>
#include <fs/util.h>
#include <fs/maccess.h>

#include "bdev.h"
#include "phdsk.h"
#include "lodsk.h"
#include "ide.h"
#include "idereq.h"
#include "bqueue.h"

/*+ some kernel depend features implemented +*/
#include "ideglue.h"

/*
 * TRACER FLAGS
 */

/* if defined use tracer user events 1 and 2 before and after blocking
 * for synchronization (so we can compute the waiting time for a
 * request)
 */
#define TRACEWAIT
#undef TRACEWAIT
     
#ifdef TRACEWAIT
#include <trace/types.h>
#include <kernel/trace.h>
#endif

/*
 * FLAGS
 */

/* if defined:
 * enable "set features" commands (so we can set features on ide drives).
 */
#define IDE_ENABLESETFEATURES 1
//#undef IDE_ENABLESETFEATURES

/* if defined:
 * add a delay into the IRQ handler
 * (my old cdrom seems to clear the IRQ at the end of the data in/out
 * not when reading the STATUS register)
 * PS: if the DEBUG_SERVER is defined this delay isn't required.
 */
#define IDE_OLDATAPIDELAY 1
#undef IDE_OLDATAPIDELAY

/* if defined:
 * after the soft reset timeout expired do a soft reset
 * also if the device is busy
 * (some old cdrom seems not to clear the busy bits ?!?)
 */
#define IDE_FORCERESET 1
//#undef IDE_FORCERESET

/*
 * if defined:
 * after issuing a command does not test for command in progress
 * (does not read the status register!)
 * DANGER: need for all TX motherboard
 */
#define IDE_SKIPSTATUSTEST 1
//#undef IDE_SKIPSTATUSTEST

/*
 * if defined:
 * skip all test when issuing a command!
 * (it imply IDE_SKIPSTATUSTEST)
 * DANGER: need on some TX motherboard
 * on few TX motherboards PIO mode is not safe (DMA must be used)
 */
#define IDE_SKIPALLTEST 1
#undef IDE_SKIPALLTEST

#ifdef IDE_SKIPALLTEST
#define IDE_SKIPSTATUSTEST 1
#endif
 
/*
 * DEBUG
 */

/* show server activity */
#define DEBUG_SERVER KERN_DEBUG
#undef DEBUG_SERVER

/* show request and result */
#define DEBUG_SHOWREQUEST KERN_DEBUG
#undef DEBUG_SHOWREQUEST

/* trace soft reset function */
#define DEBUG_SOFTRESET KERN_DEBUG
#undef DEBUG_SOFTRESET

/* trace REQ_EPILOG (the epilog of every device request) */
#define DEBUG_REQEPILOG KERN_DEBUG
#undef DEBUG_REQEPILOG

/* trace do_io_request() (WARNIG can cause a deadline miss)*/
#define DEBUG_IOREQUEST KERN_DEBUG
#undef DEBUG_IOREQUEST

/**/

#ifdef DEBUG_SERVER
#define printk0(ideif,fmt,args...) \
        printk(DEBUG_SERVER "ide%i server:" fmt,ideif,##args)
#else
#define printk0(fmt,args...)
#endif

#ifdef DEBUG_SHOWREQUEST
#define printk1(ideif,fmt,args...) \
        printk(DEBUG_SHOWREQUEST "ide%i request:" fmt,ideif,##args)
#else
#define printk1(fmt,args...)
#endif

#ifdef DEBUG_SOFTRESET
#define printk2(ideif,fmt,args...) \
        printk(DEBUG_SOFTRESET "ide%i soft reset:" fmt,ideif,##args)
#else
#define printk2(fmt,args...)
#endif

#ifdef DEBUG_REQEPILOG
#define printk3(ideif,fmt,args...) \
        printk(DEBUG_REQEPILOG "ide%i req_epilog:" fmt,ideif,##args)
#else
#define printk3(fmt,args...)
#endif

#ifdef DEBUG_IOREQUEST
#define printk4(ideif,fmt,args...) \
        printk(DEBUG_IOREQUEST fmt)
#else
#define printk4(fmt,args...)
#endif

/*
 *
 * REGISTERS
 *
 */

/* 
 * from ATA/ATAPI 4 (T13/1153D revision 18)
 * paragraph 7.x
 */

/* io_port registers (usually on 0x1f0 for first IDE interface) */
#define REG_DATA     0x00
#define REG_ERROR    0x01
#define REG_FEATURES 0x01
#define REG_SECCOU   0x02
#define REG_SECNUM   0x03
#define REG_CYLLOW   0x04
#define REG_CYLHIG   0x05
#define REG_DEVHEAD  0x06
#define REG_STATUS   0x07
#define REG_COMMAND  0x07

/* to register the i/o space */
#define IOPORT_OFFSET 0
#define IOPORT_LEN    8

/* io_port2 registers (usually on 0x3f0 for first IDE interface) */
#define REG_ALTSTATUS 0x06
#define REG_DEVCTRL   0x06

/* to register the i/o space */
#define IOPORT2_OFFSET 6
#define IOPORT2_LEN    1

/* to use with VM_in/VM_out */
/* for read/write */
#define IDE_REG_DATA      (ide[ideif].io_port+REG_DATA)
#define IDE_REG_SECCOU    (ide[ideif].io_port+REG_SECCOU)
#define IDE_REG_SECNUM    (ide[ideif].io_port+REG_SECNUM)
#define IDE_REG_CYLLOW    (ide[ideif].io_port+REG_CYLLOW)
#define IDE_REG_CYLHIG    (ide[ideif].io_port+REG_CYLHIG)
#define IDE_REG_DEVHEAD   (ide[ideif].io_port+REG_DEVHEAD)
/* for read */
#define IDE_REG_STATUS    (ide[ideif].io_port+REG_STATUS)
#define IDE_REG_ERROR     (ide[ideif].io_port+REG_ERROR)
#define IDE_REG_ALTSTATUS (ide[ideif].io_port2+REG_ALTSTATUS)
/* for write */
#define IDE_REG_COMMAND   (ide[ideif].io_port+REG_COMMAND)
#define IDE_REG_FEATURES  (ide[ideif].io_port+REG_ERROR)
#define IDE_REG_DEVCTRL   (ide[ideif].io_port2+REG_DEVCTRL)
     
/* for status & alt_status register */
#define STATUS_BUSY        0x80
#define STATUS_DRDY        0x40
#define STATUS_R_FAULT     0x20
#define STATUS_R_SEEKCOM   0x10
#define STATUS_DRQ         0x08
#define STATUS_R_DWC       0x04
#define STATUS_R_CMDPROG   0x02
#define STATUS_ERR         0x01

/* for error register */
#define ERROR_OBS_BADBLOCK    0x80
#define ERROR_OBS_CRCECC      0x40
#define ERROR_OBS_IDNOTFOUND  0x10
#define ERROR_ABORTCMD        0x04
#define ERROR_OBS_TRACK00     0x02
#define ERROR_OBS_DAMNOTFOUND 0x01

/* for devhead register */
#define DEVHEAD_DEV  0x10

/* for devctrl (device control) register */
#define DEVCTRL_SRST  0x04
#define DEVCTRL_NIEN  0x02

#define DEVCTRL_SOFTRESET  (DEVCTRL_SRST)
#define DEVCTRL_ENABLEINT  (0)
#define DEVCTRL_DISABLEINT (DEVCTRL_NIEN)
     
#define DEVCTRL_NORMALOP   (DEVCTRL_ENABLEINT)

/*
 * these are not from ATA specification
 */

/* bus master registers */      
#define REG_BMICX     0
#define REG_BMISX     2
#define REG_BMIDTPX   4
     
/* to use with VM_in/VM_out */
#define BM_REG_COMMAND    (ide[ideif].io_bmdma+REG_BMICX)
#define BM_REG_STATUS     (ide[ideif].io_bmdma+REG_BMISX)
#define BM_REG_PRDADDR    (ide[ideif].io_bmdma+REG_BMIDTPX)

/* to register the i/o space */
#define BMDMAPORT_OFFSET 0
#define BMDMAPORT_LEN    8

/*
 *
 *
 *
 */

static int decode_error_result(int ideif)
{

  BYTE error;
  error=inp(IDE_REG_ERROR);

  /*
   * obsoleted by ATA 4
   *
  if (error&ERR_BADBLOCK) return IDE_ERR_BADBLOCK;
  else if (error&ERR_CRCECC) return IDE_ERR_CRCECC;
  else if (error&ERR_IDNOTFOUND) return IDE_ERR_IDNOTFOUND;
  else if (error&ERR_ABORTCMD) return IDE_ERR_ABORTCMD;
  else if (error&ERR_TRACK00) return IDE_ERR_TRACK00;
  else if (error&ERR_DAMNOTFOUND) return IDE_ERR_DAMNOTFOUND;
  */
  
  if (error&ERROR_ABORTCMD) return IDE_ERR_ABORTCMD;
  return IDE_ERR_UNKNOWN;
}

static void debug_status(char *s,BYTE status,BYTE mask)
{
  printk(IDEDEBUGLOG "%s: %s\n",s,status&mask?"yes":"no");
}

void ide_dump_interface_status(int ideif)
{
  BYTE status;

  printk(IDEDEBUGLOG "ide%i status\n",ideif);
  
  status=inp(IDE_REG_ALTSTATUS);

  debug_status("busy               ",status,STATUS_BUSY);
  if (status&STATUS_BUSY) return;

  debug_status("ready              ",status,STATUS_DRDY);
  //debug_status("write fault        ",status,ST_WRFAULT);
  //debug_status("seek complete      ",status,ST_SEEKCOM);
  debug_status("data request       ",status,STATUS_DRQ);
  //debug_status("data was corrected ",status,ST_DWC);
  //debug_status("command in progress",status,ST_CMDPROG);
  debug_status("error              ",status,STATUS_ERR);
    
}


//#ifndef NDEBUG

/*+ for the IDE_ERR_??? constants +*/
char *ide_error_msg[]={
  "no error",
  "there are too much request pending",
  "there was an interrupt but the interface is busy or can not do a command",
  "IDE error: bad block",
  "IDE error: crc/ecc error",
  "IDE error: sector id not found",
  "IDE error: abort command",
  "IDE error: track00 seek error",
  "IDE error: DAM not found",
  "generic error",
  "there was an interrupt, we aspect data but there is not data",
  "there was an interrupt, we do not aspect data but there is data",
  "interface not ready for a command",
  "a timeout waiting for a reply",
  "the interface must be busy but it is not",
  "device fault"
};

//#endif

/*
 *
 *
 *
 */

/* on i386 machine */
/* (todo better) */
/* done better! */
//#define ntohs(x) ((((x)&0xff00)>>8)|(((x)&0x00ff)<<8))

/* Well... the strings reported are swapped so if we have
 * a "Maxtor" hard disk the following string is reported: "aMtxro"
 */

/* from Linux kernel (modified!) */
static void ide_fixstring (BYTE *s, const int bytecount, const int byteswap)
{
  BYTE *p = s, *end = &s[bytecount & ~1]; /* bytecount must be even */

  if (byteswap) {
    /* convert from big-endian to host byte order */
    for (p = end ; p != s;) {
      WORD *pp = (WORD *) (p -= 2);
      *pp = ntohs(*pp);
    }
  }

  /* strip leading blanks */
  while (s != end && *s == ' ')
    ++s;

  /* compress internal blanks and strip trailing blanks */
  while (s != end && *s) {
    if (*s++ != ' ' || (s != end && *s && *s != ' '))
      *p++ = *(s-1);
  }

  /* wipe out trailing garbage */
  while (p != end)
    *p++ = '\0';
}

static void ide_fixdiskid(struct ata_diskid *ptr)
{
  ide_fixstring(ptr->firmware,sizeof(ptr->firmware),1);
  ptr->firmware[sizeof(ptr->firmware)-1]='\0';
  ide_fixstring(ptr->model,sizeof(ptr->model),1);
  ptr->model[sizeof(ptr->model)-1]='\0';
  ide_fixstring(ptr->serial,sizeof(ptr->serial),1);
  ptr->serial[sizeof(ptr->serial)-1]='\0';
}

static void ide_fixatapidiskid(struct atapi_diskid *ptr)
{
  ide_fixstring(ptr->firmware,sizeof(ptr->firmware),1);
  ptr->firmware[sizeof(ptr->firmware)-1]='\0';
  ide_fixstring(ptr->model,sizeof(ptr->model),1);
  ptr->model[sizeof(ptr->model)-1]='\0';
  ide_fixstring(ptr->serial,sizeof(ptr->serial),1);
  ptr->serial[sizeof(ptr->serial)-1]='\0';
}

static void lba2chs(int ideif, int id, DWORD lsector, int req)
{
  
  if (ide[ideif].info[id].use_lba) {
    idereq[req].secnum=lsector&0xff;
    lsector>>=8;
    idereq[req].cyllow=lsector&0xff;
    lsector>>=8;
    idereq[req].cylhig=lsector&0xff;
    lsector>>=8;
    idereq[req].devhead=0xe0|(lsector&0x0f)|((id&0x01)<<4); 
  } else {    
    /* for old hard-disk; is this correct? */
    int sect,head,cyl,track;
    struct dskgeometry *drive=&(ide[ideif].pdisk[id]->pd_phgeom);
    track = lsector / drive->sectors;
    sect  = lsector % drive->sectors + 1;
    idereq[req].secnum=sect;    
    head  = track % drive->heads;
    cyl   = track / drive->heads;
    idereq[req].cyllow=cyl&0xff;
    idereq[req].cylhig=(cyl>>8)&0xff;
    idereq[req].devhead=0xa0|(head&0x0f)|((id&0x01)<<4);
  } 
}

int ide_register(__uint16_t io_port, __uint16_t io_port2,
		 __uint8_t irq,
		 __uint8_t dma, __uint16_t io_bmdma)
{
  //struct phdskinfo    disk;
  //MODEL      m=BASE_MODEL;
  //int ind;
  int ideif;
  int res;
  
  //int req;
  //int ret;

  /* find an IDE interface sequential number */
  ideif=nextideif();
  if (ideif==MAXIDEINTERFACES) {
    if (ide_showinfo_flag)
      printk(IDELOG "ide: can't register (unavailable space)");
    return -1;
  }

  /* request to use resources... */
  if (!__request_irq(irq)) return -1;
  if (!__request_io_space(io_port+IOPORT_OFFSET,IOPORT_LEN)) {
    mark_ide_free(ideif);
    __release_irq(irq);
    return -1;
  }
  if (!__request_io_space(io_port2+IOPORT2_OFFSET,IOPORT2_LEN)) {
    mark_ide_free(ideif);
    __release_irq(irq);
    __release_io_space(io_port+IOPORT_OFFSET,IOPORT_LEN);
    return -1;
  }
  if (io_bmdma!=0)
    if (!__request_io_space(io_bmdma+BMDMAPORT_OFFSET,BMDMAPORT_LEN)) {
      mark_ide_free(ideif);
      __release_irq(irq);
      __release_io_space(io_port+IOPORT_OFFSET,IOPORT_LEN);
      __release_io_space(io_port2+IOPORT2_OFFSET,IOPORT2_LEN);
      return -1;
    }
    
#ifdef _PARANOIA
  ide[ideif].magic=IDE_MAGIC;
#endif
  
  ide[ideif].io_port=io_port;
  ide[ideif].io_port2=io_port2;
  ide[ideif].irq=irq;
  ide[ideif].dma=dma;
  ide[ideif].io_bmdma=io_bmdma;
  ide[ideif].pdisk[IDE_MASTER]=NULL;
  ide[ideif].info[IDE_MASTER].use_lba=0;
  ide[ideif].pdisk[IDE_SLAVE]=NULL;
  ide[ideif].info[IDE_SLAVE].use_lba=0;
  ide[ideif].actreq=-1;
  ide[ideif].actdrv=-1;
  bqueue_init(&ide[ideif].queue[0]);
  bqueue_init(&ide[ideif].queue[1]);
  
  /*
   * glue initialization
   */

  res=ide_glue_activate_interface(ideif);
  if (res==-1) {
    mark_ide_free(ideif);
    __release_irq(irq);
    __release_io_space(io_port+IOPORT_OFFSET,IOPORT_LEN);
    __release_io_space(io_port2+IOPORT2_OFFSET,IOPORT2_LEN);
    if (io_bmdma!=0) 
      __release_io_space(io_bmdma+BMDMAPORT_OFFSET,BMDMAPORT_LEN);
    return -1;
  }

  return ideif;
}

void ide_unregister(int ideif)
{

  ide_glue_unactivate_interface(ideif);

  __release_irq(ide[ideif].irq);
  __release_io_space(ide[ideif].io_port+IOPORT_OFFSET,IOPORT_LEN);
  __release_io_space(ide[ideif].io_port2+IOPORT2_OFFSET,IOPORT2_LEN);

  mark_ide_free(ideif);
  //if (ideif!=ideiftail-1) 
  //  memcpy(ide+ideif,ide+ideiftail-1,sizeof(ideinfo_t));
}

/*
 *
 *
 *
 */

/* commands used */
#define ATA_READ       0x20
#define ATA_WRITE      0x30
#define ATA_SEEK       0x70
#define ATA_DMAREAD    0xc8
#define ATA_DMAWRITE   0xca
#define ATA_IDENTIFY   0xec
#define ATA_PIDENTIFY  0xa1
#define ATA_SETFEATURE 0xef

/* for 'set feature' command */
#define ATA_FEATURE_ENABLELOOKAHEAD  0xaa
#define ATA_FEATURE_DISABLELOOKAHEAD 0x55
#define ATA_FEATURE_SETTRANSFERTMODE 0x03

/* commands type */
#define ATA_TYPE_PIOIN   0
#define ATA_TYPE_PIOOUT  1
#define ATA_TYPE_DMA     2
#define ATA_TYPE_NODATA  3

static inline int cmd2type(int cmd)
{
  switch(cmd) {
    case ATA_READ:
    case ATA_IDENTIFY:
    case ATA_PIDENTIFY:
      return ATA_TYPE_PIOIN;
    case ATA_WRITE:
      return ATA_TYPE_PIOOUT;
    case ATA_DMAREAD:
    case ATA_DMAWRITE:
      return ATA_TYPE_DMA;
    case ATA_SEEK:
    case ATA_SETFEATURE:
      return ATA_TYPE_NODATA;
  }
  return ATA_TYPE_NODATA;
}

#if defined(DEBUG_SERVER)||defined(DEBUG_SHOWREQUEST)
static char *cmd2str(int cmd)
{
  switch(cmd) {
    case ATA_READ: return "'read'";
    case ATA_WRITE: return "'write'";
    case ATA_SEEK: return "'seek'";
    case ATA_DMAREAD: return "'dma read'";
    case ATA_DMAWRITE: return "'dma write'";
    case ATA_IDENTIFY: return "'identify'";
    case ATA_PIDENTIFY: return "'packet identify'";
    case ATA_SETFEATURE: return "'set feature'";
  }
  return "'unknown'";
}
#endif

/* 
 * from ATA/ATAPI 4 (T13/1153D revision 18)
 * paragraph 9.7 (figure 12)
 */

#define RETRY 3


/*from Linux 2.2.13*/
/*
static int do_io_request(int ideif)
{
  int req=ide[ideif].reqhead;

  VM_out(IDE_REG_DEVCTRL,DEVCTRL_NORMALOP);
  outp(IDE_REG_SECCOU,idereq[req].seccou);  
  
  outp(IDE_REG_SECNUM,idereq[req].secnum);  
  outp(IDE_REG_CYLLOW,idereq[req].cyllow);
  outp(IDE_REG_CYLHIG,idereq[req].cylhig);
  outp(IDE_REG_DEVHEAD,idereq[req].devhead);

  outp(IDE_REG_COMMAND,idereq[req].cmd);
  
  return IDE_OK;  
}
*/

static int do_io_request(int ideif)
{
  int req;
  BYTE status;
  int n;
  WORD *ptr;
#ifndef IDE_SKIPALLTEST
  int retries;
#endif

  printk4(ideif,"A");
  
  //outp(IDE_REG_DEVCTRL,DEVCTRL_NORMALOP);
  //outp(IDE_REG_SECCOU,idereq[req].seccou);  

  req=first_idereq(ideif);
  assertk(req!=NIL);
  
  /* requirement for some commands */
  switch(idereq[req].cmd) {
    
    /* DRDY must be one */
    case ATA_READ:
    case ATA_DMAREAD:      
    case ATA_WRITE:
    case ATA_DMAWRITE:
    case ATA_SEEK:
    case ATA_IDENTIFY:
    case ATA_SETFEATURE:
      
#ifndef IDE_SKIPALLTEST
      retries=RETRY;
      while (--retries>=0) {
	status=inp(IDE_REG_STATUS);
	if ((status&STATUS_DRDY)!=0) break;
      }
      if (retries<0) return idereq[req].result=IDE_ERR_NOTDRDY;
      break;
#endif
      
      /* no requirement */
    case ATA_PIDENTIFY:
      break;
  }

  printk4(ideif,"B");

  /* a command can be issued when BUSY==0 && DRQ==0 */
#ifndef IDE_SKIPALLTEST
  retries=RETRY;  
  while (--retries>=0) {
    status=inp(IDE_REG_STATUS);
    if ((status&STATUS_BUSY)==0&&(status&STATUS_DRQ)==0) break;
  }  
  if (retries<0) {    
    /* if the ide device is in stand-by the request fail! */       
    if (status&STATUS_BUSY) return idereq[req].result=IDE_ERR_BUSY;
    else return idereq[req].result=IDE_ERR_NOTREADY;
  }
#endif

  printk4(ideif,"C");
  
  /* write "change device" register */
  outp(IDE_REG_DEVHEAD,idereq[req].devhead);

  /* to wait for 400ns (I hope) */
#ifndef IDE_SKIPALLTEST
  inp(IDE_REG_ALTSTATUS);
#endif
  
  /* wait for "change device" to take effect */
#ifndef IDE_SKIPALLTEST
  retries=RETRY;  
  while (--retries>=0) {
    status=inp(IDE_REG_STATUS);
    if ((status&STATUS_BUSY)==0&&(status&STATUS_DRQ)==0) break;
  }
  if (retries<0) {
    /* Well, if I do a command to a disk that does not exist an
     * interrupt is generated... so I MUST report no error
     * (the error is manage into the ide server)
     */
    return IDE_OK;
    if (status&STATUS_BUSY) return idereq[req].result=IDE_ERR_BUSY;
    else return idereq[req].result=IDE_ERR_NOTREADY;
  }
#endif

  printk4(ideif,"D");

  /* write all the registers */
  outp(IDE_REG_FEATURES,idereq[req].features);  
  outp(IDE_REG_SECCOU,idereq[req].seccou);  
  outp(IDE_REG_SECNUM,idereq[req].secnum);
  outp(IDE_REG_CYLLOW,idereq[req].cyllow);
  outp(IDE_REG_CYLHIG,idereq[req].cylhig);
  //outp(IDE_REG_DEVHEAD,idereq[req].devhead);
  
  if (cmd2type(idereq[req].cmd)==ATA_TYPE_DMA) {

    /*
     *
     * Bus Master DMA commands
     *
     */
    
    /* these code come from Linux 2.2.12 (modified!) */

    __uint32_t addr,*ptr;
    unsigned int size;
    int count;

    /* make PRD table */
    
    addr=__lin_to_phy(idereq[req].buffer);
    if (addr&3) {
      /* misaligned buffer */
      printk(KERN_ERR "ide do_io_request: misaligned DMA buffer (0x%08lx)",
	     (long)addr);
      return IDE_ERR_UNKNOWN;
    }
    size=IDE_SECTOR_SIZE; /* for now only 1 sector */
    
    ptr=ide[ideif].prd;
    count=0;
    while (size) {
      if (++count>=MAXPRDENTRIES) {	
	/* table to small for the request */
	printk(KERN_ERR "ide do_io_request: PRD table too small");
	return IDE_ERR_UNKNOWN;
      } else {
	unsigned int xcount, bcount = 0x10000 - (addr & 0xffff);	
	if (bcount > size)
	  bcount = size;
	*ptr++ = addr;
	xcount = bcount & 0xffff;
	//if (is_trm290_chipset)
	  //  xcount = ((xcount >> 2) - 1) << 16;	
	*ptr++ = xcount;		
	addr += bcount;
	size -= bcount;
      }
    }
    
    printk4(ideif,"E3");
      
    /* PRD table */
    outpd(BM_REG_PRDADDR,__lin_to_phy(ide[ideif].prd));
    
    /* specify r/w */
    if (idereq[req].cmd==ATA_DMAREAD) outp(BM_REG_COMMAND,1<<3);
    else {
      /* for default now... read */
      /*outp(BM_REG_COMMAND,1<<3);*/
      /* for write */
      outp(BM_REG_COMMAND,0);
    }
    
    /* clear INTR & ERROR flags */
    outp(BM_REG_STATUS,inp(BM_REG_STATUS)|6);
    
    /* write command*/
    outp(IDE_REG_COMMAND,idereq[req].cmd);

    /* start DMA */
    outp(BM_REG_COMMAND,inp(BM_REG_COMMAND)|1);

    printk4(ideif,"F3");
    
    return IDE_OK;    
  }

  /*
   *
   * PIO IN/OUT and NO_DATA commands
   *
   */
  
  /* write command*/
  outp(IDE_REG_COMMAND,idereq[req].cmd);

  /* to wait for 400ns; I hope */
#ifndef  IDE_SKIPSTATUSTEST
  inp(IDE_REG_STATUS);
#endif
  
  switch (cmd2type(idereq[req].cmd)) {

    /* for PIO data in commands and NODATA commands */
    
    case ATA_TYPE_PIOIN:
    case ATA_TYPE_NODATA:
      
      /* Well, the host should set the BUSY flag*/
#ifndef IDE_SKIPSTATUSTEST
      retries=RETRY;
      while (--retries>=0) {
	status=inp(IDE_REG_ALTSTATUS);
	if ((status&STATUS_BUSY)!=0) break;
      }
      if (retries<0) return IDE_ERR_NOTBUSY;
#endif
      printk4(ideif,"E1");

      return IDE_OK;

      /* for PIO data out commands */
      
    case ATA_TYPE_PIOOUT:
      
      for (;;) {
	/* Well, perhaps a timeout is necessary! */
	status=inp(IDE_REG_ALTSTATUS);
#ifndef IDE_SKIPALLTEST
	if ((status&STATUS_BUSY)!=0) break;
#else
	break;
#endif
      }
      printk4(ideif,"E2");

      if ((status&STATUS_ERR)||(status&STATUS_R_FAULT)) {
	/* an error is detected! */
	idereq[req].result=((status&STATUS_ERR)?
			    decode_error_result(ideif):IDE_ERR_FAULT
			    );
	return idereq[req].result;
	
      }
      if (!(status&STATUS_DRQ)) {
	/* error,  */
	return IDE_ERR_NODATAREQ;
      }

      /* Data I/O */
      ptr=(WORD*)idereq[req].buffer;
      for (n=0;n<IDE_SECTOR_SIZE>>1;n++)
	*ptr++=inpw(IDE_REG_DATA);

      /* to wait for 400ns; I hope ;-) */
#ifndef IDE_SKIPALLTEST
      inp(IDE_REG_ALTSTATUS);
#endif
      
#ifndef IDE_SKIPALLTEST
      status=inp(IDE_REG_ALTSTATUS);
      if ((status&STATUS_BUSY)==0) return IDE_ERR_NOTBUSY;
#endif
      
      printk4(ideif,"F2");

      return IDE_OK;
  }

  return IDE_ERR_UNKNOWN;
}

/* (must be 6sec for ATA specs) */
#define WAITENDRESET_ELAPSE 200000l

int do_ide_softreset(int ideif)
{
  unsigned long t;
  int flag;
  int status;
  
  printk2(ideif,"START");

  printk2(ideif,"waiting for not busy...");
  /* 1 msec */
  flag=1;
  t=__gettimer()+1000;
  while (__gettimer()<t) {
    status=inp(IDE_REG_ALTSTATUS);
    if (!(status&STATUS_BUSY)) { flag=0; break; }
  }
  if (flag) {
    printk2(ideif,"device is busy!");
    #ifndef IDE_FORCERESET
    printk2(ideif,"END");
    return IDE_ERR_TIMEOUT;
    #endif
    printk2(ideif,"with FORCERESET");
  } else
    printk2(ideif,"not busy");

  printk2(ideif,"soft resetting");
  outp(IDE_REG_DEVCTRL,DEVCTRL_SOFTRESET);
  __delayk(5);
  outp(IDE_REG_DEVCTRL,DEVCTRL_NORMALOP);

  __delayk(2000);

  printk2(ideif,"waiting for soft resetting end");
  /* 6sec */
  flag=1;
  t=__gettimer()+WAITENDRESET_ELAPSE;
  while (__gettimer()<t) {
    status=inp(IDE_REG_ALTSTATUS);
    if (!(status&STATUS_BUSY||!(status&STATUS_DRDY))) { flag=0; break; }
  }
  if (flag) {
    printk2(ideif,"not ending!!");
    printk2(ideif,"END");
    return IDE_ERR_TIMEOUT;
  }
  printk2(ideif,"resetting end");
  
  printk2(ideif,"END");
  return IDE_OK;
}

/*
 *
 *
 *
 */

/* abort per i packet */

static int REQ_PROLOG(void)
{
  int req;

  req=get_idereq();
  if (req==NIL)
    return IDE_ERR_TOOMUCHREQ;

  /* ide[].resetonerror:
   *
   * used by calling thread
   *    0 -> no soft reset on error
   *    1 -> request a soft reset on error
   * used by called thread (the server)
   *    2 -> the calling thread MUST do a soft reset
   */
  
  /* for safety */
  idereq[req].resetonerror=1;
  
  return req;
}

/* elapse for a timeouted request in usec (must be 2sec for ATA spec) */
#define TIMED_ELAPSE 80000l

static int __REQ_EPILOG(int ideif, int drv, int req, int timed)
{
  int res,ret;
  //long num;
  
  printk3(ideif,"START");
  
  ret=insert_idereq(ideif,drv,req);
  if (ret) {
    printk3(ideif,"activating server task");
    ide_glue_send_request(ideif);
  } else
    printk3(ideif,"server task already running");
  
  if (timed) {
    unsigned long timer,timer2,timer3;
    printk3(ideif,"waiting timed server reply");
    res=1;
    timer=__gettimer();
    printk3(ideif,"AA");
    assertk(timer!=0);
    timer+=TIMED_ELAPSE;
    printk3(ideif,"BB");
    timer3=0;

    /*
    {
      SYS_FLAGS f;      
      f=kern_fsave();
      kern_frestore(f);
      cprintf("[f=0x%x]",(int)f);
    }
    */
    
    while ((timer2=__gettimer())<timer) {
      
      //cprintf("<%li>",timer2);

      
      //if (timer2<timer3) break;      
      //timer3=timer2;
      
      res=__b_sem_trywait(&idereq[req].wait);
      if (!res) break;
    }
    printk3(ideif,"CC");
    if (res) {
      /* DANGER: if this is sent and an'interrupt occur
       * the reqpending assertion of the server fail
       */
      printk3(ideif,"timer expired.. try to remove request");
      ide_glue_send_request(ideif);
      __b_sem_wait(&idereq[req].wait);
      res=IDE_ERR_TIMEOUT;
    } else {
      res=idereq[req].result;
      printk3(ideif,"server reply ok");
    }
  } else {
    printk3(ideif,"waiting server reply");
#ifdef TRACEWAIT
    num=exec_shadow;
    trc_logevent(TRC_USER1,&num);
#endif
    __b_sem_wait(&idereq[req].wait);
#ifdef TRACEWAIT
    trc_logevent(TRC_USER2,&num);
#endif
    printk3(ideif,"server reply ok");
    res=idereq[req].result;
  }
  free_idereq(req);
  
  if (idereq[req].resetonerror==2) {
    printk3(ideif,"SOFT RESET");
    do_ide_softreset(ideif);
    ret=releasequeue_idereq(ideif);
    if (ret) {
      /* there are request pending... */
      ide_glue_send_request(ideif);
    }
  }

  
  printk3(ideif,"END");
  return res;
}

#define REQ_EPILOG(ideif,drv,req) __REQ_EPILOG(ideif,drv,req,0)
#define TIMED_REQ_EPILOG(ideif,drv,req) __REQ_EPILOG(ideif,drv,req,1)

/**/

static void fill_prologue(int req,
			  int cmd,
			  unsigned lsector,
			  struct phdskinfo *pdisk)
{
  unsigned track;

  if (cmd==REQ_DUMMY) {
    idereq[req].info.sector=0;
    idereq[req].info.head=0;
    idereq[req].info.cylinder=0;
    idereq[req].info.nsectors=0;
    idereq[req].info.operation=cmd;
    return;
  }
  track=lsector/pdisk->pd_phgeom.sectors;
  idereq[req].info.sector=lsector%pdisk->pd_phgeom.sectors+1;
  idereq[req].info.head=track%pdisk->pd_phgeom.heads;
  idereq[req].info.cylinder=track/pdisk->pd_phgeom.heads;
  idereq[req].info.nsectors=1;
  idereq[req].info.operation=cmd;
}

int ide_identify(int ideif, int id, struct ata_diskid *buffer)
{
  int req,ret;

  printk1(ideif,"%s start",cmd2str(ATA_IDENTIFY));
	  
  req=REQ_PROLOG();
  if (req<0) {
    printk1(ideif,"%s end=%i",cmd2str(ATA_IDENTIFY),req);
    return req;
  }
  
  idereq[req].cmd=ATA_IDENTIFY;
  idereq[req].features=0;
  idereq[req].cyllow=0;
  idereq[req].cylhig=0;
  idereq[req].seccou=0;
  idereq[req].secnum=0;
  idereq[req].devhead=0xa0|((id&0x01)<<4);
  idereq[req].buffer=(BYTE*)buffer;
  idereq[req].resetonerror=0;
  fill_prologue(req,REQ_DUMMY,0,ide[ideif].pdisk[id]);
		
  ret=TIMED_REQ_EPILOG(ideif,id,req);
  
  if (ret==IDE_OK) ide_fixdiskid(buffer);

  printk1(ideif,"%s end=%i",cmd2str(ATA_IDENTIFY),ret);  
  return ret;
}

int ide_pidentify(int ideif, int id, struct atapi_diskid *buffer)
{
  int req,ret;

  printk1(ideif,"%s start",cmd2str(ATA_PIDENTIFY));

  req=REQ_PROLOG();
  if (req<0) {
    printk1(ideif,"%s end=%i",cmd2str(ATA_PIDENTIFY),req);
    return req;
  }
  
  idereq[req].cmd=ATA_PIDENTIFY;
  idereq[req].features=0;
  idereq[req].cyllow=0;
  idereq[req].cylhig=0;
  idereq[req].seccou=0;
  idereq[req].secnum=0;
  idereq[req].devhead=0xa0|((id&0x01)<<4);
  idereq[req].buffer=(BYTE*)buffer;
  idereq[req].resetonerror=0;
  fill_prologue(req,REQ_DUMMY,0,ide[ideif].pdisk[id]);

  ret=TIMED_REQ_EPILOG(ideif,id,req);
  
  if (ret==IDE_OK) ide_fixatapidiskid(buffer);
  
  printk1(ideif,"%s end=%i",cmd2str(ATA_PIDENTIFY),ret);
  return ret;
}

#define IDE_READ_RETRIES  1
#define IDE_SEEK_RETRIES  1
#define IDE_WRITE_RETRIES 1

int ide_read(int ideif, int id, __blkcnt_t lsector, BYTE *buffer)
{
  int req,ret;
  int i;
  
  printk1(ideif,"%s start",cmd2str(ATA_READ));

  for (i=0;i<IDE_READ_RETRIES;i++) {
    
    req=REQ_PROLOG();
    if (req<0) {
      printk1(ideif,"%s end(1)=%i",cmd2str(ATA_READ),req);
      return req;
    }

    idereq[req].cmd=ATA_READ;
    assertk(ide[ideif].info[id].use_dma==0);
    if (ide[ideif].info[id].use_bm_dma) idereq[req].cmd=ATA_DMAREAD;
    lba2chs(ideif,id,lsector,req);
    idereq[req].features=0;
    idereq[req].seccou=1;
    idereq[req].buffer=buffer;
    idereq[req].resetonerror=1;
    fill_prologue(req,REQ_READ,lsector,ide[ideif].pdisk[id]);

    ret=REQ_EPILOG(ideif,id,req);
    if (ret==IDE_OK) break;
    if (i!=0) printk1(ideif,"%s retry",cmd2str(ATA_READ));
  }
  printk1(ideif,"%s end(2)=%i",cmd2str(ATA_READ),ret);
  return ret;
}

int ide_seek(int ideif, int id, __blkcnt_t lsector)
{  
  int req,ret;
  int i;
  
  printk1(ideif,"%s start",cmd2str(ATA_SEEK));

  for (i=0;i<IDE_SEEK_RETRIES;i++) {
    
    req=REQ_PROLOG();
    if (req<0) {
      printk1(ideif,"%s end=%i",cmd2str(ATA_SEEK),req);
      return req;
    }
  
    idereq[req].cmd=ATA_SEEK;
    lba2chs(ideif,id,lsector,req);
    idereq[req].features=0;
    idereq[req].seccou=0;
    idereq[req].buffer=NULL;
    idereq[req].resetonerror=1;
    fill_prologue(req,REQ_SEEK,lsector,ide[ideif].pdisk[id]);

    ret=REQ_EPILOG(ideif,id,req);
    if (ret==IDE_OK) break;
    if (i!=0) printk1(ideif,"%s retry",cmd2str(ATA_SEEK));
  }
  printk1(ideif,"%s end=%i",cmd2str(ATA_SEEK),ret);
  return ret;
}

int ide_enablelookahead(int ideif, int id)
{  
  int req,ret;

#ifndef IDE_ENABLESETFEATURES
  printk(KERN_NOTICE "ide command 'enable look-ahead' not yet implementated");
  return IDE_ERR_UNKNOWN;
#endif
  
  printk1(ideif,"%s start (enable look a head)",cmd2str(ATA_SETFEATURE));
    
  req=REQ_PROLOG();
  if (req<0) {
    printk1(ideif,"%s end=%i",cmd2str(ATA_SETFEATURE),req);
    return req;
  }

  idereq[req].cmd=ATA_SETFEATURE;
  idereq[req].features=ATA_FEATURE_ENABLELOOKAHEAD;
  idereq[req].cyllow=0;
  idereq[req].cylhig=0;
  idereq[req].seccou=0;
  idereq[req].secnum=0;
  idereq[req].devhead=((id&0x01)<<4);
  idereq[req].buffer=NULL;
  idereq[req].resetonerror=1;
  fill_prologue(req,REQ_DUMMY,0,ide[ideif].pdisk[id]);

  ret=REQ_EPILOG(ideif,id,req);

  //ide_dump_interface_status(ideif);

  __delayk(5);
  outp(IDE_REG_DEVCTRL,DEVCTRL_NORMALOP);

  printk1(ideif,"%s end=%i",cmd2str(ATA_SETFEATURE),ret);
  return ret;
}

int ide_disablelookahead(int ideif, int id)
{  
  int req,ret;
  
#ifndef IDE_ENABLESETFEATURES
  printk(KERN_NOTICE "ide command 'disable look-ahead' not yet implementated");
  return IDE_ERR_UNKNOWN;
#endif
  
  printk1(ideif,"%s start (enable look a head)",cmd2str(ATA_SETFEATURE));
    
  req=REQ_PROLOG();
  if (req<0) {
    printk1(ideif,"%s end=%i",cmd2str(ATA_SETFEATURE),req);
    return req;
  }

  idereq[req].cmd=ATA_SETFEATURE;
  idereq[req].features=ATA_FEATURE_DISABLELOOKAHEAD;
  idereq[req].cyllow=0;
  idereq[req].cylhig=0;
  idereq[req].seccou=0;
  idereq[req].secnum=0;
  idereq[req].devhead=((id&0x01)<<4);
  idereq[req].buffer=NULL;
  idereq[req].resetonerror=1;
  fill_prologue(req,REQ_DUMMY,0,ide[ideif].pdisk[id]);

  ret=REQ_EPILOG(ideif,id,req);

  printk1(ideif,"%s end=%i",cmd2str(ATA_SETFEATURE),ret);
  return ret;
}

int ide_settransfertmode(int ideif, int id, int mode)
{  
  int req,ret;

#ifndef IDE_ENABLESETFEATURES
  printk(KERN_NOTICE "ide command 'set transfert mode' not yet implementated");
  return IDE_ERR_UNKNOWN;
#endif
  
  printk1(ideif,"%s start (set transfert mode)",cmd2str(ATA_SETFEATURE));
    
  req=REQ_PROLOG();
  if (req<0) {
    printk1(ideif,"%s end=%i",cmd2str(ATA_SETFEATURE),req);
    return req;
  }

  idereq[req].cmd=ATA_SETFEATURE;
  idereq[req].features=ATA_FEATURE_SETTRANSFERTMODE;
  idereq[req].cyllow=0;
  idereq[req].cylhig=0;
  idereq[req].seccou=mode;
  idereq[req].secnum=0;
  idereq[req].devhead=((id&0x01)<<4);
  idereq[req].buffer=NULL;
  idereq[req].resetonerror=1;
  fill_prologue(req,REQ_DUMMY,0,ide[ideif].pdisk[id]);

  ret=REQ_EPILOG(ideif,id,req);

  ide_dump_interface_status(ideif);
  
  printk1(ideif,"%s end=%i",cmd2str(ATA_SETFEATURE),ret);
  return ret;
}

int ide_write(int ideif, int id, __blkcnt_t lsector, BYTE *buffer)
{  
  int req,ret;
  int i;
  
  printk1(ideif,"%s start",cmd2str(ATA_WRITE));

  for (i=0;i<IDE_WRITE_RETRIES;i++) {
    
    req=REQ_PROLOG();
    if (req<0) {
      printk1(ideif,"%s end=%i",cmd2str(ATA_WRITE),req);
      return req;
    }
  
    idereq[req].cmd=ATA_WRITE;    
    assertk(ide[ideif].info[id].use_dma==0);
    if (ide[ideif].info[id].use_bm_dma) idereq[req].cmd=ATA_DMAWRITE;
    else {
      panic("there is no support for polled write (only DMA allowed)!");
    }
    lba2chs(ideif,id,lsector,req);
    idereq[req].features=0;
    idereq[req].seccou=1;
    idereq[req].buffer=buffer;
    idereq[req].resetonerror=1;
    fill_prologue(req,REQ_WRITE,lsector,ide[ideif].pdisk[id]);

    ret=REQ_EPILOG(ideif,id,req);
    if (ret==IDE_OK) break;
    if (i!=0) printk1(ideif,"%s retry",cmd2str(ATA_WRITE));
  }
  printk1(ideif,"%s end=%i",cmd2str(ATA_WRITE),ret);
  return ret;
}

/**/

void ide_service_request(int ideif)
{
  static int reqpending=0;
  BYTE status,dma_status;
  int res=0,req=0;
  int n;

  printk0(ideif,"ACTIVATED");
  //assertk(ide[ideif].reqhead!=NIL);
    
  if (!reqpending) {
    printk0(ideif,"doing a new request");
    reqpending=1;
    goto DO_REQUEST;
  }
  
  printk0(ideif,"start to serve %s request",cmd2str(idereq[req].cmd));
  status=inp(IDE_REG_STATUS);    
  //req=ide[ideif].reqhead;
  req=actual_idereq(ideif);

  /*
    if (req==NIL) {
    printk(KERN_INFO "unaspceted INTR catch"); 
    continue;
    }
   */
     
  if (status&STATUS_BUSY||status&STATUS_ERR) {
    idereq[req].cmd=0;
    /* to FIX! */
  }
    
  switch (idereq[req].cmd) {

    /*
     * DMA COMMANDS
     *
     * DMAREAD
     */

    case ATA_DMAREAD:
    case ATA_DMAWRITE:
      /* from Linux 2.2.12 */
      
      /* stop DMA */      
      outp(BM_REG_COMMAND,inp(BM_REG_COMMAND)&~1);

      /* get DMA status */     
      dma_status = inp(BM_REG_STATUS);
      
      /* clear the INTR & ERROR bits */
      outp(BM_REG_STATUS,dma_status|6);
      
      /* verify good DMA status (0 -> OK)*/
      dma_status=(dma_status&7)!=4;	

      if (dma_status||status&STATUS_DRQ)
	idereq[req].result=((status&STATUS_ERR)?
			    decode_error_result(ideif):IDE_ERR_DMA
			    );
      else
	idereq[req].result=IDE_OK;
      break;
      
    /*
     * NO DATA COMMANDS
     *
     * SEEK and SET FEATURE
     */
    
    case ATA_SEEK:
    case ATA_SETFEATURE:
      
      if (status&STATUS_DRQ) {
	idereq[req].result=((status&STATUS_ERR)?
			    decode_error_result(ideif):IDE_ERR_DATA
			    );
      } else
	idereq[req].result=IDE_OK;
      
      break;
      
    case ATA_PIDENTIFY:
    case ATA_IDENTIFY:
    case ATA_READ:
	
      /*
       * PIO IN COMMANDS
       *
       * PIDENTIFY, IDENTIFY and READ commands
       */
	
      if (status&STATUS_DRQ) {
	WORD *ptr=(WORD*)idereq[req].buffer;
	for (n=0;n<IDE_SECTOR_SIZE>>1;n++)
	  *ptr++=inpw(IDE_REG_DATA);
	status=inp(IDE_REG_ALTSTATUS);    

      } else {
	idereq[req].result=((status&STATUS_ERR)?
			    decode_error_result(ideif):IDE_ERR_NODATA
			    );
      }
      status=inp(IDE_REG_STATUS);    
      if (status&STATUS_DRQ) idereq[req].result=IDE_ERR_NODATA; /*fix*/
      else idereq[req].result=IDE_OK;
	
      break;

      /*
       * PIO OUT COMMANDS
       *
       * WRITE command
       */
      
    case ATA_WRITE:
      /* all work is done into do_io_request() */
      idereq[req].result=IDE_OK;
      break;
      
      /*
       * BOH ?!?
       */
	
    default:
      /* an abort is more appropiate! */
      inp(IDE_REG_ALTSTATUS);    
      idereq[req].result=IDE_ERR_UNKNOWN;
      break;      
  }

#ifdef IDE_OLDATAPIDELAY
  if (idereq[req].cmd==ATA_PIDENTIFY) {
    /* delay for old ATAPI device */
    /* on my old cdrom a n>700 is ok */
    for (n=0;n<750;n++) {
      status=inp(IDE_REG_ALTSTATUS);
      if (status&0x100) break;
    }
  }
#endif

  reqpending=remove_idereq(ideif);
  printk0(ideif,"end to serve request (result=%i)",idereq[req].result);
  __b_sem_signal(&idereq[req].wait);
  if (reqpending) printk0(ideif,"another request into the queue");
    
DO_REQUEST:
  /* if there are requests pending... */
  if (reqpending)
    /* loop until no request error! */
    for (;;) {
      /* made request! */
      printk0(ideif,"made new request");
      res=do_io_request(ideif);
      if (res!=IDE_OK) {
	/* if request fail... */

	/* update result */
	printk0(ideif,"new request fail (code: %i)",res);
	
	//req=ide[ideif].reqhead;
	req=actual_idereq(ideif);
	idereq[req].result=res;

	/* if "request on error" ... */
	if (idereq[req].resetonerror==1) {
	  /* request a soft error */
	  printk0(ideif,"ERROR: soft reset in progress..");
	  idereq[req].resetonerror=2;
	  ide[ideif].errors++;
	  /* remove request (blocking new ones) and wake up blocked thread */
	  reqpending=remove_idereq_blocking(ideif);	
	  __b_sem_signal(&idereq[req].wait);
	  break;
	}

	/* remove request and wake up waiting thread */
	reqpending=remove_idereq(ideif);	
	__b_sem_signal(&idereq[req].wait);
		
	if (reqpending) printk0(ideif,"redoing a new request");
	/* if no more request... go out of the loop! */
	if (!reqpending) break;
      } else
        /* if request does not fail... */
	/* go out of the loop */
	printk0(ideif,"new request in progress");
	break;
    }
}
