#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS  = hello

include $(BASE)/config/example.mk

hello:
	make -f $(SUBMAKE) APP=hello INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT=
