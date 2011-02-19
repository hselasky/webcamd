#!/bin/sh

OPT='-N -s'

#
# driver patches - mostly to improve usability
#

patch $OPT ../v4l-dvb/linux/drivers/media/video/uvc/uvcvideo.h uvcvideo.h.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/stk-webcam.h stk-webcam.h.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/gspca/gspca.c gspca.c.diff
patch $OPT ../v4l-dvb/linux/drivers/media/dvb/as102/as102_drv.h as102_drv.h.diff
patch $OPT ../v4l-dvb/linux/drivers/media/dvb/dvb-usb/pctv452e.c pctv452e.c.diff
patch $OPT ../v4l-dvb/linux/drivers/media/dvb/dvb-usb/dib0700_devices.c dib0700_devices.c.diff
patch $OPT ../v4l-dvb/linux/include/media/videobuf-dvb.h videobuf-dvb.h.diff
patch $OPT ../v4l-dvb/linux/drivers/media/dvb/dvb-usb/af9015.c af9015.c.diff
patch $OPT ../v4l-dvb/linux/drivers/media/dvb/dvb-usb/pctv452e.c pctv452e.c.diff
patch $OPT ../v4l-dvb/linux/include/linux/dvb/frontend.h frontend.h.diff
patch $OPT ../v4l-dvb/linux/drivers/media/dvb/dvb-usb/dw2102.c dw2102.c.diff
cat lirc-patches.diff | patch $OPT

exit 0
