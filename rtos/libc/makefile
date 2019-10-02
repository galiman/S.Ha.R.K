#
# The standard lib C
#

ifndef BASE
BASE=..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc

SRCS= init.c

OBJS= $(patsubst %.c,%.o,$(SRCS))

install: all $(LIB_PATH)/lib$(LIBRARY).a

clean cleanall depend::
	make -C libio $@
	make -C quad $@
	make -C stdio $@
	make -C stdlib $@
	make -C assert $@
	make -C unistd $@
	make -C utsname $@
	make -C ctype $@
	make -C string $@
	make -C getopt $@


#
# Dependency rules used to make a library
# derived directly from /config/lib.mak
#

.PHONY: all install clean cleanall depend

#

all: $(OBJS)
	make -C libio $@
	make -C quad $@
	make -C stdio $@
	make -C stdlib $@
	make -C assert $@
	make -C unistd $@
	make -C utsname $@
	make -C ctype $@
	make -C string $@
	make -C getopt $@
	$(AR) rs lib$(LIBRARY).a $(OBJS)

clean::
	$(RM) *.o
	$(RM) *.err
	$(RM) lib$(LIBRARY).a

cleanall:: clean
	$(RM) deps
	$(RM) *.bak
	$(RM) *.~
	$(RM) *.?~
	$(RM) *.??~
	$(RM) $(LIBRARYPATHNAME)

depend::
	$(CC) $(C_OPT) -M $(OBJS:.o=.c) > deps

deps:
	$(CC) $(C_OPT) -M $(OBJS:.o=.c) > deps

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
include deps
endif
endif

# Common rules
%.o : %.s
	$(REDIR) $(CC) $(ASM_OPT) $(ASM_OUTPUT) -c $<
%.o : %.c
	$(REDIR) $(CC) $(C_OPT) $(C_OUTPUT) -c $<
%.s : %.c
	$(REDIR) $(CC) $(C_OPT) $(C_OUTPUT) -S $<
%.o : %.cpp
	$(REDIR) $(CPP) $(C_OPT) $(C_OUTPUT) -c $<

$(LIB_PATH)/lib$(LIBRARY).a: lib$(LIBRARY).a copylibrary

copylibrary:
	$(CP) lib$(LIBRARY).a $(LIBRARYDIR)

