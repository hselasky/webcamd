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

#ifndef _LINUX_TASK_H_
#define	_LINUX_TASK_H_

struct tasklet_struct;
struct work_struct;
struct workqueue_struct;

typedef void (*work_func_t)(struct work_struct *work);

typedef struct work_struct {
	TAILQ_ENTRY(work_struct) entry;
	work_func_t func;
} work_t;

#define	DECLARE_WORK(name, fn) struct work_struct name = { .func = fn }
#define	to_delayed_work(pwork) ((struct delayed_work *)(pwork))
#define	system_freezable_wq ((struct workqueue_struct *)0)

typedef void (tasklet_func_t)(unsigned long);
typedef void (tasklet_callback_t)(struct tasklet_struct *);

struct tasklet_struct {
	struct work_struct work;	/* must be first */
	tasklet_func_t *func;
	unsigned long data;
};

typedef struct delayed_work {
	struct work_struct work;
	struct timer_list timer;
} delayed_work_t;

typedef struct execute_work {
	struct work_struct work;
} execute_work_t;

#define	from_tasklet(a, b, c) \
	container_of(b, typeof(*(a)), c)

struct rcu_head;
typedef void rcu_func_t (struct rcu_head *);

struct rcu_head {
	struct rcu_head *next;
	rcu_func_t *func;
};

void	INIT_WORK(struct work_struct *work, work_func_t func);
#define	INIT_DEFERRABLE_WORK(...) INIT_DELAYED_WORK(__VA_ARGS__)
void	INIT_DELAYED_WORK(struct delayed_work *work, work_func_t func);
int	schedule_work(struct work_struct *work);
int	queue_delayed_work(struct workqueue_struct *, struct delayed_work *, unsigned long);
int	schedule_delayed_work(struct delayed_work *work, unsigned long delay);
void	destroy_workqueue(struct workqueue_struct *wq);
int	queue_work(struct workqueue_struct *wq, struct work_struct *work);
struct workqueue_struct *create_workqueue(const char *name);
struct workqueue_struct *create_singlethread_workqueue(const char *name);
#define	alloc_workqueue(...) \
	create_workqueue(NULL)
bool	flush_work(struct work_struct *work);
void	flush_workqueue(struct workqueue_struct *wq);
void	flush_scheduled_work(void);
void	cancel_rearming_delayed_work(struct delayed_work *);
void	cancel_delayed_work(struct delayed_work *);
void	cancel_delayed_work_sync(struct delayed_work *);
void	cancel_work(struct work_struct *);
void	cancel_work_sync(struct work_struct *);
void	tasklet_schedule(struct tasklet_struct *t);
void	tasklet_init(struct tasklet_struct *t, tasklet_func_t *, unsigned long data);
void	tasklet_setup(struct tasklet_struct *t, tasklet_callback_t *);
void	tasklet_kill(struct tasklet_struct *t);
void	call_rcu(struct rcu_head *, rcu_func_t *);

#endif					/* _LINUX_TASK_H_ */
