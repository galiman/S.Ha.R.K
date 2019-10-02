# The console Devices (console)

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk


LIBRARY       = cons

OBJS_PATH     = $(BASE)/drivers/cons

OBJS  = cons1.o cons2.o message.o cprintf.o crtwin.o

OTHERINCL += -I$(BASE)/drivers/cons/include

include $(BASE)/config/lib.mk

