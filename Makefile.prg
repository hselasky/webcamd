#
# Makefile for USB webcam daemon
#

PROG=webcamd
MAN=

.if !defined(LINUXDIR)
LINUXDIR=${.CURDIR}/linux
.endif

.PATH: \
${.CURDIR} \
${.CURDIR}/dummy \
${LINUXDIR}/drivers \
${LINUXDIR}/drivers/media \
${LINUXDIR}/drivers/media/common \
${LINUXDIR}/drivers/media/common/tuners \
${LINUXDIR}/drivers/media/dvb \
${LINUXDIR}/drivers/media/dvb/b2c2 \
${LINUXDIR}/drivers/media/dvb/bt8xx \
${LINUXDIR}/drivers/media/dvb/dm1105 \
${LINUXDIR}/drivers/media/dvb/dvb-core \
${LINUXDIR}/drivers/media/dvb/dvb-usb \
${LINUXDIR}/drivers/media/dvb/firewire \
${LINUXDIR}/drivers/media/dvb/frontends \
${LINUXDIR}/drivers/media/dvb/pluto2 \
${LINUXDIR}/drivers/media/dvb/siano \
${LINUXDIR}/drivers/media/dvb/ttpci \
${LINUXDIR}/drivers/media/dvb/ttusb-budget \
${LINUXDIR}/drivers/media/dvb/ttusb-dec \
${LINUXDIR}/drivers/media/radio \
${LINUXDIR}/drivers/media/video \
${LINUXDIR}/drivers/media/video/au0828 \
${LINUXDIR}/drivers/media/video/bt8xx \
${LINUXDIR}/drivers/media/video/cpia2 \
${LINUXDIR}/drivers/media/video/cx18 \
${LINUXDIR}/drivers/media/video/cx23885 \
${LINUXDIR}/drivers/media/video/cx25840 \
${LINUXDIR}/drivers/media/video/cx88 \
${LINUXDIR}/drivers/media/video/em28xx \
${LINUXDIR}/drivers/media/video/et61x251 \
${LINUXDIR}/drivers/media/video/gspca \
${LINUXDIR}/drivers/media/video/gspca/m5602 \
${LINUXDIR}/drivers/media/video/gspca/stv06xx \
${LINUXDIR}/drivers/media/video/ivtv \
${LINUXDIR}/drivers/media/video/pwc \
${LINUXDIR}/drivers/media/video/ovcamchip \
${LINUXDIR}/drivers/media/video/uvc \
${LINUXDIR}/drivers/media/video/zoran

#
# FreeBSD specific files
#

SRCS+= usb_global.c

SRCS+= linux_defs.c
SRCS+= linux_func.c
SRCS+= linux_struct.c
SRCS+= linux_task.c
SRCS+= linux_thread.c
SRCS+= linux_timer.c
SRCS+= linux_usb.c

#
# Video4Linux specific files
#

SRCS+= v4l2-common.c
SRCS+= v4l2-compat-ioctl32.c
SRCS+= v4l2-dev.c
SRCS+= v4l2-device.c
SRCS+= v4l2-int-device.c
SRCS+= v4l2-ioctl.c
SRCS+= v4l2-subdev.c

SRCS+= dvb_demux.c
SRCS+= dmxdev.c
SRCS+= dvbdev.c
SRCS+= dvb_ringbuffer.c
SRCS+= dvb_frontend.c
SRCS+= videobuf-core.c
SRCS+= videobuf-dvb.c
SRCS+= videobuf-dma-contig.c
#SRCS+= videobuf-dma-sg.c
SRCS+= videobuf-vmalloc.c

#
# GSPCA
#
SRCS+= gspca.c
SRCS+= conex.c
SRCS+= etoms.c
SRCS+= finepix.c
SRCS+= mars.c
SRCS+= ov519.c
SRCS+= ov534.c
SRCS+= pac207.c
SRCS+= pac7311.c
SRCS+= sonixb.c
SRCS+= sonixj.c
SRCS+= spca500.c
SRCS+= spca501.c
SRCS+= spca505.c
SRCS+= spca506.c
SRCS+= spca508.c
SRCS+= spca561.c
SRCS+= sunplus.c
SRCS+= stk014.c
SRCS+= t613.c
SRCS+= tv8532.c
SRCS+= vc032x.c
SRCS+= zc3xx.c

#
# USB Video Class
#
SRCS+= uvc_driver.c
SRCS+= uvc_queue.c
SRCS+= uvc_v4l2.c
SRCS+= uvc_ctrl.c
SRCS+= uvc_status.c
SRCS+= uvc_isight.c
SRCS+= uvc_video.c

#
# End Section (must be last)
#
SRCS+= linux_end_section.c

CFLAGS+= -I${LINUXDIR}/drivers/media/video/gspca
CFLAGS+= -I${LINUXDIR}

CFLAGS+= -I${.CURDIR}
CFLAGS+= -I${.CURDIR}/dummy

CFLAGS+= -include usb_global.h

CFLAGS+= -O2 -Wall

LDFLAGS+= -lusb -lpthread

.include <bsd.prog.mk>
