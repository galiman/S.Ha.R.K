# The Frame Buffer Device

ifndef BASE
BASE=../../..
endif

include $(BASE)/config/config.mk

LIBRARY       = glu

OBJS_PATH     = $(BASE)/ports/mesa/src-glu/

GLU = glu.o mipmap.o nurbs.o nurbsutl.o project.o tess.o\
	nurbscrv.o nurbssrf.o polytest.o quadric.o tesselat.o 

OBJS = $(GLU)

C_OPT += -I../../../drivers/linuxc26/include -I../include -I. -I..

include $(BASE)/config/lib.mk

