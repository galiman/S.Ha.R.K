#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS = mesademo

include $(BASE)/config/example.mk

mesademo:
	make -f $(SUBMAKE) APP=mesademo INIT= OTHEROBJS="initfile.o" SHARKOPT="__OSMESA__ __LINUXC26__ __PCI__ __INPUT__ __FB__"

