#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS=  dosfs

include $(BASE)/config/example.mk

dosfs:
	make -f $(SUBMAKE) APP=dosfs INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT=

