#
#
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = blk

OBJS_PATH     = $(BASE)/drivers/block

OBJS= 	bdev.o 	 \
	lodsk.o  \
	phdsk.o  \
	glue.o 	 \
	ide.o 	 \
	idereq.o \
	idelow.o

# for debug
#C_DEF += -D_PARANOIA

# for NO debug
C_DEF += -DNDEBUG

C_MAC += -imacros $(BASE)/include/fs/bdevconf.h
C_INC += -I.

#
#
#

_SCHED_FCFS=1

#

ifdef _SCHED_FCFS
C_DEF += -D_SCHED_FCFS
OBJS  += fcfs/fcfs.o
endif

ifdef _SCHED_SSTF
C_DEF += -D_SCHED_SSTF
OBJS  += sstf/sstf.o
endif

ifdef _SCHED_LOOK
C_DEF += -D_SCHED_LOOK
OBJS  += look/look.o
endif

ifdef _SCHED_CLOOK
C_DEF += -D_SCHED_CLOOK
OBJS  += clook/clook.o
endif

ifdef _SCHED_EDF
C_DEF += -D_SCHED_EDF
OBJS  += edf/edf.o
endif

ifdef _SCHED_PSCAN
C_DEF += -D_SCHED_PSCAN
OBJS  += pscan/pscan.o
endif

#
#
#

include $(BASE)/config/lib.mk

SCHEDDIRS := fcfs sstf look clock edf pscan
SCHEDOBJS := $(foreach DIR,$(SCHEDDIRS),$(wildcard $(DIR)/*.o))

clean::
	rm -f $(SCHEDOBJS)		
