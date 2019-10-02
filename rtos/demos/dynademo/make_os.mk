#

ifndef BASE
BASE=../..
endif

PROGS  = os

include $(BASE)/config/config.mk
include $(BASE)/config/example.mk
	
$(PROGS):
	make -f $(SUBMAKE)	APP=$(PROGS)  \
						INIT=		\
						OTHEROBJS=	\
						OTHERINCL= 		\
						SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __DYNALINK__"
						

