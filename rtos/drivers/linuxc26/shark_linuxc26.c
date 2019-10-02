/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Giacomo Guidi       <giacomo@gandalf.sssup.it>
 *
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/* Linux 2.6 Emulation Library */

#include <kernel/kern.h>

extern int devices_init(void);
extern int buses_init(void);
extern int classes_init(void);
extern int linuxcomp_init(void);
extern int shark_interrupt_server(int use_intdrv);

unsigned long read_time(void) {

   return kern_gettime(NULL);

}

void do_rescheduler(void) {

  event_need_reschedule();

}

/* Init the Emulation Library */
int LINUXC26_register_module(int use_intdrive) {

  int res;

  printk("LINUXC26_register_module\n");

  linuxcomp_init();
  devices_init();
  buses_init(); 
  classes_init();

  res = shark_interrupt_server(use_intdrive);
  if (res != 0) {
	printk("ERROR: CANNOT REGISTER LINUX COMPATIBILITY LAYER\n");
	exit(1);
  }

  return 0;

}
