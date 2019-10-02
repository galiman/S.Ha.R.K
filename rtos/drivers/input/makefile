# Input support from linux 2.6.0

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = input

OBJS_PATH     = $(BASE)/drivers/input

OBJS = input.o serio/serio.o serio/i8042.o\
	mouse/psmouse-base.o mouse/logips2pp.o mouse/synaptics.o\
	keyboard/atkbd.o\
	misc/pcspkr.o\
	gameport/gameport.o gameport/ns558.o\
	joystick/analog.o joystick/joydump.o\
	handler/mouse.o handler/keyboard.o handler/speaker.o\
	handler/evbug.o handler/joystick.o\
	shark/shark_input.o shark/shark_mouse.o\
	shark/shark_keymap.o shark/shark_keyb.o\
	shark/mcurtxt.o shark/mcurgrx.o\
	shark/shark_spk.o shark/shark_joy.o

OTHERINCL += -I$(BASE)/drivers/linuxc26/include

C_OPT += -D__KERNEL__

include $(BASE)/config/lib.mk
