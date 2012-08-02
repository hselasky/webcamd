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

TAILQ_HEAD(work_head, work_struct);

static struct work_struct *work_curr;
static struct work_head work_head = TAILQ_HEAD_INITIALIZER(work_head);
static pthread_t work_thread;
static pthread_cond_t work_cond;

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

void
flush_workqueue(struct workqueue_struct *wq)
{

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
	while (work == work_curr)
		schedule();
	atomic_unlock();
}

void
flush_scheduled_work(void)
{
	usleep(100000);
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
