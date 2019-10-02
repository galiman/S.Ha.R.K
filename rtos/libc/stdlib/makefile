#
# The stdlib sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc/stdlib

SRCS=	abort.c		\
	atexit.c	\
	calloc.c	\
	free.c		\
	malloc.c	\
	realloc.c	\
	strtod.c	\
	strtol.c	\
	strtoul.c	\
	qsort.c		\
	bsearch.c


OBJS=	$(patsubst %.c,%.o,$(SRCS))

C_DEF += -D_THREAD_SAFE -Di386

include $(BASE)/config/sublib.mk
