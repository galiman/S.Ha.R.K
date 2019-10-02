#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS=  bca

include $(BASE)/config/example.mk

bca:
	make -f $(SUBMAKE) APP=bca INIT= OTHEROBJS="initfile.o fab_lib.o bca_lib.o esplo.o missile.o cannone.o aereo.o control.o dummy.o scenario.o" SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

