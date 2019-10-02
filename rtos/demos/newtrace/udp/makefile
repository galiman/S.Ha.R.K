#
#
#

ifndef BASE
BASE=../../..
endif
include $(BASE)/config/config.mk

PROGS  = udptrace

include $(BASE)/config/example.mk

udptrace:
	make -f $(SUBMAKE) BASE=$(BASE) APP=udptrace INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __NET__"

