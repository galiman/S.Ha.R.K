#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS=  asteroid

include $(BASE)/config/example.mk

asteroid:
	make -f $(SUBMAKE) APP=asteroid INIT= OTHEROBJS="initfile.o astro.o stat.o rock.o" SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

