# PCI support from linux 2.6.0
#_I386_SEMAPHORE_H 

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = usb

OBJS_PATH     = $(BASE)/demos/usb

OBJS = core/usb.o core/usb-debug.o core/hub.o core/hcd.o core/urb.o core/message.o \
	core/config.o core/file.o core/buffer.o core/driverfs.o core/hcd-pci.o\
        host/ohci-hcd.o host/ehci-hcd.o host/uhci-hcd.o \
	shark_glue/shark_usb.o\
	input/usbmouse.o input/usbkbd.o \
	media/pwc-if.o media/pwc-ctrl.o media/pwc-misc.o media/pwc-uncompress.o \
	input/hid-core.o input/hid-input.o serial/bus.o serial/generic.o \
	serial/usb-serial.o serial/pl2303.o serial/tty_io.o

OTHERINCL += -I$(BASE)/drivers/linuxc26/include -I./core -I./host

#
# add	-DCONFIG_USB_DEBUG to enable USB core debug messages
# 	-DCONFIG_USB_SERIAL_DEBUG to enable USB/serial converters debug messages
#
C_OPT += -D__KERNEL__  -DCONFIG_USB_HIDINPUT -DCONFIG_USB_SERIAL_PL2303 -DCONFIG_USB_DEBUG

include $(BASE)/config/lib.mk

clean::
	rm -f $(OBJS)
