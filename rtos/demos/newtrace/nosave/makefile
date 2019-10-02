#
#
#

ifndef BASE
BASE=../../..
endif
include $(BASE)/config/config.mk

PROGS  = nosave

include $(BASE)/config/example.mk

nosave:
	make -f $(SUBMAKE) BASE=$(BASE) APP=nosave INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__"

