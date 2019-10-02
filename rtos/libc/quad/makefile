#
# The quad (64bits integer aritmetics) sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc/quad

SRCS=	adddi3.c	\
	anddi3.c	\
	ashldi3.c	\
	ashrdi3.c	\
	cmpdi2.c	\
	divdi3.c	\
	fixdfdi.c	\
	fixsfdi.c	\
	fixunsdf.c	\
	fixunssf.c	\
	floatdid.c	\
	floatdis.c	\
	floatuns.c	\
	iordi3.c	\
	lshldi3.c	\
	lshrdi3.c	\
	moddi3.c	\
	muldi3.c	\
	negdi2.c	\
	notdi2.c	\
	qdivrem.c	\
	subdi3.c	\
	ucmpdi2.c	\
	udivdi3.c	\
	umoddi3.c	\
	xordi3.c

OBJS=	$(patsubst %.c,%.o,$(SRCS))

C_DEF += -D_THREAD_SAFE

include $(BASE)/config/sublib.mk

