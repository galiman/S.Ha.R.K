# The Network Drivers

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = net

OBJS_PATH     = $(BASE)/drivers/net

OBJS  = eth.o \
	8390.o \
	3c509.o \
	3c59x.o \
	eepro100.o \
	ne.o \
	rtl8139.o \
	netbuff.o \
	arp.o \
	udpip.o \
	net.o \
	net_init.o \
	skbuff.o \
	misc.o

OTHERINCL += -I./include

C_OPT += -D__KERNEL__

include $(BASE)/config/lib.mk

