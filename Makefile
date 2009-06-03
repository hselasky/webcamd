LINUXDIR=${.CURDIR}/libv4l/linux

all:
	make -f Makefile.lib all install

	gmake -C libv4l/v4l2-apps/libv4l \
		CFLAGS="-DCONFIG_SYS_WRAPPER" \
		LDFLAGS="-lv4lxdrivers" all install

	make -C pwcview all install

clean:
	make -f Makefile.lib clean

	gmake -C libv4l/v4l2-apps/libv4l clean

	make -C pwcview clean

fetch:
	rm -v -r -f libv4l-* libv4l

	[ -f tip.tar.bz2 ] || \
		fetch http://linuxtv.org/hg/~hgoede/libv4l/archive/tip.tar.bz2

	tar -jxvf tip.tar.bz2

	ln -s libv4l-* libv4l

	sed -i \
	-e "s/__u64/uint64_t/g" \
	-e "s/__u32/uint32_t/g" \
	-e "s/__u16/uint16_t/g" \
	-e "s/__u8/uint8_t/g" \
	-e "s/__s64/int64_t/g" \
	-e "s/__s32/int32_t/g" \
	-e "s/__s16/int16_t/g" \
	-e "s/__s8/int8_t/g" \
	-e "linux.ioctl.h/sys\/ioctl.h/g" \
	-e "linux.types.h/sys\/types.h/g" \
	${LINUXDIR}/include/linux/videodev.h \
	${LINUXDIR}/include/linux/videodev2.h

	[ -d /usr/local/include/linux ] || mkdir -p /usr/local/include/linux

	cp -v 	${LINUXDIR}/include/linux/videodev.h \
		${LINUXDIR}/include/linux/videodev2.h \
		/usr/local/include/linux/
