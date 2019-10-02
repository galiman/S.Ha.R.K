The graphic driver works invoking the BIOS services through soft interrupts.
This can be done using x_callBIOS() or vm86_callBIOS(): the first solution
works in all the possible systems, but can be used only if the program is run
through X.exe, whereas the second one (vm86_callBIOS()) can be used also if
the system is booted using GRUB. The drawback of the vm86 based solution is
that it may crash some buggy systems (in particular, systems based on funny
video bioses that invoke soft INTs inside the BIOS).

This problem will be fixed in the future; for the moment, it is possible to
compile the library to use x_callBIOS (default) or vm86_callBIOS (add -DVM86
to the c compiler options in the makefile - you can use C_OPT += -DVM86).

WARNING: the banked ops may have some problems...
please report bugs to luca@hartik.sssup.it
