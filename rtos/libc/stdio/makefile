#
# The stdio sub-library
#

ifndef BASE
BASE=../..
endif

include $(BASE)/config/config.mk

LIBRARY       = c

OBJS_PATH     = $(BASE)/libc/stdio

#perror.c
#	already into kernel
#tmpfile.c
#	do not work	
# sprintf.c vsprintf.c sscanf.c vsscanf.c
#	into the ll starte

SRCS=	asprintf.c	\
	clrerr.c	\
	fclose.c	\
	fdopen.c	\
	feof.c		\
	ferror.c	\
	fflush.c	\
	fgetc.c		\
	fgetln.c	\
	fgetpos.c	\
	fgets.c		\
	fileno.c	\
	findfp.c	\
	flags.c		\
	fopen.c		\
	fprintf.c	\
	fpurge.c	\
	fputc.c		\
	fputs.c		\
	fread.c		\
	freopen.c	\
	fscanf.c	\
	fseek.c		\
	fsetpos.c	\
	ftell.c		\
	funopen.c	\
	fvwrite.c	\
	fwalk.c		\
	fwrite.c	\
	getc.c		\
	getchar.c	\
	gets.c		\
	getw.c		\
	makebuf.c	\
	mktemp.c	\
	printf.c	\
	putc.c		\
	putchar.c	\
	puts.c		\
	putw.c		\
	refill.c	\
	remove.c	\
	rewind.c	\
	rget.c		\
	scanf.c		\
	setbuf.c	\
	setbuffe.c	\
	setvbuf.c	\
	snprintf.c	\
	stdio.c		\
	tempnam.c	\
	tmpnam.c	\
	ungetc.c	\
	vasprint.c	\
	vfprintf.c	\
	vfscanf.c	\
	vprintf.c	\
	vscanf.c        \
	vsnprint.c	\
	wbuf.c		\
	wsetup.c

OBJS=	$(patsubst %.c,%.o,$(SRCS))

C_DEF += -D_THREAD_SAFE

include $(BASE)/config/sublib.mk
