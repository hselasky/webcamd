#!/bin/sh

OPT='-Nslf'

#
# driver patches - mostly to improve usability
#

patch $OPT ../media_tree/drivers/media/video/stk-webcam.h stk-webcam.h.diff
patch $OPT ../media_tree/drivers/media/dvb/dvb-usb/usb-urb.c usb-urb.c.diff
patch $OPT ../media_tree/drivers/media/dvb/dvb-usb/af9015.c af9015.c.diff
patch $OPT ../media_tree/drivers/media/video/gspca/gspca.c gspca.diff 

#
# New driver
#
cat as102.diff | patch $OPT -p1

exit 0
