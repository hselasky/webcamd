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

exit 0
