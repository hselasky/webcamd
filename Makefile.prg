#
# Makefile for USB webcam daemon
#
PROG=webcamd
SRCS+= usb_compat_linux.c
CFLAGS+= -I.
LDFLAGS+= -lusb

.include <bsd.prog.mk>
