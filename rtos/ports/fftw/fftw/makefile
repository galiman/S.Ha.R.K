# The complex FFTW library

ifndef BASE
BASE=../../..
endif

include $(BASE)/config/config.mk

LIBRARY       = fftc

OBJS_PATH     = $(BASE)/ports/fftw/fftw

OTHERINCL += -I$(BASE)/ports/fftw/include

OBJS  = fn_1.o fn_2.o fn_3.o fn_4.o fn_5.o fn_6.o fn_7.o fn_8.o fn_9.o     \
        fn_10.o fn_11.o fn_12.o fn_13.o fn_14.o fn_15.o fn_16.o fn_32.o    \
        fn_64.o ftw_2.o ftw_3.o ftw_4.o ftw_5.o ftw_6.o ftw_7.o ftw_8.o    \
        ftw_9.o ftw_10.o ftw_16.o ftw_32.o ftw_64.o fni_1.o fni_2.o        \
        fni_3.o fni_4.o fni_5.o fni_6.o fni_7.o fni_8.o fni_9.o fni_10.o   \
        fni_11.o fni_12.o fni_13.o fni_14.o fni_15.o fni_16.o fni_32.o     \
        fni_64.o ftwi_2.o ftwi_3.o ftwi_4.o ftwi_5.o ftwi_6.o ftwi_7.o     \
        ftwi_8.o ftwi_9.o ftwi_10.o ftwi_16.o ftwi_32.o ftwi_64.o          \
        config.o planner.o twiddle.o executor.o generic.o fftwnd.o malloc.o \
        rader.o putils.o        \
        wisdom.o wisdomio.o
        #fftwf77.o timer.o

include $(BASE)/config/lib.mk

