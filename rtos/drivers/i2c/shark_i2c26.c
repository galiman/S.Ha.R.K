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

/* Glue Layer for Linux PCI 2.6 Driver */
    
#include <kernel/kern.h>

extern int i2c_dev_init(void);
extern int i2c_init(void);

static int i2c_installed = FALSE;

/* Init the Linux I2C 2.6 Driver */
int I2C26_installed(void)
{
	return i2c_installed;
}

int I2C26_init() {

	if (i2c_installed == TRUE)
		return 0;

	i2c_init();

	i2c_dev_init();

	i2c_installed = TRUE;

	return 0;

}
