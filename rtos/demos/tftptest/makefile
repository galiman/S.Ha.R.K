#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS = tftptest

include $(BASE)/config/example.mk

tftptest:
	make -f $(SUBMAKE) APP=tftptest INIT= OTHEROBJS="initfile.o" OTHERINCL= SHARKOPT="__LINUXC26__ __PCI__ __NET__ __INPUT__ __TFTP__"

