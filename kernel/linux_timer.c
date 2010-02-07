/*-
 * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static u64 jiffies64;			/* we use jiffies = milliseconds */

#include <signal.h>

TAILQ_HEAD(timer_head, timer_list);

static struct timer_head timer_head;
static pthread_t timer_thread;
static int timer_thread_started;
static int timer_needed;

int
timer_pending(const struct timer_list *timer)
{
	int retval;

	atomic_lock();
	retval = (timer->entry.tqe_prev != NULL);
	atomic_unlock();

	return (retval);
}

void
add_timer(struct timer_list *timer)
{
	del_timer(timer);

	atomic_lock();
	TAILQ_INSERT_TAIL(&timer_head, timer, entry);
	atomic_unlock();
}

int
del_timer(struct timer_list *timer)
{
	int retval;

	atomic_lock();
	if (timer->entry.tqe_prev != NULL) {
		TAILQ_REMOVE(&timer_head, timer, entry);
		timer->entry.tqe_prev = NULL;
		retval = 1;
	} else {
		retval = 0;
	}
	atomic_unlock();
	return (retval);
}

int
del_timer_sync(struct timer_list *timer)
{
	return (del_timer(timer));
}

static void
timer_exec_hup(int dummy)
{

}

static void *
timer_exec(void *arg)
{
	int64_t delta;

#ifdef HAVE_WEBCAMD
	uint64_t last_check = 0;

#endif
	struct timer_list *t;
	uint32_t ms_delay = 0;

	pthread_set_kernel_prio();

	signal(SIGHUP, timer_exec_hup);

	timer_thread_started = 1;

	while (1) {

		atomic_lock();

#ifdef HAVE_WEBCAMD
		delta = jiffies64 - last_check;

		if ((delta >= 1000) || (delta < 0)) {

			last_check = jiffies64;

			check_signal();
		}
#endif
		jiffies64 += ms_delay;	/* ms */

		if (TAILQ_FIRST(&timer_head) || timer_needed)
			ms_delay = 20;
		else
			ms_delay = 1000;/* relax it */

restart:
		TAILQ_FOREACH(t, &timer_head, entry) {
			delta = t->expires - jiffies64;
			if (delta < 0) {
				TAILQ_REMOVE(&timer_head, t, entry);
				t->entry.tqe_prev = NULL;
				atomic_unlock();
				t->function(t->data);
				atomic_lock();
				goto restart;
			}
		}

		atomic_unlock();

		usleep(ms_delay * 1000);
	}
	return (NULL);
}

uint64_t
get_jiffies_64(void)
{
	uint64_t i;

	atomic_lock();
	i = jiffies64;
	atomic_unlock();
	return (i);
}

void
init_timer(struct timer_list *timer)
{
	memset(timer, 0, sizeof(*timer));
}

static int
timer_init(void)
{
	TAILQ_INIT(&timer_head);

	if (pthread_create(&timer_thread, NULL, timer_exec, NULL)) {
		printf("Failed creating timer process\n");
	} else {
		while (timer_thread_started == 0)
			pthread_yield();
	}
	return (0);
}

void
need_timer(int flag)
{
	atomic_lock();
	if (flag) {
		timer_needed++;

		if (timer_needed > 1)
			flag = 0;
	} else {
		timer_needed--;
	}
	atomic_unlock();

	if (flag && (timer_thread != NULL))
		pthread_kill(timer_thread, SIGHUP);
}

module_init(timer_init);
