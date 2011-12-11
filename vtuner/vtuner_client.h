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
 * BSD vTuner Client API
 *
 * Inspired by code written by:
 * Honza Petrous <jpetrous@smartimp.cz>
 */

#ifndef _VTUNER_CLIENT_PRIV_H
#define	_VTUNER_CLIENT_PRIV_H

enum {
	VTUNERC_DT_FRONTEND,
	VTUNERC_DT_DMX,
	VTUNERC_DT_DVR,
};

struct vtunerc_ctx {

	struct semaphore xchange_sem;
	struct semaphore rd_sem;
	struct semaphore ioctl_sem;
	wait_queue_head_t rd_queue;

	pthread_t reader_thread;

	struct vtuner_message msgbuf;

	int	fd_control;
	int	fd_data;

	int	frontend_opened;
	int	dmx_opened;
	int	dvr_opened;

	u32	buffer_off;
	u32	buffer_rem;
	struct vtuner_data_hdr buffer_hdr;
	u32	buffer[VTUNER_BUFFER_MAX];

	u8	buffer_typ;

	char	cport[16];
	char	dport[16];
};

#endif
