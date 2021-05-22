/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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

#ifndef _WEBCAMD_GLOBAL_H_
#define	_WEBCAMD_GLOBAL_H_

#include <build/config.h>

#include <machine/param.h>

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/ctype.h>
#include <sys/ioccom.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/limits.h>

#include <netinet/in.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <syslog.h>

#include <libusb20.h>
#include <libusb20_desc.h>

#include <kernel/linux_defs.h>
#include <kernel/linux_section.h>
#include <kernel/linux_struct.h>
#include <kernel/linux_file.h>
#include <kernel/linux_func.h>
#include <kernel/linux_list.h>
#include <kernel/linux_timer.h>
#include <kernel/linux_task.h>
#include <kernel/linux_thread.h>
#include <kernel/linux_usb.h>
#include <kernel/linux_firmware.h>
#include <kernel/linux_mod_param.h>
#include <kernel/linux_compat32.h>
#include <kernel/linux_radix.h>
#include <kernel/linux_xarray.h>

#include <media_tree/include/linux/mod_devicetable.h>
#include <media_tree/include/linux/videodev2.h>

#include <media_tree/include/linux/poison.h>
#include <media_tree/include/linux/llist.h>
#include <media_tree/include/linux/rculist.h>

extern const char *webcamd_devnames[F_V4B_MAX];

extern uid_t v4b_get_uid(void);
extern gid_t v4b_get_gid(void);
extern int v4b_get_perm(void);

extern unsigned short webcamd_vendor;
extern unsigned short webcamd_product;
extern unsigned int webcamd_speed;

#define	getmode __getmode
#define	setmode __setmode
#define	sync __sync

#ifndef O_CLOEXEC
#define	O_CLOEXEC	0x00100000
#endif

#define	WEBCAMD_IOCTL_GET_USB_VENDOR_ID _IOR('q', 250, unsigned short)
#define	WEBCAMD_IOCTL_GET_USB_PRODUCT_ID _IOR('q', 251, unsigned short)
#define	WEBCAMD_IOCTL_GET_USB_SPEED _IOR('q', 252, unsigned int)

struct v4l2_buffer_compat32 {
	uint32_t index;
	uint32_t type;
	uint32_t bytesused;
	uint32_t flags;
	uint32_t field;
	struct compat_timeval timestamp;
	struct v4l2_timecode timecode;
	uint32_t sequence;
	uint32_t memory;
	union {
		uint32_t offset;
	} m;
	uint32_t length;
	uint32_t input;
	uint32_t reserved;
};

#define	_VIDIOC_QUERYBUF32 _IOWR('V',  9, struct v4l2_buffer_compat32)

#endif					/* _WEBCAMD_GLOBAL_H_ */
