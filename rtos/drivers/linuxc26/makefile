# The LINUXCOMP 2.6 directory

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = comp26

OBJS_PATH     = $(BASE)/drivers/linuxc26

OBJS  = bus.o linuxcomp.o core.o driver.o vsprintf.o interface.o kobject.o\
	shark_linuxc26.o shark_glue.o class.o videodev.o linux_kthread.o waitqueue.o\
	int.o timer.o cmdline.o video-buf.o shark_videodev.o

C_OPT += -I../linuxc26/include

C_OPT += -D__KERNEL__

include $(BASE)/config/lib.mk


