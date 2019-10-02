#
# GNU-C 32 bit makefile
#

HARTIK=$(BASE)

include $(HARTIK)/config/config.mak

# to remove
CP=cp
RM=rm -f

C_DEF += -D_THREAD_SAFE
#C_INC += -I.

#
# Device driver files
#

C_SRCS= anddi3.c ashldi3.c ashrdi3.c cmpdi2.c divdi3.c fixdfdi.c \
	fixsfdi.c fixunsdf.c fixunssf.c floatdid.c floatdis.c \
	floatuns.c iordi3.c lshldi3.c lshrdi3.c moddi3.c \
	muldi3.c negdi2.c notdi2.c subdi3.c adddi3.c qdivrem.c \
	ucmpdi2.c udivdi3.c umoddi3.c xordi3.c

S_SRCS=

SRCS= $(C_SRCS) $(S_SRCS)

#

LIBNAME= c

#
# Finally the dependency rules!
#

.PHONY : all install depend clean allclean


#

all install: ../lib$(LIBNAME).a

depend deps:
	$(CC) $(C_OPT) -M $(C_SRCS) >deps

clean :
	-del *.o
	-del *.err
	-del *.a

allclean : clean

#
#
#

include deps

../lib$(LIBNAME).a : $(SRCS:.c=.o)
	ar rs ../lib$(LIBNAME).a $(SRCS:.c=.o)
