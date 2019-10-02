#
# Dependency rules used to make some objects
#

.PHONY: all install clean cleanall depend

#

LIB_OBJS=$(addprefix $(LIB_PATH)/,$(OBJS))

#

install:: $(LIB_OBJS)

all:: $(OBJS)

# c'e' "cp" e non "copy" poiche' la linea di comando per copy
# e' troppo lunga!
# lo stesso per cleanall (vedi)!

$(LIB_OBJS): $(OBJS)
	cp $(OBJS) $(LIB_PATH)

clean::
	$(RM) *.o
	$(RM) *.err

cleanall:: clean
	$(RM) deps
	$(RM) *.bak
	$(RM) *.~
	$(RM) *.?~
	$(RM) *.??~
	-rm -f $(LIB_OBJS)

depend deps:
	$(CC) $(C_OPT) -M $(wildcard $(OBJS:.o=.c) $(OBJS:.o=.s)) > deps

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
include deps
endif
endif
