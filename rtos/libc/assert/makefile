#
# The assert sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc/assert

SRCS=	assert.c

OBJS=	$(patsubst %.c,%.o,$(SRCS))

include $(BASE)/config/sublib.mk
