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

#include <linux/rcupdate.h>

TAILQ_HEAD(work_head, work_struct);

static struct work_struct *work_curr;
static struct work_head work_head = TAILQ_HEAD_INITIALIZER(work_head);
static pthread_t work_thread;
static pthread_cond_t work_cond;
static int flush_work_var;

int
schedule_work(struct work_struct *work)
{
	int retval;

	atomic_lock();
	if (work->entry.tqe_prev == NULL) {
		TAILQ_INSERT_TAIL(&work_head, work, entry);
		pthread_cond_signal(&work_cond);
		retval = 1;
	} else {
		retval = 0;
	}
	atomic_unlock();
	return (retval);
}

static void
delayed_work_timer_fn(unsigned long __data)
{
	struct work_struct *work =
	(struct work_struct *)__data;

	schedule_work(work);
}

int
queue_delayed_work(struct workqueue_struct *dummy,
    struct delayed_work *pwork, unsigned long delay)
{
	return (schedule_delayed_work(pwork, delay));
}

int
schedule_delayed_work(struct delayed_work *work, unsigned long delay)
{
	int retval;

	if (delay == 0)
		return (schedule_work(&work->work));

	if (timer_pending(&work->timer)) {
		retval = 0;
	} else {
		retval = 1;
	}

	if (retval) {
		work->timer.data = (long)&work->work;
		work->timer.expires = jiffies + delay;
		work->timer.function = delayed_work_timer_fn;
		add_timer(&work->timer);
	}
	return (retval);
}

void
INIT_WORK(struct work_struct *work, work_func_t func)
{
	memset(work, 0, sizeof(*work));
	work->func = func;
}

void
INIT_DELAYED_WORK(struct delayed_work *work, work_func_t func)
{
	memset(work, 0, sizeof(*work));
	work->work.func = func;
}

static void *
work_exec(void *arg)
{
	struct work_struct *t;

	atomic_lock();
	while (1) {
		t = TAILQ_FIRST(&work_head);
		if (t != NULL) {
			TAILQ_REMOVE(&work_head, t, entry);
			t->entry.tqe_prev = NULL;
			work_curr = t;
			atomic_unlock();
			t->func(t);
			atomic_lock();
			work_curr = NULL;
		} else {
			flush_work_var = 0;
			atomic_pre_sleep();
			pthread_cond_wait(&work_cond, atomic_get_lock());
			atomic_post_sleep();
		}
	}
	atomic_unlock();
	return (NULL);
}

int
queue_work(struct workqueue_struct *wq, struct work_struct *work)
{
	return (schedule_work(work));
}

bool
flush_work(struct work_struct *work)
{
	bool retval;

	atomic_lock();
	retval = (work->entry.tqe_prev != NULL);
	while (work->entry.tqe_prev != NULL)
		schedule();
	atomic_unlock();

	return (retval);
}

void
flush_workqueue(struct workqueue_struct *wq)
{
	flush_scheduled_work();
}

void
cancel_delayed_work(struct delayed_work *_work)
{
	cancel_work(&_work->work);
}

void
cancel_delayed_work_sync(struct delayed_work *_work)
{
	cancel_work(&_work->work);

	cancel_work_sync(&_work->work);
}

void
cancel_rearming_delayed_work(struct delayed_work *_work)
{
	cancel_work(&_work->work);
}

void
cancel_work(struct work_struct *work)
{
	atomic_lock();
	if (work->entry.tqe_prev != NULL) {
		TAILQ_REMOVE(&work_head, work, entry);
		work->entry.tqe_prev = NULL;
	}
	atomic_unlock();
}

void
cancel_work_sync(struct work_struct *work)
{
	atomic_lock();
	if (work->entry.tqe_prev != NULL) {
		TAILQ_REMOVE(&work_head, work, entry);
		work->entry.tqe_prev = NULL;
	}
	while (work == work_curr)
		schedule();
	atomic_unlock();
}

void
flush_scheduled_work(void)
{
	uint32_t drops;

	atomic_lock();
	flush_work_var = 1;
	while (1) {
		pthread_cond_signal(&work_cond);
		if (flush_work_var == 0)
			break;
		drops = atomic_drop();
		atomic_unlock();
		usleep(10000);
		atomic_lock();
		atomic_pickup(drops);
	}
	atomic_unlock();
}

void
destroy_workqueue(struct workqueue_struct *wq)
{

}

struct workqueue_struct *
create_workqueue(const char *name)
{
	/* TODO: we currently reuse the existing thread */
	return ((struct workqueue_struct *)1);
}

struct workqueue_struct *
create_singlethread_workqueue(const char *name)
{
	/* TODO: we currently reuse the existing thread */
	return ((struct workqueue_struct *)1);
}

static int
work_init(void)
{
	pthread_cond_init(&work_cond, NULL);

	if (pthread_create(&work_thread, NULL, work_exec, NULL)) {
		printf("Failed creating work process\n");
	}
	return (0);
}

module_init(work_init);

static void
tasklet_wrapper_callback(struct work_struct *work)
{
	struct tasklet_struct *task =
	(struct tasklet_struct *)work;

	(task->func) (task->data);
}

void
tasklet_schedule(struct tasklet_struct *t)
{
	schedule_work(&t->work);
}

void
tasklet_init(struct tasklet_struct *t, tasklet_func_t *func,
    unsigned long data)
{
	INIT_WORK(&t->work, tasklet_wrapper_callback);

	t->func = func;
	t->data = data;
}

void
tasklet_kill(struct tasklet_struct *t)
{
	atomic_lock();
	if (t->work.entry.tqe_prev != NULL) {
		TAILQ_REMOVE(&work_head, &t->work, entry);
		t->work.entry.tqe_prev = NULL;
	}
	atomic_unlock();
}

static pthread_t rcu_thread;
static pthread_cond_t rcu_cond;
static struct rcu_head *rcu_head;

static void *
rcu_exec(void *arg)
{
	struct rcu_head *t;

	atomic_lock();
	while (1) {
		t = rcu_head;
		if (t != NULL) {
			rcu_head = t->next;
			t->next = NULL;
			atomic_unlock();
			t->func(t);
			atomic_lock();
		} else {
			atomic_pre_sleep();
			pthread_cond_wait(&rcu_cond, atomic_get_lock());
			atomic_post_sleep();
		}
	}
	atomic_unlock();
	return (NULL);
}

static int
rcu_init(void)
{
	pthread_cond_init(&rcu_cond, NULL);

	if (pthread_create(&rcu_thread, NULL, rcu_exec, NULL)) {
		printf("Failed creating RCU process\n");
	}
	return (0);
}

module_init(rcu_init);

void
call_rcu(struct rcu_head *head, rcu_func_t *func)
{
	atomic_lock();
	if (head->next == NULL) {
		head->next = rcu_head;
		head->func = func;
		rcu_head = head;
		pthread_cond_signal(&rcu_cond);
	}
	atomic_unlock();
}
