# Servo Control

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = servo

OBJS_PATH     = $(BASE)/ports/servo

SERVO = servo.o

OBJS = $(SERVO)

C_OPT += -I./include -I. -I$(BASE)/drivers/serial/include

include $(BASE)/config/lib.mk

