# The real FFTW library

ifndef BASE
BASE=../../..
endif

include $(BASE)/config/config.mk

LIBRARY       = fftr

OBJS_PATH     = $(BASE)/ports/fftw/rfftw

OTHERINCL += -I$(BASE)/ports/fftw/include

OBJS  = frc_1.o frc_2.o frc_3.o frc_4.o frc_5.o frc_6.o frc_7.o frc_8.o   \
        frc_9.o frc_10.o frc_11.o frc_12.o frc_13.o frc_14.o frc_15.o     \
        frc_16.o frc_32.o frc_64.o frc_128.o fhf_2.o fhf_3.o fhf_4.o      \
        fhf_5.o fhf_6.o fhf_7.o fhf_8.o fhf_9.o fhf_10.o fhf_16.o         \
        fhf_32.o fcr_1.o fcr_2.o fcr_3.o fcr_4.o fcr_5.o fcr_6.o          \
        fcr_7.o fcr_8.o fcr_9.o fcr_10.o fcr_11.o fcr_12.o fcr_13.o       \
        fcr_14.o fcr_15.o fcr_16.o fcr_32.o fcr_64.o fcr_128.o fhb_2.o    \
        fhb_3.o fhb_4.o fhb_5.o fhb_6.o fhb_7.o fhb_8.o fhb_9.o fhb_10.o  \
        fhb_16.o fhb_32.o rconfig.o rplanner.o rexec.o rexec2.o rfftwnd.o \
        rgeneric.o
        # rfftwf77.o

include $(BASE)/config/lib.mk
