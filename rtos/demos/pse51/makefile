#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS+= ptest1 ptest2 ptest3 ptest4 ptest5 ptest6

include $(BASE)/config/example.mk

ptest1:
	make -f $(SUBMAKE) APP=ptest1 INIT= OTHEROBJS="pinit.o" SHARKOPT=
ptest2:
	make -f $(SUBMAKE) APP=ptest2 INIT= OTHEROBJS="pinit.o" SHARKOPT=
ptest3:
	make -f $(SUBMAKE) APP=ptest3 INIT= OTHEROBJS="pinit.o" SHARKOPT=
ptest4:
	make -f $(SUBMAKE) APP=ptest4 INIT= OTHEROBJS="pinit.o" SHARKOPT=
ptest5:
	make -f $(SUBMAKE) APP=ptest5 INIT= OTHEROBJS="pinit.o" SHARKOPT=
ptest6:
	make -f $(SUBMAKE) APP=ptest6 INIT= OTHEROBJS="pinit.o" SHARKOPT=

