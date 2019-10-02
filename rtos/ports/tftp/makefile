# TFTP

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = tftp

OBJS_PATH     = $(BASE)/ports/tftp

OBJS = tftp.o endn.o

OTHERINCL += -I$(BASE)/ports/tftp/include -I$(BASE)/drivers/net/include/

include $(BASE)/config/lib.mk

clean::
	rm -f *.o
