# Frame Buffer Linux 2.6 Driver

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = fb

OBJS_PATH     = $(BASE)/drivers/fb

OBJS = fbmem.o fbcmap.o cfbfillrect.o softcursor.o cfbcopyarea.o cfbimgblt.o\
	modedb.o vga16fb.o vgastate.o vesafb.o radeonfb.o shark_fb26.o gdvesa.o\
	logo.o logo_bmp.o ./riva/fbdev.o ./riva/nv_driver.o ./riva/riva_hw.o fun16.o\
	./matrox/g450_pll.o ./matrox/matroxfb_accel.o ./matrox/matroxfb_base.o\
	./matrox/matroxfb_crtc2.o ./matrox/matroxfb_DAC1064.o ./matrox/matroxfb_g450.o\
	./matrox/matroxfb_maven.o ./matrox/matroxfb_misc.o ./matrox/matroxfb_Ti3026.o	

OTHERINCL += -I$(BASE)/drivers/linuxc26/include -I./include -I.

C_OPT += -D__KERNEL__ -D__i386__ $(CFG_VIDEO_OPT)

include $(BASE)/config/lib.mk

clean::
	rm -f $(OBJS)

