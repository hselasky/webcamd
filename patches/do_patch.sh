#!/bin/sh

OPT=

patch $OPT ../libv4l/v4l2-apps/libv4l/libv4lconvert/control/libv4lcontrol.c libv4lcontrol.c.diff
patch $OPT ../libv4l/linux/drivers/media/video/uvc/uvc_status.c uvc_status.c.diff
patch $OPT ../libv4l/linux/drivers/media/video/uvc/uvc_video.c uvc_video.c.diff
patch $OPT ../libv4l/linux/drivers/media/video/uvc/uvcvideo.h uvcvideo.h.diff


