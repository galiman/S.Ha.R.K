# The Serial Devices

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = serial

OBJS_PATH     = $(BASE)/drivers/serial

OBJS  = scom.o scomirq.o

OTHERINCL += -I$(BASE)/drivers/serial/include

include $(BASE)/config/lib.mk

