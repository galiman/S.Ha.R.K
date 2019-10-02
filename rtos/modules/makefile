ifndef BASE
BASE=..
endif

include $(BASE)/config/config.mk

LIBRARY       = modules

OBJS_PATH     = $(BASE)/modules

include $(wildcard */subdir.mk)

OTHERINCL += -I$(BASE)/modules

include $(BASE)/config/lib.mk

