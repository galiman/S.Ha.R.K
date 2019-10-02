/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: comp.h,v 1.1 2003/03/24 13:26:13 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2003/03/24 13:26:13 $
 ------------
**/

/*
 * Copyright (C) 2000 Luca Abeni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef VGA_COMP
#define VGA_COMP

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    int width;
    int height;
    int bytesperpixel;
    int colors;
    int linewidth;		/* scanline width in bytes */
    int maxlogicalwidth;	/* maximum logical scanline width */
    int startaddressrange;	/* changeable bits set */
    int maxpixels;		/* video memory / bytesperpixel */
    int haveblit;		/* mask of blit functions available */
    int flags;		/* other flags */

    /* Extended fields: */
    int chiptype;		/* Chiptype detected */
    int memory;		/* videomemory in KB */
    int linewidth_unit;	/* Use only a multiple of this as parameter for
    				set_logicalwidth and set_displaystart */
    LIN_ADDR linear_aperture;	/* points to mmap secondary mem aperture
    				of card (NULL if unavailable) */
    int aperture_size;	/* size of aperture in KB if size>=videomemory.
    				0 if unavail */
    void (*set_aperture_page) (int page); /* if aperture_size<videomemory
    				select a memory page */
    void *extensions;	/* points to copy of eeprom for mach32 */
	/* depends from actual driver/chiptype.. etc. */
} grx_vga_modeinfo;

#ifdef __cplusplus
}

#endif
#endif				/* VGA_H */

