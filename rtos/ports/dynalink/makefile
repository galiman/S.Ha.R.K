# Dynalink

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = dynalink

OBJS_PATH     = $(BASE)/dynalink


OBJS = dynalink.o elf.o

OTHERINCL += -I$(BASE)/drivers/linuxc26/include -I./include -I.
OTHERINCL += -I$(BASE)/drivers/pci/include
OTHERINCL += -I$(BASE)/drivers/input/include



C_OPT += -D__KERNEL__ -D__i386__ $(CFG_VIDEO_OPT)

include $(BASE)/config/lib.mk

clean::
	rm -f $(OBJS)
	rm -f deps

