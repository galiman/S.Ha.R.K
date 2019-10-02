#
# The ctype sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc/ctype

SRCS=	ctype.c

OBJS=	$(patsubst %.c,%.o,$(SRCS))

include $(BASE)/config/sublib.mk
