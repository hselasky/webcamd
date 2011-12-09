/*
 * vtuner_server: Internal defines
 *
 * Copyright (C) 2010-11 Honza Petrous <jpetrous@smartimp.cz>
 * [Created 2010-03-23]
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _VTUNER_SERVER_PRIV_H
#define	_VTUNER_SERVER_PRIV_H

struct vtuners_ctx {

	int	unit;

	struct cdev_handle *frontend_fd;
	struct cdev_handle *demux_fd;
	struct cdev_handle *streaming_fd;

	struct semaphore writer_sem;

	pthread_t writer_thread;
	pthread_t control_thread;

	struct dtv_property dtv_props[VTUNER_PROP_MAX];

	union vtuner_dvb_message dvb;

	struct vtuner_message msgbuf;

	struct vtuner_data_hdr buffer_hdr;
	u32	buffer[VTUNER_BUFFER_MAX / 4];

	int	fd_data;
	int	fd_control;

	char	cport[16];
	char	dport[16];
};

#endif					/* _VTUNER_SERVER_PRIV_H */
