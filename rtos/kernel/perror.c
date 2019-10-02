/* Copyright (C) 1991, 1992, 1993, 1997, 1998, 1999
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <kernel/const.h>
#include <ll/stdlib.h>
#include <ll/stdio.h>
#include <kernel/func.h>

/* This is a list of all known `errno' codes.  */

#define N_(x) x

const char *const _sys_errlist[] =
  {
    N_("Success"),

    /* LIBC errors */
    N_("Operation not permitted"),
    N_("No such file or directory"),
    N_("No such process"),
    N_("Interrupted system call"),
    N_("Input/output error"),
    N_("Device not configured"),
    N_("Argument list too long"),
    N_("Exec format error"),
    N_("Bad file descriptor"),
    N_("No child processes"),
    N_("Resource temporarily unavailable"),
    N_("Cannot allocate memory"),
    N_("Permission denied"),
    N_("Bad address"),
    N_("Block device required"),
    N_("Device or resource busy"),
    N_("File exists"),
    N_("Invalid cross-device link"),
    N_("No such device"),
    N_("Not a directory"),
    N_("Is a directory"),
    N_("Invalid argument"),
    N_("Too many open files in system"),
    N_("Too many open files"),
    N_("Inappropriate ioctl for device"),
    N_("Text file busy"),
    N_("File too large"),
    N_("No space left on device"),
    N_("Illegal seek"),
    N_("Read-only file system"),
    N_("Too many links"),
    N_("Broken pipe"),
    N_("Numerical argument out of domain"),
    N_("Numerical result out of range"),
    N_("Resource deadlock avoided"),
    N_("File name too long"),
    N_("No locks available"),
    N_("Function not implemented"),
    N_("Directory not empty"),
    N_("Too many levels of symbolic links"),
    N_("(null)"),
    N_("No message of desired type"),
    N_("Identifier removed"),
    N_("Channel number out of range"),
    N_("Level 2 not synchronized"),
    N_("Level 3 halted"),
    N_("Level 3 reset"),
    N_("Link number out of range"),
    N_("Protocol driver not attached"),
    N_("No CSI structure available"),
    N_("Level 2 halted"),
    N_("Invalid exchange"),
    N_("Invalid request descriptor"),
    N_("Exchange full"),
    N_("No anode"),
    N_("Invalid request code"),
    N_("Invalid slot"),
    N_("(null)"),
    N_("Bad font file format"),
    N_("Device not a stream"),
    N_("No data available"),
    N_("Timer expired"),
    N_("Out of streams resources"),
    N_("Machine is not on the network"),
    N_("Package not installed"),
    N_("Object is remote"),
    N_("Link has been severed"),
    N_("Advertise error"),
    N_("Srmount error"),
    N_("Communication error on send"),
    N_("Protocol error"),
    N_("Multihop attempted"),
    N_("RFS specific error"),
    N_("Bad message"),
    N_("Value too large for defined data type"),
    N_("Name not unique on network"),
    N_("File descriptor in bad state"),
    N_("Remote address changed"),
    N_("Can not access a needed shared library"),
    N_("Accessing a corrupted shared library"),
    N_(".lib section in a.out corrupted"),
    N_("Attempting to link in too many shared libraries"),
    N_("Cannot exec a shared library directly"),
    N_("Invalid or incomplete multibyte or wide character"),
    N_("Interrupted system call should be restarted"),
    N_("Streams pipe error"),
    N_("Too many users"),
    N_("Socket operation on non-socket"),
    N_("Destination address required"),
    N_("Message too long"),
    N_("Protocol wrong type for socket"),
    N_("Protocol not available"),
    N_("Protocol not supported"),
    N_("Socket type not supported"),
    N_("Operation not supported"),
    N_("Protocol family not supported"),
    N_("Address family not supported by protocol"),
    N_("Address already in use"),
    N_("Cannot assign requested address"),
    N_("Network is down"),
    N_("Network is unreachable"),
    N_("Network dropped connection on reset"),
    N_("Software caused connection abort"),
    N_("Connection reset by peer"),
    N_("No buffer space available"),
    N_("Transport endpoint is already connected"),
    N_("Transport endpoint is not connected"),
    N_("Cannot send after transport endpoint shutdown"),
    N_("Too many references: cannot splice"),
    N_("Connection timed out"),
    N_("Connection refused"),
    N_("Host is down"),
    N_("No route to host"),
    N_("Operation already in progress"),
    N_("Operation now in progress"),
    N_("Stale NFS file handle"),
    N_("Structure needs cleaning"),
    N_("Not a XENIX named type file"),
    N_("No XENIX semaphores available"),
    N_("Is a named type file"),
    N_("Remote I/O error"),
    N_("Disk quota exceeded"),
    N_("No medium found"),
    N_("Wrong medium type"),
    N_("Cancelled"),

    /* S.Ha.R.K.'s errors */
    N_("Wrong interrupt number"),
    N_("Already used interrupt number"),
    N_("Unused interrupt number"),
    N_("Too much init functions"),
    N_("Too much exit functions"),
    N_("No task descriptor available"),
    N_("Schedule module not found for the model used"),
    N_("Error during task_create"),
    N_("Resource module not found for the model used"),
    N_("Created task cannot be guaranteed"),
    N_("No stack memory available"),
    N_("No TSS available to create another task"),
    N_("Invalid kill"),
    N_("Invalid task ID"),
    N_("Invalid group"),

    N_("No more port descriptor"),
    N_("No more port interface"),
    N_("Incompatible message declaration"),
    N_("Port already open"),
    N_("No more hash entries"),
    N_("2 connection on same receive port"),
    N_("Unsupported port access"),
    N_("Wrong port operation"),
    N_("Wrong port type"),
    N_("Invalid Port Descriptor"),

    N_("Cab id not valid"),
    N_("Cab is closed"),
    N_("Unvalid number of message"),
    N_("No more cab free"),
    N_("Number of message exceeded"),

    NULL
  };

/* Print a line on stderr consisting of the text in S, a colon, a space,
   a message describing the meaning of the contents of `errno' and a newline.
   If S is NULL or "", the colon and space are omitted.  */
void
perror (const char *s)
{
  int errnum = errno;
  const char *colon, *st;

  if (s == NULL || *s == '\0')
    st = colon = "";
  else {
    colon = ": ";
    st = s;
  }

  if (errnum > LAST_ERR_NUMBER)
    errnum = LAST_ERR_NUMBER;

  kern_printf ("%s%s%s\n", st, colon, _sys_errlist[errnum]);
}

const char *strerror(int errnum)
{
  if (errnum > LAST_ERR_NUMBER || errnum < 0 )
    errnum = LAST_ERR_NUMBER;

  return _sys_errlist[errnum];
}


