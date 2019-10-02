# The Frame Buffer Device

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = zlib

OBJS_PATH     = $(BASE)/ports/zlib

ZLIB = adler32.o compress.o crc32.o deflate.o gzio.o\
	infblock.o infcodes.o inffast.o inflate.o inftrees.o\
	infutil.o trees.o uncompr.o zutil.o  

OBJS = $(ZLIB)

C_OPT += -I../../../drivers/linuxc26/include -I./include -I. -I..

include $(BASE)/config/lib.mk

