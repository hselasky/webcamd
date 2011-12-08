/*
 * vtuner_client: Internal defines
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

#ifndef _VTUNER_CLIENT_PRIV_H
#define	_VTUNER_CLIENT_PRIV_H

#define	MAX_VTUNER_BUFFER (2 * 65536)

struct vtuner_message;

struct vtunerc_ctx {

	/* DVB API */
	struct dmx_frontend hw_frontend;
	struct dmx_frontend mem_frontend;
	struct dmxdev dmxdev;
	struct dvb_adapter dvb_adapter;
	struct dvb_demux demux;
	struct dvb_frontend *fe;
	struct dvb_device *ca;

	struct semaphore xchange_sem;

	pthread_t reader_thread;

	/* TCP-IP links */
	int	fd_control;
	int	fd_data;

	u32	buffer[MAX_VTUNER_BUFFER - (MAX_VTUNER_BUFFER % VTUNER_TS_ALIGN)];

	/* Internals */
	u16	pidtab[VTUNER_MAX_PID];

	u8	vtype;

	/* Leftover TS-frame */
	u8	trailbuf[VTUNER_TS_ALIGN];
	u8	trailsize;

	char	cport[16];
	char	dport[16];
};

#endif
