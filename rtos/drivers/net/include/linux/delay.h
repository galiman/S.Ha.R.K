#ifndef __DELAY__
#define __DELAY__

#include <linux/compatib.h>
#define udelay(microsec)	do { int _i = 4*microsec; while (--_i > 0) { __SLOW_DOWN_IO; }} while (0)

#endif
