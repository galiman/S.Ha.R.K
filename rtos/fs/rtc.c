/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   Luca Abeni          <luca@gandalf.sssup.it>
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/**
 ------------
 CVS :        $Id: rtc.c,v 1.1 2004/05/26 10:12:41 giacomo Exp $

 File:        $File$
 Revision:    $Revision: 1.1 $
 Last update: $Date: 2004/05/26 10:12:41 $
 ------------

 Author:      Massimiliano Giorgi

 A source from Linux 2.2.9 modified to work with S.Ha.R.K.

**/

/*
 * Copyright (C) 2000 Paolo Gai
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 *	Real Time Clock interface for Linux	
 *
 *	Copyright (C) 1996 Paul Gortmaker
 *
 *	This driver allows use of the real time clock (built into
 *	nearly all computers) from user space. It exports the /dev/rtc
 *	interface supporting various ioctl() and also the /proc/rtc
 *	pseudo-file for status information.
 *
 *	The ioctls can be used to set the interrupt behaviour and
 *	generation rate from the RTC via IRQ 8. Then the /dev/rtc
 *	interface can be used to make use of these timer interrupts,
 *	be they interval or alarm based.
 *
 *	The /dev/rtc interface will block on reads until an interrupt
 *	has been received. If a RTC interrupt has already happened,
 *	it will output an unsigned long and then block. The output value
 *	contains the interrupt status in the low byte and the number of
 *	interrupts since the last read in the remaining high bytes. The 
 *	/dev/rtc interface can also be used with the select(2) call.
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *	Based on other minimal char device drivers, like Alan's
 *	watchdog, Ted's random, etc. etc.
 *
 *	1.07	Paul Gortmaker.
 *	1.08	Miquel van Smoorenburg: disallow certain things on the
 *		DEC Alpha as the CMOS clock is also used for other things.
 *	1.09	Nikita Schmidt: epoch support and some Alpha cleanup.
 *
 */

#define RTC_VERSION		"1.09"

#define RTC_IRQ 	8	/* Can't see this changing soon.	*/
#define RTC_IO_EXTENT	0x10	/* Only really two ports, but...	*/

/*
 *	Note that *all* calls to CMOS_READ and CMOS_WRITE are done with
 *	interrupts disabled. Due to the index-port/data-port (0x70/0x71)
 *	design of the RTC, we don't want two different things trying to
 *	get to it at once. (e.g. the periodic 11 min sync from time.c vs.
 *	this driver.)
 */

#include <kernel/kern.h>
#include <rtcfunc.h>
#include "rtc.h"
/*
#define CMOS_READ(addr) ({ \
outb_p((addr),RTC_PORT(0)); \
inb_p(RTC_PORT(1)); \
})
#define CMOS_WRITE(val, addr) ({ \
outb_p((addr),RTC_PORT(0)); \
outb_p((val),RTC_PORT(1)); \
})
*/

#define CMOS_READ(addr) (     \
  ll_out(RTC_PORT(0),addr),   \
  ll_in(RTC_PORT(1))          \
)

#define CMOS_WRITE(val, addr) (\
  ll_out(RTC_PORT(0),addr),    \
  ll_out(RTC_PORT(1),val)      \
)

/*
 *	We sponge a minor off of the misc major. No need slurping
 *	up another valuable major dev number for this. If you add
 *	an ioctl, make sure you don't conflict with SPARC's RTC
 *	ioctls.
 */


int get_rtc_time (struct rtc_time *rtc_tm);
int set_rtc_time (struct rtc_time *rtc_tm);
int get_rtc_alm_time (struct rtc_time *alm_tm);

void set_rtc_irq_bit(unsigned char bit);
void mask_rtc_irq_bit(unsigned char bit);

static inline unsigned char rtc_is_updating(void);

/*
 *	Bits in rtc_status. (6 bits of room for future expansion)
 */

#define RTC_IS_OPEN		0x01	/* means /dev/rtc is in use	*/
#define RTC_TIMER_ON		0x02	/* missed irq timer active	*/

unsigned char rtc_status = 0;		/* bitmapped status byte.	*/
unsigned long rtc_freq = 0;		/* Current periodic IRQ rate	*/
unsigned long rtc_irq_data = 0;		/* our output to the world	*/

/*
 *	If this driver ever becomes modularised, it will be really nice
 *	to make the epoch retain its value across module reload...
 */

static unsigned long epoch = 1900;	/* year corresponding to 0x00	*/

unsigned char days_in_mo[] = 
{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*
 * Returns true if a clock update is in progress
 */
static inline unsigned char rtc_is_updating(void)
{
  SYS_FLAGS flags;
  unsigned char uip;

  flags=kern_fsave();
  uip = (CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP);
  kern_frestore(flags);
  return uip;
}

int get_rtc_time(struct rtc_time *rtc_tm)
{
  SYS_FLAGS flags;
  unsigned char ctrl;
  unsigned retries=0;
  struct timespec delay;

  /*
   * read RTC once any update in progress is done. The update
   * can take just over 2ms. We wait 10 to 20ms. There is no need to
   * to poll-wait (up to 1s - eeccch) for the falling edge of RTC_UIP.
   * If you need to know *exactly* when a second has started, enable
   * periodic update complete interrupts, (via ioctl) and then 
   * immediately read /dev/rtc which will block until you get the IRQ.
   * Once the read clears, read the RTC time (again via ioctl). Easy.
   */

  /*
  if (rtc_is_updating() != 0)
    while (jiffies - uip_watchdog < 2*HZ/100)
      barrier();
  */

  delay.tv_nsec = 1000000;
  delay.tv_sec = 0;
  while (rtc_is_updating()&&++retries<=5) nanosleep(&delay, NULL);
  if (retries>5) return -1;

  /*
   * Only the values that we read from the RTC are set. We leave
   * tm_wday, tm_yday and tm_isdst untouched. Even though the
   * RTC has RTC_DAY_OF_WEEK, we ignore it, as it is only updated
   * by the RTC when initially set to a non-zero value.
   */
  flags=kern_fsave();
  rtc_tm->tm_sec = CMOS_READ(RTC_SECONDS);
  rtc_tm->tm_min = CMOS_READ(RTC_MINUTES);
  rtc_tm->tm_hour = CMOS_READ(RTC_HOURS);
  rtc_tm->tm_mday = CMOS_READ(RTC_DAY_OF_MONTH);
  rtc_tm->tm_mon = CMOS_READ(RTC_MONTH);
  rtc_tm->tm_year = CMOS_READ(RTC_YEAR);
  ctrl = CMOS_READ(RTC_CONTROL);
  kern_frestore(flags);

  if (!(ctrl & RTC_DM_BINARY) || RTC_ALWAYS_BCD)
    {
      BCD_TO_BIN(rtc_tm->tm_sec);
      BCD_TO_BIN(rtc_tm->tm_min);
      BCD_TO_BIN(rtc_tm->tm_hour);
      BCD_TO_BIN(rtc_tm->tm_mday);
      BCD_TO_BIN(rtc_tm->tm_mon);
      BCD_TO_BIN(rtc_tm->tm_year);
    }

  /*
   * Account for differences between how the RTC uses the values
   * and how they are defined in a struct rtc_time;
   */
  if ((rtc_tm->tm_year += (epoch - 1900)) <= 69)
    rtc_tm->tm_year += 100;

  rtc_tm->tm_mon--;

  return 0;
}

int set_rtc_time(struct rtc_time *rtc_tm)
{
  unsigned char mon, day, hrs, min, sec, leap_yr;
  unsigned char save_control, save_freq_select;
  unsigned int yrs;
  SYS_FLAGS flags;
			
  yrs = rtc_tm->tm_year + 1900;
  mon = rtc_tm->tm_mon + 1;   /* tm_mon starts at zero */
  day = rtc_tm->tm_mday;
  hrs = rtc_tm->tm_hour;
  min = rtc_tm->tm_min;
  sec = rtc_tm->tm_sec;

  if (yrs < 1970)
    return -EINVAL;

  leap_yr = ((!(yrs % 4) && (yrs % 100)) || !(yrs % 400));

  if ((mon > 12) || (day == 0))
    return -EINVAL;

  if (day > (days_in_mo[mon] + ((mon == 2) && leap_yr)))
    return -EINVAL;
			
  if ((hrs >= 24) || (min >= 60) || (sec >= 60))
    return -EINVAL;

  if ((yrs -= epoch) > 255)    /* They are unsigned */
    return -EINVAL;

  flags=kern_fsave();
  if (!(CMOS_READ(RTC_CONTROL) & RTC_DM_BINARY)
      || RTC_ALWAYS_BCD) {
    if (yrs > 169) {
      kern_frestore(flags);
      return -EINVAL;
    }
    if (yrs >= 100)
      yrs -= 100;

    BIN_TO_BCD(sec);
    BIN_TO_BCD(min);
    BIN_TO_BCD(hrs);
    BIN_TO_BCD(day);
    BIN_TO_BCD(mon);
    BIN_TO_BCD(yrs);
  }

  save_control = CMOS_READ(RTC_CONTROL);
  CMOS_WRITE((save_control|RTC_SET), RTC_CONTROL);
  save_freq_select = CMOS_READ(RTC_FREQ_SELECT);
  CMOS_WRITE((save_freq_select|RTC_DIV_RESET2), RTC_FREQ_SELECT);

  CMOS_WRITE(yrs, RTC_YEAR);
  CMOS_WRITE(mon, RTC_MONTH);
  CMOS_WRITE(day, RTC_DAY_OF_MONTH);
  CMOS_WRITE(hrs, RTC_HOURS);
  CMOS_WRITE(min, RTC_MINUTES);
  CMOS_WRITE(sec, RTC_SECONDS);

  CMOS_WRITE(save_control, RTC_CONTROL);
  CMOS_WRITE(save_freq_select, RTC_FREQ_SELECT);

  kern_frestore(flags);

  return 0;
}

int get_rtc_alm_time(struct rtc_time *alm_tm)
{
  SYS_FLAGS flags;
  unsigned char ctrl;

  /*
   * Only the values that we read from the RTC are set. That
   * means only tm_hour, tm_min, and tm_sec.
   */
  flags=kern_fsave();
  alm_tm->tm_sec = CMOS_READ(RTC_SECONDS_ALARM);
  alm_tm->tm_min = CMOS_READ(RTC_MINUTES_ALARM);
  alm_tm->tm_hour = CMOS_READ(RTC_HOURS_ALARM);
  ctrl = CMOS_READ(RTC_CONTROL);
  kern_frestore(flags);

  if (!(ctrl & RTC_DM_BINARY) || RTC_ALWAYS_BCD)
    {
      BCD_TO_BIN(alm_tm->tm_sec);
      BCD_TO_BIN(alm_tm->tm_min);
      BCD_TO_BIN(alm_tm->tm_hour);
    }

  return 0;
}

/*
 * Used to disable/enable interrupts for any one of UIE, AIE, PIE.
 * Rumour has it that if you frob the interrupt enable/disable
 * bits in RTC_CONTROL, you should read RTC_INTR_FLAGS, to
 * ensure you actually start getting interrupts. Probably for
 * compatibility with older/broken chipset RTC implementations.
 * We also clear out any old irq data after an ioctl() that
 * meddles with the interrupt enable/disable bits.
 */

void mask_rtc_irq_bit(unsigned char bit)
{
  unsigned char val;
  SYS_FLAGS flags;

  flags=kern_fsave();
  //cli();
  val = CMOS_READ(RTC_CONTROL);
  val &=  ~bit;
  CMOS_WRITE(val, RTC_CONTROL);
  CMOS_READ(RTC_INTR_FLAGS);
  kern_frestore(flags);
  //rtc_irq_data = 0;
}

void set_rtc_irq_bit(unsigned char bit)
{
  unsigned char val;
  SYS_FLAGS flags;

  flags=kern_fsave();
  //cli();
  val = CMOS_READ(RTC_CONTROL);
  val |= bit;
  CMOS_WRITE(val, RTC_CONTROL);
  CMOS_READ(RTC_INTR_FLAGS);
  //rtc_irq_data = 0;
  kern_frestore(flags);
}

/* added by Massy */
/* to find the date in seconds from the Epoch (1 Gen 1970 00:00 GMT) */
/* (modifing a source from Linux) */
  
static int day_n[]={
  0,31,59,90,120,151,181,212,243,273,304,334,0,0,0,0
};
		 
time_t sys_getdate(void)
{
  struct rtc_time rtc;
  time_t secs;

  get_rtc_time (&rtc);
    
  secs = rtc.tm_sec+60l*rtc.tm_min+rtc.tm_hour*3600l+86400l*
    (rtc.tm_mday-1+day_n[rtc.tm_mon]+(rtc.tm_year/4l)
    +rtc.tm_year*365l-
    ((rtc.tm_year & 3) == 0 && rtc.tm_mon < 2 ? 1 : 0)+3653l);
  
  /* days since 1.1.70 plus 80's leap day */
  /*secs += sys_tz.tz_minuteswest*60;*/
  /*if (sys_tz.tz_dsttime) secs -= 3600;*/
  return secs;
}
