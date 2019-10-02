include $(BASE)/shark.cfg

CPP = gcc
CC  = gcc
AS  = gcc
LD  = ld
AR  = ar

INCL   = $(BASE)/include
LIB_PATH    = $(BASE)/lib
OSLIB = $(BASE)/oslib
OSLIB_PATH = $(OSLIB)/lib
LIB_DIR  = $(BASE)/lib

LIBRARYPATHNAME= $(LIB_PATH)/lib$(LIBRARY).a
LIBRARYDIR= $(LIB_PATH)
LIBRARYOBJS= $(LIB_OBJS)

CFG_OPT = -D__LINUX__

ifeq ($(TSC),TRUE)
CFG_OPT += -D__TSC__
ifeq ($(APIC),TRUE)
CFG_OPT += -D__APIC__
endif
endif

ifeq ($(findstring VM86,$(BIOS)) , VM86)
CFG_OPT += -DVM86
endif

ifeq ($(findstring FORCE_PXC,$(FG)) , FORCE_PXC)
CFG_OPT += -D__FORCE_PXC__
endif

ifeq ($(findstring NEW,$(TRACER)) , NEW)
ifeq ($(TSC),TRUE)
CFG_OPT += -D__NEW_TRACER__
endif
endif
ifeq ($(findstring OLD,$(TRACER)) , OLD)
CFG_OPT += -D__OLD_TRACER__
endif
INCL += -I$(BASE)/tracer/include

ifeq ($(findstring VESA,$(FB)) , VESA)
CFG_VIDEO_OPT += -DCONFIG_FB_VESA -DCONFIG_LOGO
endif
ifeq ($(findstring FINDPCI,$(FB)) , FINDPCI)
CFG_VIDEO_OPT += -DCONFIG_FB_RIVA -DCONFIG_FB_RADEON -DCONFIG_FB_MATROX\
		-DCONFIG_LOGO -DCONFIG_FB_MATROX_G100 -DCONFIG_FB_MATROX_MILLENIUM\
		-DCONFIG_FB_MATROX_MYSTIQUE
endif
ifeq ($(findstring VGA16,$(FB)) , VGA16)
CFG_VIDEO_OPT += -DCONFIG_FB_VGA16
endif

C_WARN       = -Wimplicit-function-declaration -Wall
C_FLAGS      = -O -fno-builtin -nostdinc
C_INC        = -I$(INCL) $(OTHERINCL) -I$(OSLIB)
C_MAC        = $(CFG_OPT)

ASM_WARN     =
ASM_FLAGS    = -x assembler-with-cpp
ASM_INC      = -I$(INCL) $(OTHERINCL) -I$(OSLIB)
ASM_MAC      = $(CFG_OPT)

LINK_OPT = -Bstatic -Ttext $(MEM_START) -s -nostartfiles -nostdlib -L$(LIB_PATH) -L$(OSLIB_PATH)

C_OPT        = $(C_DEF) $(C_WARN) $(C_INC) $(C_MAC) $(C_FLAGS)
C_OUTPUT     = -o $*.o

ASM_OPT      = $(ASM_DEF) $(ASM_WARN) $(ASM_INC) $(ASM_MAC) $(ASM_FLAGS)
ASM_OUTPUT   = -o $*.o

MKDIR   = mkdir 
CP	= cp
CAT	= cat
RM	= rm -f
CD	= cd

# Common rules

%.o : %.s
	$(REDIR) $(CC) $(ASM_OPT) $(ASM_OUTPUT) -c $<
%.o : %.c
	$(REDIR) $(CC) $(C_OPT) $(C_OUTPUT) -c $<
%.s : %.c
	$(REDIR) $(CC) $(C_OPT) $(C_OUTPUT) -S $<
%.o : %.cpp
	$(REDIR) $(CPP) $(C_OPT) $(C_OUTPUT) -c $<
