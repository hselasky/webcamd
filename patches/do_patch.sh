#!/bin/sh

OPT=-N

# drivers

patch $OPT ../v4l-dvb/v4l2-apps/libv4l/libv4lconvert/control/libv4lcontrol.c libv4lcontrol.c.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/uvc/uvc_status.c uvc_status.c.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/uvc/uvc_video.c uvc_video.c.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/uvc/uvcvideo.h uvcvideo.h.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/stk-webcam.h stk-webcam.h.diff
patch $OPT ../v4l-dvb/v4l2-apps/libv4l/libv4lconvert/libv4lsyscall-priv.h libv4lsyscall-priv.h.diff
patch $OPT ../v4l-dvb/linux/drivers/media/video/gspca/gspca.c gspca.c.diff

# libv4l

patch $OPT ../libv4l/v4l2-apps/libv4l/libv4lconvert/control/libv4lcontrol.c libv4lcontrol.c.diff
patch $OPT ../libv4l/v4l2-apps/libv4l/libv4lconvert/libv4lsyscall-priv.h libv4lsyscall-priv.h.diff
