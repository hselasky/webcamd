#
# Copyright (c) 2010-2020 Hans Petter Selasky. All rights reserved.
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

PROG=	webcamd

#
# List of all manual pages
#
MAN=

.if defined(HAVE_MAN)
MAN+=	webcamd.8
MAN+=	man4/a800.4
MAN+=	man4/af9005.4
MAN+=	man4/af9015.4
MAN+=	man4/anysee.4
MAN+=	man4/au0828.4
MAN+=	man4/au6610.4
MAN+=	man4/b2c2.4
MAN+=	man4/benq.4
MAN+=	man4/ce6230.4
MAN+=	man4/cinergy.4
MAN+=	man4/conex.4
MAN+=	man4/cpiax.4
MAN+=	man4/cxusb.4
MAN+=	man4/dib0700.4
MAN+=	man4/digitv.4
MAN+=	man4/dtt200u.4
MAN+=	man4/dtv5100.4
MAN+=	man4/dw2102.4
MAN+=	man4/ec168.4
MAN+=	man4/em28xx.4
MAN+=	man4/et61x251.4
MAN+=	man4/finepix.4
MAN+=	man4/friio.4
MAN+=	man4/gl860.4
MAN+=	man4/gl861.4
MAN+=	man4/gp8psk.4
MAN+=	man4/hdpvr.4
MAN+=	man4/ibmcam.4
MAN+=	man4/jeilinj.4
MAN+=	man4/m5602.4
MAN+=	man4/m920x.4
MAN+=	man4/mars.4
MAN+=	man4/mr800.4
MAN+=	man4/mr97310a.4
MAN+=	man4/nova-t.4
MAN+=	man4/opera.4
MAN+=	man4/ov519.4
MAN+=	man4/ov534.4
MAN+=	man4/pacxxx.4
MAN+=	man4/pvrusb2.4
MAN+=	man4/pwcusb.4
MAN+=	man4/s2255.4
MAN+=	man4/se401.4
MAN+=	man4/siano.4
MAN+=	man4/sn9c102.4
MAN+=	man4/sn9c20x.4
MAN+=	man4/sonixj.4
MAN+=	man4/spca5xx.4
MAN+=	man4/sq905c.4
MAN+=	man4/stk014.4
MAN+=	man4/stv06xx.4
MAN+=	man4/sunplus.4
MAN+=	man4/t613.4
MAN+=	man4/ttusb2.4
MAN+=	man4/tv8532.4
MAN+=	man4/umt.4
MAN+=	man4/usbvision.4
MAN+=	man4/uvc.4
MAN+=	man4/vc032x.4
MAN+=	man4/vp702x.4
MAN+=	man4/vp7045.4
MAN+=	man4/zc3xx.4
MAN+=	man4/zr364xx.4
.endif

PREFIX?=	/usr/local
LOCALBASE?=	/usr/local
BINDIR=		${PREFIX}/sbin
MANDIR=		${PREFIX}/man/man
LIBDIR=		${PREFIX}/lib
LINUXDIR=	media_tree
MKLINT=		no
NOGCCERROR=
MLINKS=
PTHREAD_LIBS?=	-lpthread

.if exists(${.CURDIR}/build/obj-y/Makefile)
.include "${.CURDIR}/build/obj-y/Makefile"
.endif

.PATH: ${.CURDIR} ${LINUXDIR}/lib

#
# List of compiler flags
#
CFLAGS+= -D_GNU_SOURCE
CFLAGS+= -DLINUX
CFLAGS+= -Wall -Wno-pointer-sign -Wno-unused-variable

CFLAGS+= -I${.CURDIR}
CFLAGS+= -I${.CURDIR}/dummy
CFLAGS+= -I${.CURDIR}/headers

CFLAGS+= -I${LINUXDIR}/drivers/media/common
CFLAGS+= -I${LINUXDIR}/drivers/media/common/tuners
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/dvb-core
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/dvb-usb
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/frontends
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/ttpci
CFLAGS+= -I${LINUXDIR}/drivers/media/video/gspca
CFLAGS+= -I${LINUXDIR}/drivers/media/video/hdpvr

#
# 3.7 paths
#
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb-core
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb-usb
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb-frontends
CFLAGS+= -I${LINUXDIR}/drivers/media/tuners
CFLAGS+= -I${LINUXDIR}/drivers/media/pci/ttpci
CFLAGS+= -I${LINUXDIR}/drivers/media/common/siano
CFLAGS+= -I${LINUXDIR}/drivers/media/common/b2c2
CFLAGS+= -I${LINUXDIR}/drivers/media/usb/gspca
CFLAGS+= -I${LINUXDIR}/drivers/media/usb/dvb-usb

#
# 3.15 paths
#
CFLAGS+= -I${LINUXDIR}/drivers/media/usb/dvb-usb-v2

CFLAGS+= -I${LINUXDIR}/include
CFLAGS+= -I${LINUXDIR}/include/uapi

#
# 3.17 paths
#
CFLAGS+= -I${LINUXDIR}/include/media

CFLAGS+= -I${LOCALBASE}
CFLAGS+= -I${LOCALBASE}/include

CFLAGS+= -include webcamd_global.h

.if defined(HAVE_DEBUG)
CFLAGS+= -DCONFIG_DVB_USB_DEBUG
CFLAGS+= -DHAVE_DEBUG
CFLAGS+= -g
.endif

#
# List of linker flags
#
LDFLAGS+= -L${LIBDIR} -lusb ${PTHREAD_LIBS} -lutil -lcuse

#
# List of source files which need to be built separately:
#
SRCS+= kfifo.c
SRCS+= rbtree.c
SRCS+= webcamd.c

.include <bsd.prog.mk>

patch:
	cd patches ; ./do_patch.sh

help:
	@echo "#"
	@echo "# Webcamd usage example:"
	@echo "#"
	@echo "# kldload cuse"
	@echo "# webcamd -B"
	@echo "# pwcview"
	@echo "#"

tools/linux_make/linux_make:
	make -C tools/linux_make

configure: tools/linux_make/linux_make
	@echo "Configuring webcamd for:"
	@echo "#" > config
	@echo "# This file has been automatically generated" >> config
	@echo "# Please do not edit" >> config
	@echo "#" >> config

	@(cat config.in ; echo "") >> config

.if defined(HAVE_COMPAT32)
	@echo " * i386 compatibility"
	@echo "CONFIG_COMPAT=y" >> config
	@echo "CONFIG_COMPAT_32BIT_TIME=y" >> config
	@echo "CONFIG_X86_64=y" >> config
.endif
.if defined(HAVE_DVB_DRV) || defined(HAVE_ALL_DRV)
	@echo " * DVB devices"
	@(cat config_dvb.in ; echo "") >> config
.endif
.if defined(HAVE_V4L2LOOPBACK_DRV) || defined(HAVE_ALL_DRV)
	@echo " * V4L2LOOPBACK devices"
	@(cat config_loopback.in ; echo "") >> config
.endif
.if defined(HAVE_WEBCAM_DRV) || defined(HAVE_ALL_DRV)
	@echo " * Webcam devices"
	@(cat config_webcam.in ; echo "") >> config
.endif
.if defined(HAVE_INPUT_DRV) || defined(HAVE_ALL_DRV)
	@echo " * Input devices"
	@(cat config_input.in ; echo "") >> config
.endif
.if defined(HAVE_KEYBOARD_DRV) || defined(HAVE_ALL_DRV)
	@echo " * Keyboard devices"
	@(cat config_keyboard.in ; echo "") >> config
.endif
.if defined(HAVE_MOUSE_DRV) || defined(HAVE_ALL_DRV)
	@echo " * Mouse devices"
	@(cat config_mouse.in ; echo "") >> config
.endif
.if defined(HAVE_RADIO_DRV) || defined(HAVE_ALL_DRV)
	@echo " * Radio devices"
	@(cat config_radio.in ; echo "") >> config
.endif
.if defined(HAVE_VTUNER_CLIENT) || defined(HAVE_ALL_DRV)
	@echo " * VirtualTuner client"
	@(cat config_vtuner_client.in ; echo "") >> config
.endif
.if defined(HAVE_VTUNER_SERVER) || defined(HAVE_ALL_DRV)
	@echo " * VirtualTuner server"
	@(cat config_vtuner_server.in ; echo "") >> config
.endif
.if defined(HAVE_MLX5)
	@echo " * Connect-X 4/5/6"
	@(cat config_mlx5.in ; echo "") >> config
.endif
	tools/linux_make/linux_make -c config \
		-x v4l2-clk.o \
		-x uvc_debugfs.o \
		-i kernel \
		-i vtuner \
		-i media_tree/drivers/hid \
		-i media_tree/drivers/input \
		-i media_tree/drivers/leds \
		-i media_tree/drivers/media \
		-i media_tree/drivers/base/regmap \
		-i media_tree/drivers/staging/media \
		-i media_tree/sound/i2c/other \
		-i media_tree/lib/lzo \
		-i media_tree/drivers/net/ethernet/mellanox/mlx5/core \
		-i contrib/dvb-usb \
		-i contrib/v4l2loopback \
		-o build/
