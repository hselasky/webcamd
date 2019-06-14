/*-
 * Copyright (c) 2019 Damjan Jovanovic. All rights reserved.
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

#ifndef _LINUX_COMPAT32_H_
#define	_LINUX_COMPAT32_H_

/*
 * Relevant Linux kernel defines from include/linux/compat.h
 * and the 369 other header files it includes.
 */

#define	COMPAT_USE_64BIT_TIME 0

typedef u32 compat_uint_t;

typedef s32 compat_long_t;
typedef u32 compat_ulong_t;

typedef u32 compat_caddr_t;
typedef s32 compat_time_t;
typedef u32 compat_uptr_t;

typedef	s64 __attribute__((aligned(4))) compat_s64;
typedef u64 __attribute__((aligned(4))) compat_u64;

struct compat_timespec {
	compat_time_t tv_sec;
	s32	tv_nsec;
};

struct compat_timeval {
	compat_time_t tv_sec;
	s32	tv_usec;
};

static inline int
compat_put_timespec(const struct timespec *kts, struct compat_timespec *uts)
{
	int err;

	if (copy_to_user(uts, kts, sizeof(struct timespec)) == 0)
		err = 0;
	else
		err = -EFAULT;

	return (err);
}

static inline void *
compat_ptr(compat_uptr_t ptr)
{
	return (void *)(u64) ptr;
}

static inline compat_uptr_t
ptr_to_compat(void *uptr)
{
	return (u32)(u64)uptr;
}

extern bool in_compat_syscall(void);

/*
 * Allocate zeroed memory which is automatically freed when the compat
 * ioctl handler returns:
 */
extern void *compat_zalloc_tls_space(unsigned long len);

/* Frees everything allocated by one or more calls to the above */
extern void compat_free_all_tls_space(void);

typedef int mm_segment_t;

#define	KERNEL_DS 1

extern int get_fs();
extern void set_fs(int);

/* From arch/x86/include/asm/uaccess.h
 * but never used anywhere except in 32 bit compatibility code.
 */

#define	access_ok(type,addr,size) 1
#define	VERIFY_READ 0
#define	VERIFY_WRITE 1

#endif					/* _LINUX_COMPAT32_H_ */
