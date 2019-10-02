# Snapshot Library

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = snapshot

OBJS_PATH     = $(BASE)/ports/snapshot

SNAPSHOT = snapshot.o

OBJS = $(SNAPSHOT)

C_OPT += -I./include -I. -I.. -I../../drivers/fb/include

include $(BASE)/config/lib.mk

