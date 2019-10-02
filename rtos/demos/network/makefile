#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS  = net talk

include $(BASE)/config/example.mk

net:
	make -f $(SUBMAKE) BASE=$(BASE) APP=net INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__PCI__ __LINUXC26__ __INPUT__ __NET__"

talk:
	make -f $(SUBMAKE) BASE=$(BASE) APP=talk INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__PCI__ __LINUXC26__ __INPUT__ __NET__"

notify:
	make -f $(SUBMAKE) BASE=$(BASE) APP=notify INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__PCI__ __LINUXC26__ __INPUT__ __NET__"
