/*
 * framebuffer driver for VBE 2.0 compliant graphic boards
 *
 * switching to graphics mode happens at boot time (while
 * running in real mode, see arch/i386/boot/video.S).
 *
 * (c) 1998 Gerd Knorr <kraxel@goldbach.in-berlin.de>
 *
 */

#include <linuxcomp.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/ioport.h>
#include <linux/init.h>
#ifdef __i386__
#include <video/edid.h>
#endif
#include <asm/io.h>
#include <asm/mtrr.h>

#define dac_reg	(0x3c8)
#define dac_val	(0x3c9)

#include <vesa.h>

#define INIT_MODE 0x4101

/* --------------------------------------------------------------------- */

static struct fb_var_screeninfo vesafb_defined __initdata = {
	.activate	= FB_ACTIVATE_NOW,
	.height		= -1,
	.width		= -1,
	.right_margin	= 32,
	.upper_margin	= 16,
	.lower_margin	= 4,
	.vsync_len	= 4,
	.vmode		= FB_VMODE_NONINTERLACED,
};

static struct fb_fix_screeninfo vesafb_fix __initdata = {
	.id	= "VESA VGA",
	.type	= FB_TYPE_PACKED_PIXELS,
	.accel	= FB_ACCEL_NONE,
};

static struct fb_info fb_info;
static u32 pseudo_palette[17];

static int             inverse   = 0;
static int             mtrr      = 1;

static int             pmi_setpal = 0;	/* pmi for palette changes ??? */
static int             ypan       = 0;  /* 0..nothing, 1..ypan, 2..ywrap */
static unsigned short  *pmi_base  = 0;
static void            (*pmi_start)(void);
static void            (*pmi_pal)(void);

/* --------------------------------------------------------------------- */

static int vesafb_pan_display(struct fb_var_screeninfo *var,
                              struct fb_info *info)
{
#ifdef __i386__
	int offset;

	if (!ypan)
		return -EINVAL;
	if (var->xoffset)
		return -EINVAL;
	if (var->yoffset > var->yres_virtual)
		return -EINVAL;
	if ((ypan==1) && var->yoffset+var->yres > var->yres_virtual)
		return -EINVAL;

	offset = (var->yoffset * info->fix.line_length + var->xoffset) / 4;

        __asm__ __volatile__(
                "call *(%%edi)"
                : /* no return value */
                : "a" (0x4f07),         /* EAX */
                  "b" (0),              /* EBX */
                  "c" (offset),         /* ECX */
                  "d" (offset >> 16),   /* EDX */
                  "D" (&pmi_start));    /* EDI */
#endif
	return 0;
}

static void vesa_setpalette(int regno, unsigned red, unsigned green, unsigned blue)
{
#ifdef __i386__
	struct { u_char blue, green, red, pad; } entry;

	if (pmi_setpal) {
		entry.red   = red   >> 10;
		entry.green = green >> 10;
		entry.blue  = blue  >> 10;
		entry.pad   = 0;
	        __asm__ __volatile__(
                "call *(%%esi)"
                : /* no return value */
                : "a" (0x4f09),         /* EAX */
                  "b" (0),              /* EBX */
                  "c" (1),              /* ECX */
                  "d" (regno),          /* EDX */
                  "D" (&entry),         /* EDI */
                  "S" (&pmi_pal));      /* ESI */
	} else {
		/* without protected mode interface, try VGA registers... */
		outb_p(regno,       dac_reg);
		outb_p(red   >> 10, dac_val);
		outb_p(green >> 10, dac_val);
		outb_p(blue  >> 10, dac_val);
	}
#endif
}

static int vesafb_setcolreg(unsigned regno, unsigned red, unsigned green,
			    unsigned blue, unsigned transp,
			    struct fb_info *info)
{
	/*
	 *  Set a single color register. The values supplied are
	 *  already rounded down to the hardware's capabilities
	 *  (according to the entries in the `var' structure). Return
	 *  != 0 for invalid regno.
	 */
	
	if (regno >= info->cmap.len)
		return 1;

	switch (info->var.bits_per_pixel) {
	case 8:
		vesa_setpalette(regno,red,green,blue);
		break;
	case 16:
		if (info->var.red.offset == 10) {
			/* 1:5:5:5 */
			((u32*) (info->pseudo_palette))[regno] =	
					((red   & 0xf800) >>  1) |
					((green & 0xf800) >>  6) |
					((blue  & 0xf800) >> 11);
		} else {
			/* 0:5:6:5 */
			((u32*) (info->pseudo_palette))[regno] =	
					((red   & 0xf800)      ) |
					((green & 0xfc00) >>  5) |
					((blue  & 0xf800) >> 11);
		}
		break;
	case 24:
		red   >>= 8;
		green >>= 8;
		blue  >>= 8;
		((u32 *)(info->pseudo_palette))[regno] =
			(red   << info->var.red.offset)   |
			(green << info->var.green.offset) |
			(blue  << info->var.blue.offset);
		break;
	case 32:
		red   >>= 8;
		green >>= 8;
		blue  >>= 8;
		((u32 *)(info->pseudo_palette))[regno] =
			(red   << info->var.red.offset)   |
			(green << info->var.green.offset) |
			(blue  << info->var.blue.offset);
		break;
    }
    return 0;
}

static int vesafb_release(struct fb_info *info, int user)
{

 	vbe_restore_vga();
	return 0;

}

int __init vesafb_setup(char *options)
{
	char *this_opt;
	
	if (!options || !*options)
		return 0;
	
	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt) continue;
		
		if (! strcmp(this_opt, "inverse"))
			inverse=1;
		else if (! strcmp(this_opt, "redraw"))
			ypan=0;
		else if (! strcmp(this_opt, "ypan"))
			ypan=1;
		else if (! strcmp(this_opt, "ywrap"))
			ypan=2;
		else if (! strcmp(this_opt, "vgapal"))
			pmi_setpal=0;
		else if (! strcmp(this_opt, "pmipal"))
			pmi_setpal=1;
		else if (! strcmp(this_opt, "mtrr"))
			mtrr=1;
		else if (! strcmp(this_opt, "nomtrr"))
			mtrr=0;
	}
	return 0;
}

extern VbeInfoBlock VbeInfo;
extern int vbe_getifo(void);
extern void vm86_init(void);

static int vesafb_set_par(struct fb_info *info)
{
	ModeInfoBlock ModeInfo;
	int wx,wy,bpp,mode;
	int video_cmap_len;

	wx = info->var.xres;
	wy = info->var.yres;
	bpp = info->var.bits_per_pixel;
	
	mode = vbe_modenum(wx,wy,bpp);
	if (mode == -1) return -1;
	
	screen_info.lfb_base = vbe_getmodeinfo(&ModeInfo,mode);
	
	screen_info.orig_video_isVGA = VIDEO_TYPE_VLFB;

	if (screen_info.orig_video_isVGA != VIDEO_TYPE_VLFB)
		return -ENXIO;

	screen_info.lfb_depth = ModeInfo.BitsPerPixel;
	screen_info.lfb_width = ModeInfo.XResolution;
	screen_info.lfb_height = ModeInfo.YResolution;
	screen_info.lfb_linelength = ModeInfo.BytesPerScanLine;
	screen_info.lfb_size = VbeInfo.TotalMemory;
	screen_info.pages = ModeInfo.NumberOfImagePages;

	screen_info.vesapm_seg = 0;
	screen_info.vesapm_off = 0;

	screen_info.red_pos = ModeInfo.RedFieldPosition;
	screen_info.red_size = ModeInfo.RedMaskSize;
	screen_info.green_pos = ModeInfo.GreenFieldPosition;
	screen_info.green_size = ModeInfo.GreenMaskSize;
	screen_info.blue_pos = ModeInfo.BlueFieldPosition;
	screen_info.blue_size = ModeInfo.BlueMaskSize;
	screen_info.rsvd_pos = ModeInfo.RsvdFieldPosition;
	screen_info.rsvd_size = ModeInfo.RsvdMaskSize;

	vbe_setmode(mode);

	info->fix.smem_start = screen_info.lfb_base;
	info->var.bits_per_pixel = screen_info.lfb_depth;
	if (15 == info->var.bits_per_pixel)
		info->var.bits_per_pixel = 16;
	info->var.xres = screen_info.lfb_width;
	info->var.yres = screen_info.lfb_height;
	info->fix.line_length = screen_info.lfb_linelength;
	info->fix.smem_len = screen_info.lfb_size;
	info->fix.visual   = (info->var.bits_per_pixel == 8) ?
		FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_TRUECOLOR;

	/* limit framebuffer size to 16 MB.  Otherwise we'll eat tons of
	 * kernel address space for nothing if the gfx card has alot of
	 * memory (>= 128 MB isn't uncommon these days ...) */
	if (info->fix.smem_len > 16 * 1024 * 1024)
		info->fix.smem_len = 16 * 1024 * 1024;

	fb_info.screen_base = ioremap(vesafb_fix.smem_start, vesafb_fix.smem_len);

	/* some dummy values for timing to make fbset happy */
	info->var.pixclock     = 10000000 / info->var.xres * 1000 / info->var.yres;
	info->var.left_margin  = (info->var.xres / 8) & 0xf8;
	info->var.hsync_len    = (info->var.xres / 8) & 0xf8;
	
	if (info->var.bits_per_pixel > 8) {
		info->var.red.offset    = screen_info.red_pos;
		info->var.red.length    = screen_info.red_size;
		info->var.green.offset  = screen_info.green_pos;
		info->var.green.length  = screen_info.green_size;
		info->var.blue.offset   = screen_info.blue_pos;
		info->var.blue.length   = screen_info.blue_size;
		info->var.transp.offset = screen_info.rsvd_pos;
		info->var.transp.length = screen_info.rsvd_size;
		video_cmap_len = 16;
	} else {
		info->var.red.length   = 6;
		info->var.green.length = 6;
		info->var.blue.length  = 6;
		video_cmap_len = 256;
	}

	info->fix.ypanstep  = ypan     ? 1 : 0;
	info->fix.ywrapstep = (ypan>1) ? 1 : 0;
	
	return 0;

}

static int vesafb_check_var(struct fb_var_screeninfo *var,
                             struct fb_info *info)
{

	return 0;

}

static struct fb_ops vesafb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= vesafb_setcolreg,
	.fb_pan_display	= vesafb_pan_display,
	.fb_set_par	= vesafb_set_par,
	.fb_check_var	= vesafb_check_var,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_cursor	= soft_cursor,
	.fb_release	= vesafb_release,
};


int __init vesafb_init(void)
{
	ModeInfoBlock ModeInfo;
	int video_cmap_len;
	int i;

	vm86_init();

	if (vbe_getinfo())
		return -EIO;

	printk(KERN_INFO "VBE Extension Found.\n");

	screen_info.lfb_base = vbe_getmodeinfo(&ModeInfo,INIT_MODE);

	screen_info.orig_video_isVGA = VIDEO_TYPE_VLFB;

	if (screen_info.orig_video_isVGA != VIDEO_TYPE_VLFB)
		return -ENXIO;

	screen_info.lfb_depth = ModeInfo.BitsPerPixel;
	screen_info.lfb_width = ModeInfo.XResolution;
	screen_info.lfb_height = ModeInfo.YResolution;
	screen_info.lfb_linelength = ModeInfo.BytesPerScanLine;
	screen_info.lfb_size = VbeInfo.TotalMemory;
	screen_info.pages = ModeInfo.NumberOfImagePages;

	screen_info.vesapm_seg = 0;
	screen_info.vesapm_off = 0;

	screen_info.red_pos = ModeInfo.RedFieldPosition;
	screen_info.red_size = ModeInfo.RedMaskSize;
	screen_info.green_pos = ModeInfo.GreenFieldPosition;
        screen_info.green_size = ModeInfo.GreenMaskSize;
	screen_info.blue_pos = ModeInfo.BlueFieldPosition;
        screen_info.blue_size = ModeInfo.BlueMaskSize;
	screen_info.rsvd_pos = ModeInfo.RsvdFieldPosition;
        screen_info.rsvd_size = ModeInfo.RsvdMaskSize;

	vbe_setmode(INIT_MODE);

	vesafb_fix.smem_start = screen_info.lfb_base;
	vesafb_defined.bits_per_pixel = screen_info.lfb_depth;
	if (15 == vesafb_defined.bits_per_pixel)
		vesafb_defined.bits_per_pixel = 16;
	vesafb_defined.xres = screen_info.lfb_width;
	vesafb_defined.yres = screen_info.lfb_height;
	vesafb_fix.line_length = screen_info.lfb_linelength;
	vesafb_fix.smem_len = screen_info.lfb_size;
	vesafb_fix.visual   = (vesafb_defined.bits_per_pixel == 8) ?
		FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_TRUECOLOR;

	/* limit framebuffer size to 16 MB.  Otherwise we'll eat tons of
	 * kernel address space for nothing if the gfx card has alot of
	 * memory (>= 128 MB isn't uncommon these days ...) */
	if (vesafb_fix.smem_len > 16 * 1024 * 1024)
		vesafb_fix.smem_len = 16 * 1024 * 1024;

#ifndef __i386__
	screen_info.vesapm_seg = 0;
#endif

	if (!request_mem_region(vesafb_fix.smem_start, vesafb_fix.smem_len, "vesafb")) {
		printk(KERN_WARNING
		       "vesafb: abort, cannot reserve video memory at 0x%lx\n",
			vesafb_fix.smem_start);
		/* We cannot make this fatal. Sometimes this comes from magic
		   spaces our resource handlers simply don't know about */
	}

        fb_info.screen_base = ioremap(vesafb_fix.smem_start, vesafb_fix.smem_len);
	if (!fb_info.screen_base) {
		release_mem_region(vesafb_fix.smem_start, vesafb_fix.smem_len);
		printk(KERN_ERR
		       "vesafb: abort, cannot ioremap video memory 0x%x @ 0x%lx\n",
			vesafb_fix.smem_len, vesafb_fix.smem_start);
		return -EIO;
	}

	printk(KERN_INFO "vesafb: framebuffer at 0x%lx, mapped to 0x%p, size %dk\n",
	       vesafb_fix.smem_start, fb_info.screen_base, vesafb_fix.smem_len/1024);
	printk(KERN_INFO "vesafb: mode is %dx%dx%d, linelength=%d, pages=%d\n",
	       vesafb_defined.xres, vesafb_defined.yres, vesafb_defined.bits_per_pixel, vesafb_fix.line_length, screen_info.pages);

	if (screen_info.vesapm_seg) {
		printk(KERN_INFO "vesafb: protected mode interface info at %04x:%04x\n",
		       screen_info.vesapm_seg,screen_info.vesapm_off);
	}

	if (screen_info.vesapm_seg < 0xc000)
		ypan = pmi_setpal = 0; /* not available or some DOS TSR ... */

	if (ypan || pmi_setpal) {
		pmi_base  = (unsigned short*)phys_to_virt(((unsigned long)screen_info.vesapm_seg << 4) + screen_info.vesapm_off);
		pmi_start = (void*)((char*)pmi_base + pmi_base[1]);
		pmi_pal   = (void*)((char*)pmi_base + pmi_base[2]);
		printk(KERN_INFO "vesafb: pmi: set display start = %p, set palette = %p\n",pmi_start,pmi_pal);
		if (pmi_base[3]) {
			printk(KERN_INFO "vesafb: pmi: ports = ");
				for (i = pmi_base[3]/2; pmi_base[i] != 0xffff; i++)
					printk("%x ",pmi_base[i]);
			printk("\n");

			if (pmi_base[i] != 0xffff) {
				/*
				 * memory areas not supported (yet?)
				 *
				 * Rules are: we have to set up a descriptor for the requested
				 * memory area and pass it in the ES register to the BIOS function.
				 */
				printk(KERN_INFO "vesafb: can't handle memory requests, pmi disabled\n");
				ypan = pmi_setpal = 0;
			}
		}
	}

	vesafb_defined.xres_virtual = vesafb_defined.xres;
	vesafb_defined.yres_virtual = vesafb_fix.smem_len / vesafb_fix.line_length;
	if (ypan && vesafb_defined.yres_virtual > vesafb_defined.yres) {
		printk(KERN_INFO "vesafb: scrolling: %s using protected mode interface, yres_virtual=%d\n",
		       (ypan > 1) ? "ywrap" : "ypan",vesafb_defined.yres_virtual);
	} else {
		printk(KERN_INFO "vesafb: scrolling: redraw\n");
		vesafb_defined.yres_virtual = vesafb_defined.yres;
		ypan = 0;
	}

	/* some dummy values for timing to make fbset happy */
	vesafb_defined.pixclock     = 10000000 / vesafb_defined.xres * 1000 / vesafb_defined.yres;
	vesafb_defined.left_margin  = (vesafb_defined.xres / 8) & 0xf8;
	vesafb_defined.hsync_len    = (vesafb_defined.xres / 8) & 0xf8;
	
	if (vesafb_defined.bits_per_pixel > 8) {
		vesafb_defined.red.offset    = screen_info.red_pos;
		vesafb_defined.red.length    = screen_info.red_size;
		vesafb_defined.green.offset  = screen_info.green_pos;
		vesafb_defined.green.length  = screen_info.green_size;
		vesafb_defined.blue.offset   = screen_info.blue_pos;
		vesafb_defined.blue.length   = screen_info.blue_size;
		vesafb_defined.transp.offset = screen_info.rsvd_pos;
		vesafb_defined.transp.length = screen_info.rsvd_size;
		printk(KERN_INFO "vesafb: directcolor: "
		       "size=%d:%d:%d:%d, shift=%d:%d:%d:%d\n",
		       screen_info.rsvd_size,
		       screen_info.red_size,
		       screen_info.green_size,
		       screen_info.blue_size,
		       screen_info.rsvd_pos,
		       screen_info.red_pos,
		       screen_info.green_pos,
		       screen_info.blue_pos);
		video_cmap_len = 16;
	} else {
		vesafb_defined.red.length   = 6;
		vesafb_defined.green.length = 6;
		vesafb_defined.blue.length  = 6;
		video_cmap_len = 256;
	}

	vesafb_fix.ypanstep  = ypan     ? 1 : 0;
	vesafb_fix.ywrapstep = (ypan>1) ? 1 : 0;

	/* request failure does not faze us, as vgacon probably has this
	 * region already (FIXME) */
	request_region(0x3c0, 32, "vesafb");

	if (mtrr) {
		int temp_size = vesafb_fix.smem_len;
		/* Find the largest power-of-two */
		while (temp_size & (temp_size - 1))
                	temp_size &= (temp_size - 1);

                /* Try and find a power of two to add */
		while (temp_size && mtrr_add(vesafb_fix.smem_start, temp_size, MTRR_TYPE_WRCOMB, 1)==-EINVAL) {
			temp_size >>= 1;
		}
	}
	
	fb_info.fbops = &vesafb_ops;
	fb_info.var = vesafb_defined;
	fb_info.fix = vesafb_fix;
	fb_info.pseudo_palette = pseudo_palette;
	fb_info.flags = FBINFO_FLAG_DEFAULT;

	fb_alloc_cmap(&fb_info.cmap, video_cmap_len, 0);

	if (register_framebuffer(&fb_info)<0)
		return -EINVAL;

	printk(KERN_INFO "fb%d: %s frame buffer device\n",
	       fb_info.node, fb_info.fix.id);
	return 0;
}

/*
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */

MODULE_LICENSE("GPL");
