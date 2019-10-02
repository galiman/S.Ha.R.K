#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS = load

include $(BASE)/config/example.mk

load:
	make -f $(SUBMAKE) APP=load INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__LINUXC26__ __INPUT__ __SERVO__"
