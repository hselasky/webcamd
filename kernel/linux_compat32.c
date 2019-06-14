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
#include <fs/cuse/cuse_ioctl.h>
#else
#define	HAVE_CUSE_IOCTL
#include <cuse4bsd.h>
#endif

#define	WEBCAMD_USER_ALLOC_MAX 4096UL
#define	WEBCAMD_USER_ALLOC_ADDR (CUSE_BUF_MAX_PTR - WEBCAMD_USER_ALLOC_MAX)

#if ((CUSE_BUF_MAX_PTR - CUSE_BUF_MIN_PTR) / 2) < WEBCAMD_USER_ALLOC_MAX
#error "The CUSE IOCTL allocation window is too small!"
#endif

struct tls_memory {
	void *ptr;
	unsigned long offset;
};

static __thread struct tls_memory linux_tls_memory = {};

void *
compat_alloc_user_space(unsigned long len)
{
	unsigned long rem;
	void *ptr;

	if (in_compat_syscall() == 0 ||
	    len > WEBCAMD_USER_ALLOC_MAX)
		return (NULL);

	if (linux_tls_memory.ptr == NULL) {
		linux_tls_memory.ptr = malloc(WEBCAMD_USER_ALLOC_MAX);
		linux_tls_memory.offset = 0;
		if (linux_tls_memory.ptr == NULL)
			return (NULL);
	}

	rem = WEBCAMD_USER_ALLOC_MAX - linux_tls_memory.offset;
	if (len > rem)
		return (NULL);

	/* zero memory */
	memset((uint8_t *)linux_tls_memory.ptr + linux_tls_memory.offset, 0, len);

	/* compute fake user-space pointer */
	ptr = (void *)(linux_tls_memory.offset + WEBCAMD_USER_ALLOC_ADDR);

	/* allocate the memory */
	linux_tls_memory.offset += len;

	return (ptr);
}

void
compat_free_all_user_space(void)
{

	free(linux_tls_memory.ptr);
	linux_tls_memory.ptr = NULL;
}

int
compat_copy_to_user(void *to, const void *from, unsigned long len)
{

	if (len > WEBCAMD_USER_ALLOC_MAX ||
	    (uintptr_t)to < WEBCAMD_USER_ALLOC_ADDR ||
	    ((uintptr_t)to + len) > (WEBCAMD_USER_ALLOC_ADDR + WEBCAMD_USER_ALLOC_MAX) ||
	    linux_tls_memory.ptr == NULL)
		return (-ERANGE);

	memcpy((void *)((uintptr_t)linux_tls_memory.ptr +
			(uintptr_t)to - WEBCAMD_USER_ALLOC_ADDR), from, len);
	return (0);

}

int
compat_copy_from_user(void *to, const void *from, unsigned long len)
{

	if (len > WEBCAMD_USER_ALLOC_MAX ||
	    (uintptr_t)from < WEBCAMD_USER_ALLOC_ADDR ||
	    ((uintptr_t)from + len) > (WEBCAMD_USER_ALLOC_ADDR + WEBCAMD_USER_ALLOC_MAX) ||
	    linux_tls_memory.ptr == NULL)
		return (-ERANGE);

	memcpy(to,
	       (const void *)((uintptr_t)linux_tls_memory.ptr +
			      (uintptr_t)from - WEBCAMD_USER_ALLOC_ADDR), len);
	return (0);
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
