/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

/*
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


#ifndef __ELASTIC_H__
#define __ELASTIC_H__

#include <kernel/config.h>
#include <kernel/types.h>

__BEGIN_DECLS

/*+ flags... +*/
#define ELASTIC_DISABLE_ALL           0  /*+ Task Guarantee enabled +*/
#define ELASTIC_ENABLE_GUARANTEE      1  /*+ Task Guarantee enabled +*/
#define ELASTIC_ENABLE_ALL            1

/* C Scaling factor define */
#define SCALING_UNIT 10000

LEVEL ELASTIC_register_level(int flags, LEVEL master, ext_bandwidth_t U);

int ELASTIC_set_period(PID p, TIME period);
int ELASTIC_get_period(PID p);

int ELASTIC_set_Tmin(PID p, TIME Tmin);
int ELASTIC_get_Tmin(PID p);

int ELASTIC_set_Tmax(PID p, TIME Tmax);
int ELASTIC_get_Tmax(PID p);

int ELASTIC_set_C(PID p, TIME C);
int ELASTIC_get_C(PID p);

int ELASTIC_set_E(PID p, int E);
int ELASTIC_get_E(PID p);

int ELASTIC_set_beta(PID p, int beta);
int ELASTIC_get_beta(PID p);

int ELASTIC_set_bandwidth(LEVEL lev, ext_bandwidth_t);
ext_bandwidth_t ELASTIC_get_bandwidth(LEVEL lev);

int ELASTIC_set_scaling_factor(LEVEL level, int scaling_factor);
int ELASTIC_get_scaling_factor(LEVEL level);

__END_DECLS
#endif
