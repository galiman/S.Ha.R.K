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


#ifndef __FS_MACCESS_H
#define __FS_MACCESS_H

/*+ if defined: the following functions are 'dummy' function +*/
#define DUMMY_MACCESS
//#undef  DUMMY_MACCESS

#ifdef DUMMY_MACCESS
#include <fs/util.h>
#endif

/*
 * port in/out
 */

#include <ll/i386/hw-instr.h>

/*
 *
 * address translation
 *
 */

//#include <x86/xsys.h>
//#define __lin_to_phy(ptr) appl2linear(ptr)
//#define __phy_to_lin(addr) linear2appl(addr)

#define __lin_to_phy(ptr)  ((int)ptr)
#define __phy_to_lin(addr) ((void*)addr)

/*
 *
 * verify read/write access to user space
 *
 */
#include "ll/sys/cdefs.h"

__BEGIN_DECLS

extern int verifyuserread(const void * addr, unsigned long size);
extern int verifyuserwrite(const void * addr, unsigned long size);
extern int verifyuserreadnolen(const void *addr);

#ifdef DUMMY_MACCESS

#define __verify_write(x,y) (-1)
#define __verify_read(x,y) (-1)
#define __verify_read_nolen(x) (1)

#else

#define __verify_write(x,y) verifyuserwrite(x,y)
#define __verify_read(x,y) verifyuserread(x,y)
#define __verify_read_nolen(x) verifyuserreadnolen(x)

#endif
         
/*
 *
 * copy data to/from user space
 *
 */
     
#ifdef DUMMY_MACCESS

extern __inline__ void memcpytouser(void *to, void *from, int size)
{ memcpy(to,from,size); }
extern __inline__ void memcpyfromuser(void *to, void *from, int size)
{ memcpy(to,from,size); }

#define declare_buffer(type,name,maxlen) type *name
#define init_buffer(name) name=NULL
#define __copy_from_user(to,from,size) ((to)=(from))
#define __copy_to_user(to,from,size) memcpy(to,from,size)

#else

extern void memcpytouser(void *to, void *from, int size);
extern void memcpyfromuser(void *to, void *from, int size);

#define declare_buffer(type,name,maxlen) type name[maxlen]
#define init_buffer(name) 
#define __copy_from_user(to,from,size) memcpyfromuser(to,from,size)
#define __copy_to_user(to,from,size) memcpytouser(to,from,size)
						    
#endif    

/*
 *
 * physical resource allocator
 *
 */

extern int requestiospace(void *startaddr, int size);
extern int releaseiospace(void *startaddr, int size);

#ifdef DUMMY_MACCESS

#define __request_io_space(x,y) (-1)
#define __release_io_space(x,y) 

#else

#define __request_io_space(x,y) requestiospace(x,y)
#define __release_io_space(x,y) releaseiospace(x,y)

#endif
__END_DECLS
#endif
