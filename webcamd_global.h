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

#include <machine/param.h>

#include <sys/types.h>
#include <sys/stdint.h>
#include <sys/queue.h>
#include <sys/ctype.h>
#include <sys/ioccom.h>
#include <sys/fcntl.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>

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
#include <kernel/linux_end_section.h>
#include <kernel/linux_input.h>

#include <include/linux/videodev2.h>
#include <include/linux/videodev.h>
#include <include/linux/dvb/version.h>
#include <include/linux/dvb/dmx.h>
#include <include/linux/dvb/frontend.h>

#include <include/media/v4l2-chip-ident.h>
#include <include/media/v4l2-common.h>
#include <include/media/v4l2-dev.h>
#include <include/media/v4l2-device.h>
#include <include/media/v4l2-int-device.h>
#include <include/media/v4l2-ioctl.h>
#include <include/media/v4l2-subdev.h>
#include <include/media/videobuf-core.h>
#include <include/media/videobuf-dma-contig.h>
#include <include/media/videobuf-dma-sg.h>

#include <drivers/media/dvb/dvb-core/dmxdev.h>
#include <drivers/media/dvb/dvb-core/demux.h>
#include <drivers/media/dvb/dvb-core/dvb_demux.h>
#include <drivers/media/dvb/dvb-core/dvb_net.h>
#include <drivers/media/dvb/dvb-core/dvb_frontend.h>

#include <drivers/media/dvb/frontends/dvb-pll.h>
#include <drivers/media/dvb/frontends/af9013.h>

#include <include/media/videobuf-dvb.h>
#include <include/media/videobuf-vmalloc.h>

#include <drivers/media/video/uvc/uvcvideo.h>

#include <include/media/tuner.h>
#include <include/media/msp3400.h>

#define	PUBLIC_API __attribute__((visibility("default")))

#endif					/* _WEBCAMD_GLOBAL_H_ */
