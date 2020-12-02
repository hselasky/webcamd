<CENTER><IMG SRC="https://raw.githubusercontent.com/hselasky/hpsjam/main/www/webcamd_logo.png"></IMG></CENTER>

# Introduction

<B>Webcamd</B> is a small daemon that enables about 1500 different USB
based webcam, DVB and remote control USB devices under the FreeBSD
operating system. The webcam daemon is basically an application which
is a port of Linux USB drivers into userspace on FreeBSD. The daemon
currently depends on libc, pthreads, libusb and libcuse.

# Licensing

<B>Webcamd</B> is GPLv2 licensed due to the external Linux part which
is GPLv2, though some files inside the webcamd remains BSD licensed
which allows for building similar BSD licensed daemons.

# Testing a USB webcam device

<PRE>
pwcview -d /dev/video0 -s vga
</PRE>

# Privacy policy

<B>Webcamd</B> does not collect any information from its users.

# Support

Feedback and bug reports are welcome. The software is available from FreeBSD ports, <A HREF="https://www.freshports.org/multimedia/webcamd">/usr/ports/multimedia/webcamd</A> .
