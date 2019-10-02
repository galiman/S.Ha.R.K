#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS=  orbit

include $(BASE)/config/example.mk

orbit:
	make -f $(SUBMAKE) APP=orbit INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

