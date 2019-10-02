
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


#ifndef __MSDOS_H__
#define __MSDOS_H__

#include "dentry.h"
#include "inode.h"
#include "super.h"

#include "dentryop.h"
#include "inodeop.h"
#include "superop.h"

extern struct dentry_operations msdos_dentry_operations;
extern struct inode_operations  msdos_inode_ro_operations;
extern struct file_operations   msdos_file_ro_operations;
extern struct super_operations  msdos_super_operations;

extern struct inode_operations  msdos_inode_rw_operations;
extern struct file_operations   msdos_file_rw_operations;

/*
 * directory entry
 */

#define ATTR_RDONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME 0x08
#define ATTR_DIR    0x10
#define ATTR_ARCH   0x20

struct directoryentry {
  __uint8_t  name[8];
  __uint8_t  ext[3];
  __uint8_t  attribute;
  __uint8_t  reserved[10];
  __uint16_t time;
  __uint16_t date;
  __uint16_t cluster;
  __uint32_t size;
} __attribute__ ((packed));

/* for debugging */
extern void msdos_dump_direntry(struct directoryentry *ptr);

/* directory entry types */

#define LASTENTRYCHAR   0x00
#define ERASEDENTRYCHAR 0xe5

extern __inline__ int msdos_islastentry(struct directoryentry *de)
{
  return (*((de)->name)==LASTENTRYCHAR);
}

extern __inline__ int msdos_isfreeentry(struct directoryentry *de)
{
  return (*((de)->name)==LASTENTRYCHAR||
	  *((de)->name)==ERASEDENTRYCHAR);
}

extern __inline__ int msdos_isunvalidentry(struct directoryentry *de)
{
  return (*((de)->name)==ERASEDENTRYCHAR)||
    ((de)->attribute&ATTR_VOLUME)||
    (msdos_islastentry(de));
}

extern __inline__ void msdos_markentrybusy(struct directoryentry *de)
{
  *((de)->name)='~';
}

extern __inline__ void msdos_markentryfree(struct directoryentry *de)
{
  *((de)->name)=ERASEDENTRYCHAR;
}

/* */
extern int msdos_formatname(struct directoryentry *de, struct qstr *str);

/*
 * constants
 */

#define CLUSTER_MASK    0xffff0000
#define CLUSTER_SHIFT   16
#define DENTRY_MASK     0x0000ffff
#define DENTRY_SHIFT    0

#define ROOT_CLUSTER    0x0000
#define SPECIAL_CLUSTER 0x0001
#define FREE_CLUSTER    0xfffe
#define NO_CLUSTER      0xffff

#define DIRENTRYSIZE    32
#define SECTORSIZE      512

/* first cluster of a msdos volume */
#define STARTCLUSTER    2

/* these on msdos directory structure */
#define FREECLUSTER 0x0000
#define LASTCLUSTER 0xffff
#define BADCLUSTER  0xfff7

/* max numbers of fats for volume (that must be supported) */
#define MAXFATS         4
/*
 * 0x01230008 e' la 8' entry del 123 cluster
 * 0x00000000 e' la root dir (non ha entry)
 * 0x00010005 e' la 5' entry nella root
 */

#define CLUOFF2INODE(cluster,deoffs) ((((__uint32_t)cluster)<<CLUSTER_SHIFT)|\
				       ((deoffs)&DENTRY_MASK)\
				       )
#define INODE2CLUSTER(inode) ((__uint16_t)((inode)>>CLUSTER_SHIFT))
#define INODE2DEOFFS(inode)  ((__uint16_t)((inode)&DENTRY_MASK))

#define ROOT_INODE      CLUOFF2INODE(ROOT_CLUSTER,0)

/*
 *
 */

/* what is the logical sector of a cluster */
extern __inline__ __uint32_t msdos_cluster2sector(struct inode *in,
						  __uint16_t cluster)
{
  if (cluster==NO_CLUSTER||
      cluster==SPECIAL_CLUSTER||
      cluster==FREE_CLUSTER) return (__uint32_t)(-1);
  if (cluster==ROOT_CLUSTER) return MSDOS_SB(in->i_sb).lroot;
  return (cluster-2)*MSDOS_SB(in->i_sb).spc+MSDOS_SB(in->i_sb).ldata;   
}

/*
#define msdos_cluster2sector(in,cl) \
( \
 ((cl)==NO_CLUSTER||(cl)==SPECIAL_CLUSTER||(cl)==FREE_CLUSTER)? \
 (-1): \
 ( \
  ((cl)==ROOT_CLUSTER?) \
  MSDOS_SB((in-)>i_sb).lroot: \
  ((cl)-2)*MSDOS_SB((in)->i_sb).spc+MSDOS_SB((in)->i_sb).ldata \
 ) \
)
*/
     
__uint32_t msdos_cluster2sector(struct inode *in, __uint16_t cluster);

__uint16_t msdos_nextcluster(struct super_block *sb, __uint16_t cluster);
void msdos_freecluster(struct super_block *sb, __uint16_t cluster);
void msdos_lastcluster(struct super_block *sb, __uint16_t cluster);
__uint16_t msdos_addcluster(struct super_block *sb, __uint16_t lacluster);
void msdos_freeclusterchain(struct super_block *sb, __uint16_t cluster);

#endif
