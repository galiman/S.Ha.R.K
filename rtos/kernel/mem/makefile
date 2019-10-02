# The List-based Memory Manager (from OSKit!)

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = mem

OBJS_PATH     = $(BASE)/kernel/mem

#
# Object files
#

OBJS =  addfree.o        \
        addregio.o       \
        alloc.o          \
        alloc_al.o       \
        alloc_ge.o       \
        alloc_pa.o       \
        avail.o          \
        avail_in.o       \
        dump.o           \
        findfree.o       \
        free.o           \
        freepage.o       \
        init.o           \
        removefr.o       \
        stats.o

include $(BASE)/config/lib.mk

