# The Hartik 3.3.1 Sound Library

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = hsnd

OBJS_PATH     = $(BASE)/drivers/oldsnd

OTHERINCL += -I./include

OBJS =  sbio.o blaster.o dma.o sound.o

include $(BASE)/config/lib.mk

