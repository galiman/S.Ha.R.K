# The Tracer Library

ifndef BASE
BASE=..
endif

include $(BASE)/config/config.mk
include $(BASE)/shark.cfg

LIBRARY       = tracer

INCL += -I./include -I$(BASE)/drivers/net/include

OBJS_PATH     = $(BASE)/tracer

# Object files

ifeq ($(findstring NEW,$(TRACER)) , NEW)
OBJS =  newtrace/FTrace.o newtrace/FTrace_OSD.o newtrace/FTrace_OSD_ll.o\
	newtrace/FTrace_udp.o newtrace/FTrace_disk.o
CFG_OPT += -D__NEW_TRACER__ 
endif
ifeq ($(findstring OLD,$(TRACER)) , OLD)
OBJS = oldtrace/oldtrace.o oldtrace/trccirc.o oldtrace/trcdfix.o \
	oldtrace/trcudp.o oldtrace/trcfixed.o oldtrace/trcdummy.o oldtrace/ctable.o
CFG_OPT += -D__OLD_TRACER__
endif

ifneq ($(findstring NO,$(TRACER)) , NO)
include $(BASE)/config/lib.mk
endif

install all clean cleanall depend::

