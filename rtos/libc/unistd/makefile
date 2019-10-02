#
# The unistd sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc/unistd

SRCS= 	fpathcon.c \
	pathconf.c \
	sysconf.c 

OBJS=	$(patsubst %.c,%.o,$(SRCS))

include $(BASE)/config/sublib.mk
