/*-
 * Copyright (c) 2009-2012 Hans Petter Selasky. All rights reserved.
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

static pthread_cond_t sema_cond;
static pthread_mutex_t atomic_mutex;
static volatile uint32_t atomic_recurse;
static pthread_key_t wrapper_key;

struct task_struct linux_task = {
	.comm = "WEBCAMD",
	.pid = 1,
};

void
atomic_pre_sleep(void)
{
	atomic_recurse--;
}

void
atomic_post_sleep(void)
{
	atomic_recurse++;
}

uint32_t
atomic_drop(void)
{
	uint32_t drops = 0;

	while (atomic_recurse > 1) {
		atomic_unlock();
		drops++;
	}
	return (drops);
}

void
atomic_pickup(uint32_t drops)
{
	while (drops--)
		atomic_lock();
}

void
init_waitqueue_head(wait_queue_head_t *q)
{
	memset(q, 0, sizeof(*q));
}

void
uninit_waitqueue_head(wait_queue_head_t *q)
{
}

void
interruptible_sleep_on(wait_queue_head_t *q)
{
	int ref;

	atomic_lock();
	ref = q->sleep_ref;
	atomic_unlock();

	wait_event(*q, (q->sleep_ref != ref));
}

uint64_t
interruptible_sleep_on_timeout(wait_queue_head_t *q, uint64_t timeout)
{
	int ref;

	atomic_lock();
	ref = q->sleep_ref;
	atomic_unlock();

	timeout = wait_event_timeout(*q, (q->sleep_ref != ref), timeout);
	return (timeout);
}

int
waitqueue_active(wait_queue_head_t *q)
{
	int count;

	atomic_lock();
	count = q->sleep_count;
	atomic_unlock();
	return (count != 0);
}

void
wake_up_all_internal(void)
{
	atomic_lock();
	pthread_cond_broadcast(&sema_cond);
	atomic_unlock();

	poll_wakeup_internal();
}

void
wake_up(wait_queue_head_t *q)
{
	atomic_lock();
	q->sleep_ref++;
	pthread_cond_broadcast(&sema_cond);
	atomic_unlock();

	poll_wakeup_internal();
}

void
wake_up_all(wait_queue_head_t *q)
{
	atomic_lock();
	q->sleep_ref++;
	pthread_cond_broadcast(&sema_cond);
	atomic_unlock();

	poll_wakeup_internal();
}

void
wake_up_nr(wait_queue_head_t *q, uint32_t nr)
{
	atomic_lock();
	q->sleep_ref++;
	pthread_cond_broadcast(&sema_cond);
	atomic_unlock();

	poll_wakeup_internal();
}

void
__wait_event(wait_queue_head_t *q)
{
	uint32_t drops;

	drops = atomic_drop();
	atomic_pre_sleep();

	q->sleep_count++;

	pthread_cond_wait(&sema_cond, atomic_get_lock());

	q->sleep_count--;

	atomic_post_sleep();
	atomic_pickup(drops);
}

void
__wait_get_timeout(uint64_t timeout, struct timespec *ts)
{
	timeout++;

	timeout = (1000000000ULL / HZ) * (uint64_t)timeout;

	ts[0].tv_nsec = timeout % 1000000000ULL;
	ts[0].tv_sec = timeout / 1000000000ULL;

	clock_gettime(CLOCK_MONOTONIC, ts + 1);

	ts[0].tv_sec += ts[1].tv_sec;
	ts[0].tv_nsec += ts[1].tv_nsec;

	if (ts[0].tv_nsec >= 1000000000) {
		ts[0].tv_sec++;
		ts[0].tv_nsec -= 1000000000;
	}
}

int
__wait_event_timed(wait_queue_head_t *q, struct timespec *ts)
{
	int err;
	uint32_t drops;

	drops = atomic_drop();
	atomic_pre_sleep();

	q->sleep_count++;

	err = pthread_cond_timedwait(&sema_cond, atomic_get_lock(), ts);

	q->sleep_count--;

	atomic_post_sleep();
	atomic_pickup(drops);

	return (err == ETIMEDOUT);
}

void
add_wait_queue(wait_queue_head_t *qh, wait_queue_t *wq)
{
	/* NOP */
}

void
remove_wait_queue(wait_queue_head_t *qh, wait_queue_t *wq)
{
	/* NOP */
}

int
schedule_timeout(long timeout)
{
	uint32_t drops;

	atomic_lock();
	drops = atomic_drop();
	atomic_unlock();

	usleep(1000 * timeout);

	atomic_lock();
	atomic_pickup(drops);
	atomic_unlock();

	return (0);
}

int
schedule_timeout_interruptible(long timeout)
{
	return (schedule_timeout(timeout));
}

void
schedule(void)
{
	schedule_timeout(4);
}

void
sema_init(struct semaphore *sem, int32_t value)
{
	memset(sem, 0, sizeof(*sem));
	sem->value = value;
	sem->owner = MUTEX_NO_OWNER;
}

void
sema_uninit(struct semaphore *sem)
{
}

void
up(struct semaphore *sem)
{
	atomic_lock();
	sem->value++;
	if (sem->value == 1)
		pthread_cond_broadcast(&sema_cond);
	atomic_unlock();
}

int
down_read_trylock(struct semaphore *sem)
{
	int ret;

	atomic_lock();
	if (sem->value > 0) {
		down(sem);
		ret = 1;		/* success */
	} else {
		ret = 0;		/* congested */
	}
	atomic_unlock();
	return (ret);
}

int
down_trylock(struct semaphore *sem)
{
	int ret;

	atomic_lock();
	if (sem->value > 0) {
		down(sem);
		ret = 1;		/* success */
	} else {
		ret = 0;		/* congested */
	}
	atomic_unlock();
	return (ret);
}

void
down(struct semaphore *sem)
{
	atomic_lock();
	while (sem->value <= 0) {
		uint32_t drops;

		drops = atomic_drop();
		atomic_pre_sleep();
		pthread_cond_wait(&sema_cond, atomic_get_lock());
		atomic_post_sleep();
		atomic_pickup(drops);
	}
	sem->value--;
	atomic_unlock();
}

void
poll_wait(struct file *filp, wait_queue_head_t *wq, poll_table * p)
{
	if (p && wq) {
		/* XXX register file in polling table */
	}
}

void
init_completion(struct completion *x)
{
	x->done = 0;
	init_waitqueue_head(&x->wait);
}

void
uninit_completion(struct completion *x)
{
	uninit_waitqueue_head(&x->wait);
}

int
wait_for_completion_interruptible(struct completion *x)
{
	int ret = 0;

	atomic_lock();
	while (x->done == 0) {
		if (check_signal()) {
			ret = -ERESTARTSYS;
			break;
		}
		__wait_event(&x->wait);
	}
	if (ret == 0)
		x->done--;
	atomic_unlock();

	return (ret);
}

void
wait_for_completion(struct completion *x)
{
	atomic_lock();
	while (x->done == 0) {
		__wait_event(&x->wait);
	}
	x->done--;
	atomic_unlock();
}

uint64_t
wait_for_completion_timeout(struct completion *x,
    uint64_t timeout)
{
	struct timespec ts[2];
	uint64_t ret = timeout;

	__wait_get_timeout(ret, ts);

	atomic_lock();
	while (1) {
		if (x->done != 0) {
			x->done--;
			break;
		}
		if (__wait_event_timed(&x->wait, ts)) {
			ret = 0;
			break;
		}
	}
	atomic_unlock();

	return (ret);
}

void
complete(struct completion *x)
{
	atomic_lock();
	x->done++;
	pthread_cond_broadcast(&sema_cond);
	atomic_unlock();
}

struct funcdata {
	threadfn_t *volatile func;
	void   *volatile data;
};

struct thread_wrapper {
	uint32_t stopping;
};

static void
thread_urg(int dummy)
{
	struct thread_wrapper *pw;

	pw = pthread_getspecific(wrapper_key);
	if (pw != NULL)
		pw->stopping = 1;
}

int
thread_got_stopping(void)
{
	struct thread_wrapper *pw;

	pw = pthread_getspecific(wrapper_key);
	if (pw != NULL) {
		if (pw->stopping)
			return (0);
	}
	return (EINVAL);
}

static void *
kthread_wrapper(void *arg)
{
	struct funcdata fd = *(struct funcdata *)arg;
	struct thread_wrapper wrapper = {0};

	pthread_setspecific(wrapper_key, &wrapper);

	signal(SIGURG, thread_urg);

	pthread_mutex_lock(&atomic_mutex);
	((struct funcdata *)arg)->func = NULL;
	pthread_cond_broadcast(&sema_cond);
	pthread_mutex_unlock(&atomic_mutex);

	fd.func(fd.data);

	pthread_setspecific(wrapper_key, NULL);

	pthread_exit(NULL);
}

void
wake_up_process(struct task_struct *task)
{
	/* Not implemented */
}

struct task_struct *
kthread_create(threadfn_t *func, void *data, char *fmt,...)
{
	/* Assume that "wake_up_process()" is called immediately */
	return (kthread_run(func, data, ""));
}

struct task_struct *
kthread_run(threadfn_t *func, void *data, char *fmt,...)
{
	pthread_t ptd;
	struct funcdata *fd = malloc(sizeof(*fd));

	if (fd == NULL)
		return (ERR_PTR(-ENOMEM));

	fd->func = func;
	fd->data = data;

	if (pthread_create(&ptd, NULL, kthread_wrapper, fd)) {
		free(fd);
		return (ERR_PTR(-ENOMEM));
	}
	pthread_mutex_lock(&atomic_mutex);
	while (fd->func != NULL)
		pthread_cond_wait(&sema_cond, &atomic_mutex);
	pthread_mutex_unlock(&atomic_mutex);

	free(fd);
	return ((struct task_struct *)ptd);
}

int
kthread_stop(struct task_struct *k)
{
	pthread_t ptd = (pthread_t)k;

	pthread_kill(ptd, SIGURG);
	pthread_join(ptd, NULL);

	return (0);
}

int
kthread_should_stop(void)
{
	return (thread_got_stopping() == 0);
}

int
try_to_freeze(void)
{
	return (0);
}

int
freezing(struct task_struct *p)
{
	return (0);
}

void
set_freezable(void)
{

}

void
atomic_lock(void)
{
	pthread_mutex_lock(&atomic_mutex);
	atomic_recurse++;
}

void
atomic_unlock(void)
{
	atomic_recurse--;
	pthread_mutex_unlock(&atomic_mutex);
}

pthread_mutex_t *
atomic_get_lock(void)
{
	return (&atomic_mutex);
}

int
thread_init(void)
{
	pthread_mutexattr_t mattr;
	pthread_condattr_t cattr;

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&atomic_mutex, &mattr);
	pthread_mutexattr_destroy(&mattr);

	pthread_condattr_init(&cattr);
	pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC);
	pthread_cond_init(&sema_cond, &cattr);
	pthread_condattr_destroy(&cattr);

	pthread_key_create(&wrapper_key, NULL);

	return (0);
}

void
thread_exit(void)
{
	pthread_cond_destroy(&sema_cond);
}

void
prepare_to_wait(wait_queue_head_t *qh, wait_queue_t *wait, int flag)
{

}

void
finish_wait(wait_queue_head_t *qh, wait_queue_t *wait)
{

}

int
mutex_trylock(struct mutex *m)
{
	pthread_t self;
	int retval = 1;

	self = pthread_self();

	atomic_lock();
	/* check for recursive locking first */
	if (m->sem.owner == self) {
		m->sem.value--;
	} else if (m->sem.owner == MUTEX_NO_OWNER) {
		down(&m->sem);
		m->sem.owner = self;
	} else {
		retval = 0;
	}
	atomic_unlock();

	return (retval);
}

void
mutex_lock(struct mutex *m)
{
	pthread_t self;

	self = pthread_self();

	atomic_lock();
	/* check for recursive locking first */
	if (m->sem.owner == self) {
		m->sem.value--;
	} else {
		down(&m->sem);
		m->sem.owner = self;
	}
	atomic_unlock();
}

int
mutex_lock_killable(struct mutex *m)
{
	mutex_lock(m);
	return (0);
}

void
mutex_unlock(struct mutex *m)
{
	atomic_lock();
	up(&m->sem);
	if (m->sem.value > 0) {
		/* clear owner variable */
		m->sem.owner = MUTEX_NO_OWNER;
		/* guard against double unlock */
		m->sem.value = 1;
	}
	atomic_unlock();
}
