/*-
 * Copyright (c) 2019 Damjan Jovanovic. All rights reserved.
 * Copyright (c) 2019 Hans Petter Selasky. All rights reserved.
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

#ifdef HAVE_CUSE
#include <cuse.h>
#else
#include <cuse4bsd.h>
#endif

struct tls_memory {
	void   *memory;
	SLIST_ENTRY(tls_memory) entry;
};

SLIST_HEAD(tls_memory_head, tls_memory);

static __thread struct tls_memory_head tls_memory_list =
    SLIST_HEAD_INITIALIZER(tls_memory_head);

void *
compat_zalloc_tls_space(unsigned long len)
{
	struct tls_memory *tls_mem;

	tls_mem = malloc(sizeof(struct tls_memory));
	if (tls_mem != NULL) {
		tls_mem->memory = malloc(len);
		if (tls_mem->memory != NULL) {
			memset(tls_mem->memory, 0, len);
			SLIST_INSERT_HEAD(&tls_memory_list, tls_mem, entry);
			return (tls_mem->memory);
		} else
			free(tls_mem);
	}
	return (NULL);
}

void
compat_free_all_tls_space(void)
{
	struct tls_memory *tls_mem;

	while ((tls_mem = SLIST_FIRST(&tls_memory_list)) != NULL) {
		SLIST_REMOVE_HEAD(&tls_memory_list, entry);
		free(tls_mem->memory);
		free(tls_mem);
	}
}

int
get_fs()
{
	return (cuse_get_local());
}

void
set_fs(int ds)
{
	cuse_set_local(ds);
}
