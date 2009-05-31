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

typedef struct task_struct {

} task_struct_t;

typedef struct wait_queue_head {
	pthread_cond_t cond;
} wait_queue_head_t;

typedef struct semaphore {
	pthread_cond_t cond;
	int32_t	value;
} semaphore_t;

typedef struct completion {
	uint32_t done;
	wait_queue_head_t wait;
} completion_t;

void	init_waitqueue_head(wait_queue_head_t *q);
void	uninit_waitqueue_head(wait_queue_head_t *q);
void	wake_up(wait_queue_head_t *q);
void	wake_up_all(wait_queue_head_t *q);
void	wake_up_nr(wait_queue_head_t *q, uint32_t nr);
void	__wait_event(wait_queue_head_t *q);
uint64_t __wait_event_timed(wait_queue_head_t *q, uint64_t timeout);

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

#define	wait_event_interruptible(wq, condition)	\
({						\
	atomic_lock();				\
	while (!(condition))			\
		__wait_event(&(wq));		\
	atomic_unlock();			\
	0;					\
})

#define	wait_event_interruptible_timeout(wq, condition, timeout)        \
({                                                                      \
	uint64_t __ret = timeout;					\
	atomic_lock();							\
	while (!(condition))						\
		__ret -= __wait_event_timed(&(wq), __ret);		\
	atomic_unlock();						\
	__ret;								\
})

#define	wait_event_timeout(wq, condition, timeout)		\
({								\
	uint64_t __ret = timeout;				\
	atomic_lock();						\
	while (!(condition))					\
		__ret -= __wait_event_timed(&(wq), __ret);	\
	atomic_unlock();					\
	__ret;							\
})

void	sema_init(struct semaphore *, int32_t val);
void	sema_uninit(struct semaphore *sem);

void	up (struct semaphore *);
void	down(struct semaphore *);
void	poll_wait(struct file *filp, wait_queue_head_t *wq, poll_table * p);

#define	wait_for_completion_interruptible(x) wait_for_completion(x)
#define	wait_for_completion_killable(x) wait_for_completion(x)

void	init_completion(struct completion *x);
void	uninit_completion(struct completion *x);
void	wait_for_completion(struct completion *x);
uint64_t wait_for_completion_timeout(struct completion *x, uint64_t timeout);
void	complete(struct completion *x);
void	schedule(void);

typedef int (threadfn_t)(void *data);
struct task_struct *kthread_run(threadfn_t *func, void *data, char *fmt,...);
int	kthread_stop(struct task_struct *k);
int	kthread_should_stop(void);

int	try_to_freeze(void);
int	freezing(struct task_struct *p);
void	set_freezable(void);

#endif					/* _LINUX_THREAD_H_ */
