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

#ifndef _LINUX_THREAD_H_
#define	_LINUX_THREAD_H_

#define	MUTEX_NO_OWNER ((pthread_t)-1UL)

typedef struct task_struct {
	const char *comm;
	pid_t	pid;
} task_struct_t;

typedef struct wait_queue {
} wait_queue_t;

#define	DEFINE_WAIT(name) wait_queue_t name = {}

typedef struct wait_queue_head {
	int	sleep_ref;
	int	sleep_count;
} wait_queue_head_t;

#define	DECLARE_WAIT_QUEUE_HEAD(name) \
	struct wait_queue_head name = { 0, 0 }

typedef struct semaphore {
	int32_t	value;
	pthread_t owner;
} semaphore_t;

#define	DEFINE_SPINLOCK(n) struct spinlock n = { }
#define	DEFINE_MUTEX(n) struct mutex n = { .sem.value = 1, .sem.owner = MUTEX_NO_OWNER, }
struct mutex {
	struct semaphore sem;
};

typedef struct completion {
	uint32_t done;
	wait_queue_head_t wait;
} completion_t;

uint32_t atomic_drop(void);
void	atomic_pickup(uint32_t);
void	init_waitqueue_head(wait_queue_head_t *q);
void	uninit_waitqueue_head(wait_queue_head_t *q);
void	interruptible_sleep_on(wait_queue_head_t *q);
uint64_t interruptible_sleep_on_timeout(wait_queue_head_t *q, uint64_t timeout);
int	waitqueue_active(wait_queue_head_t *q);

void	wake_up(wait_queue_head_t *q);
void	wake_up_all(wait_queue_head_t *q);
void	wake_up_nr(wait_queue_head_t *q, uint32_t nr);
void	__wait_event(wait_queue_head_t *q);
int	__wait_event_timed(wait_queue_head_t *q, struct timespec *ts);
void	__wait_get_timeout(uint64_t timeout, struct timespec *ts);

void	add_wait_queue(wait_queue_head_t *, wait_queue_t *);
void	remove_wait_queue(wait_queue_head_t *, wait_queue_t *);
int	schedule_timeout(long);
int	schedule_timeout_interruptible(long);
void	schedule(void);

#define	wake_up_interruptible(q)        wake_up(q)
#define	wake_up_interruptible_nr(q, nr) wake_up_nr(q,nr)
#define	wake_up_interruptible_all(q)    wake_up_all(q)
#define	wake_up_interruptible_sync(q)   wake_up(q)

#define	wait_event(wq, condition)		\
do {						\
	atomic_lock();				\
	while (!(condition))			\
		__wait_event(&(wq));		\
	atomic_unlock();			\
} while (0)

#define	wait_event_interruptible_exclusive(wq,condition) \
	wait_event_interruptible(wq,condition)

#define	wait_event_interruptible(wq, condition)	\
({						\
	int __ret = 0;				\
	atomic_lock();				\
	while (!(condition)) {			\
	  if (check_signal()) {		\
		__ret = -ERESTARTSYS;		\
		break;				\
	  }					\
	  __wait_event(&(wq));			\
	}					\
	atomic_unlock();			\
	__ret;					\
})

#define	wait_event_timeout(wq, condition, timeout)	\
({							\
	struct timespec ts[2];				\
	long __ret = timeout;				\
	__wait_get_timeout(__ret, ts);			\
	atomic_lock();					\
	while (!(condition)) {				\
	  if (__wait_event_timed(&(wq), ts)) {		\
		__ret = 0;				\
		break;					\
	  }						\
	}						\
	atomic_unlock();				\
	__ret;						\
})

#define	wait_event_interruptible_timeout(wq, condition, timeout) \
({							\
	struct timespec ts[2];				\
	long __ret = timeout;				\
	__wait_get_timeout(__ret, ts);			\
	atomic_lock();					\
	while (!(condition)) {				\
	  if (check_signal()) {			\
		__ret = -ERESTARTSYS;			\
		break;					\
	  }						\
	  if (__wait_event_timed(&(wq), ts)) {		\
		__ret = 0;				\
		break;					\
	  }						\
	}						\
	atomic_unlock();				\
	__ret;						\
})

#define	DECLARE_WAITQUEUE(name, thread) \
	wait_queue_t name = { }

void	sema_init(struct semaphore *, int32_t val);
void	sema_uninit(struct semaphore *sem);

void	up (struct semaphore *);
void	down(struct semaphore *);
int	down_read_trylock(struct semaphore *sem);
int	down_trylock(struct semaphore *sem);
void	poll_wait(struct file *filp, wait_queue_head_t *wq, poll_table * p);

void	mutex_lock(struct mutex *m);
int	mutex_lock_killable(struct mutex *m);
int	mutex_trylock(struct mutex *m);
void	mutex_unlock(struct mutex *m);

#define	mutex_init(m) sema_init(&(m)->sem, 1)
#define	mutex_destroy(m) sema_uninit(&(m)->sem)
#define	mutex_lock_interruptible(m) (mutex_lock(m),0)
#define	mutex_is_locked(x) ({			\
    int __ret;					\
    atomic_lock();				\
    __ret = ((x)->sem.owner != MUTEX_NO_OWNER);	\
    atomic_unlock();				\
    __ret;})
#define	init_MUTEX(s) sema_init(s,1)
#define	init_MUTEX_LOCKED(s) sema_init(s, 0)
#define	down_interruptible(x) (down(x),0)

#define	wait_for_completion_killable(x) wait_for_completion_interruptible(x)

void	init_completion(struct completion *x);

#define	INIT_COMPLETION(x) \
    init_completion(&(x))
void	uninit_completion(struct completion *x);
void	wait_for_completion(struct completion *x);
int	wait_for_completion_interruptible(struct completion *x);
uint64_t wait_for_completion_timeout(struct completion *x, uint64_t timeout);

#define	complete_all(x)	complete(x)
void	complete(struct completion *x);

void	wake_up_process(struct task_struct *task);

#define	current (&linux_task)

typedef int (threadfn_t)(void *data);
struct task_struct *kthread_create(threadfn_t *func, void *data, char *fmt,...);
struct task_struct *kthread_run(threadfn_t *func, void *data, char *fmt,...);
int	kthread_stop(struct task_struct *k);
int	kthread_should_stop(void);

int	try_to_freeze(void);
int	freezing(struct task_struct *p);
void	set_freezable(void);

void	atomic_lock(void);
void	atomic_unlock(void);
pthread_mutex_t *atomic_get_lock();
void	atomic_pre_sleep(void);
void	atomic_post_sleep(void);

int	thread_init(void);
int	thread_got_stopping(void);

void	prepare_to_wait(wait_queue_head_t *, wait_queue_t *, int);
void	finish_wait(wait_queue_head_t *, wait_queue_t *);

extern struct task_struct linux_task;

int	check_signal(void);
void	wake_up_all_internal(void);
void	poll_wakeup_internal(void);

/*
 * RT-mutex variant
 */
#define	rt_mutex mutex
#define	rt_mutex_init(x) mutex_init(x)
#define	rt_mutex_destroy(x) mutex_destroy(x)
#define	rt_mutex_lock(x) mutex_lock(x)
#define	rt_mutex_unlock(x) mutex_unlock(x)
#define	DEFINE_RT_MUTEX(x) DEFINE_MUTEX(x)
#define	rt_mutex_is_locked(x) mutex_is_locked(x)

#endif					/* _LINUX_THREAD_H_ */
