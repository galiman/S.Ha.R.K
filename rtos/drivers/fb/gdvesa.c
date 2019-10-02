#include <ll/i386/hw-data.h>
#include <ll/i386/mem.h>
#include <ll/i386/string.h>
#include <ll/i386/x-dos.h>
#include <ll/i386/x-dosmem.h>
#include <ll/i386/cons.h>
#include <ll/sys/ll/ll-func.h>

#include "vesa.h"

VbeInfoBlock  VbeInfo;                 /* VBE 2.0 Informations */
DWORD vbe_screen;             /* Frame Linear Buffer Address */

char VideoModeNames [96][40] = {
   "100 - 640x400      256 colors", "101 - 640x480      256 colors",  "102 - 800x600       16 colors",
   "103 - 800x600      256 colors", "104 - 1024x768      16 colors",  "105 - 1024x768     256 colors",
   "106 - 1280x1024     16 colors", "107 - 1280x1024    256 colors",  "108 - 80x60         Text Mode",
   "109 - 132x25        Text Mode", "10A - 132x43        Text Mode",  "10B - 132x50        Text Mode",
   "10C - 132x60        Text Mode", "10D - 320x200   32,768 colors",  "10E - 320x200   65,536 colors",
   "10F - 320x200      16m colors", "110 - 640x480   32,768 colors",  "111 - 640x480   65,536 colors",
   "112 - 640x480      16m colors", "113 - 800x600   32,768 colors",  "114 - 800x600   65,536 colors",
   "115 - 800x600      16m colors", "116 - 1024x768  32,768 colors",  "117 - 1024x768  65,536 colors",
   "118 - 1024x768     16m colors", "119 - 1280x1024 32,768 colors",  "11A - 1280x1024 65,536 colors",
   "11B - 1280x1024    16m colors", "11C - 640x350      256 colors",  "11D - 640x350   32,768 colors",
   "11E - 640x400   32,768 colors", "11F - 640x350   65,536 colors",  "120 - 640x400   65,536 colors",
   "121 - 640x350      16m colors", "122 - 640x400      16m colors",  "123 - 1600x1200     16 colors",
   "124 - 1600x1200    256 colors", "125 - 1600x1200 32,768 colors",  "126 - 1600x1200 65,536 colors",
   "127 - 1600x1200    16m colors", "128 - 640x480   16m colors(*)",  "129 - 800x600   16m colors(*)",
   "12A - 1024x768  16m colors(*)", "12B - 1280x1024 16m colors(*)",  "12C - 1600x1200 16m colors(*)",
   "12D - 320x240   32,768 colors", "12E - 320x400   32,768 colors",  "12F - 360x200   32,768 colors",
   "130 - 360x240   32,768 colors", "131 - 360x400   32,768 colors",  "132 - 320x240   65,536 colors",
   "133 - 320x400   65,536 colors", "134 - 360x200   65,536 colors",  "135 - 360x240   65,536 colors",
   "136 - 360x400   65,536 colors", "137 - 320x240      16m colors",  "138 - 320x400      16m colors",
   "139 - name n/a",                "13A - name n/a",                 "13B - name n/a",
   "13C - name n/a",                "13D - name n/a",                 "13E - name n/a",
   "13F - name n/a",                "140 - name n/a",                 "141 - name n/a",
   "142 - 640x350   16m colors(*)", "143 - 640x400   16m colors(*)",  "144 - name n/a",
   "145 - name n/a",                "146 - name n/a",                 "147 - name n/a",
   "148 - name n/a",                "149 - name n/a",                 "14A - name n/a",
   "14B - name n/a",                "14C - name n/a",                 "14D - name n/a",
   "14E - name n/a",                "14F - name n/a",                 "150 - 640x350       16 colors",
   "151 - 640x400       16 colors", "152 - 640x480       16 colors",  "153 - 320x200      256 colors",
   "154 - 320x240      256 colors", "155 - 320x400      256 colors",  "156 - 360x200      256 colors",
   "157 - 360x240      256 colors", "158 - 360x400      256 colors",  "159 - name n/a",
   "15A - name n/a",                "15B - name n/a",                 "15C - name n/a",
   "15D - name n/a",                "15E - name n/a",                 "15F - name n/a"
};

#define NMODES 13
static struct gmode allmodes[NMODES] = {
	{640, 400, 640, 8, 0x4100},
	{640, 480, 640, 8, 0x4101},
	{800, 600, 800, 8, 0x4103},
	{1024, 768, 1024, 8, 0x4105},
	{640, 480, 1280, 15, 0x4110},
	{800, 600, 1600, 15, 0x4113},
	{1024, 768, 2048, 15, 0x4116},
	{640, 480, 1280, 16, 0x4111},
	{800, 600, 1600, 16, 0x4114},
	{1024, 768, 2048, 16, 0x4117},
	{640, 480, 1920, 24, 0x4112},
	{800, 600, 2400, 24, 0x4115},
	{1024, 768, 3072, 24, 0x4118}
};

int vbe_check_status (WORD vbe_function_result)
{
	if (vbe_function_result != 0x004f) {
		switch (vbe_function_result) {
			case 0x014f : return -1;	/* VBE Generic error */
			case 0x024f : return -2;	/* Hardware incompatibility error */
			case 0x034f : return -3;	/* Unvalid function requested */
			default     : return -4;	/* Unknown error */
		}
	}
	return 1;
}

int vbe_modenum(WORD x, WORD y, BYTE depth)
{
	int  mode, i;

	mode = -1;

	for (i = 0; i < NMODES; i++) {
		if ((depth == allmodes[i].bpp) && (x == allmodes[i].xdim) && (y == allmodes[i].ydim)) {
			mode = allmodes[i].modenum;
		}
	}

	return mode;
}

void vbe_restore_vga(void)
{
	X_REGS16 inregs, outregs;
	X_SREGS16 sregs;
#ifndef VM86
	BYTE p1,p2;
#endif

	inregs.x.ax = 0x03;
#ifndef VM86
	p1 = inp(0x21);
	p2 = inp(0xA1);
	outp(0x21,0xFF);
	outp(0xA1,0xFF);
	X_callBIOS(0x10, &inregs, &outregs, &sregs);
	outp(0x21,p1);
	outp(0xA1,p2);
#else
	vm86_callBIOS(0x10, &inregs, &outregs, &sregs);
#endif
}

int vbe_getinfo(void)
{
	X_REGS16  inregs, outregs;              /* registri normali */
	X_SREGS16 sregs;             /* registri estesi */
	LIN_ADDR DOSaddr;
	DWORD linearaddr;
#ifndef VM86
	BYTE p1, p2;
#endif

	/* Allochiamo mem per il nostro blocco */
	if ((DOSaddr = DOS_alloc(sizeof(VbeInfoBlock))) == 0) {
		/*    set_text_mode ();*/
		return -1;
	} else {
		/*linearaddr = appl2linear(DOSaddr);*/
		linearaddr = (DWORD) DOSaddr;

		/* 0x00 = Get Vbe2.0 Info */
		inregs.x.ax = 0x4f00;
		/* Indirizzo reale del nostro blocco */
		inregs.x.di = linearaddr & 0x000F;
		sregs.es = ((linearaddr & 0xFFFF0) >> 4);
		sregs.ds = ((linearaddr & 0xFFFF0) >> 4);

		/* Settiamo il VbeSignature correttamente */
		VbeInfo.VbeSignature[0] = 'V';
		VbeInfo.VbeSignature[1] = 'B';
		VbeInfo.VbeSignature[2] = 'E';
		VbeInfo.VbeSignature[3] = '2';

		memcpy((void *)linearaddr, &VbeInfo, sizeof(VbeInfo));

#ifndef VM86
		p1 = inp(0x21);
		p2 = inp(0xA1);
		outp(0x21,0xFF);
		outp(0xA1,0xFF);
		X_callBIOS(0x10, &inregs, &outregs, &sregs);
		outp(0x21,p1);
		outp(0xA1,p2);
#else
		vm86_callBIOS(0x10, &inregs, &outregs, &sregs);
#endif
		if (vbe_check_status(outregs.x.ax) < 0)
			return -1;
		memcpy(&VbeInfo, (void *)linearaddr, sizeof(VbeInfo));

		return 0;
	}
}

void vbe_showmodes(void)
{
	short int modec;
	WORD list;
	LIN_ADDR VideoModeaddr;

	list=0;
	VideoModeaddr = (LIN_ADDR)((VbeInfo.SupportedModesSegment<<4) + VbeInfo.SupportedModesOffset);

	cprintf ("\nList of modes supported:\n");
	do {
		memcpy(&modec, VideoModeaddr, 2);
		VideoModeaddr += 2;
		if (modec != -1) cprintf ("    %s   ", VideoModeNames[modec - 0x100]);
		list++;
		if (list % 2 == 0) cprintf ("\n");
	} while ((modec != -1) && (list < 40));

	cprintf ("\nToal modes supported: %d", list);
}

DWORD vbe_getmodeinfo(ModeInfoBlock *ModeInfo, WORD Vbe_Mode)
{
	X_REGS16  inregs, outregs;
	X_SREGS16 sregs;
	LIN_ADDR dosaddr;
#ifndef VM86
	BYTE p1, p2;
#endif

	if ((dosaddr = DOS_alloc(sizeof(ModeInfoBlock))) == 0) {
		return(-1);
	}

	/* 0x01 = Get Vbe Mode Info */
	inregs.x.ax = 0x4f01;
	inregs.x.cx  = Vbe_Mode;

	inregs.x.di = (DWORD)dosaddr & 0x000F;
	sregs.es = (((DWORD)dosaddr & 0xFFFF0) >> 4);
	sregs.ds = (((DWORD)dosaddr & 0xFFFF0) >> 4);

#ifndef VM86
	p1 = inp(0x21);
	p2 = inp(0xA1);
	outp(0x21,0xFF);
	outp(0xA1,0xFF);
	X_callBIOS(0x10, &inregs, &outregs, &sregs);
	outp(0x21,p1);
	outp(0xA1,p2);
#else
	vm86_callBIOS(0x10, &inregs, &outregs, &sregs);
#endif
	if (vbe_check_status (outregs.x.ax) < 0) {
		return -1;
	}

	memcpy(ModeInfo, dosaddr, sizeof(ModeInfoBlock));

	vbe_screen = ModeInfo->PhysBasePtr;
	if(vbe_screen == -1) {
		return -1;
	}
	return vbe_screen;
}

DWORD vbe_getflb(void)
{
	return vbe_screen;
}

int vbe_setmode (WORD Vbe_Mode)
{
	X_REGS16 inregs, outregs;
	X_SREGS16 sregs;
#ifndef VM86
	BYTE p1, p2;
#endif

	memset (&inregs, 0, sizeof(inregs));
	/* Set Vesa Vbe mode */
	inregs.x.ax = 0x4f02;
	inregs.x.bx = Vbe_Mode;
#ifndef VM86
	p1 = inp(0x21);
	p2 = inp(0xA1);
	outp(0x21,0xFF);
	outp(0xA1,0xFF);
	X_callBIOS(0x10, &inregs, &outregs, &sregs);
	outp(0x21,p1);
	outp(0xA1,p2);
#else
	vm86_callBIOS(0x10, &inregs, &outregs, &sregs);
#endif
	return 1; //vbe_check_status (outregs.x.ax);
}

WORD vbe_getbpr(ModeInfoBlock *ModeInfo)
{
	return ModeInfo->BytesPerScanLine;
}

int vbe_setbank(ModeInfoBlock *ModeInfo, BYTE bank)
{
	X_REGS16 inregs, outregs;
	X_SREGS16 sregs;
#ifndef VM86
	BYTE p1, p2;
#endif

	memset (&inregs, 0, sizeof(inregs));
	memset (&sregs, 0, sizeof(sregs));
	/* Set Window */
	inregs.x.ax = 0x4f05;
	inregs.h.bh = 0x00;
	inregs.h.bl = 0x00;
	inregs.x.dx = (64 / ModeInfo->WinGranularity) * bank;
#ifndef VM86
	p1 = inp(0x21);
	p2 = inp(0xA1);
	outp(0x21,0xFF);
	outp(0xA1,0xFF);
	X_callBIOS(0x10, &inregs, &outregs, &sregs);
	outp(0x21,p1);
	outp(0xA1,p2);
#else
	vm86_callBIOS(0x10, &inregs, &outregs, &sregs);
#endif
	return vbe_check_status (outregs.x.ax);
}

DWORD vbe_getmem(void)
{
	return (VbeInfo.TotalMemory<<6) * 1024;
}
