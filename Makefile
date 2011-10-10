#
# $FreeBSD: $
#
# Copyright (c) 2010-2011 Hans Petter Selasky. All rights reserved.
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

#
# Basic software version information
#
VERSION=	3.1.0.2
PROG=		webcamd

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
MAN+=	man4/pwc.4
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
BITS_PER_LONG!=${CC} -o long_size_test ${.CURDIR}/tests/long_size_test.c && ./long_size_test

.include "${.CURDIR}/build/obj-y/Makefile"

#
# List of source paths
#
SRCPATHS+= kernel
SRCPATHS+= ${LINUXDIR}/kernel

.PATH: ${.CURDIR} ${SRCPATHS}

#
# List of compiler flags
#
CFLAGS+= -D_GNU_SOURCE
CFLAGS+= -DCURR_FILE_NAME=\"${.TARGET:C/\.o//g}\"
CFLAGS+= -DBITS_PER_LONG=${BITS_PER_LONG}
CFLAGS+= -DLINUX
CFLAGS+= -Wall -Wno-pointer-sign -Wno-unused-variable

CFLAGS+= -I${.CURDIR}
CFLAGS+= -I${.CURDIR}/dummy
CFLAGS+= -I${.CURDIR}/headers

CFLAGS+= -I${LINUXDIR}/drivers/media/common/tuners
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/dvb-core
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/dvb-usb
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/frontends
CFLAGS+= -I${LINUXDIR}/drivers/media/dvb/ttpci
CFLAGS+= -I${LINUXDIR}/drivers/media/video/gspca
CFLAGS+= -I${LINUXDIR}/drivers/media/video/hdpvr
CFLAGS+= -I${LINUXDIR}/include

CFLAGS+= -I${LOCALBASE}
CFLAGS+= -I${LOCALBASE}/include

CFLAGS+= -include webcamd_global.h

.if defined(HAVE_HAL)
HAL_CFLAGS!= pkg-config --cflags hal
HAL_LDFLAGS!= pkg-config --libs hal
CFLAGS+= ${HAL_CFLAGS} -DHAVE_HAL
LDFLAGS+= ${HAL_LDFLAGS}
.endif

.if defined(HAVE_DEBUG)
CFLAGS+= -DCONFIG_DVB_USB_DEBUG
CFLAGS+= -DHAVE_DEBUG
CFLAGS+= -g
.endif

#
# List of linker flags
#
LDFLAGS+= -L${LIBDIR} -lusb -lcuse4bsd -lpthread -lutil

#
# List of Linux specific sources
#
SRCS+= kfifo.c

#
# List of FreeBSD specific sources
#
SRCS+= linux_start_section.c
SRCS+= linux_thread.c
SRCS+= linux_defs.c
SRCS+= linux_func.c
SRCS+= linux_file.c
SRCS+= linux_struct.c
SRCS+= linux_task.c
SRCS+= linux_timer.c
SRCS+= linux_usb.c
SRCS+= linux_firmware.c
SRCS+= linux_i2c.c
SRCS+= linux_mod_param.c

SRCS+= webcamd.c
SRCS+= webcamd_hal.c

.include <bsd.prog.mk>

patch:
	cd patches ; ./do_patch.sh

fetch_clean:

fetch:

	@echo "Try make fetch_clean if you get problems below!"

#
# Fetch latest Video4Linux:
#

	[ -L media_tree ] || [ -d media_tree ] || (git clone git://linuxtv.org/media_tree.git media_tree)

	cd media_tree ; git pull

package:

	@make clean cleandepend HAVE_MAN=YES

	[ -d ${LINUXDIR}/drivers/media/dvb/as102 ] || mkdir ${LINUXDIR}/drivers/media/dvb/as102

	tar -cvf temp.tar --exclude="*~" --exclude="*#" --exclude=".git" \
		--exclude=".svn" --exclude="*.orig" --exclude="*.rej" \
		Makefile man4/*.4 dummy headers tests/*.[ch] webcamd*.[ch] webcamd.8 \
		config \
		${SRCPATHS} build/ media_tree/include \
		patches/do_patch.sh patches/*.diff \
		tools/linux_make/*.[ch] \
		tools/linux_make/Makefile

	rm -rf webcamd-${VERSION}

	mkdir webcamd-${VERSION}

	tar -xvf temp.tar -C webcamd-${VERSION}

	rm -rf temp.tar

	tar -jcvf webcamd-${VERSION}.tar.bz2 webcamd-${VERSION}

help:
	@echo "#"
	@echo "# Webcamd usage example:"
	@echo "#"
	@echo "# kldload cuse4bsd"
	@echo "# webcamd -B"
	@echo "# pwcview"
	@echo "#"

configure:
	linux_make -c config -i media_tree/drivers/input -i media_tree/drivers/media -o build/
