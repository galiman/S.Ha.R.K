#
# The stdlib sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc/stdlib

SRCS=  getopt.c getopt1.c

OBJS=	$(patsubst %.c,%.o,$(SRCS))

C_DEF += -D_THREAD_SAFE -Di386   

include $(BASE)/config/sublib.mk
