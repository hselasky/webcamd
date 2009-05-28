#ifndef _USB_GLOBAL_H_
#define _USB_GLOBAL_H_

#include <sys/types.h>
#include <sys/stdint.h>
#include <sys/queue.h>
#include <sys/ctype.h>
#include <sys/ioccom.h>
#include <sys/fcntl.h>
#include <sys/msg.h>

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>

#include <linux_defs.h>
#include <linux_struct.h>
#include <linux_func.h>
#include <linux_list.h>

#include <include/linux/videodev2.h>
#include <include/linux/videodev.h>

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
#include <include/linux/dvb/dmx.h>
#include <drivers/media/dvb/dvb-core/dmxdev.h>
#include <drivers/media/dvb/dvb-core/demux.h>
#include <drivers/media/dvb/dvb-core/dvb_demux.h>
#include <drivers/media/dvb/dvb-core/dvb_net.h>
#include <include/media/videobuf-dvb.h>
#include <include/media/videobuf-vmalloc.h>

#include <libusb20.h>
#include <libusb20_desc.h>

#include <usb_compat_linux.h>

#endif /* _USB_GLOBAL_H_ */
