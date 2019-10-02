ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS += shutdown

include $(BASE)/config/example.mk

shutdown:
	make -f $(SUBMAKE) APP=shutdown INIT= OTHEROBJS="ishut.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__"
