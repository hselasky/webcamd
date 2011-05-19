#!/bin/sh

OPT='-N -s -l'

#
# driver patches - mostly to improve usability
#

patch $OPT ../media_tree/drivers/media/video/uvc/uvcvideo.h uvcvideo.h.diff
patch $OPT ../media_tree/drivers/media/video/stk-webcam.h stk-webcam.h.diff
patch $OPT ../media_tree/drivers/media/dvb/dvb-usb/dib0700_devices.c dib0700_devices.c.diff
patch $OPT ../media_tree/include/media/videobuf-dvb.h videobuf-dvb.h.diff
patch $OPT ../media_tree/drivers/media/dvb/dvb-usb/af9015.c af9015.c.diff
patch $OPT ../media_tree/include/linux/dvb/frontend.h frontend.h.diff
patch $OPT ../media_tree/drivers/media/dvb/dvb-usb/dw2102.c dw2102.c.diff
patch $OPT ../media_tree/drivers/media/dvb/frontends/stb0899_algo.c stb0899_algo.c.diff
patch $OPT ../media_tree/drivers/media/dvb/ttusb-budget/dvb-ttusb-budget.c dvb-ttusb-budget.c.diff
patch $OPT ../media_tree/drivers/media/dvb/dvb-usb/usb-urb.c usb-urb.c.diff
patch $OPT ../media_tree/drivers/media/dvb/frontends/stb0899_drv.c stb0899_drv.c.diff
patch $OPT ../media_tree/drivers/media/dvb/frontends/cx24116.c cx24116.c.diff

#cat lirc-patches.diff | patch $OPT
#cat af9013.c.diff | patch $OPT

cat as102.diff | patch $OPT -p1
cat dvb_net.diff | patch $OPT -p1
cat gspca.diff | patch $OPT -p1
cat tda7432.diff | patch $OPT -p1
cat tda8261.diff | patch $OPT -p1

exit 0
