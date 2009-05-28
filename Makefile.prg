#
# Makefile for USB webcam daemon
#

PROG=webcamd
MAN=

.if !defined(LINUXDIR)
LINUXDIR=${.CURDIR}/linux
.endif

.PATH: \
${CURDIR} \
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
${LINUXDIR}/drivers/media/video/zoran

#
# FreeBSD specific files
#

SRCS+= usb_compat_linux.c

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

SRCS+= videobuf-core.c
SRCS+= videobuf-dma-contig.c
SRCS+= videobuf-dma-sg.c
SRCS+= videobuf-dvb.c
SRCS+= videobuf-vmalloc.c

#
# GSPCA
#
SRCS+= gspca_main.c
SRCS+= gspca_conex.c
SRCS+= gspca_etoms.c
SRCS+= gspca_finepix.c
SRCS+= gspca_mars.c
SRCS+= gspca_ov519.c
SRCS+= gspca_ov534.c
SRCS+= gspca_pac207.c
SRCS+= gspca_pac7311.c
SRCS+= gspca_sonixb.c
SRCS+= gspca_sonixj.c
SRCS+= gspca_spca500.c
SRCS+= gspca_spca501.c
SRCS+= gspca_spca505.c
SRCS+= gspca_spca506.c
SRCS+= gspca_spca508.c
SRCS+= gspca_spca561.c
SRCS+= gspca_sunplus.c
SRCS+= gspca_stk014.c
SRCS+= gspca_t613.c
SRCS+= gspca_tv8532.c
SRCS+= gspca_vc032x.c
SRCS+= gspca_zc3xx.c

SRCS+= gspca_stv06xx.c
SRCS+= stv06xx.c
SRCS+= stv06xx_vv6410.c
SRCS+= stv06xx_hdcs.c
SRCS+= stv06xx_pb0100.c

SRCS+= gspca_m5602.c
SRCS+= m5602_core.c
SRCS+= m5602_ov9650.c
SRCS+= m5602_mt9m111.c
SRCS+= m5602_po1030.c
SRCS+= m5602_s5k83a.c
SRCS+= m5602_s5k4aa.c

#
# USB Video Class
#
SRCS+= uvc_driver.c
SRCS+= uvc_queue.c
SRCS+= uvc_v4l2.c
SRCS+= uvc_video.c
SRCS+= uvc_ctrl.c
SRCS+= uvc_status.c
SRCS+= uvc_isight.c
SRCS+= uvcvideo.c

CFLAGS+= -I${LINUXDIR}/drivers/media/video/gspca
CFLAGS+= -I${LINUXDIR}

CFLAGS+= -I${.CURDIR}
CFLAGS+= -I${.CURDIR}/dummy

CFLAGS+= -include usb_global.h

LDFLAGS+= -lusb -lpthread

.include <bsd.prog.mk>
