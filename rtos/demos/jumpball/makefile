#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS=  ecp ern rrp rrn

include $(BASE)/config/example.mk

ecp:
	make -f $(SUBMAKE) APP=ecp INIT= OTHEROBJS="demo.o ball.o jetctrl.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

ern:
	make -f $(SUBMAKE) APP=ern INIT= OTHEROBJS="demo.o ball.o jetctrl.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

rrp:
	make -f $(SUBMAKE) APP=rrp INIT= OTHEROBJS="demo.o ball.o jetctrl.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"

rrn:
	make -f $(SUBMAKE) APP=rrn INIT= OTHEROBJS="demo.o ball.o jetctrl.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __INPUT__ __FB__"
