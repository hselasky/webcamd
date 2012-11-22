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

#include <linux/idr.h>

extern struct module_init_struct __start_linux_parm_mod;
extern struct module_init_struct __start_linux_init_mod;
extern struct module_exit_struct __start_linux_exit_mod;

void
linux_parm(void)
{
	struct module_init_struct *t = &__start_linux_parm_mod;

	while (t->magic == MODULE_PARM_MAGIC) {
		t->func();
		t++;
	}
}

void
linux_init(void)
{
	struct module_init_struct *t = &__start_linux_init_mod;

	thread_init();

	idr_init_cache();

	while (t->magic == MODULE_INIT_MAGIC) {
		t->func();
		t++;
	}
}

void
linux_exit(void)
{
	struct module_exit_struct *t = &__start_linux_exit_mod;

	while (t->magic == MODULE_EXIT_MAGIC) {
		t->func();
		t++;
	}
}
