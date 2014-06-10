#!/bin/sh

OPT='-Nslf'

#
# driver patches - mostly to improve usability
#

find_media_file()
{
    grep -m 1 -F "${1}" do_patch.tmp || echo "../not_found/${1}"
}

echo -n "" > do_patch.tmp

#
# Auto-detect location of various files
#
for F in \
../media_tree/drivers/media/dvb-core/dvb_frontend.c \
../media_tree/drivers/media/dvb/dvb-core/dvb_frontend.c \
../media_tree/drivers/media/usb/uvc/uvc_video.c \
../media_tree/drivers/media/usb/uvc/uvcvideo.h \
../media_tree/drivers/media/video/uvc/uvc_video.c \
../media_tree/drivers/media/video/uvc/uvcvideo.h \
../media_tree/drivers/media/v4l2-core/v4l2-async.c \
../media_tree/drivers/media/v4l2-core/v4l2-dev.c \
../media_tree/drivers/media/v4l2-core/videobuf2-core.c \
../media_tree/drivers/media/v4l2-core/videobuf2-memops.c \
../media_tree/drivers/media/v4l2-core/videobuf2-vmalloc.c \
../media_tree/drivers/media/video/v4l2-dev.c \
../media_tree/drivers/media/video/videobuf2-core.c \
../media_tree/drivers/media/video/videobuf2-memops.c \
../media_tree/drivers/media/video/videobuf2-vmalloc.c \
../media_tree/include/uapi/linux/dvb/frontend.h \
../media_tree/include/linux/dvb/frontend.h \
../media_tree/include/uapi/linux/input.h \
../media_tree/include/linux/input.h \
../media_tree/drivers/staging/media/as102/as102_drv.h \
../media_tree/drivers/media/usb/dvb-usb/usb-urb.c \
../media_tree/drivers/media/dvb/dvb-usb/usb-urb.c \
../media_tree/drivers/media/usb/dvb-usb-v2/usb_urb.c \
../media_tree/drivers/media/usb/gspca/gspca.c \
../media_tree/drivers/media/video/gspca/gspca.c \
../media_tree/drivers/media/usb/stkwebcam/stk-webcam.h \
../media_tree/drivers/media/video/stk-webcam.h \
../media_tree/include/uapi/linux/dvb/ca.h \
../media_tree/include/linux/dvb/ca.h \
../media_tree/drivers/media/tuners/tda18212.c \
../media_tree/drivers/media/common/tuners/tda18212.c \
../media_tree/drivers/media/dvb-frontends/cx24123.c \
../media_tree/drivers/media/dvb-frontends/dib3000mb.c \
../media_tree/drivers/media/dvb-frontends/dib3000mb_priv.h \
../media_tree/drivers/media/dvb-frontends/m88rs2000.c \
../media_tree/drivers/media/dvb-frontends/tda18271c2dd.c \
../media_tree/drivers/media/dvb/frontends/cx24123.c \
../media_tree/drivers/media/dvb/frontends/dib3000mb.c \
../media_tree/drivers/media/dvb/frontends/dib3000mb_priv.h \
../media_tree/drivers/media/dvb/frontends/tda18271c2dd.c \
../media_tree/drivers/media/dvb/frontends/m88rs2000.c \
../media_tree/drivers/media/usb/dvb-usb-v2/lmedm04.c \
../media_tree/drivers/media/usb/dvb-usb-v2/mxl111sf-tuner.c \
../media_tree/drivers/media/usb/dvb-usb-v2/mxl111sf.c \
../media_tree/drivers/media/i2c/tvp514x.c \
../media_tree/drivers/media/i2c/adv7343.c \
../media_tree/drivers/media/i2c/tvp7002.c \
../media_tree/drivers/input/tablet/wacom.h
do
  [ -f $F ] && (echo "$F" >> do_patch.tmp)
done

#
# find ../media_tree/ -name "*.[ch]" >> do_patch.tmp
#

echo "Trying to patch ..."

find_media_file dvb_frontend.c
find_media_file uvc_video.c
find_media_file uvcvideo.h
find_media_file v4l2-dev.c
find_media_file v4l2-async.c
find_media_file videobuf2-core.c
find_media_file videobuf2-memops.c
find_media_file videobuf2-vmalloc.c
find_media_file frontend.h
find_media_file input.h
find_media_file as102_drv.h
find_media_file usb-urb.c
find_media_file usb_urb.c
find_media_file gspca.c
find_media_file stk-webcam.h
find_media_file ca.h
find_media_file tda18212.c
find_media_file cx24123.c
find_media_file dib3000mb.c
find_media_file dib3000mb_priv.h
find_media_file tda18271c2dd.c
find_media_file m88rs2000.c
find_media_file lmedm04.c
find_media_file mxl111sf-tuner.c
find_media_file mxl111sf.c
find_media_file wacom.h
find_media_file tvp514x.c
find_media_file adv7343.c
find_media_file tvp7002.c

patch $OPT $(find_media_file dvb_frontend.c) dvb_frontend.c.diff
patch $OPT -R $(find_media_file uvc_video.c) uvc_video.c.diff
patch $OPT $(find_media_file uvcvideo.h) uvcvideo.h.diff
patch $OPT -R $(find_media_file v4l2-dev.c) v4l2-dev.c.diff
patch $OPT $(find_media_file v4l2-async.c) v4l2-async.c.diff
patch $OPT $(find_media_file videobuf2-core.c) videobuf2-core.c.diff
patch $OPT $(find_media_file videobuf2-memops.c) videobuf2-memops.c.diff
patch $OPT $(find_media_file videobuf2-vmalloc.c) videobuf2-vmalloc.c.diff
patch $OPT -R $(find_media_file frontend.h) frontend.h.diff
patch $OPT -R $(find_media_file input.h) input.h.diff
patch $OPT $(find_media_file as102_drv.h) as102_drv.h.diff
patch $OPT $(find_media_file usb-urb.c) usb-urb.c.diff
patch $OPT $(find_media_file usb_urb.c) usb_urb.c.diff
patch $OPT $(find_media_file gspca.c) gspca.diff 
patch $OPT $(find_media_file stk-webcam.h) stk-webcam.h.diff
patch $OPT $(find_media_file ca.h) ca.h.diff
patch $OPT $(find_media_file wacom.h) wacom.h.diff
patch $OPT $(find_media_file tvp514x.c) tvp514x.c.diff
patch $OPT $(find_media_file adv7343.c) adv7343.c.diff
patch $OPT $(find_media_file tvp7002.c) tvp7002.c.diff

[ -f ../media_tree/drivers/media/common/tuners/tda18212.c ] && sed -e "s/dbg[(]/dib_&/g" -i .orig $(find_media_file tda18212.c)
sed -e "s/err[(]/cx_&/g" -e "s/info[(]/cx_&/g" -i .orig $(find_media_file cx24123.c)
sed -e "s/err[(]/dib_&/g" -e "s/info[(]/dib_&/g" -e "s/deb_dib_/deb_/g" -i .orig $(find_media_file dib3000mb.c)
sed -e "s/err[(]/dib_&/g" -e "s/info[(]/dib_&/g" -e "s/warn[(]/dib_&/g" -e "s/deb_dib_/deb_/g" -i .orig $(find_media_file dib3000mb_priv.h)
sed -e "s/sleep[(,]/do_&/g" -e "s/mdo_sleep/msleep/g" -i .orig $(find_media_file tda18271c2dd.c)
sed -e "s/[ 	]info[(]/ m88_info(/g" -i .orig $(find_media_file m88rs2000.c)
sed -e "s/[ 	]info[(]/ med_info(/g" -i .orig $(find_media_file lmedm04.c)
sed -e "s/[ 	]err[(]/ mxl_err(/g" -e "s/[ 	]info[(]/ mxl_info(/g" -e "s/define.err./define mxl_err /g" -e "s/define.info./define mxl_info /g" -i .orig $(find_media_file mxl111sf-tuner.c)
sed -e "s/[ 	]err[(]/ mxl_err(/g" -e "s/[ 	]info[(]/ mxl_info(/g" -e "s/define.err./define mxl_err /g" -e "s/define.info./define mxl_info /g" -i .orig $(find_media_file mxl111sf.c)

# DVBSKY support
[ -f dvbsky-linux-3.15-hps.diff ] && (cat dvbsky-linux-3.15-hps.diff | patch $OPT -d ../media_tree -p1) && echo "Applied DVBSKY patches ..."

exit 0
