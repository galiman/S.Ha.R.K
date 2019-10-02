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

  CVS :        $Id: ide.h,v 1.2 2003/03/13 13:43:13 pj Exp $
 
  Revision:    $Revision: 1.2 $

  Last update: $Date: 2003/03/13 13:43:13 $

  Header file for all the modules of the IDE block device interface
  sub-system.

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

#ifndef __IDE_H__
#define __IDE_H__

#include "glue.h"
#include "bdev.h"
#include "bqueue.h"
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

/*+ max number of ide interface (must be <= 8) +*/
#define MAXIDEINTERFACES 4

/*---*/

/*
 * CONSTANTS
 */

/*+ max number of "minor number" required +*/
#define MAXIDEMINOR (MAXIDEINTERFACES<<5)

/*+ max number of "logical partition" per hard disk +*/
#define MAXIDELODSK 16

/*+ number of physical region descriptors used by DMA bus master operations +*/
#define MAXPRDENTRIES 4

/*+ for a master hard disk +*/
#define IDE_MASTER 0x00
/*+ for a slave hard disk +*/
#define IDE_SLAVE  0x01

/*+ sector size in bytes +*/
#define IDE_SECTOR_SIZE 512

/*+ Log Level for errors +*/
#define IDEERRLOG   KERN_ERR
/*+ Log Level for log +*/
#define IDELOG      KERN_NOTICE
/*+ Log Level for debug purpose +*/
#define IDEDEBUGLOG KERN_DEBUG

/*
 * for result
 */

/*+
  This values are returned by the low level routine to indicate
  an error.
  +latex+ \\

  No error +*/
#define IDE_OK                0
/*+ there are too much request pending +*/
#define IDE_ERR_TOOMUCHREQ   -1
/*+ the interface is busy or can not do a command+*/
#define IDE_ERR_BUSY         -2
/*+ the following codes are from the IDE error register +*/
#define IDE_ERR_BADBLOCK     -3
#define IDE_ERR_CRCECC       -4
#define IDE_ERR_IDNOTFOUND   -5
#define IDE_ERR_ABORTCMD     -6
#define IDE_ERR_TRACK00      -7
#define IDE_ERR_DAMNOTFOUND  -8
/*+ generic error (when there are not other IDE_ERR_?? available) +*/
#define IDE_ERR_UNKNOWN      -9
/*+ there was an interrupt, we aspect data but there is not data +*/
#define IDE_ERR_NODATA      -10
/*+ there was an interrupt, we do not aspect data but there is data +*/
#define IDE_ERR_DATA        -11
/*+ interface not ready for a command +*/
#define IDE_ERR_NOTREADY    -12 
/*+ a timeout waiting for a reply +*/
#define IDE_ERR_TIMEOUT     -13
/*+ the interface must be busy but it is not +*/
#define IDE_ERR_NOTBUSY     -14
/*+ device fault +*/
#define IDE_ERR_FAULT       -15
/*+ we aspect to send data but there is no data request +*/
#define IDE_ERR_NODATAREQ   -16
/*+ the DRDY signal is not preset +*/
#define IDE_ERR_NOTDRDY     -17
/*+ DMA cycle error +*/
#define IDE_ERR_DMA         -18

/*+ for the IDE_ERR_??? constants +*/
extern char *ide_error_msg[];

/*
 * DATA STRUCTURES
 */

/*+ 
  Information on IDE devices capabilities
  +*/
struct ide_diskinfo {
  __uint32_t use_lba:1;     /*+ use LBA address translation +*/
  __uint32_t use_dma:1;     /*+ use old dma operation +*/
  __uint32_t use_bm_dma:1;  /*+ use new bus master dma operation +*/
  __int8_t  max_pio_mode;   /*+ max pio mode supported (-1 no support) +*/ 
  __int8_t  max_dma_mode;   /*+ max multi word dma support (-1 no support) +*/
  __int8_t  max_udma_mode;  /*+ max ultra dma mode support (-1 no support) +*/
};

/*+ 
  Information on IDE interfaces 
  +*/
struct ideinfo {

#ifdef _PARANOIA
  __uint32_t magic;        /*+ magic number +*/
#endif

  __uint16_t io_port;      /*+ I/O address +*/
  __uint16_t io_port2;     /*+ auxiliary I/O address +*/
  __uint8_t  irq;          /*+ irq number +*/
  __uint8_t  dma;          /*+ dma channel +*/
  __uint16_t io_bmdma;     /*+ I/O address for a bus master DMA cycle */

  /* this must be 4-bytes aligned */
  __uint32_t prd[MAXPRDENTRIES*2];
                           /*+ physical region descriptors (used by DMA) */
  
  struct phdskinfo    *pdisk[2]; /*+ physical disk pointer for master/slave +*/
  struct ide_diskinfo info[2];   /*+ more disk information for master/slave +*/

  bqueue_t   queue[2];
  int        actreq;       /*+ actual request that we are serving +*/
  int        actdrv;       /*+ drive id of the actual request +*/
  
  int        errors;       /*+ incremented for every error */
  
  /* for the glue routines (see ideglue.c) */
  __pid_t    server;       /*+ interface server pid +*/

} __attribute__ ((aligned (4)));

/*+
  Information on minor devices
  +*/
struct ideminorinfo {
  
#ifdef _PARANOIA
  __uint32_t  magic;     /*+ magic number +*/  
#endif
  
  __blkcnt_t  start;     /*+ first block of this logical disk (partition) +*/
  __blkcnt_t  size;      /*+ size of the logical disk (in sectors) +*/
  
  __uint16_t  used:1;    /*+ is this entry used? +*/
  __uint16_t  blocked:1; /*+ is this entry blocked? (is someone using it?) +*/
};

/*
 * DATA
 */

/* these are here because are used both ide.c and idelow.c */

extern struct ideinfo ide[MAXIDEINTERFACES];

extern int ide_showinfo_flag;

extern void *ide_parm_initserver;

/*
 * FUNCTIONS
 */

/* register an hardware interface on first free slot */
int ide_register(__uint16_t io_port, __uint16_t io_port2,
		 __uint8_t irq,
		 __uint8_t dma, __uint16_t io_bmdma);
/* release resource used by the ideif interface */
void ide_unregister(int ideif);

/* scan an interface to found device */
int ide_scan(int ideif);

/* scan an hard disk to found a partitions scheme */
int ide_scandisk(int hwif, int drv);


struct ata_diskid;
struct atapi_diskid;

int do_ide_softreset(int ideif);
int ide_read(int ideif, int id, __blkcnt_t lsector, __uint8_t *buffer);
int ide_write(int ideif, int id, __blkcnt_t lsector, __uint8_t *buffer);
int ide_seek(int ideif, int id, __blkcnt_t lsector);
int ide_pidentify(int ideif, int id, struct atapi_diskid *buffer);
int ide_identify(int ideif, int id, struct ata_diskid *buffer);
int ide_enablelookahead(int ideif, int id);
int ide_disablelookahead(int ideif, int id);
int ide_settransfertmode(int ideif, int id, int mode);

TASK ide_server(int ideif);

#define is_ide_free(ideif) (ide[(ideif)].io_port==0)
#define mark_ide_free(ideif) (ide[(ideif)].io_port=0)

static inline int nextideif(void)
{
  int ideif;
  for (ideif=0;ideif<MAXIDEINTERFACES;ideif++)
    if (is_ide_free(ideif)) break;
  return ideif;
}

/*
 * LOW LEVEL data structures
 */

/* ATA-4 */
/* obs: obsolete res: reserved ven: vendor specific */

/*+ IDENTIFY command response (from ATA 4 specifications) +*/
struct ata_diskid {
  __uint16_t  config;
  __uint16_t  def_cyls;
  __uint16_t  res2;
  __uint16_t  def_heads;
  __uint16_t  res4;
  __uint16_t  res5;
  __uint16_t  def_sects;
  __uint16_t  ven7;
  __uint16_t  ven8;
  __uint16_t  ven9;
  __uint8_t   serial[20];
  __uint16_t  ven20;
  __uint16_t  ven21;
  __uint16_t  obs22;
  __uint8_t   firmware[8];
  __uint8_t   model[40];
  __uint16_t  maxmulsect;
  __uint16_t  res48;
  __uint16_t  capabilities;
  __uint16_t  capabilities2;
  __uint8_t   ven51;
  __uint8_t   PIO_mode;
  __uint16_t  ven52;
  __uint16_t  fields_valid;
  __uint16_t  act_cyls;      // 54
  __uint16_t  act_heads;
  __uint16_t  act_sects;
  __uint16_t  cur_capacity0;
  __uint16_t  cur_capacity1; // 58
  __uint8_t   multsect;      
  __uint8_t   multsect_valid;
  __uint32_t  lba_capacity;
  __uint16_t  obs62;
  __uint16_t  DMA_mword;
  __uint8_t   eide_PIO_modes;
  __uint8_t   res64 ;
  __uint16_t  eide_dma_min; 
  __uint16_t  eide_dma_time;
  __uint16_t  eide_pio;     
  __uint16_t  eide_pio_iordy;
  __uint16_t  res69;
  __uint16_t  res70;          // 70
  __uint16_t  res71;
  __uint16_t  res72;
  __uint16_t  res73;
  __uint16_t  res74;
  __uint16_t  queue_depth;
  __uint16_t  res76;
  __uint16_t  res77;
  __uint16_t  res78;
  __uint16_t  res79;  
  __uint16_t  major_version;
  __uint16_t  minor_version;
  __uint16_t  command_sets;
  __uint16_t  command_sets2;
  __uint16_t  command_sets_ext;
  __uint16_t  command_sets_ena;
  __uint16_t  command_sets_ena2;
  __uint16_t  command_sets_def;
  __uint16_t  UDMA_cap;
  __uint16_t  erase_time;
  __uint16_t  erase_time2;
  __uint16_t  current_power;
  __uint16_t  reservedA[126-92+1];
  __uint16_t  status_notification;
  __uint16_t  security;
  __uint16_t  vendor[159-129+1];
  __uint16_t  reservedB[255-160+1];
} __attribute__((packed));

#define is_ATAdevice(p)    ((p)->config&0x8000)
#define is_removable(p)    ((p)->config&0x0080)
#define is_notremovable(p) ((p)->config&0x0040)

// 54 -> 58
#define is_actcurvalid(p)  ((p)->fields_valid&0x0001)
// 64 -> 70
#define is_eidevalid(p)    ((p)->fields_valid&0x0002)
// 88
#define is_udmavalid(p)  ((p)->fields_valid&0x0004)

#define is_ATA(p)          ((p)->major_version&0x02)
#define is_ATA2(p)         ((p)->major_version&0x04)
#define is_ATA3(p)         ((p)->major_version&0x08)
#define is_ATA4(p)         ((p)->major_version&0x10)

#define is_LBAcapable(p)   ( \
                            ((p)->capabilities&0x0200) \
                            || is_ATA3(p) \
                            || is_ATA4(p) \
                           )

/*+ PIDENTIFY command response (for ATAPI device from ATA 4 specs) +*/
struct atapi_diskid {
  __uint16_t  config;
  __uint16_t  res1;
  __uint16_t  res2;
  __uint16_t  res3;
  __uint16_t  res4;
  __uint16_t  res5;
  __uint16_t  res6;
  __uint16_t  ven7;
  __uint16_t  ven8;
  __uint16_t  ven9;
  __uint8_t   serial[20];
  __uint16_t  res20;
  __uint16_t  res21;
  __uint16_t  res22;
  __uint8_t   firmware[8];
  __uint8_t   model[40];
  __uint16_t  res47;
  __uint16_t  res48;
  __uint16_t  capabilities;
  __uint16_t  res50;
  __uint8_t   ven51;
  __uint8_t   tPIO;
  __uint16_t  res52;
  __uint16_t  fields_valid;
  __uint16_t  res54;   //54
  __uint16_t  res55;
  __uint16_t  res56;
  __uint16_t  res57;
  __uint16_t  res58;  // 58
  __uint16_t  res59;
  __uint16_t  res60;
  __uint16_t  res61;
  __uint16_t  res62;
  __uint16_t  dma_mword;
  __uint16_t  eide_pio_modes; //64
  __uint16_t  eide_dma_min; 
  __uint16_t  eide_dma_time;
  __uint16_t  eide_pio;     
  __uint16_t  eide_pio_iordy;
  __uint16_t  res69;
  __uint16_t  res70;          // 70
  __uint16_t  time_packet;
  __uint16_t  time_service;
  __uint16_t  res73;
  __uint16_t  res74;
  __uint16_t  queue_depth;
  __uint16_t  res76;
  __uint16_t  res77;
  __uint16_t  res78;
  __uint16_t  res79;  
  __uint16_t  major_version;
  __uint16_t  minor_version;
  __uint16_t  command_sets;
  __uint16_t  command_sets2;
  __uint16_t  command_sets_ext;
  __uint16_t  command_sets_ena;
  __uint16_t  command_sets_ena2;
  __uint16_t  command_sets_def;
  __uint16_t  udma;
  __uint16_t  reservedA[126-89+1];
  __uint16_t  status_notification;
  __uint16_t  security;
  __uint16_t  vendor[159-129+1];
  __uint16_t  reservedB[255-160+1];
} __attribute__((packed));

#define is_ATAPIdevice(p)  (((p)->config&0xc000)==0x8000)
//#define is_removable(p)    ((p)->config&0x0080)
#define is_packet12capable(p) (((p)->conig&0x0003)==0x0000)
#define is_packet16capable(p) (((p)->conig&0x0003)==0x0001)

#define is_directdev(p)        ((((p)->config&0x1f00)>>8)==0x00)
#define is_sequentialdev(p)    ((((p)->config&0x1f00)>>8)==0x01)
#define is_printerdev(p)       ((((p)->config&0x1f00)>>8)==0x02)
#define is_processordev(p)     ((((p)->config&0x1f00)>>8)==0x03)
#define is_writeoncedev(p)     ((((p)->config&0x1f00)>>8)==0x04)
#define is_cdromdev(p)         ((((p)->config&0x1f00)>>8)==0x05)
#define is_scannerdev(p)       ((((p)->config&0x1f00)>>8)==0x06)
#define is_opticalmemorydev(p) ((((p)->config&0x1f00)>>8)==0x07)
#define is_mediachengerdev(p)  ((((p)->config&0x1f00)>>8)==0x08)
#define is_communicatordev(p)  ((((p)->config&0x1f00)>>8)==0x09)
#define is_arraydev(p)         ((((p)->config&0x1f00)>>8)==0x0c)

__END_DECLS
#endif






