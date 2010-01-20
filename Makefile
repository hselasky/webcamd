#
# $FreeBSD: $
#
# Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#
# Makefile for Webcam Daemon
#
VERSION=	0.1.4
PROG=		webcamd
MAN=
BINDIR=		%%PREFIX%%/sbin
LINUXDIR=	${.CURDIR}/v4l-dvb/linux
MKLINT=		no
NOGCCERROR=
MLINKS=

.PATH: \
${.CURDIR} \
${.CURDIR}/kernel \
${.CURDIR}/pwcview \
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
${LINUXDIR}/drivers/media/video/pvrusb2 \
${LINUXDIR}/drivers/media/video/ovcamchip \
${LINUXDIR}/drivers/media/video/usbvideo \
${LINUXDIR}/drivers/media/video/uvc \
${LINUXDIR}/drivers/media/video/zoran

#
# Start Section (must be first)
#
SRCS+= linux_start_section.c
SRCS+= linux_thread.c

#
# FreeBSD specific files
#

SRCS+= linux_defs.c
SRCS+= linux_func.c
SRCS+= linux_file.c
SRCS+= linux_input.c
SRCS+= linux_struct.c
SRCS+= linux_task.c
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
SRCS+= v4l1-compat.c

#
# GSPCA
#
SRCS+= gspca.c
SRCS+= conex.c
SRCS+= etoms.c
SRCS+= finepix.c
SRCS+= mars.c
SRCS+= mr97310a.c
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
SRCS+= sq905.c
SRCS+= stk014.c
SRCS+= sunplus.c
SRCS+= t613.c
SRCS+= tv8532.c
SRCS+= vc032x.c
SRCS+= zc3xx.c
SRCS+= stv06xx.c
SRCS+= stv06xx_hdcs.c
SRCS+= stv06xx_pb0100.c
SRCS+= stv06xx_st6422.c
SRCS+= stv06xx_vv6410.c

#
# PVR
#
SRCS+= pvrusb2-devattr.c

#
# PWC
#
SRCS+= pwc-ctrl.c
SRCS+= pwc-dec1.c
SRCS+= pwc-dec23.c
SRCS+= pwc-if.c
SRCS+= pwc-kiara.c
SRCS+= pwc-misc.c
SRCS+= pwc-timon.c
SRCS+= pwc-uncompress.c
SRCS+= pwc-v4l.c

#
# Various
#
#SRCS+= au0828-cards.c
#SRCS+= cpia2_usb.c
#SRCS+= em28xx-cards.c
#SRCS+= et61x251_core.c
#SRCS+= s2255drv.c
#SRCS+= se401.c
#SRCS+= sn9c102_core.c
SRCS+= stk-webcam.c
SRCS+= stk-sensor.c
SRCS+= ibmcam.c
SRCS+= konicawc.c
SRCS+= quickcam_messenger.c
SRCS+= usbvideo.c
SRCS+= ultracam.c
#SRCS+= vicam.c
#SRCS+= w9968cf.c
#SRCS+= zr364xx.c

#
# DVB
#
#SRCS+= af9005.c
#SRCS+= af9015.c
#SRCS+= anysee.c
#SRCS+= au6610.c
#SRCS+= cinergyT2-core.c
#SRCS+= dib0700_devices.c
#SRCS+= dtt200u.c
#SRCS+= dtv5100.c
#SRCS+= dw2102.c
#SRCS+= gl861.c
#SRCS+= gp8psk.c
#SRCS+= nova-t-usb2.c
#SRCS+= opera1.c
#SRCS+= vp702x.c
#SRCS+= smsusb.c
#SRCS+= dvb-ttusb-budget.c
#SRCS+= ttusb_dec.c

#
# Radio
#
SRCS+= radio-mr800.c

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

SRCS+= webcamd.c

CFLAGS+= -I${LINUXDIR}/drivers/media/video/gspca
CFLAGS+= -I${LINUXDIR}/include
CFLAGS+= -I${LINUXDIR}

CFLAGS+= -I${.CURDIR}
CFLAGS+= -I${.CURDIR}/dummy

CFLAGS+= -DCONFIG_VIDEO_V4L1_COMPAT
CFLAGS+= -DHAVE_WEBCAMD

CFLAGS+= -include webcamd_global.h

CFLAGS+= -O2 -Wall -Wno-pointer-sign

CFLAGS+= -fvisibility=hidden

.if exists(${.CURDIR}/../video4bsd/video4bsd.h)
CFLAGS+= -I${.CURDIR}/../video4bsd
.endif

.if exists(%%PREFIX%%/include/video4bsd.h)
CFLAGS+= -I%%PREFIX%%/include
.endif

LDFLAGS+= -lusb -lpthread -lutil

.include <bsd.prog.mk>

patch:
	sed -i '' \
	-e "s/__u64/uint64_t/g" \
	-e "s/__u32/uint32_t/g" \
	-e "s/__u16/uint16_t/g" \
	-e "s/__u8/uint8_t/g" \
	-e "s/__s64/int64_t/g" \
	-e "s/__s32/int32_t/g" \
	-e "s/__s16/int16_t/g" \
	-e "s/__s8/int8_t/g" \
	-e "s/__le64/uint64_t/g" \
	-e "s/__be64/uint64_t/g" \
	-e "s/__le32/uint32_t/g" \
	-e "s/__be32/uint32_t/g" \
	-e "s/__le16/uint16_t/g" \
	-e "s/__be16/uint16_t/g" \
	-e "s/linux.ioctl.h/sys\/ioctl.h/g" \
	-e "s/linux.types.h/sys\/types.h/g" \
	-e "s/linux.compiler.h/sys\/types.h/g" \
	-e "s/__user//g" \
	${LINUXDIR}/include/linux/videodev.h \
	${LINUXDIR}/include/linux/videodev2.h

	[ -d /usr/local/include/linux ] || mkdir -p /usr/local/include/linux

	cp -v 	${LINUXDIR}/include/linux/videodev.h \
		${LINUXDIR}/include/linux/videodev2.h \
		/usr/local/include/linux/

	cd patches ; ./do_patch.sh

fetch:
	rm -v -r -f v4l-dvb-* libv4l-* libv4l v4l-dvb tip0.tar.bz2 tip1.tar.bz2

#
# Fetch latest Video4Linux:
#
	[ -f tip1.tar.bz2 ] || \
		fetch -o tip1.tar.bz2 http://linuxtv.org/hg/v4l-dvb/archive/tip.tar.bz2

	tar -jxvf tip1.tar.bz2

	ln -s v4l-dvb-* v4l-dvb

package: clean

	tar -jcvf temp.tar.bz2 --exclude="*.txt" --exclude=".svn" \
		--exclude="Documentation" --exclude="v4l2-apps" \
		Makefile *.[ch] dummy kernel/*.[ch] \
		patches/do_patch.sh patches/*.diff \
		v4l-dvb v4l-dvb-*

	rm -rf webcamd-${VERSION}

	mkdir webcamd-${VERSION}

	tar -jxvf temp.tar.bz2 -C webcamd-${VERSION}

	rm -rf temp.tar.bz2

	tar -jcvf webcamd-${VERSION}.tar.bz2 webcamd-${VERSION}

help:
	@echo "#"
	@echo "# Webcamd usage example:"
	@echo "#"
	@echo "# kldload video4bsd"
	@echo "# webcamd -B"
	@echo "# pwcview"
	@echo "#"
