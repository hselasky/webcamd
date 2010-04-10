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
# Makefile for Linux USB Device Driver Daemon
#

VERSION=	0.1.10
PROG=		webcamd
MAN=
BINDIR=		%%PREFIX%%/sbin
LIBDIR?=	%%PREFIX%%/lib
LINUXDIR=	${.CURDIR}/v4l-dvb/linux
MKLINT=		no
NOGCCERROR=
MLINKS=
BITS_PER_LONG!=${CC} ${.CURDIR}/tests/long_size_test.c && ./a.out

.PATH: \
${.CURDIR} \
${.CURDIR}/kernel \
${.CURDIR}/pwcview \
${LINUXDIR}/drivers \
${LINUXDIR}/drivers/media \
${LINUXDIR}/drivers/media/common \
${LINUXDIR}/drivers/media/common/tuners \
${LINUXDIR}/drivers/media/dvb \
${LINUXDIR}/drivers/media/dvb/as102 \
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
${LINUXDIR}/drivers/media/video/gspca/gl860 \
${LINUXDIR}/drivers/media/video/gspca/m5602 \
${LINUXDIR}/drivers/media/video/gspca/stv06xx \
${LINUXDIR}/drivers/media/video/hdpvr \
${LINUXDIR}/drivers/media/video/ivtv \
${LINUXDIR}/drivers/media/video/ovcamchip \
${LINUXDIR}/drivers/media/video/pvrusb2 \
${LINUXDIR}/drivers/media/video/pwc \
${LINUXDIR}/drivers/media/video/sn9c102 \
${LINUXDIR}/drivers/media/video/usbvideo \
${LINUXDIR}/drivers/media/video/uvc \
${LINUXDIR}/drivers/media/video/zc0301 \
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
SRCS+= linux_firmware.c
SRCS+= linux_i2c.c

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
# DVB specific files
#

SRCS+= dmxdev.c
SRCS+= dvb_ca_en50221.c
SRCS+= dvb_demux.c
SRCS+= dvb_filter.c
SRCS+= dvb_frontend.c
SRCS+= dvb_math.c
#SRCS+= dvb_net.c
SRCS+= dvb_ringbuffer.c
SRCS+= dvbdev.c

#
# GSPCA based Webcams
#

SRCS+= benq.c
SRCS+= conex.c
SRCS+= etoms.c
SRCS+= finepix.c
SRCS+= gl860.c
SRCS+= gl860-mi1320.c
SRCS+= gl860-mi2020.c
SRCS+= gl860-ov2640.c
SRCS+= gl860-ov9655.c
SRCS+= gspca.c
SRCS+= jeilinj.c
SRCS+= m5602_core.c
SRCS+= m5602_mt9m111.c
SRCS+= m5602_ov7660.c
SRCS+= m5602_ov9650.c
SRCS+= m5602_po1030.c
SRCS+= m5602_s5k4aa.c
SRCS+= m5602_s5k83a.c
SRCS+= mars.c
SRCS+= mr97310a.c
SRCS+= ov519.c
SRCS+= ov534.c
SRCS+= ov534_9.c
SRCS+= pac207.c
SRCS+= pac7302.c
SRCS+= pac7311.c
SRCS+= sn9c20x.c
SRCS+= sonixb.c
SRCS+= sonixj.c
SRCS+= spca500.c
SRCS+= spca501.c
SRCS+= spca505.c
SRCS+= spca506.c
SRCS+= spca508.c
SRCS+= spca561.c
SRCS+= sq905.c
SRCS+= sq905c.c
SRCS+= stk014.c
SRCS+= stv0680.c
SRCS+= stv06xx.c
SRCS+= stv06xx_hdcs.c
SRCS+= stv06xx_pb0100.c
SRCS+= stv06xx_st6422.c
SRCS+= stv06xx_vv6410.c
SRCS+= sunplus.c
SRCS+= t613.c
SRCS+= tv8532.c
SRCS+= vc032x.c
SRCS+= zc3xx.c

#
# PWC based webcams
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
# sn9c102 based webcams
#

SRCS+= sn9c102_core.c
SRCS+= sn9c102_hv7131d.c
SRCS+= sn9c102_hv7131r.c
SRCS+= sn9c102_mi0343.c
SRCS+= sn9c102_mi0360.c
SRCS+= sn9c102_mt9v111.c
SRCS+= sn9c102_ov7630.c
SRCS+= sn9c102_ov7660.c
SRCS+= sn9c102_pas106b.c
SRCS+= sn9c102_pas202bcb.c
SRCS+= sn9c102_tas5110c1b.c
SRCS+= sn9c102_tas5110d.c
SRCS+= sn9c102_tas5130d1b.c

#
# zc0301 based webcams
#

SRCS+= zc0301_core.c
SRCS+= zc0301_pas202bcb.c
SRCS+= zc0301_pb0330.c

#
# cpia2 based webcams
#

SRCS+= cpia2_core.c
SRCS+= cpia2_usb.c
SRCS+= cpia2_v4l.c

#
# et61x251 based webcams
#

SRCS+= et61x251_core.c
SRCS+= et61x251_tas5130d1b.c

#
# em28xx
#

#SRCS+= em28xx-audio.c
#SRCS+= em28xx-cards.c
#SRCS+= em28xx-core.c
#SRCS+= em28xx-dvb.c
#SRCS+= em28xx-i2c.c
#SRCS+= em28xx-input.c
#SRCS+= em28xx-vbi.c
#SRCS+= em28xx-video.c

#
# hdpvr
#

SRCS+= hdpvr-control.c
SRCS+= hdpvr-core.c
SRCS+= hdpvr-i2c.c
SRCS+= hdpvr-video.c

#
# ivtv
#

#SRCS+= ivtv-cards.c
#SRCS+= ivtv-controls.c
#SRCS+= ivtv-driver.c
#SRCS+= ivtv-fileops.c
#SRCS+= ivtv-firmware.c
#SRCS+= ivtv-gpio.c
#SRCS+= ivtv-i2c.c
#SRCS+= ivtv-ioctl.c
#SRCS+= ivtv-irq.c
#SRCS+= ivtv-mailbox.c
#SRCS+= ivtv-udma.c
#SRCS+= ivtv-queue.c
#SRCS+= ivtv-routing.c
#SRCS+= ivtv-streams.c
#SRCS+= ivtv-vbi.c
#SRCS+= ivtv-yuv.c
#SRCS+= ivtvfb.c

#
# pvrusb2
#

SRCS+= pvrusb2-audio.c
SRCS+= pvrusb2-context.c
SRCS+= pvrusb2-cs53l32a.c
SRCS+= pvrusb2-ctrl.c
SRCS+= pvrusb2-cx2584x-v4l.c
SRCS+= pvrusb2-debugifc.c
SRCS+= pvrusb2-devattr.c
SRCS+= pvrusb2-dvb.c
SRCS+= pvrusb2-eeprom.c
SRCS+= pvrusb2-encoder.c
SRCS+= pvrusb2-hdw.c
SRCS+= pvrusb2-i2c-core.c
SRCS+= pvrusb2-io.c
SRCS+= pvrusb2-ioread.c
SRCS+= pvrusb2-main.c
SRCS+= pvrusb2-std.c
SRCS+= pvrusb2-sysfs.c
SRCS+= pvrusb2-v4l2.c
SRCS+= pvrusb2-video-v4l.c
SRCS+= pvrusb2-wm8775.c
SRCS+= cx2341x.c
SRCS+= tveeprom.c

#
# DVB USB
#

SRCS+= a800.c
SRCS+= af9005-fe.c
SRCS+= af9005-remote.c
SRCS+= af9005.c
SRCS+= af9015.c
SRCS+= anysee.c
SRCS+= au6610.c
SRCS+= ce6230.c
SRCS+= cinergyT2-core.c
SRCS+= cinergyT2-fe.c
SRCS+= cxusb.c
SRCS+= dib0700_core.c
SRCS+= dib0700_devices.c
SRCS+= dib3000mc.c
SRCS+= dibx000_common.c
SRCS+= dibusb-common.c
SRCS+= dibusb-mb.c
SRCS+= dibusb-mc.c
SRCS+= digitv.c
SRCS+= dtt200u-fe.c
SRCS+= dtt200u.c
SRCS+= dtv5100.c
SRCS+= dvb-usb-dvb.c
SRCS+= dvb-usb-firmware.c
SRCS+= dvb-usb-i2c.c
SRCS+= dvb-usb-init.c
SRCS+= dvb-usb-remote.c
SRCS+= dvb-usb-urb.c
SRCS+= dw2102.c
SRCS+= ec168.c
SRCS+= friio-fe.c
SRCS+= friio.c
SRCS+= gl861.c
SRCS+= gp8psk-fe.c
SRCS+= gp8psk.c
SRCS+= m920x.c
SRCS+= nova-t-usb2.c
SRCS+= opera1.c
SRCS+= ttusb2.c
SRCS+= umt-010.c
SRCS+= usb-urb.c
SRCS+= vp702x-fe.c
SRCS+= vp702x.c
SRCS+= vp7045-fe.c
SRCS+= vp7045.c

#
# Siano DVB USB
#

SRCS+= sms-cards.c
SRCS+= smscoreapi.c
SRCS+= smsdvb.c
SRCS+= smsendian.c
SRCS+= smsir.c
SRCS+= smsusb.c

#
# PCTV 74E
#

.if exists(${LINUXDIR}/drivers/media/dvb/as102/as102_usb_drv.c)
SRCS+= as102_drv.c
SRCS+= as102_fe.c
SRCS+= as102_fw.c
SRCS+= as102_usb_drv.c
SRCS+= as10x_cmd.c
SRCS+= as10x_cmd_cfg.c
SRCS+= as10x_cmd_stream.c
.endif

#
# Tevii S660
#

CFLAGS+= -DCONFIG_DVB_STV0288
SRCS+= stv0288.c
CFLAGS+= -DCONFIG_DVB_MT312
SRCS+= mt312.c
CFLAGS+= -DCONFIG_DVB_DS3000
SRCS+= ds3000.c

.if exists(${LINUXDIR}/drivers/media/dvb/dvb-usb/pctv452e.c)
SRCS+= pctv452e.c
SRCS+= ttpci-eeprom.c
CFLAGS+= -DCONFIG_DVB_STB0899
SRCS+= stb0899_drv.c
SRCS+= stb0899_algo.c
CFLAGS+= -DCONFIG_DVB_STB6100
SRCS+= stb6100.c
CFLAGS+= -DCONFIG_DVB_LNBP22
SRCS+= lnbp22.c
.endif

#
# TT DVB USB
#

SRCS+= dvb-ttusb-budget.c
SRCS+= ttusb_dec.c
SRCS+= ttusbdecfe.c

#
# B2C2 DVB USB
#

SRCS+= flexcop-dma.c
SRCS+= flexcop-eeprom.c
SRCS+= flexcop-fe-tuner.c
SRCS+= flexcop-hw-filter.c
SRCS+= flexcop-i2c.c
SRCS+= flexcop-misc.c
SRCS+= flexcop-sram.c
SRCS+= flexcop-usb.c
SRCS+= flexcop.c

#
# Various webcams
#
#SRCS+= au0828-cards.c
#SRCS+= s2255drv.c
SRCS+= stv680.c
SRCS+= ov511.c
SRCS+= se401.c
SRCS+= stk-webcam.c
SRCS+= stk-sensor.c
SRCS+= ibmcam.c
SRCS+= konicawc.c
SRCS+= quickcam_messenger.c
SRCS+= usbvideo.c
SRCS+= ultracam.c
#SRCS+= vicam.c
SRCS+= w9968cf.c
SRCS+= zr364xx.c
SRCS+= videobuf-core.c
SRCS+= videobuf-vmalloc.c

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

CFLAGS+= -DBITS_PER_LONG=${BITS_PER_LONG}

CFLAGS+= -I${.CURDIR}/dummy
CFLAGS+= -I${.CURDIR}/headers

CFLAGS+= -I${LINUXDIR}/drivers/media/video/gspca
CFLAGS+= -I${LINUXDIR}/drivers/media/video/hdpvr
CFLAGS+= -I${LINUXDIR}/drivers/media/common/tuners
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/dvb-core
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/frontends
CFLAGS+= -I${LINUXDIR}/include
CFLAGS+= -I${LINUXDIR}

CFLAGS+= -I${.CURDIR}

CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/ttpci

CFLAGS+= -DLINUX
CFLAGS+= -DCONFIG_INPUT
CFLAGS+= -DCONFIG_VIDEO_V4L1_COMPAT
CFLAGS+= -DCONFIG_DVB_DIB3000MC
CFLAGS+= -DCONFIG_VIDEO_PVRUSB2_DVB
CFLAGS+= -DCONFIG_I2C
CFLAGS+= -DCONFIG_DVB_CORE
CFLAGS+= -DCONFIG_AS102_USB
CFLAGS+= -DCONFIG_FW_LOADER
CFLAGS+= -DHAVE_WEBCAMD

CFLAGS+= -include webcamd_global.h

CFLAGS+= -O2 -Wall -Wno-pointer-sign

CFLAGS+= -fvisibility=hidden

.if exists(${.CURDIR}/../cuse4bsd/cuse4bsd.h)
CFLAGS+= -I${.CURDIR}/../cuse4bsd
.endif

.if exists(%%PREFIX%%/include/cuse4bsd.h)
CFLAGS+= -I%%PREFIX%%/include
.endif

LDFLAGS+= -L${LIBDIR} -lusb -lcuse4bsd -lpthread -lutil

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
	-e "s/linux.compiler.h/linux\/types.h/g" \
	-e "s/__user//g" \
	${LINUXDIR}/include/linux/videodev.h \
	${LINUXDIR}/include/linux/videodev2.h \
	${LINUXDIR}/include/linux/dvb/audio.h \
	${LINUXDIR}/include/linux/dvb/ca.h \
	${LINUXDIR}/include/linux/dvb/dmx.h \
	${LINUXDIR}/include/linux/dvb/frontend.h \
	${LINUXDIR}/include/linux/dvb/net.h \
	${LINUXDIR}/include/linux/dvb/osd.h \
	${LINUXDIR}/include/linux/dvb/version.h \
	${LINUXDIR}/include/linux/dvb/video.h

	[ -d /usr/local/include/linux ] || mkdir -p /usr/local/include/linux

	@echo "#include <stdint.h>" > ${LINUXDIR}/include/linux/types.h
	@echo "#include <time.h>" >> ${LINUXDIR}/include/linux/types.h
	@echo "" >> ${LINUXDIR}/include/linux/types.h
	@echo "#ifndef HAVE_LINUX_INTEGER_TYPES" >> ${LINUXDIR}/include/linux/types.h
	@echo "#define HAVE_LINUX_INTEGER_TYPES" >> ${LINUXDIR}/include/linux/types.h
	@echo "" >> ${LINUXDIR}/include/linux/types.h
	@echo "typedef struct timespec __kernel_time_t;" >> ${LINUXDIR}/include/linux/types.h
	@echo "" >> ${LINUXDIR}/include/linux/types.h
	@echo "typedef uint64_t __u64;" >> ${LINUXDIR}/include/linux/types.h
	@echo "typedef uint32_t __u32;" >> ${LINUXDIR}/include/linux/types.h
	@echo "typedef uint16_t __u16;" >> ${LINUXDIR}/include/linux/types.h
	@echo "typedef uint8_t __u8;" >> ${LINUXDIR}/include/linux/types.h
	@echo "" >> ${LINUXDIR}/include/linux/types.h
	@echo "typedef int64_t __s64;" >> ${LINUXDIR}/include/linux/types.h
	@echo "typedef int32_t __s32;" >> ${LINUXDIR}/include/linux/types.h
	@echo "typedef int16_t __s16;" >> ${LINUXDIR}/include/linux/types.h
	@echo "typedef int8_t __s8;" >> ${LINUXDIR}/include/linux/types.h
	@echo "#endif" >> ${LINUXDIR}/include/linux/types.h

	cp -v 	${LINUXDIR}/include/linux/videodev.h \
		${LINUXDIR}/include/linux/videodev2.h \
		${LINUXDIR}/include/linux/types.h \
		/usr/local/include/linux/

	[ -d /usr/local/include/linux/dvb ] || mkdir -p /usr/local/include/linux/dvb

	cp -v 	${LINUXDIR}/include/linux/dvb/audio.h \
		${LINUXDIR}/include/linux/dvb/ca.h \
		${LINUXDIR}/include/linux/dvb/dmx.h \
		${LINUXDIR}/include/linux/dvb/frontend.h \
		${LINUXDIR}/include/linux/dvb/net.h \
		${LINUXDIR}/include/linux/dvb/osd.h \
		${LINUXDIR}/include/linux/dvb/version.h \
		${LINUXDIR}/include/linux/dvb/video.h \
		/usr/local/include/linux/dvb/

	cd patches ; ./do_patch.sh

fetch_clean:

	rm -v -r -f v4l-dvb-* libv4l-* linux-* libv4l v4l-dvb \
		tip0.tar.bz2 tip1.tar.bz2 tip2.tar.bz2 tip3.tar.bz2 \
		tip4.tar.bz2 s2-liplianin-*

fetch:

	@echo "Try make fetch_clean if you get problems below!"

#
# Fetch latest Video4Linux:
#
	[ -f tip1.tar.bz2 ] || \
		fetch -o tip1.tar.bz2 http://linuxtv.org/hg/v4l-dvb/archive/tip.tar.bz2

#
# Fetch latest Linux kernel:
#
#
#	[ -f tip2.tar.bz2 ] || \
#		fetch -o tip2.tar.bz2 http://www.kernel.org/pub/linux/kernel/v2.6/linux-2.6.32.6.tar.bz2

#
# Fetch latest Linux PCTV 74E driver
#
	[ -f tip3.tar.bz2 ] || \
		fetch -o tip3.tar.bz2 http://kernellabs.com/hg/~dheitmueller/v4l-dvb-as102/archive/tip.tar.bz2

#
# Fetch latest PCTV 452E driver
#
	[ -f tip4.tar.bz2 ] || \
		fetch -o tip4.tar.bz2 http://mercurial.intuxication.org/hg/s2-liplianin/archive/tip.tar.bz2

	@echo "Extracting Files ... Please wait"

	tar -jxf tip1.tar.bz2

	ln -s v4l-dvb-* v4l-dvb

# TIP3

	tar -jxf tip3.tar.bz2 "*/as102/*"

	mkdir -p v4l-dvb/linux/drivers/media/dvb/as102

	cp -v v4l-dvb-as102-*/linux/drivers/media/dvb/as102/*.[ch] v4l-dvb/linux/drivers/media/dvb/as102/

	rm -rf v4l-dvb-as102-*

# TIP4

	tar -jxf tip4.tar.bz2 "*/pctv452e.c" "*/lnbp22.c" \
		"*/lnbp22.h" "*/dvb-usb-ids.h" "*/ttpci-eeprom.c" \
		"*/ttpci-eeprom.h"

	mkdir -p v4l-dvb/linux/drivers/media/dvb/dvb-usb
	mkdir -p v4l-dvb/linux/drivers/media/dvb/frontends
	mkdir -p v4l-dvb/linux/drivers/media/dvb/ttpci

	cp -v s2-liplianin-*/linux/drivers/media/dvb/dvb-usb/pctv452e.c \
		v4l-dvb/linux/drivers/media/dvb/dvb-usb/

	cp -v s2-liplianin-*/linux/drivers/media/dvb/frontends/lnbp22.[ch] \
		v4l-dvb/linux/drivers/media/dvb/frontends/

	cp -v s2-liplianin-*/linux/drivers/media/dvb/ttpci/ttpci-eeprom.[ch] \
		v4l-dvb/linux/drivers/media/dvb/ttpci/

	cat \
	v4l-dvb/linux/drivers/media/dvb/dvb-usb/dvb-usb-ids.h \
	s2-liplianin-*/linux/drivers/media/dvb/dvb-usb/dvb-usb-ids.h \
	| grep "^#define.*USB_" | sort | uniq > temp

	mv temp v4l-dvb/linux/drivers/media/dvb/dvb-usb/dvb-usb-ids.h

	rm -rf s2-liplianin-*

package: clean

	tar -jcvf temp.tar.bz2 \
		--exclude="v4l2-apps" --exclude="media-specs" --exclude="v4l_experimental" \
		--exclude="*.txt" --exclude=".svn" \
		--exclude="Documentation" --exclude="v4l2-apps" \
		Makefile *.[ch] dummy headers tests/*.[ch] kernel/*.[ch] \
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
	@echo "# kldload cuse4bsd"
	@echo "# webcamd -B"
	@echo "# pwcview"
	@echo "#"
