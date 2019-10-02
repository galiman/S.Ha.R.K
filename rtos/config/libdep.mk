#
# this file contains the library dependencies
#

OSLIB_LIB=$(OSLIB)/lib

LINK_STARTUP = $(OSLIB_LIB)/x0.o

LINK_LIB =
LIB_DEP =


# Dependencies
# -------------------------------------------------------

# these are the libraries the use wants to use
USELIB = $(SHARKOPT)

ifeq ($(findstring __PNG__,$(SHARKOPT)) , __PNG__)
USELIB += __ZLIB__
endif

INCL += -I$(BASE)/drivers/linuxc26/include
                                                                                                                             
# PCI
# ----------------------------------------------------------------
ifeq ($(findstring __PCI__,$(USELIB)) , __PCI__)
    
INCL += -I$(BASE)/drivers/pci/include
                                                                                                                             
ifeq ($(LIB_PATH)/libpci.a,$(wildcard $(LIB_PATH)/libpci.a))
LINK_LIB += -lpci
LIB_DEP += $(LIB_PATH)/libpci.a
endif

endif

# NETWORK
# ----------------------------------------------------------------
ifeq ($(findstring __NET__,$(USELIB)) , __NET__)

INCL += -I$(BASE)/drivers/net/include
                                                                                                                             
ifeq ($(LIB_PATH)/libnet.a,$(wildcard $(LIB_PATH)/libnet.a))
LINK_LIB += -lnet
LIB_DEP += $(LIB_PATH)/libnet.a
endif

endif

# tracer
ifeq ($(LIB_PATH)/libtracer.a,$(wildcard $(LIB_PATH)/libtracer.a))
LINK_LIB += -ltracer
LIB_DEP += $(LIB_PATH)/libtracer.a
endif
                                                                                                                             
# hgd
ifeq ($(LIB_PATH)/libhgd.a,$(wildcard $(LIB_PATH)/libhgd.a))
LINK_LIB += -lhgd
LIB_DEP += $(LIB_PATH)/libhgd.a
endif

# grx
# ----------------------------------------------------------------
ifeq ($(findstring __GRX__,$(USELIB)) , __GRX__)

INCL += -I$(BASE)/drivers/grx/include

ifeq ($(LIB_PATH)/libgrx.a,$(wildcard $(LIB_PATH)/libgrx.a))
LINK_LIB += -lgrx
LIB_DEP += $(LIB_PATH)/libgrx.a
endif

endif

# Frame Buffer
# ----------------------------------------------------------------
ifeq ($(findstring __FB__,$(USELIB)) , __FB__)
                                                                                                                             
INCL += -I$(BASE)/drivers/fb/include
                                                                                                                             
ifeq ($(LIB_PATH)/libfb.a,$(wildcard $(LIB_PATH)/libfb.a))
LINK_LIB += -lfb
LIB_DEP += $(LIB_PATH)/libfb.a
endif
                                                                                                                             
endif

# oldchar
# ----------------------------------------------------------------
ifeq ($(findstring __OLDCHAR__,$(USELIB)) , __OLDCHAR__)

INCL += -I$(BASE)/drivers/oldchar/include

ifeq ($(LIB_PATH)/liboldch.a,$(wildcard $(LIB_PATH)/liboldch.a))
LINK_LIB += -loldch
LIB_DEP += $(LIB_PATH)/liboldch.a
endif

endif

# hsnd
ifeq ($(LIB_PATH)/libhsnd.a,$(wildcard $(LIB_PATH)/libhsnd.a))
LINK_LIB += -lhsnd
LIB_DEP += $(LIB_PATH)/libhsnd.a
endif

# mem
ifeq ($(LIB_PATH)/libmem.a,$(wildcard $(LIB_PATH)/libmem.a))
LINK_LIB += -lmem
LIB_DEP += $(LIB_PATH)/libmem.a
endif

# gkern
ifeq ($(LIB_PATH)/libgkern.a,$(wildcard $(LIB_PATH)/libgkern.a))
LINK_LIB += -lgkern
LIB_DEP += $(LIB_PATH)/libgkern.a
endif

# mod
ifeq ($(LIB_PATH)/libmod.a,$(wildcard $(LIB_PATH)/libmod.a))
LINK_LIB += -lmod
LIB_DEP += $(LIB_PATH)/libmod.a
endif

# kl
ifeq ($(OSLIB_LIB)/libkl.a,$(wildcard $(OSLIB_LIB)/libkl.a))
LINK_LIB += -lkl
LIB_DEP += $(OSLIB_LIB)/libkl.a
endif

# hx
ifeq ($(OSLIB_LIB)/libhx.a,$(wildcard $(OSLIB_LIB)/libhx.a))
LINK_LIB += -lhx
LIB_DEP += $(OSLIB_LIB)/libhx.a
endif

# hc
ifeq ($(OSLIB_LIB)/libhc.a,$(wildcard $(OSLIB_LIB)/libhc.a))
LINK_LIB += -lhc
LIB_DEP += $(OSLIB_LIB)/libhc.a
endif

# hm
ifeq ($(OSLIB_LIB)/libhm.a,$(wildcard $(OSLIB_LIB)/libhm.a))
LINK_LIB += -lhm
LIB_DEP += $(OSLIB_LIB)/libhm.a
endif

# cons
ifeq ($(LIB_PATH)/libcons.a,$(wildcard $(LIB_PATH)/libcons.a))

INCL += -I$(BASE)/drivers/cons/include

LINK_LIB += -lcons
LIB_DEP += $(LIB_PATH)/libcons.a
endif

# blk
ifeq ($(LIB_PATH)/libblk.a,$(wildcard $(LIB_PATH)/libblk.a))
LINK_LIB += -lblk
LIB_DEP += $(LIB_PATH)/libblk.a
endif

# fs
ifeq ($(findstring YES,$(SHARK_FS)) , YES)
ifeq ($(LIB_PATH)/libfs.a,$(wildcard $(LIB_PATH)/libfs.a))
LINK_LIB += -lfs
LIB_DEP += $(LIB_PATH)/libfs.a
endif
endif

# c
ifeq ($(LIB_PATH)/libc.a,$(wildcard $(LIB_PATH)/libc.a))
LINK_LIB += -lc
LIB_DEP += $(LIB_PATH)/libc.a
endif

# serial
ifeq ($(LIB_PATH)/libserial.a,$(wildcard $(LIB_PATH)/libserial.a))
INCL += -I$(BASE)/drivers/serial/include
LINK_LIB += -lserial
LIB_DEP += $(LIB_PATH)/libserial.a
endif

# Linux Comp 2.6
# ----------------------------------------------------------------
ifeq ($(findstring __LINUXC26__,$(USELIB)) , __LINUXC26__)

# Linux Emulation Layer 2.6
ifeq ($(LIB_PATH)/libcomp26.a,$(wildcard $(LIB_PATH)/libcomp26.a))
LINK_LIB += -lcomp26
LIB_DEP += $(LIB_PATH)/libcomp26.a
endif

endif
# ----------------------------------------------------------------

# mpeg
ifeq ($(LIB_PATH)/libmpeg.a,$(wildcard $(LIB_PATH)/libmpeg.a))
LINK_LIB += -lmpeg
LIB_DEP += $(LIB_PATH)/libmpeg.a
endif

# mp3
ifeq ($(LIB_PATH)/libmp3.a,$(wildcard $(LIB_PATH)/libmp3.a))
LINK_LIB += -lmp3
LIB_DEP += $(LIB_PATH)/libmp3.a
endif

# snapshot
ifeq ($(findstring __SNAPSHOT__,$(USELIB)) , __SNAPSHOT__)

INCL += -I$(BASE)/ports/snapshot/include

ifeq ($(LIB_PATH)/libsnapshot.a,$(wildcard $(LIB_PATH)/libsnapshot.a))
LINK_LIB += -lsnapshot
LIB_DEP += $(LIB_PATH)/libsnapshot.a
endif

endif

# 6025e
# ----------------------------------------------------------------
ifeq ($(findstring __6025E__,$(USELIB)) , __6025E__)

INCL += -I$(BASE)/drivers/pci6025e/include

ifeq ($(LIB_PATH)/lib6025e.a,$(wildcard $(LIB_PATH)/lib6025e.a))
LINK_LIB += -l6025e
LIB_DEP += $(LIB_PATH)/lib6025e.a
endif

endif

# PCL833
# ----------------------------------------------------------------
ifeq ($(findstring __PCL833__,$(USELIB)) , __PCL833__)
                                                                                                                             
INCL += -I$(BASE)/drivers/pcl833/include
                                                                                                                             
ifeq ($(LIB_PATH)/libpcl833.a,$(wildcard $(LIB_PATH)/libpcl833.a))
LINK_LIB += -lpcl833
LIB_DEP += $(LIB_PATH)/libpcl833.a
endif
                                                                                                                             
endif

# Parport
# ----------------------------------------------------------------
ifeq ($(findstring __PPORT__,$(USELIB)) , __PPORT__)

INCL += -I$(BASE)/drivers/parport/include

ifeq ($(LIB_PATH)/libpport.a,$(wildcard $(LIB_PATH)/libpport.a))
LINK_LIB += -lpport
LIB_DEP += $(LIB_PATH)/libpport.a
endif

endif

# PCLAB
# ----------------------------------------------------------------
ifeq ($(findstring __PCLAB__,$(USELIB)) , __PCLAB__)

INCL += -I$(BASE)/drivers/pcl812/include

ifeq ($(LIB_PATH)/libpclab.a,$(wildcard $(LIB_PATH)/libpclab.a))
LINK_LIB += -lpclab
LIB_DEP += $(LIB_PATH)/libpclab.a
endif

endif


# Ports

# FFT
# ----------------------------------------------------------------
ifeq ($(findstring __FFT__,$(USELIB)) , __FFT__)

INCL += -I$(BASE)/ports/fftw/include

#FFTR
ifeq ($(LIB_PATH)/libfftr.a,$(wildcard $(LIB_PATH)/libfftr.a))
LINK_LIB += -lfftr
LIB_DEP += $(LIB_PATH)/libfftr.a
endif
# FFTC
ifeq ($(LIB_PATH)/libfftc.a,$(wildcard $(LIB_PATH)/libfftc.a))
LINK_LIB += -lfftc
LIB_DEP += $(LIB_PATH)/libfftc.a
endif
endif

# OSMESA
# ----------------------------------------------------------------
ifeq ($(findstring __OSMESA__,$(USELIB)) , __OSMESA__)

INCL += -I$(BASE)/ports/mesa/include

ifeq ($(LIB_PATH)/libosmesa.a,$(wildcard $(LIB_PATH)/libosmesa.a))
LINK_LIB += -losmesa
LIB_DEP += $(LIB_PATH)/libosmesa.a
endif

# GLUT
ifeq ($(LIB_PATH)/libglut.a,$(wildcard $(LIB_PATH)/libglut.a))
LINK_LIB += -lglut
LIB_DEP += $(LIB_PATH)/libglut.a
endif

# GLU
ifeq ($(LIB_PATH)/libglu.a,$(wildcard $(LIB_PATH)/libglu.a))
LINK_LIB += -lglu
LIB_DEP += $(LIB_PATH)/libglu.a
endif

endif

# PNG
# ----------------------------------------------------------------
ifeq ($(findstring __PNG__,$(USELIB)) , __PNG__)

INCL += -I$(BASE)/ports/png/include

# PNG
ifeq ($(LIB_PATH)/libpng.a,$(wildcard $(LIB_PATH)/libpng.a))
LINK_LIB += -lpng
LIB_DEP += $(LIB_PATH)/libpng.a
endif

endif

# ZLIB
# ----------------------------------------------------------------
ifeq ($(findstring __ZLIB__,$(USELIB)) , __ZLIB__)

INCL += -I$(BASE)/ports/zlib/include

# PNG
ifeq ($(LIB_PATH)/libzlib.a,$(wildcard $(LIB_PATH)/libzlib.a))
LINK_LIB += -lzlib
LIB_DEP += $(LIB_PATH)/libzlib.a
endif

endif

# DIDMA
# ----------------------------------------------------------------
ifeq ($(findstring __DIDMA__, $(USELIB)), __DIDMA__)

INCL += -I$(BASE)/ports/didma/include

# DIDMA
ifeq ($(LIB_PATH)/libdidma.a,$(wildcard $(LIB_PATH)/libdidma.a))
LINK_LIB += -ldidma
LINK_DEP += $(LIB_PATH)/libdidma.a
endif

endif

# FIRST
# ----------------------------------------------------------------
ifeq ($(findstring __FIRST__, $(USELIB)), __FIRST__)

INCL += -I$(BASE)/ports/first/include -I$(BASE)/ports/first/fsf_include

ifeq ($(LIB_PATH)/libfirst.a,$(wildcard $(LIB_PATH)/libfirst.a))
LINK_LIB += -lfirst
LINK_DEP += $(LIB_PATH)/libfirst.a
endif

endif

# BTTV
# ----------------------------------------------------------------
ifeq ($(findstring __BTTV__, $(USELIB)), __BTTV__)

INCL += -I$(BASE)/drivers/bttv/include

# BTTV
ifeq ($(LIB_PATH)/libbttv.a,$(wildcard $(LIB_PATH)/libbttv.a))
LINK_LIB += -lbttv
LINK_DEP += $(LIB_PATH)/libbttv.a
endif

endif

# I2C
# ----------------------------------------------------------------
ifeq ($(findstring __I2C__, $(USELIB)), __I2C__)
                                                                                                                             
INCL += -I$(BASE)/drivers/i2c/include
                                                                                                                             
ifeq ($(LIB_PATH)/libi2c.a,$(wildcard $(LIB_PATH)/libi2c.a))
LINK_LIB += -li2c
LINK_DEP += $(LIB_PATH)/libi2c.a
endif
                                                                                                                             
endif

# CM7326
# ----------------------------------------------------------------
ifeq ($(findstring __CM7326__, $(USELIB)), __CM7326__)
                                                                                                                             
INCL += -I$(BASE)/drivers/cm7326/include
                                                                                                                             
ifeq ($(LIB_PATH)/libcm7326.a,$(wildcard $(LIB_PATH)/libcm7326.a))
LINK_LIB += -lcm7326
LINK_DEP += $(LIB_PATH)/libcm7326.a
endif
                                                                                                                             
endif

# TFTP
# ----------------------------------------------------------------
ifeq ($(findstring __TFTP__, $(USELIB)), __TFTP__)

INCL += -I$(BASE)/ports/tftp/include

# TFTP
ifeq ($(LIB_PATH)/libtftp.a,$(wildcard $(LIB_PATH)/libtftp.a))
LINK_LIB += -ltftp
LINK_DEP += $(LIB_PATH)/libtftp.a
endif

endif

# SERVO
# ----------------------------------------------------------------
ifeq ($(findstring __SERVO__, $(USELIB)), __SERVO__)

INCL += -I$(BASE)/ports/servo/include

# SERVO
ifeq ($(LIB_PATH)/libservo.a,$(wildcard $(LIB_PATH)/libservo.a))
LINK_LIB += -lservo
LINK_DEP += $(LIB_PATH)/libservo.a
endif

endif

# INPUT
# ----------------------------------------------------------------
ifeq ($(findstring __INPUT__, $(USELIB)), __INPUT__)

INCL += -I$(BASE)/drivers/input/include

# JOY
ifeq ($(LIB_PATH)/libinput.a,$(wildcard $(LIB_PATH)/libinput.a))
LINK_LIB += -linput
LINK_DEP += $(LIB_PATH)/libinput.a
endif

endif

# CPU
# ----------------------------------------------------------------
ifeq ($(findstring __CPU__, $(USELIB)), __CPU__)

INCL += -I$(BASE)/drivers/cpu/include

# CPU
ifeq ($(LIB_PATH)/libcpu.a,$(wildcard $(LIB_PATH)/libcpu.a))
LINK_LIB += -lcpu
LINK_DEP += $(LIB_PATH)/libcpu.a
endif

endif

# USB
# ----------------------------------------------------------------
ifeq ($(findstring __USB__, $(USELIB)), __USB__)
                                                                                                                             
INCL += -I$(BASE)/drivers/usb/include
# CPU
ifeq ($(LIB_PATH)/libusb.a,$(wildcard $(LIB_PATH)/libusb.a))
LINK_LIB += -lusb
LINK_DEP += $(LIB_PATH)/libusb.a

endif

endif


# Scheduling Modules
# ----------------------------------------------------------------

#INCL += -I$(BASE)/modules into config.mk!!!

ifeq ($(LIB_PATH)/libmodules.a,$(wildcard $(LIB_PATH)/libmodules.a))
LINK_LIB += -lmodules
LINK_DEP += $(LIB_PATH)/libmodules.a
endif

# DYNALINK (added by Lex N. 14:08 9-7-06)
# ----------------------------------------------------------------
ifeq ($(findstring __DYNALINK__, $(USELIB)), __DYNALINK__)

INCL += -I$(BASE)/ports/dynalink

# DYNALINK
ifeq ($(LIB_PATH)/libdynalink.a,$(wildcard $(LIB_PATH)/libdynalink.a))
LINK_LIB += -ldynalink
LINK_DEP += $(LIB_PATH)/libdynalink.a
endif

endif
