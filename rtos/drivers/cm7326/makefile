# CM7326 - Driver for pc104+ framegrabber

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = cm7326

OBJS_PATH     = $(BASE)/drivers/cm7326

OBJS = cm7326.o saa7146_core.o saa7146_hlp.o saa7146_i2c.o saa7146_video.o saa7146_fops.o

OTHERINCL += -I$(BASE)/drivers/cm7326/include -I$(BASE)/drivers/linuxc26/include -I$(BASE)/drivers/i2c/include

C_OPT += -D__KERNEL__

include $(BASE)/config/lib.mk

clean::
	rm -f $(OBJS)

