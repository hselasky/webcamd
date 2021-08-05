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

#include <signal.h>

TAILQ_HEAD(timer_head, timer_list);

static struct timer_head timer_head = TAILQ_HEAD_INITIALIZER(timer_head);
static pthread_t timer_thread;
static volatile int timer_thread_started;
static int timer_needed;
static struct timespec timer_last;
static uint32_t timer_nsec_rem;

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

int
mod_timer(struct timer_list *timer, unsigned long expires)
{
	int retval;

	atomic_lock();
	timer->expires = expires;
	retval = (timer->entry.tqe_prev != NULL);
	atomic_unlock();

	return (retval);
}

static void
timer_io(int dummy)
{
}

static void *
timer_exec(void *arg)
{
	uint64_t last_jiffies;
	uint64_t last_check;
	int64_t delta;

	struct timer_list *t;
	uint32_t ms_delay = 0;

	signal(SIGIO, &timer_io);

	timer_thread_started = 1;

	last_check = get_jiffies_64();

	while (1) {

		/* update last_jiffies */
		last_jiffies = get_jiffies_64();

		/* optimise number of external wakeup requests */
		wake_up_inhibit(true);

		atomic_lock();

		delta = last_jiffies - last_check;

		if ((delta >= 1000) || (delta < 0)) {

			last_check = last_jiffies;

			/* make sure signals gets delivered */
			wake_up_all_internal();
		}
		/* compute timeout for next timer check */
		if (TAILQ_FIRST(&timer_head) || timer_needed)
			ms_delay = 25;
		else
			ms_delay = 500;	/* relax it */

restart:
		TAILQ_FOREACH(t, &timer_head, entry) {
			delta = t->expires - last_jiffies;
			if (delta < 0) {
				TAILQ_REMOVE(&timer_head, t, entry);
				t->entry.tqe_prev = NULL;
				atomic_unlock();
				t->function(t);
				atomic_lock();
				goto restart;
			}
		}

		atomic_unlock();

		/* optimise number of external wakeup requests */
		wake_up_inhibit(false);

		usleep(ms_delay * 1000);
	}
	return (NULL);
}

uint64_t
get_jiffies_64(void)
{
	enum {
		DIV = (1000000000 / HZ),
	};
	struct timespec ts;
	int32_t delta;
	uint64_t retval;
	static u64 jiffies64;		/* we use jiffies = milliseconds */

	atomic_lock();

	/* get last time */
	clock_gettime(CLOCK_REALTIME_FAST, &ts);
	delta = ts.tv_nsec - timer_last.tv_nsec;
	timer_last = ts;

	/* compute the delta */
	if (delta < 0)
		delta += 1000000000;
	/* range check */
	if (delta < 0)
		delta = 0;
	if (delta > 999999999)
		delta = 999999999;

	/* update remainder */
	timer_nsec_rem += delta;

	/* update jiffies64 */
	delta = timer_nsec_rem / DIV;
	if (delta > 0) {
		timer_nsec_rem %= DIV;
		jiffies64 += (int64_t)delta;
	}
	retval = jiffies64;
	atomic_unlock();

	return (retval);
}

void
init_timer(struct timer_list *timer)
{
	memset(timer, 0, sizeof(*timer));
}

static int
timer_init(void)
{
	get_jiffies_64();

	if (pthread_create(&timer_thread, NULL, timer_exec, NULL)) {
		printf("Failed creating timer process\n");
	} else {
		while (timer_thread_started == 0)
			schedule();
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

	if (flag && timer_thread_started)
		pthread_kill(timer_thread, SIGIO);
}

module_init(timer_init);
