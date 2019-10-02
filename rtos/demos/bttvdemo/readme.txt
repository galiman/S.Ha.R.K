--------------------------------------
BTTV Frame Grabber Demo

by

Giacomo Guidi	<giacomo@gandalf.sssup.it>

Last update 12/05/2004
--------------------------------------
Ex:

x bttv [channel number]

if the channel is not specified 0 is taken.
If you don't see a grabbed image, you
can try to set a channel number from 0 to 4.

This is a simple demo for the BTTV driver.
It starts the frame grabber at 25 fps 320x200. 

--------------------------------------

The demo is composed by:

MAKEFILE     The makefile used to compile the application
README.TXT   This file
INITFILE.C   The init file
BTTV.C       The BTTV Demo

--------------------------------------

