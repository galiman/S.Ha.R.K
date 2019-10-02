#
# The I/O sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc/libio

SRCS=	close.c 	\
	dir.c		\
	dup.c		\
	dup2.c		\
	getcwd.c	\
	getumask.c	\
	getwd.c		\
	lseek.c		\
	read.c		\
	truncate.c	\
	umask.c	 	\
	unlink.c	\
	wrappers.c	\
	write.c

OBJS=	$(patsubst %.c,%.o,$(SRCS))

include $(BASE)/config/sublib.mk
