# The Char Devices (mouse, serial ports, keyboard)

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = oldch

OBJS_PATH     = $(BASE)/drivers/oldchar

OBJS  = keyb.o mouse.o ps2mouse.o 8042.o mcurtxt.o mcurgrx.o

OTHERINCL += -I$(BASE)/drivers/oldchar/include -I$(BASE)/drivers/grx/include

include $(BASE)/config/lib.mk

