# The Frame Buffer Device

ifndef BASE
BASE=../../..
endif

include $(BASE)/config/config.mk

LIBRARY       = glut

OBJS_PATH     = $(BASE)/ports/mesa/src-glut.dos/

GLUT = bitmap.o color.o globals.o models.o teapot.o\
	 window.o callback.o init.o menu.o overlay.o state.o

OBJS = $(GLUT)

C_OPT += -I../../../drivers/linuxc26/include -I../include -I. -I..

include $(BASE)/config/lib.mk

