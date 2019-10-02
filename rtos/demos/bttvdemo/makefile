#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS = bttv

include $(BASE)/config/example.mk

bttv:
	make -f $(SUBMAKE) APP=bttv INIT= OTHEROBJS="initfile.o" OTHERINCL=  SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__ __I2C__ __BTTV__"

