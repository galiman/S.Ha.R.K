#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS  = sig time preempt
PROGS += aster aster1 aster2 aster3 aster4 aster5 aster6 aster7 aster8
PROGS += pcidemo
PROGS += jointest condtest intsem semdemo pidemo pcdemo srpdemo mdemo 
PROGS += ego fly cabs

include $(BASE)/config/example.mk

# Text applications
time:
	make -f $(SUBMAKE) APP=time INIT= OTHEROBJS="ihello.o" OTHERINCL= SHARKOPT=

sig:
	make -f $(SUBMAKE) APP=sig INIT= OTHEROBJS="ihello.o" OTHERINCL= SHARKOPT=

preempt:
	make -f $(SUBMAKE) APP=preempt INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__LINUXC26__ __INPUT__"

semdemo:
	make -f $(SUBMAKE) APP=semdemo INIT= OTHEROBJS="isemdemo.o" OTHERINCL= SHARKOPT=

pidemo:
	make -f $(SUBMAKE) APP=pidemo INIT= OTHEROBJS="imdemo.o" OTHERINCL= SHARKOPT=

pcdemo:
	make -f $(SUBMAKE) APP=pcdemo INIT= OTHEROBJS="imdemo.o" OTHERINCL= SHARKOPT=

srpdemo:
	make -f $(SUBMAKE) APP=srpdemo INIT= OTHEROBJS="imdemo.o" OTHERINCL= SHARKOPT=

mdemo:
	make -f $(SUBMAKE) APP=mdemo INIT= OTHEROBJS="imdemo.o" OTHERINCL= SHARKOPT=

aster:
	make -f $(SUBMAKE) APP=aster INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__LINUXC26__ __INPUT__"

aster1:
	make -f $(SUBMAKE) APP=aster1 INIT= OTHEROBJS="iaster1.o" OTHERINCL= SHARKOPT=

aster2:
	make -f $(SUBMAKE) APP=aster2 INIT= OTHEROBJS="iaster1.o" OTHERINCL= SHARKOPT=

aster3:
	make -f $(SUBMAKE) APP=aster3 INIT= OTHEROBJS="iaster3.o" OTHERINCL= SHARKOPT=

aster4:
	make -f $(SUBMAKE) APP=aster4 INIT= OTHEROBJS="iaster4.o" OTHERINCL= SHARKOPT="__LINUXC26__ __INPUT__"

aster5:
	make -f $(SUBMAKE) APP=aster5 INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__LINUXC26__ __INPUT__"

aster6:
	make -f $(SUBMAKE) APP=aster6 INIT= OTHEROBJS="iaster6.o" OTHERINCL= SHARKOPT="__LINUXC26__ __INPUT__"

aster7:
	make -f $(SUBMAKE) APP=aster7 INIT= OTHEROBJS="iaster7.o" OTHERINCL= SHARKOPT="__LINUXC26__ __INPUT__"

aster8:
	make -f $(SUBMAKE) APP=aster8 INIT= OTHEROBJS="iaster8.o" OTHERINCL= SHARKOPT="__LINUXC26__ __INPUT__"

pcidemo:
	make -f $(SUBMAKE) APP=pcidemo INIT= OTHEROBJS="ipcidemo.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__"

jointest:
	make -f $(SUBMAKE) APP=jointest INIT= OTHEROBJS="imdemo.o" OTHERINCL= SHARKOPT=

condtest:
	make -f $(SUBMAKE) APP=condtest INIT= OTHEROBJS="imdemo.o" OTHERINCL= SHARKOPT=

intsem:
	make -f $(SUBMAKE) APP=intsem INIT= OTHEROBJS="imdemo.o" OTHERINCL= SHARKOPT=

# Graphical applications

ego:
	make -f $(SUBMAKE) APP=ego INIT= OTHEROBJS="initgraph.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

fly:
	make -f $(SUBMAKE) APP=fly INIT= OTHEROBJS="initgraph.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

cabs:
	make -f $(SUBMAKE) APP=cabs INIT= OTHEROBJS="initgraph.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

