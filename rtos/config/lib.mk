#
# Dependency rules used to make a library
#

.PHONY: all install clean cleanall depend

#

install:: $(LIB_PATH)/lib$(LIBRARY).a all

all:: lib$(LIBRARY).a

clean::
	$(RM) $(OBJS) *.o
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
	$(CC) $(C_OPT) -M $(wildcard $(OBJS:.o=.c) $(OBJS:.o=.cpp) $(OBJS:.o=.s)) > deps

deps:
	$(CC) $(C_OPT) -M $(wildcard $(OBJS:.o=.c) $(OBJS:.o=.cpp) $(OBJS:.o=.s)) > deps

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

lib$(LIBRARY).a: $(OBJS)
	$(AR) rs lib$(LIBRARY).a $(OBJS)

$(LIB_PATH)/lib$(LIBRARY).a: lib$(LIBRARY).a
	$(CP) lib$(LIBRARY).a $(LIBRARYDIR)

