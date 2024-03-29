#ifndef _I8042_H
#define _I8042_H

/*
 *  Copyright (c) 1999-2002 Vojtech Pavlik
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

/*
 * Arch-dependent inline functions and defines.
 */

#if defined(CONFIG_PPC)
#include "i8042-ppcio.h"
#elif defined(CONFIG_SPARC32) || defined(CONFIG_SPARC64)
#include "i8042-sparcio.h"
#else
#include "i8042-io.h"
#endif

/*
 * This is in 50us units, the time we wait for the i8042 to react. This
 * has to be long enough for the i8042 itself to timeout on sending a byte
 * to a non-existent mouse.
 */

#define I8042_CTL_TIMEOUT	10000

/*
 * When the device isn't opened and it's interrupts aren't used, we poll it at
 * regular intervals to see if any characters arrived. If yes, we can start
 * probing for any mouse / keyboard connected. This is the period of the
 * polling.
 */

#define I8042_POLL_PERIOD	HZ/20

/*
 * Status register bits.
 */

#define I8042_STR_PARITY	0x80
#define I8042_STR_TIMEOUT	0x40
#define I8042_STR_AUXDATA	0x20
#define I8042_STR_KEYLOCK	0x10
#define I8042_STR_CMDDAT	0x08
#define I8042_STR_MUXERR	0x04
#define I8042_STR_IBF		0x02
#define	I8042_STR_OBF		0x01

/*
 * Control register bits.
 */

#define I8042_CTR_KBDINT	0x01
#define I8042_CTR_AUXINT	0x02
#define I8042_CTR_IGNKEYLOCK	0x08
#define I8042_CTR_KBDDIS	0x10
#define I8042_CTR_AUXDIS	0x20
#define I8042_CTR_XLATE		0x40

/*
 * Commands.
 */

#define I8042_CMD_CTL_RCTR	0x0120
#define I8042_CMD_CTL_WCTR	0x1060
#define I8042_CMD_CTL_TEST	0x01aa

#define I8042_CMD_KBD_DISABLE	0x00ad
#define I8042_CMD_KBD_ENABLE	0x00ae
#define I8042_CMD_KBD_TEST	0x01ab
#define I8042_CMD_KBD_LOOP	0x11d2

#define I8042_CMD_AUX_DISABLE	0x00a7
#define I8042_CMD_AUX_ENABLE	0x00a8
#define I8042_CMD_AUX_TEST	0x01a9
#define I8042_CMD_AUX_SEND	0x10d4
#define I8042_CMD_AUX_LOOP	0x11d3

#define I8042_CMD_MUX_PFX	0x0090
#define I8042_CMD_MUX_SEND	0x1090

/*
 * Return codes.
 */

#define I8042_RET_CTL_TEST	0x55

/*
 * Expected maximum internal i8042 buffer size. This is used for flushing
 * the i8042 buffers. 32 should be more than enough.
 */

#define I8042_BUFFER_SIZE	32

/*
 * Debug.
 */

#ifdef DEBUG
static unsigned long i8042_start;
#define dbg_init() do { i8042_start = jiffies26; } while (0)
#define dbg(format, arg...) printk(KERN_DEBUG __FILE__ ": " format " [%d]\n" ,\
	 ## arg, (int) (jiffies26 - i8042_start))
#else
#define dbg_init() do { } while (0)
#define dbg(format, arg...) do {} while (0)
#endif

#endif /* _I8042_H */
