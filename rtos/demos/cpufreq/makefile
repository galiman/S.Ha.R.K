#
#
#

ifndef BASE
BASE=../..
endif
include $(BASE)/config/config.mk

PROGS=  cpufreq

include $(BASE)/config/example.mk

cpufreq:
	make -f $(SUBMAKE) APP=cpufreq INIT= OTHEROBJS="initfile.o" SHARKOPT="__INPUT__ __CPU__ __LINUXC26__ __PCI__"

