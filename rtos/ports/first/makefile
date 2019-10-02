# The Frame Buffer Device
ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = first

OBJS_PATH     = $(BASE)/ports/first

FIRST = first-contract.o first-server.o first-sync.o server-task.o first-error.o \
	./modules/grubstar.o ./modules/posixstar.o ./modules/fedfstar.o ./modules/cbsnhstar.o \
        ./modules/edfstar.o ./modules/cbsstar.o ./modules/rmstar.o \
	./modules/nonestar.o ./modules/pistar.o ./modules/tdstar.o

OBJS = $(FIRST)

C_OPT += -I./fsf_include -I./include -I. -I..

include $(BASE)/config/lib.mk
