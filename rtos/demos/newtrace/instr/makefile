#
#
#

ifndef BASE
BASE=../../..
endif
include $(BASE)/config/config.mk

PROGS  = instr

include $(BASE)/config/example.mk

instr:
	make -f $(SUBMAKE) BASE=$(BASE) APP=instr INIT= OTHEROBJS="initfile.o bezier.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __NET__"
	#make -f $(SUBMAKE) BASE=$(BASE) APP=instr INIT= OTHEROBJS="initfile.o bezier_i.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __NET__

