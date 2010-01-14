#!/bin/sh

OPT=-N

#
# driver patches - mostly to improve usability
#

patch $OPT ../v4l-dvb/linux/drivers/media/video/uvc/uvc_status.c uvc_status.c.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/uvc/uvc_video.c uvc_video.c.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/uvc/uvcvideo.h uvcvideo.h.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/stk-webcam.h stk-webcam.h.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/gspca/gspca.c gspca.c.diff

