/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   (see the web pages for full authors list)
 *
 * ReTiS Lab (Scuola Superiore S.Anna - Pisa - Italy)
 *
 * http://www.sssup.it
 * http://retis.sssup.it
 * http://shark.sssup.it
 */

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


#ifndef __MPEGSTAR_H__
#define __MPEGSTAR_H__

#include <ll/ll.h>
#include <kernel/config.h>
#include <sys/types.h>
#include <kernel/types.h>

LEVEL MPEGSTAR_register_level(int master);

int MPEGSTAR_setbudget(LEVEL l, PID p, int budget);
int MPEGSTAR_getbudget(LEVEL l, PID p);                                                                        
int MPEGSTAR_budget_has_thread(LEVEL l, int budget);

int MPEGSTAR_rescale(int budget, TIME Q, TIME T);
int MPEGSTAR_get_remain_capacity(int budget);
int MPEGSTAR_get_last_reclaiming(int budget);
int MPEGSTAR_is_frame_skipped(int budget);

#endif

