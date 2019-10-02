--------------------------------------
MESA Demo (tex)

by

Giacomo Guidi	<giacomo@gandalf.sssup.it>

Last update 17/03/2003
--------------------------------------

This is a simple test demo for the MESA (5.0)
libraries, the low level graphic drivers is
the SVGA (from the SVGAlib)

See drivers/svga/readme for supported cards

--------------------------------------

The demo is composed by:

MAKEFILE     The makefile used to compile the application
README.TXT   This file
INITFILE.C   The init file
MESATEX.C    The MESA Demo

--------------------------------------

- To specify your card change the line

#define CARD <driver name>

- The demo calls the grx and off-screen Mesa functions. 
The resolution must be 16 bitsperpixel (64K colors) and 
the graphic access mode must be linear.

- There are two buffers

	The video buffer (video_buf)
	The virtual buffer (rgb_565_buf)

	copy_videomem_16to16 links these buffers



