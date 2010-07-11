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

#ifndef _LINUX_FILE_H_
#define	_LINUX_FILE_H_

struct cdev_handle *linux_open(int f_v4b, int fflags);
int	linux_close(struct cdev_handle *);
int	linux_ioctl(struct cdev_handle *, int fflags, unsigned int cmd, void *arg);
ssize_t	linux_read(struct cdev_handle *, int fflags, char *ptr, size_t len);
ssize_t	linux_write(struct cdev_handle *, int fflags, char *ptr, size_t len);
void   *linux_mmap(struct cdev_handle *, int fflags, uint8_t *addr, size_t len, off_t offset);
int	linux_poll(struct cdev_handle *handle);
int	linux_get_user_pages(unsigned long start, int npages, int write, int force, struct page **ppages, struct vm_area_struct **pvm);

struct cdev_handle *get_current_cdev_handle(void);

#endif					/* _LINUX_FILE_H_ */
