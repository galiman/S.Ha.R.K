include $(BASE)/../shark.cfg

CC  = gcc
AS  = gcc
LD  = ld
AR  = ar
INCL   = $(BASE)
LIB_PATH    = $(BASE)/lib/
LIB_DIR  = $(BASE)/lib

CFG_OPT += -D__LINUX__

ifeq ($(TSC),TRUE)
CFG_OPT += -D__TSC__
ifeq ($(APIC),TRUE)
CFG_OPT += -D__APIC__
endif
endif

ifeq ($(findstring 1000,$(TIMER_OPT)) , 1000)
CFG_OPT += -D__O1000__
else
ifeq ($(findstring 2000,$(TIMER_OPT)) , 2000)
CFG_OPT += -D__O2000__
else
ifeq ($(findstring 4000,$(TIMER_OPT)) , 4000)
CFG_OPT += -D__O4000__
else
CFG_OPT += -D__O8000__
endif
endif
endif

ifeq ($(findstring NEW,$(TRACER)) , NEW)
ifeq ($(TSC),TRUE)
CFG_OPT += -D__NEW_TRACER__
endif
endif
ifeq ($(findstring OLD,$(TRACER)) , OLD)
CFG_OPT += -D__OLD_TRACER__
endif

ifeq ($(findstring GCC4,$(COMPILER)) , GCC4)
# this options are for newer gcc4 compilers
C_OPT =  -Wall -O -fno-builtin -nostdinc -Wno-attributes -Wno-pointer-sign -minline-all-stringops $(CFG_OPT) -DMAIN=__kernel_init__ -I$(INCL)
endif

ifeq ($(findstring GCC3,$(COMPILER)) , GCC3)
C_OPT =  -Wall -O -fno-builtin -nostdinc -minline-all-stringops $(CFG_OPT) -DMAIN=__kernel_init__ -I$(INCL)
endif

ifeq ($(findstring DJGPP,$(COMPILER)) , DJGPP)
C_OPT =  -Wall -O -fno-builtin -nostdinc -minline-all-stringops $(CFG_OPT) -DMAIN=__kernel_init__ -I$(INCL)
endif

ASM_OPT =  -x assembler-with-cpp $(CFG_OPT) -I$(INCL)
LINK_OPT = -Bstatic -Ttext $(MEM_START) -s -nostartfiles -nostdlib -L$(LIB_PATH)

MKDIR   = mkdir
CP	= cp
CAT	= cat
RM	= rm -f
RMDIR	= rm -rf

# Common rules

%.o : %.s
	$(REDIR) $(CC) $(ASM_OPT) -c $<
%.o : %.c
	$(REDIR) $(CC) $(C_OPT) -c $<
%.s : %.c
	$(REDIR) $(CC) $(C_OPT) -S $<

