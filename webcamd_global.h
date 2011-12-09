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

#include <libusb20.h>
#include <libusb20_desc.h>

#include <kernel/linux_defs.h>
#include <kernel/linux_struct.h>
#include <kernel/linux_file.h>
#include <kernel/linux_func.h>
#include <kernel/linux_list.h>
#include <kernel/linux_timer.h>
#include <kernel/linux_task.h>
#include <kernel/linux_thread.h>
#include <kernel/linux_usb.h>
#include <kernel/linux_start_section.h>
#include <kernel/linux_firmware.h>
#include <kernel/linux_mod_param.h>

#include <media_tree/include/linux/mod_devicetable.h>
#include <media_tree/include/linux/videodev2.h>

extern int webcamd_unit;
extern int webcamd_hal_register;

extern uid_t v4b_get_uid(void);
extern gid_t v4b_get_gid(void);
extern int v4b_get_perm(void);

#define	getmode __getmode
#define	setmode __setmode
#define	sync __sync

#endif					/* _WEBCAMD_GLOBAL_H_ */
