# National Instruments PCI6025E Driver

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = bttv

OBJS_PATH     = $(BASE)/drivers/bttv

OBJS = bttv-if.o btcx-risc.o bttv-cards.o bttv-driver.o bttv-risc.o shark_bttv26.o 

OTHERINCL += -I$(BASE)/drivers/bttv/include -I$(BASE)/drivers/linuxc26/include -I.

C_OPT += -D__KERNEL__

include $(BASE)/config/lib.mk

clean::
	rm -f $(OBJS)

