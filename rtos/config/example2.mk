#
#
#

.PHONY: depend

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

include $(BASE)/config/libdep.mk

ifdef INIT
INIT_FILE:=$(LIB_PATH)/$(INIT)
else
INIT_FILE:=
endif

#MYOBJS= $(addsuffix .c,$(PROGS)) $(OBJS)
MYOBJS:=$(APP).o $(INIT_FILE) $(OTHEROBJS)

$(APP):$(MYOBJS) $(LIB_DEP)
	$(LD) $(LINK_OPT) $(LINK_STARTUP) $(MYOBJS) \
	--start-group $(LINK_LIB) $(OTHERLIBS) --end-group \
	-o $(APP)

#depend:
#	$(CC) $(C_OPT) -M $(wildcard $(MYOBJS:.o=.c) $(MYOBJS:.o=.s)) > deps

deps_$(APP):
	$(CC) $(C_OPT) -M $(wildcard $(MYOBJS:.o=.c) $(MYOBJS:.o=.s)) > deps_$(APP)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
include deps_$(APP)
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








