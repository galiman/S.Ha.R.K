# This will make one single object file from multiple sources [Lex.N]

DYNALINK=1
ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

OTHERINCL += -I$(BASE)/drivers/linuxc26/include -I./include -I.
OTHERINCL += -I$(BASE)/drivers/pci/include
OTHERINCL += -I$(BASE)/drivers/input/include

# add all sources here..
OBJS = app.o 

all: sh_app.o
	$(MV) sh_app.o sh_app.bin


clean :
	$(RM) *.o

sh_app.o: $(OBJS)
	$(LD) $(LINK_OPT) $(OBJS) -r -s -o sh_app.o

	
# Common rules
%.o : %.c
	$(REDIR) $(CC) $(C_OPT) $(C_OUTPUT) -c $<
%.s : %.c
	$(REDIR) $(CC) $(C_OPT) $(C_OUTPUT) -S $<
%.o : %.s
	$(CC) $(ASM_OPT) -c $<

