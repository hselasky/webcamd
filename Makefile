LINUXDIR=${.CURDIR}/libv4l/linux

#
# Usage:
# make fetch
# sudo make patch all install
#

all:
	make -f Makefile.lib all install

	gmake -C libv4l/v4l2-apps/libv4l \
		CFLAGS="-DCONFIG_SYS_WRAPPER -I/usr/local/include/" \
		LDFLAGS="-lv4lxdrivers" all install

	make -C pwcview all install

clean:
	make -f Makefile.lib clean

	gmake -C libv4l/v4l2-apps/libv4l clean

	make -C pwcview clean

fetch:
	rm -v -r -f v4l-dvb-* libv4l-* libv4l tip.tar.bz2

#
# Alternative location:
# fetch http://linuxtv.org/hg/~hgoede/libv4l/archive/tip.tar.bz2
#
	[ -f tip.tar.bz2 ] || \
		fetch http://linuxtv.org/hg/v4l-dvb/archive/tip.tar.bz2

	tar -jxvf tip.tar.bz2

	ln -s v4l-dvb-* libv4l

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

install:
	@echo "Done."
