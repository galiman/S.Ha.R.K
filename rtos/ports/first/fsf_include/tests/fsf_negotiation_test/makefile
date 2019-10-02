#
#
ifeq ($(OS), S)
BASE=../../../../..
OSNAME = shark_glue
export BASE

include $(BASE)/config/config.mk
endif

PROGS=  $(OS)_$(WL)_negotiation_test 

ifeq ($(OS), S)
include $(BASE)/config/example.mk
endif

$(OS)_$(WL)_negotiation_test:
	make -f $(SUBMAKE) APPNAME=$(PROGS) APP=fsf_negotiation_test INIT= OTHEROBJS="../$(OSNAME)/initfile.o ../$(OSNAME)/shark.o ../$(OSNAME)/net_print.o" OTHERINCL="-I../common" SHARKOPT="__INPUT__ __FIRST__ __NEW_TRACER__  __LINUXC26__ __PCI__ __NET__  __GRX__"

ifeq ($(OS), M)

HERE=$(shell /bin/pwd)
export HERE

$(PROGS):
	cd /home/marte/fsf ; make $(PROGS)

endif

