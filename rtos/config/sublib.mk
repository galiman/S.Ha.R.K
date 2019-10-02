#
# Dependency rules used to make a sub-library
#

.PHONY: all install clean cleanall depend

#

install:: all
	make -C .. copylibrary

all:: $(OBJS)
	$(AR) rs ../lib$(LIBRARY).a $(OBJS)

clean::
	$(RM) *.o
	$(RM) *.err

cleanall:: clean
	$(RM) deps
	$(RM) *.bak
	$(RM) *.~
	$(RM) *.?~
	$(RM) *.??~

depend::
	$(CC) $(C_OPT) -M $(wildcard $(OBJS:.o=.c) $(OBJS:.o=.s)) > deps

deps:
	$(CC) $(C_OPT) -M $(wildcard $(OBJS:.o=.c) $(OBJS:.o=.s)) > deps

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

#../lib$(LIBRARY).a: $(OBJS)
#	$(AR) rs ../lib$(LIBRARY).a $(OBJS)




