#ifndef _USB_GLOBAL_H_
#define	_USB_GLOBAL_H_

#define	ffs ffs_bsd

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

#undef ffs

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

#define PUBLIC_API __attribute__((visibility("default")))

#endif					/* _USB_GLOBAL_H_ */
