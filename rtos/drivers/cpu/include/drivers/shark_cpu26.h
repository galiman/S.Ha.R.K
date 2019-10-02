/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Mauro Marinoni      <mauro.marinoni@unipv.it>
 *
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

/* Glue Layer Header Linux CPU Driver*/

#ifndef __SHARK_CPU26_H__
#define __SHARK_CPU26_H__

#define DVS_NONE		0
#define DVS_POWERNOW_K6		1
#define DVS_POWERNOW_K7		2
#define DVS_POWERNOW_K8		3
#define DVS_MEDIAGX_GEODE	4
#define DVS_P4_CLOCK_MOD	5
#define DVS_SS_CENTRINO		6
#define DVS_SS_ICH		7
#define DVS_SS_SMI		8

#define DVS_RELATION_L		0  /* lowest frequency at or above target */
#define DVS_RELATION_H		1  /* highest frequency below or at target */

#define DVS_LATENCY_ETERNAL	(-1)

#define DVS_MAX_NUM_FREQS	50

extern int cpu26_freqs[DVS_MAX_NUM_FREQS];

int CPU26_installed(void);
int CPU26_init(void);
int CPU26_close(void);

void CPU26_showinfo(void);

int CPU26_DVS_init(void);
int CPU26_DVS_close(void);
int CPU26_DVS_installed(void);

int CPU26_get_latency(void);
int CPU26_set_frequency(unsigned int target_freq, unsigned int relation);
int CPU26_get_cur_frequency(void);
int CPU26_get_min_frequency(void);
int CPU26_get_max_frequency(void);
int CPU26_get_frequencies(int *buf);
int CPU26_show_frequencies(char *buf);

int CPU26_set_suspend_modulation(unsigned char on_duration, unsigned char off_duration);

#endif
 
