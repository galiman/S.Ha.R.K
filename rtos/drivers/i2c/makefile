# I2C Driver

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = i2c

OBJS_PATH     = $(BASE)/drivers/i2c

OBJS = i2c-dev.o i2c-core.o shark_i2c26.o algos/i2c-algo-bit.o

OTHERINCL += -I$(BASE)/drivers/linuxc26/include

C_OPT += -D__KERNEL__

include $(BASE)/config/lib.mk

clean::
	rm -f $(OBJS)

