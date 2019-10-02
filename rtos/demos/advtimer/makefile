#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS  = advtimer 

include $(BASE)/config/example.mk

# Text applications
advtimer:
	make -f $(SUBMAKE) APP=advtimer INIT= OTHEROBJS="initfile.o" SHARKOPT="__LINUXC26__ __PCI__ __CPU__ __INPUT__"


