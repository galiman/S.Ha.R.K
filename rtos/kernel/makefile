# The Generic Kernel library

ifndef BASE
BASE=..
endif

include $(BASE)/config/config.mk

LIBRARY       = gkern

OBJS_PATH     = $(BASE)/kernel
INCL += -I$(BASE)/tracer/include

# Object files

OBJS =  activate.o              \
        assert.o                \
        blkact.o                \
        cancel.o                \
        conditio.o              \
        event.o                 \
        exchand.o               \
        grpcreat.o              \
        jet.o                   \
        join.o                  \
        kern.o                  \
        keys.o                  \
        kill.o                  \
        init.o                  \
        int_sem.o               \
        mem.o                   \
        mqueue.o                \
        mutex.o                 \
        nanoslp.o               \
        panic.o                 \
        printk.o                \
        perror.o                \
        pthread.o               \
        iqueue.o                \
        signal.o                \
        time.o                  \
        tpreempt.o              \
	tskmsg.o

include $(BASE)/config/lib.mk

install all clean cleanall depend::
	make -C mem $@

