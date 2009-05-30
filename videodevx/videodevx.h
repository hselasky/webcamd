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

/*
 * The following file describes a cross-platform Video4Linux interface
 * library.
 */

#ifndef _VIDEODEVX_H_
#define	_VIDEODEVX_H_

#include <linux/videodev.h>
#include <linux/videodev2.h>

/*
 * NOTE: "videodevx_open()" returns a handle which is not necessarily
 * a file-handle. Use "videodevx_getfd()" to get a file handle which
 * can be polled.
 */
int	videodevx_open(int unit);
int	videodevx_read(int hdl, void *ptr, size_t len);
int	videodevx_write(int hdl, const void *ptr, size_t len);
int	videodevx_ioctl(int hdl, unsigned long cmd, void *arg);
int	videodevx_mmap(int hdl, void *addr, size_t len, int prot, int flags, int fd, off_t offset);
int	videodevx_getfd(int hdl);
int	videodevx_close(int hdl);
int	videodevx_maxunit(void);

#endif					/* _VIDEODEVX_H_ */
