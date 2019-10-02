/*
 * Project: S.Ha.R.K.
 *
 * Coordinators: 
 *   Giorgio Buttazzo    <giorgio@sssup.it>
 *   Paolo Gai           <pj@gandalf.sssup.it>
 *
 * Authors     : 
 *   Massimiliano Giorgi <massy@gandalf.sssup.it>
 *   (see the web pages for full authors list)
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

#include <ll/sys/types.h>
#include <ll/stdlib.h>

#include <kernel/log.h>

#include <types.h>
#include <trace.h>
#include <queues.h>

static trc_event_t *dummy_queue_get(void *unused)
{
  static trc_event_t event;
  return &event;
}

static int dummy_queue_post(void *unused)
{
  return 0;
}

static int create_dummy_queue(trc_queue_t *queue, void *unused)
{
  queue->get=dummy_queue_get;
  queue->post=dummy_queue_post;
  queue->data=NULL;
  return 0;
}

static int terminate_dummy_queue(void *unused)
{
  return 0;
}

static int activate_dummy_queue(void *unused, int unused2)
{
  return 0;
}

int trc_register_dummy_queue(void)
{
  int res;
  res=trc_register_queuetype(TRC_DUMMY_QUEUE,
			     create_dummy_queue,
			     activate_dummy_queue,
			     terminate_dummy_queue);
  if (res!=0) printk(KERN_WARNING "can't register tracer dummy queue");
  return res;
}
