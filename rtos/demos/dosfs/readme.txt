--------------------------------------
DOS_fXXX Demo

by

Paolo Gai          pj@sssup.it
and 
Gabriele Bolognini gabrieleb@sssup.it
2001

--------------------------------------

The DOS_fXXX functions provided into ll/i386/x-dos.h are a few
shortcuts that can be used to call INT21's DOS calls.

These functions can be used to read and write files before/after
the kernel goes into i386 protected mode.

THEY CANNOT BE USED IF YOU BOOT WITH GRUB.

--------------------------------------

The demo is composed by:

MAKEFILE     The makefile used to compile the application;
             dosfs is the rule to compile the application 
README.TXT   This file
INITFILE.C   The init file (only RR)
DOSFS.C      The DOS_fread and DOS_fwrite demo

--------------------------------------

The demo works as follows:
- Into __kernel_register_levels__() the demo reads up to 1000 bytes from
  myfile.txt
- Then, into the main() function it prints the contents of the buffer at
  screen
- Finally, at the end of the system it writes a file called myfile.out
  with up to 30 of the first bytes of the buffer

The demo should work on any processor speed, and does not use keyboard or
graphics.

Bug: at the moment (01 Jun 2001) sometimes the demo hangs when using
the one-shot timer.

