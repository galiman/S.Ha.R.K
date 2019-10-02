#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS = mesatex

include $(BASE)/config/example.mk

mesatex:
	make -f $(SUBMAKE) APP=mesatex INIT= OTHEROBJS="initfile.o" SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__ __OSMESA__"

