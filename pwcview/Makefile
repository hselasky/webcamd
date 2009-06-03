#
# pwcview
#

PROG=		pwcview
MAN=
LDFLAGS+=	-lv4l1
SRCS+=		pwcview.c
MKLINT=		no
NOGCCERROR=
BINDIR?=	/usr/local/bin

CFLAGS+= `/usr/local/bin/sdl-config --cflags`
LDFLAGS+= `/usr/local/bin/sdl-config --libs` -ljpeg

.include <bsd.prog.mk>
