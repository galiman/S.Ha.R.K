#
#
# PCI6025E Examples and Test programs
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS=  test_dac wave test_dio test_ppi test_ec test_bec test_mes test_bms

include $(BASE)/config/example.mk

test_dac:
	make -f $(SUBMAKE) APP=test_dac OTHEROBJS=initfile.o SHARKOPT="__LINUXC26__ __PCI__ __6025E__ __INPUT__ __FB__"
wave:
	make -f $(SUBMAKE) APP=wave OTHEROBJS=initfile.o SHARKOPT="__LINUXC26__ __PCI__ __6025E__ __INPUT__ __FB__"
test_dio:
	make -f $(SUBMAKE) APP=test_dio OTHEROBJS=initfile.o SHARKOPT="__LINUXC26__ __PCI__ __6025E__ __INPUT__ __FB__"
test_ppi:
	make -f $(SUBMAKE) APP=test_ppi OTHEROBJS=initfile.o SHARKOPT="__LINUXC26__ __PCI__ __6025E__ __INPUT__ __FB__"
test_ec:
	make -f $(SUBMAKE) APP=test_ec OTHEROBJS=initfile.o SHARKOPT="__LINUXC26__ __PCI__ __6025E__ __INPUT__ __FB__"
test_bec:
	make -f $(SUBMAKE) APP=test_bec OTHEROBJS=initfile.o SHARKOPT="__LINUXC26__ __PCI__ __6025E__ __INPUT__ __FB__"
test_mes:
	make -f $(SUBMAKE) APP=test_mes OTHEROBJS=initfile.o SHARKOPT="__LINUXC26__ __PCI__ __6025E__ __INPUT__ __FB__"
test_bms:
	make -f $(SUBMAKE) APP=test_bms OTHEROBJS=initfile.o SHARKOPT="__LINUXC26__ __PCI__ __6025E__ __INPUT__ __FB__"

