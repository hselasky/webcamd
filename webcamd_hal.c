/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * File history:
 *
 * probe-video4linux.c: Probe video4linux devices
 * Copyright (C) 2007 Nokia Corporation
 * Licensed under the Academic Free License version 2.1
 *
 * Adapted to FreeBSD by: Joe Marcus Clarke <marcus@FreeBSD.org>
 * Initial webcamd version by: Juergen Lock <nox@FreeBSD.org>
 * Rewritten by: Hans Petter Selasky <hselasky@FreeBSD.org>
 */

#include <webcamd_hal.h>

#ifndef HAVE_HAL

void
hal_init(int bus, int addr, int iface)
{
	return;
}

void
hal_add_device(const char *devname)
{
	return;
}

#else

#include <linux/videodev.h>
#include <linux/videodev2.h>

#include <libhal.h>

static LibHalContext *hal_ctx;
static DBusConnection *hal_conn;
static char *hal_dev;
static int hal_dvb_index;

void
hal_init(int bus, int addr, int iface)
{
	char **ppdev;
	int n;

	while (1) {
		hal_conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
		if (hal_conn != NULL)
			break;
		usleep(1000000);
		printf("Waiting for DBUS connection.\n");
	}

	hal_ctx = libhal_ctx_new();
	if (hal_ctx == NULL)
		return;

	if (libhal_ctx_set_dbus_connection(hal_ctx, hal_conn) == 0)
		return;

	while (libhal_ctx_init(hal_ctx, NULL) == 0) {
		usleep(1000000);
		printf("Waiting for HAL connection.\n");
	}

	while (1) {
		printf("Waiting for HAL USB device.\n");

		ppdev = libhal_manager_find_device_string_match(
		    hal_ctx, "info.bus", "usb_device", &n, NULL);

		if (ppdev == NULL) {
			usleep(1000000);
			continue;
		}
		if (n > 0) {
			while (n--) {
				if (libhal_device_get_property_int(hal_ctx, ppdev[n],
				    "usb_device.bus_number", NULL) != bus)
					continue;
				if (libhal_device_get_property_int(hal_ctx, ppdev[n],
				    "usb_device.port_number", NULL) != addr)
					continue;

				hal_dev = strdup(ppdev[n]);
				break;
			}
		}
		libhal_free_string_array(ppdev);

		if (hal_dev == NULL) {
			usleep(1000000);
			continue;
		} else {
			break;
		}
	}
}

void
hal_add_device(const char *devname)
{
	struct video_capability v1cap;
	struct v4l2_capability v2cap;
	LibHalChangeSet *cset;
	char devpath[128];
	int fd;

	if (hal_dev == NULL)
		return;

	snprintf(devpath, sizeof(devpath), "/dev/%s", devname);

	if (!strncmp(devname, "video", sizeof("video") - 1)) {

		cset = libhal_device_new_changeset(hal_dev);
		if (cset == NULL)
			return;

		fd = open(devpath, O_RDONLY);
		if (fd < 0) {
			libhal_device_free_changeset(cset);
			return;
		}
		if (ioctl(fd, VIDIOC_QUERYCAP, &v2cap) == 0) {
			libhal_changeset_set_property_string(cset, "video4linux.device", devpath);
			libhal_changeset_set_property_string(cset, "info.category", "video4linux");
			libhal_changeset_set_property_string(cset, "video4linux.version", "2");
			libhal_changeset_set_property_string(cset, "info.product", (const char *)v2cap.card);
			libhal_device_add_capability(hal_ctx, hal_dev, "video4linux", NULL);

			if (v2cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.video_capture", NULL);
			if (v2cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.video_output", NULL);
			if (v2cap.capabilities & V4L2_CAP_VIDEO_OVERLAY)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.video_overlay", NULL);
			if (v2cap.capabilities & V4L2_CAP_AUDIO)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.audio", NULL);
			if (v2cap.capabilities & V4L2_CAP_TUNER)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.tuner", NULL);
			if (v2cap.capabilities & V4L2_CAP_RADIO)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.radio", NULL);

		} else if (ioctl(fd, VIDIOCGCAP, &v1cap) == 0) {
			libhal_changeset_set_property_string(cset, "video4linux.device", devpath);
			libhal_changeset_set_property_string(cset, "info.category", "video4linux");
			libhal_changeset_set_property_string(cset, "video4linux.version", "1");
			libhal_changeset_set_property_string(cset, "info.product", v1cap.name);
			libhal_device_add_capability(hal_ctx, hal_dev, "video4linux", NULL);

			if (v1cap.type & VID_TYPE_CAPTURE)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.video_capture", NULL);
			if (v1cap.type & VID_TYPE_OVERLAY)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.video_overlay", NULL);
			if (v1cap.audios != 0)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.audio", NULL);
			if (v1cap.type & VID_TYPE_TUNER)
				libhal_device_add_capability(hal_ctx, hal_dev, "video4linux.tuner", NULL);
		}
		libhal_device_commit_changeset(hal_ctx, cset, NULL);
		libhal_device_free_changeset(cset);

		close(fd);

	} else if (!strncmp(devname, "dvb/adapter", sizeof("dvb/adapter") - 1)) {
		char *pdvb = NULL;
		char *pnew;

		asprintf(&pdvb, "%s_dvb_%d", hal_dev, hal_dvb_index);

		if (pdvb == NULL)
			return;

		if (libhal_device_exists(hal_ctx, pdvb, NULL) == 0) {
			pnew = libhal_new_device(hal_ctx, NULL);
			if (pnew == NULL) {
				free(pdvb);
				return;
			}
		} else {
			pnew = pdvb;
		}

		cset = libhal_device_new_changeset(pnew);
		if (cset == NULL) {
			if (pnew != pdvb)
				free(pnew);
			free(pdvb);
		}
		libhal_changeset_set_property_string(cset, "dvb.device", devpath);
		libhal_changeset_set_property_string(cset, "info.category", "dvb");
		libhal_changeset_set_property_string(cset, "info.parent", hal_dev);
		libhal_changeset_set_property_string(cset, "info.product", "DVB Device");
		libhal_changeset_set_property_string(cset, "info.subsystem", "dvb");
		libhal_device_add_capability(hal_ctx, pnew, "dvb", NULL);

		libhal_device_commit_changeset(hal_ctx, cset, NULL);
		libhal_device_free_changeset(cset);

		if (pnew != pdvb) {
			libhal_device_commit_to_gdl(hal_ctx, pnew, pdvb, NULL);
			free(pnew);
		}
		free(pdvb);

		hal_dvb_index++;
	}
}

#endif
