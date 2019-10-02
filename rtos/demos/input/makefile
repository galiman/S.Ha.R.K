#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS=  input speak key mouse joy

include $(BASE)/config/example.mk

input:
	make -f $(SUBMAKE) APP=input INIT= OTHEROBJS="initfile.o" SHARKOPT="__INPUT__ __LINUXC26__ __PCI__ __FB__"

speak:
	make -f $(SUBMAKE) APP=speak INIT= OTHEROBJS="initspk.o"  SHARKOPT="__INPUT__ __LINUXC26__ __PCI__"

key:	
	make -f $(SUBMAKE) APP=key   INIT= OTHEROBJS="initkey.o"  SHARKOPT="__INPUT__ __LINUXC26__ __PCI__"

mouse:
	make -f $(SUBMAKE) APP=mouse INIT= OTHEROBJS="initcur.o"  SHARKOPT="__INPUT__ __LINUXC26__ __PCI__"

joy:
	make -f $(SUBMAKE) APP=joy   INIT= OTHEROBJS="initjoy.o"  SHARKOPT="__INPUT__ __LINUXC26__ __PCI__"

