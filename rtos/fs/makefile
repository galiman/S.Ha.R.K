#
# The filesystems library
#

ifndef BASE
BASE=..
endif

include $(BASE)/config/config.mk

LIBRARY       = fs

OBJS_PATH     = $(BASE)/fs

SRCS=	access.c	\
	chdir.c		\
	close.c		\
	closedir.c	\
	dcache.c	\
	dentry.c	\
	fcntl.c		\
	fdevice.c	\
	file.c		\
	fs.c		\
	fstat.c		\
	getcwd.c	\
	getumask.c	\
	inode.c		\
	lseek.c		\
	mount.c		\
	open.c		\
	opendir.c	\
	read.c		\
	readdir.c	\
	rwlock.c	\
	rtc.c		\
	stat.c		\
	super.c		\
	truncate.c	\
	umask.c		\
	umount.c        \
	unlink.c	\
	utime.c		\
	write.c

OBJS=	$(patsubst %.c,%.o,$(SRCS))

# if you want compile:
#
# without the paranoia checks (checks for data structure status/overwriting)
# compile without the -D_PARANOIA
#
# without the internals checks (made with assertions) use the
# -DNDEBUG command line switch
#

#for debug
#C_DEF += -D_PARANOIA

# for NO debug
C_DEF += -D_PARANOIA -DNDEBUG

C_MAC += -imacros $(BASE)/include/fs/fsconf.h

OTHERINCL += -I.

install:: all
all clean cleanall depend::
	make -C msdos $@

include $(BASE)/config/lib.mk

