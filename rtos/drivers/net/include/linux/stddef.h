#ifndef _LINUX_STDDEF_H
#define _LINUX_STDDEF_H

#include "ll/sys/cdefs.h"

__BEGIN_DECLS

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#undef NULL
#define NULL ((void *)0)

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

__END_DECLS

#endif
