# The Parallel Port Library, by Andrea Battistotti & Armando Leggio

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = pport

OBJS_PATH     = $(BASE)/drivers/parport

OBJS  = ppdrv.o ppnrtdrv.o pppindrv.o

OTHERINCL += -I$(BASE)/drivers/parport/include

include $(BASE)/config/lib.mk

