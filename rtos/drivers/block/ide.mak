#
#
#

APP= idetest

#
# Standard path
#

APP_PATH     = .
LIB_PATH     = $(HARTIK)/LIB

#
# Standard options
#

LINK_OPT     = -oformat coff-go32 -s -nostartfiles -nostdlib -L$(LIB_PATH)
LINK_STARTUP = $(LIB_PATH)/x0.o $(LIB_PATH)/k_standa.o
LIB_MATH     = -lhm
LD           = ld

#
# Libraries
#

LIB_VM       = -lvm
LIB_OS       = -lhart
LIB_DEV      = -lhdev
LIB_STD      = -lhlib
LIB_PCI      = -lhpci
LIB_COMP     = -lcomp
LIB_NET      = -lhnet
LIB_SND      = -lhsnd
LIB_GRAPH    = -lhgd
LIB_BLK      = -lhblk
LIB_FSUT     = -lhfsut

LINK_LIB = -(           \
	   $(LIB_OS)    \
	   $(LIB_DEV)   \
	   $(LIB_GL)    \
	   $(LIB_GRAPH) \
	   $(LIB_NET)   \
	   $(LIB_COMP)  \
	   $(LIB_PCI)   \
	   $(LIB_SND)   \
	   $(LIB_VM)    \
	   $(LIB_STD)   \
	   $(LIB_MATH)  \
	   $(LIB_BLK)   \
	   $(LIB_FSUT)  \
	   -)

#
#
#

$(APP) : $(APP).o install
	$(LD) $(LINK_OPT) $(LINK_STARTUP) $(APP).o $(LINK_LIB) -o $(APP)

