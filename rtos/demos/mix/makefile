#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS=  mix

include $(BASE)/config/example.mk

mix:
	make -f $(SUBMAKE) APP=mix INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

