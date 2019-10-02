#
# Rules and common variables
#

HOSTNAME=$(shell uname)

ifeq ($(HOSTNAME),Linux)
else
ifeq ($(HOSTNAME),MS-DOS)
else
all:: 
	@echo System "$(HOSTNAME)" unknow
	@error
endif
endif

#
#
#

LIBNAME      = stdio

#
INCLUDE_PATH = $(HARTIK)/h
LIB_PATH     = $(HARTIK)/lib

CC           = gcc

C_INC_TOREMOVE = -I$(HARTIK)/h -I$(HARTIK)/h/sys -I$(HARTIK)/h/x86
C_DEF_TOREMOVE = -D__GNU__

C_DEF        = -D_PARANOIA -D_GNU_SOURCE $(C_DEF_TOREMOVE) -Di386
C_WARN       = -Wimplicit-function-declaration -Wall
C_FLAGS      = -O -finline-functions -fno-builtin -nostdinc
C_INC        = -I. \
	-I../include -I../../include \
	-I$(HARTIK)/include -I$(HARTIK) $(C_INC_TOREMOVE)
#C_MAC        = -imacros $(HARTIK)/include/hartik/bdevconf.h
C_MAC        =
C_OPT        = $(C_DEF) $(C_WARN) $(C_INC) $(C_MAC) $(C_FLAGS)
C_OUTPUT     = -o $(notdir $*.o)

ASM_DEF      =
ASM_WARN     =
ASM_FLAGS    =
ASM_INC      = -I. -I$(HARTIK)/include
ASM_MAC      =
ASM_OPT      = $(ASM_DEF) $(ASM_WARN) $(ASM_INC) $(ASM_MAC) $(ASM_FLAGS)
ASM_OUTPUT   = -o $*.o


%.o : %.c
	$(REDIR) $(CC) $(C_OPT) $(C_OUTPUT) -c $<

%.o : %.S
	$(REDIR) $(CC) $(ASM_OPT) $(ASM_OUTPUT) -c $< 
