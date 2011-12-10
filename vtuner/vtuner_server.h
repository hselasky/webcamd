/*-
 * Copyright (c) 2011 Hans Petter Selasky. All rights reserved.
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
 * BSD vTuner Server API
 *
 * Inspired by code written by:
 * Honza Petrous <jpetrous@smartimp.cz>
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

	u32	buffer[2 + (VTUNER_BUFFER_MAX / 4)];

	int	fd_data;
	int	fd_control;

	char	cport[16];
	char	dport[16];
};

#endif					/* _VTUNER_SERVER_PRIV_H */
