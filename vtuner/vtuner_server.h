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

	vtuner_type_t type;

	struct dvb_frontend_info fe_info;

	struct cdev_handle *frontend_fd;
	struct cdev_handle *demux_fd;
	struct cdev_handle *streaming_fd;

	u16	pids[VTUNER_MAX_PID];

	int	adapter;

	int	num_props;
	struct dtv_property props[DTV_IOCTL_MAX_MSGS];
};

#endif					/* _VTUNER_SERVER_PRIV_H */
