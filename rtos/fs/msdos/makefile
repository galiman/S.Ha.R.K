#
# The MSDOS (FAT16) sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = fs

OBJS_PATH     = $(BASE)/fs/msdos

OBJS= 	msdos_i.o \
	msdos_f.o \
	msdos_s.o \
	msdos_d.o

#C_DEF += -D_PARANOIA
C_DEF += -D_PARANOIA -DNDEBUG

C_MAC += -imacros $(BASE)/include/fs/fsconf.h
C_INC += -I..

include $(BASE)/config/sublib.mk
