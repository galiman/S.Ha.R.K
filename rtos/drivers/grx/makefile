# The Graphic library

ifndef BASE
BASE=../..
endif


include $(BASE)/config/config.mk

#C_OPT += -DVM86
LIBRARY       = grx

OBJS_PATH     = $(BASE)/drivers/grx

OBJS = gd.o glib.o palette.o drivers/gdvesa.o drivers/gds3.o drivers/gdtrid.o drawfun/fun8.o drawfun/fun16.o

#vpath %.c drivers drawfun .

OTHERINCL = -I./include/

include $(BASE)/config/lib.mk

GRXDIRS := drawfun drivers
GRXOBJS := $(foreach DIR,$(GRXDIRS),$(wildcard $(DIR)/*.o))

clean::
	rm -f $(GRXOBJS)		
