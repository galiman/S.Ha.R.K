#
# The utsname sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc/utsname

SRCS= 	utsname.c

OBJS=	$(patsubst %.c,%.o,$(SRCS))

include $(BASE)/config/sublib.mk
