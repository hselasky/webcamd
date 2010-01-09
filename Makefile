LINUXDIR=${.CURDIR}/libv4l/linux

#
# Usage:
# make fetch
# sudo make patch all install
#

all:
	make -f Makefile.prg all

	gmake -C libv4l/v4l2-apps/libv4l \
		CFLAGS="-I${LINUXDIR}/include/" \
		LDFLAGS="" all

	make -C ../video4bsd all

clean:
	make -f Makefile.prg clean

	gmake -C libv4l/v4l2-apps/libv4l clean

	make -C pwcview clean

	make -C ../video4bsd clean

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

	cd patches && ./do_patch.sh

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
	make -f Makefile.prg install

	gmake -C libv4l/v4l2-apps/libv4l \
		CFLAGS="-I${LINUXDIR}/include/" \
		LDFLAGS="" install

	make -C pwcview all install

	make -C ../video4bsd install

	@echo "#"
	@echo "# Test commands:"
	@echo "# kldload video4bsd"
	@echo "# webcamd -d ugenX.Y -i 0 -v 0"
	@echo "# pwcview"
	@echo "#"

	@echo "Done."
