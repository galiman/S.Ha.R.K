/*
 *  linux/drivers/char/tty_io.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linuxcomp.h>

#include <linux/config.h>
#include <linux/types.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/devpts_fs.h>
#include <linux/file.h>
#include <linux/console.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <linux/kd.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/smp_lock.h>
#include <linux/device.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/bitops.h>

#include <linux/kbd_kern.h>
#include <linux/vt_kern.h>
#include <linux/selection.h>
#include <linux/devfs_fs_kernel.h>

#include <linux/kmod.h>

#undef TTY_DEBUG_HANGUP

#define TTY_PARANOIA_CHECK 1
#define CHECK_TTY_COUNT 1

struct tty_ldisc ldiscs[NR_LDISCS];

struct termios tty_std_termios = {	/* for the benefit of tty drivers  */
	.c_iflag = ICRNL | IXON,
	.c_oflag = OPOST | ONLCR,
        .c_cflag = B4800 | CS8 | CREAD | HUPCL,
	.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
		   ECHOCTL | ECHOKE | IEXTEN,
	.c_cc = INIT_C_CC
};

struct tty_driver *alloc_tty_driver(int lines)
{
	struct tty_driver *driver;

	driver = kmalloc(sizeof(struct tty_driver), GFP_KERNEL);
	if (driver) {
		memset(driver, 0, sizeof(struct tty_driver));
		driver->magic = TTY_DRIVER_MAGIC;
		driver->num = lines;
		/* later we'll move allocation of tables here */
	}
	return driver;
}

void put_tty_driver(struct tty_driver *driver)
{
	kfree(driver);
}

void tty_set_operations(struct tty_driver *driver, struct tty_operations *op)
{
	driver->open = op->open;
	driver->close = op->close;
	driver->write = op->write;
	driver->put_char = op->put_char;
	driver->flush_chars = op->flush_chars;
	driver->write_room = op->write_room;
	driver->chars_in_buffer = op->chars_in_buffer;
	driver->ioctl = op->ioctl;
	driver->set_termios = op->set_termios;
	driver->throttle = op->throttle;
	driver->unthrottle = op->unthrottle;
	driver->stop = op->stop;
	driver->start = op->start;
	driver->hangup = op->hangup;
	driver->break_ctl = op->break_ctl;
	driver->flush_buffer = op->flush_buffer;
	driver->set_ldisc = op->set_ldisc;
	driver->wait_until_sent = op->wait_until_sent;
	driver->send_xchar = op->send_xchar;
	driver->read_proc = op->read_proc;
	driver->write_proc = op->write_proc;
	driver->tiocmget = op->tiocmget;
	driver->tiocmset = op->tiocmset; 
}

int tty_register_driver(struct tty_driver *driver)
{
	return 0;
}

int tty_unregister_driver(struct tty_driver *driver)
{
	return 0;
}

void tty_register_device(struct tty_driver *driver, unsigned index,
			 struct device *device)
{

}

void tty_unregister_device(struct tty_driver *driver, unsigned index)
{

}

/*
 * This routine is called out of the software interrupt to flush data
 * from the flip buffer to the line discipline.
 */
static void flush_to_ldisc(void *private_)
{
	struct tty_struct *tty = (struct tty_struct *) private_;
	unsigned char	*cp;
	char		*fp;
	int		count;

	if (tty->flip.buf_num) {
		cp = tty->flip.char_buf + TTY_FLIPBUF_SIZE;
		fp = tty->flip.flag_buf + TTY_FLIPBUF_SIZE;
		tty->flip.buf_num = 0;
		tty->flip.char_buf_ptr = tty->flip.char_buf;
		tty->flip.flag_buf_ptr = tty->flip.flag_buf;
	} else {
		cp = tty->flip.char_buf;
		fp = tty->flip.flag_buf;
		tty->flip.buf_num = 1;
		tty->flip.char_buf_ptr = tty->flip.char_buf + TTY_FLIPBUF_SIZE;
		tty->flip.flag_buf_ptr = tty->flip.flag_buf + TTY_FLIPBUF_SIZE;
	}
	count = tty->flip.count;
	tty->flip.count = 0;

	tty->ldisc.receive_buf(tty, cp, fp, count);
} 


void tty_flip_buffer_push(struct tty_struct *tty)
{
	flush_to_ldisc((void *) tty); 
}

static struct tty_struct *alloc_tty_struct(void)
{
	struct tty_struct *tty;

	tty = kmalloc(sizeof(struct tty_struct), GFP_KERNEL);
	if (tty)
		memset(tty, 0, sizeof(struct tty_struct));
	return tty;
} 

void do_tty_hangup(void *data)
{ 

}

/*
 * This subroutine initializes a tty structure.
 */
static void initialize_tty_struct(struct tty_struct *tty)
{
	memset(tty, 0, sizeof(struct tty_struct));
	tty->magic = TTY_MAGIC;
	tty->ldisc = ldiscs[N_TTY];
	tty->pgrp = -1;
	tty->flip.char_buf_ptr = tty->flip.char_buf;
	tty->flip.flag_buf_ptr = tty->flip.flag_buf;
	INIT_WORK(&tty->flip.work, flush_to_ldisc, tty);
	init_MUTEX(&tty->flip.pty_sem);
	init_waitqueue_head(&tty->write_wait);
	init_waitqueue_head(&tty->read_wait);
	INIT_WORK(&tty->hangup_work, do_tty_hangup, tty);
	//*sema_init(&tty->atomic_read, 1);
	//*sema_init(&tty->atomic_write, 1);
	spin_lock_init(&tty->read_lock);
	INIT_LIST_HEAD(&tty->tty_files);
	INIT_WORK(&tty->SAK_work, NULL, NULL);
}

extern int serial_open (struct tty_struct *tty, struct file * filp);
extern int serial_write (struct tty_struct * tty, int from_user, const unsigned char *buf, int count);

static void n_tty_receive_buf(struct tty_struct *tty, const unsigned char *cp, char *fp, int count)
{
	const unsigned char *p;
	char *f, flags = TTY_NORMAL;
	int	i;
	char	buf[64];
	unsigned long cpuflags;
 
  if (!tty->read_buf)
		return;

  spin_lock_irqsave(&tty->read_lock, cpuflags);
  i = min(N_TTY_BUF_SIZE - tty->read_cnt, \
          N_TTY_BUF_SIZE - tty->read_head);
	i = min(count, i);
	memcpy(tty->read_buf + tty->read_head, cp, i);
	tty->read_head = (tty->read_head + i) & (N_TTY_BUF_SIZE-1);
  tty->read_cnt += i;
  spin_unlock_irqrestore(&tty->read_lock, cpuflags);

//**    for (i=0; i<count; i++)
//**        printk("%c", *(cp+i) );
}

/* 
 * Check whether to call the driver.unthrottle function.
 * We test the TTY_THROTTLED bit first so that it always
 * indicates the current state.
 */
static void check_unthrottle(struct tty_struct * tty)
{
	if (tty->count &&
	    test_and_clear_bit(TTY_THROTTLED, &tty->flags) && 
	    tty->driver->unthrottle)
		tty->driver->unthrottle(tty);
}

/*
 * Reset the read buffer counters, clear the flags, 
 * and make sure the driver is unthrottled. Called
 * from n_tty_open() and n_tty_flush_buffer().
 */
static void reset_buffer_flags(struct tty_struct *tty)
{
	unsigned long flags;

	spin_lock_irqsave(&tty->read_lock, flags);
	tty->read_head = tty->read_tail = tty->read_cnt = 0;
	spin_unlock_irqrestore(&tty->read_lock, flags);
	tty->canon_head = tty->canon_data = tty->erasing = 0;
	memset(&tty->read_flags, 0, sizeof tty->read_flags);
	check_unthrottle(tty);
} 

int serial_usbport_open(void **private, int port_number)
{
	int retval;
	struct tty_struct *tty;
	
	tty = alloc_tty_struct();
	if(!tty)
		goto fail_no_mem;

	initialize_tty_struct (tty);

	tty->termios = kmalloc (sizeof(struct termios), GFP_KERNEL);
	*(tty->termios) = tty_std_termios;
	tty->index = port_number;	
        tty->ldisc.receive_buf = n_tty_receive_buf;
	
        if (!tty->read_buf) {
		tty->read_buf = kmalloc(N_TTY_BUF_SIZE, GFP_KERNEL);
		if (!tty->read_buf)
			return -ENOMEM;
	}
	memset(tty->read_buf, 0, N_TTY_BUF_SIZE);
	reset_buffer_flags(tty); 

	retval = serial_open(tty, NULL);
	*private = (void*)tty; 
	return retval;

fail_no_mem:
	return -ENOMEM;
}

int serial_usbport_write(void *private, const unsigned char *buf, int count)
{
	int retval;
	struct tty_struct *tty = (struct tty_struct*) private;
 
 	retval = serial_write(tty, 0, buf, count);
	return retval;
}

int serial_usbport_read(void *private, char* data_in)
{
	char c;
	struct tty_struct *tty = (struct tty_struct*) private;
	unsigned long flags;

  if (tty->read_cnt)
  {
    c = tty->read_buf[tty->read_tail];
    tty->read_tail = ((tty->read_tail+1) & \
                      (N_TTY_BUF_SIZE-1));
    tty->read_cnt--;
    *data_in = c;
    return 1;
  }
  *data_in = 0;
  return 0;
}
