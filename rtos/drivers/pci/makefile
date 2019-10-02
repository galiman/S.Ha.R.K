# PCI support from linux 2.6.0

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = pci

OBJS_PATH     = $(BASE)/drivers/pci

OBJS = access.o bus.o names.o pci.o pci-driver.o\
	pool.o probe.o quirks.o remove.o search.o setup-res.o\
	setup-irq.o setup-bus.o syscall.o i386.o common.o\
	fixup.o irq.o legacy.o pcbios.o direct.o shark_pci26.o \
	pci20to26.o

OTHERINCL += -I$(BASE)/drivers/linuxc26/include

C_OPT += -D__KERNEL__

include $(BASE)/config/lib.mk

clean::
	rm -f $(OBJS)

