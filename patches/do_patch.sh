#!/bin/sh

set -e
echo "Applying Linux kernel patches ..."
cat 0001-Linux-kernel-patches-for-webcamd.patch | patch -Nslf -p1 -d ../media_tree
exit 0

