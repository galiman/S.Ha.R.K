---------------------------------------------
Dynalink Demo.
---------------------------------------------
This is the Dynalink for S.H.A.R.K demo.
Source files are in /shark/projects/dynademo
The SHARK kernel and the user application
are compiled seperatly by;

make_os.mk
make_app.mk

The resulting OS kernel file is a default 
SHARK mutliboot compliant ELF executable.
The resluting user Application is an ELF object file.

Also included is a small test data.bin file
to illustrate the use of extra data module parsing through GRUB.

The idea is to boot the SHARK kernel through GRUB,
and parsing the User Application plus any needed
data files as 'boot modules'.
GRUB usage example:

# For booting SHARK image from HD
title  S.H.A.R.K + Boot Modules from HD 0,0
kernel (hd0,0)/boot/os   mount root=/dev/hda1
module (hd0,0)/boot/sh_app.bin
module (hd0,0)/boot/data.bin

This could be a possible solution to get by
certain GPL restrictions since no GPL code
resides in the user application.
The user application is dynamicly linked
after the kernel is loaded.

Another advantage is that we have a simple
way of loading data files without the need for
an IDE-driver plus Filesystem.
Good enough for a few embedded solutions.

-------------------------------------------------------------
Dynalink code
-------------------------------------------------------------
The Dynalinker is based on Luca Abenia's code
as used in the FD32 project.
Source files are in /shark/dynalink
The Dynalink makefile produces libdynalink.a
which is compiled into the kernel by adding SHARKOPT =" __DYNALINK__"

Also not that I added a section to config.mk as
you can see in the included config.mk

# added for dynalink [lex]
ifndef DYNALINK
LINK_OPT = -Bstatic -Ttext $(MEM_START) -s -nostartfiles -nostdlib -L$(LIB_PATH) -L$(OSLIB_PATH)
else
LINK_OPT = -Bstatic
endif

Operation:
The Dynalinker supports only ELF objects.
Everything else is loaded and treated as Data objects.

After the SHARK kernel is booted it will execute main()
which sole purpose here is to process all parsed
boot modules, link any valid ELF objects
and put the results in a dynalink_module_list struct.
This struct is then used to run the actual User Application
and to hold info about possible data files in memory
that the User Application can use.

The dynalinker uses a syscall_table to export/import the symbols.
(review dynalink.c)
Not all SHARK/Library functions needs to be exported!
This is of course just an example.


/Lex Nahumury 5:41 19-7-06





