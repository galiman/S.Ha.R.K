# National Instruments PCI6025E Driver

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = 6025e

OBJS_PATH     = $(BASE)/drivers/pci6025e

OBJS = pci6025e.o ./tim/timer.o ./dac/dac.o ./dio_ppi/dio_ppi.o ./adc/adc.o

OTHERINCL += -I$(BASE)/drivers/pci6025e/include

include $(BASE)/config/lib.mk

PCI6025EDIRS := tim dio_ppi dac
PCI6025EOBJS := $(foreach DIR,$(PCI6025EDIRS),$(wildcard $(DIR)/*.o))

clean::
	rm -f $(PCI6025EOBJS)
