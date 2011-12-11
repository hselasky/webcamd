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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "dvb_demux.h"
#include "dvb_frontend.h"

#include <vtuner/vtuner.h>
#include <vtuner/vtuner_common.h>
#include <vtuner/vtuner_client.h>

#include <sys/filio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>

#include <cuse4bsd.h>

#include <webcamd_hal.h>

#define	VTUNER_MODULE_VERSION "1.0-hps"

#define	VTUNER_LOCAL_MEMSET(a,b,c) \
    memset(a,b,sizeof(*(a)))

#define	VTUNER_PEER_MEMSET(a,b,c) do {			\
    static const u8 dummyz[sizeof(*(a))] __aligned(4);	\
    extern int dummyc[(b) ? -1 : 1];			\
    if (copy_to_user(a,dummyz,sizeof(*(a))) != 0)	\
	*c |= -1U;					\
} while (0)

#define	VTUNER_PEER_MEMCPY(a,b,c,d) do {				\
    extern int dummyc[(sizeof(*(a.c)) != sizeof(*(b.c))) ? -1 : 1];	\
    if (copy_to_user(a.c,b.c,sizeof(*(a.c))) != 0)			\
	*d |= -1U;							\
} while (0)

#define	VTUNER_LOCAL_MEMCPY(a,b,c,d) do {				\
    extern int dummyc[(sizeof(*(a.c)) != sizeof(*(b.c))) ? -1 : 1];	\
    if (copy_from_user(a.c,b.c,sizeof(*(a.c))) != 0)		\
	*d |= -1U;							\
} while (0)

static struct vtunerc_ctx *vtunerc_tbl[CONFIG_DVB_MAX_ADAPTERS];
static int vtuner_max_unit = 0;
static char vtuner_host[64] = {"127.0.0.1"};
static char vtuner_cport[16] = {VTUNER_DEFAULT_PORT};

static cuse_open_t vtunerc_open;
static cuse_close_t vtunerc_close;
static cuse_read_t vtunerc_read;
static cuse_write_t vtunerc_write;
static cuse_ioctl_t vtunerc_ioctl;
static cuse_poll_t vtunerc_poll;

static struct cuse_methods vtunerc_methods = {
	.cm_open = vtunerc_open,
	.cm_close = vtunerc_close,
	.cm_read = vtunerc_read,
	.cm_write = vtunerc_write,
	.cm_ioctl = vtunerc_ioctl,
	.cm_poll = vtunerc_poll,
};

static void
vtunerc_connect_control(struct vtunerc_ctx *ctx)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *res0;
	int error;
	int flag;
	int s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	printk(KERN_INFO "vTuner: Trying to connect to %s:%s (control)\n",
	    vtuner_host, ctx->cport);

	if ((error = getaddrinfo(vtuner_host, ctx->cport, &hints, &res)))
		return;

	res0 = res;

	do {
		if ((s = socket(res0->ai_family, res0->ai_socktype,
		    res0->ai_protocol)) < 0)
			continue;

		flag = 1;
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, (int)sizeof(flag));

		flag = 2 * sizeof(struct vtuner_message);
		setsockopt(s, SOL_SOCKET, SO_SNDBUF, &flag, (int)sizeof(flag));
		setsockopt(s, SOL_SOCKET, SO_RCVBUF, &flag, (int)sizeof(flag));

		if (connect(s, res0->ai_addr, res0->ai_addrlen) == 0)
			break;

		close(s);
		s = -1;
	} while ((res0 = res0->ai_next) != NULL);

	freeaddrinfo(res);

	if (s < 0)
		return;

	ctx->fd_control = s;

	printk(KERN_INFO "vTuner: Connected, fd=%d (control)\n", s);
}

static void
vtunerc_connect_data(struct vtunerc_ctx *ctx)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *res0;
	int error;
	int flag;
	int s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	printk(KERN_INFO "vTuner: Trying to connect to %s:%s (data)\n",
	    vtuner_host, ctx->dport);

	if ((error = getaddrinfo(vtuner_host, ctx->dport, &hints, &res)))
		return;

	res0 = res;

	do {
		if ((s = socket(res0->ai_family, res0->ai_socktype,
		    res0->ai_protocol)) < 0)
			continue;

		flag = 2 * (sizeof(ctx->buffer) + 8);
		setsockopt(s, SOL_SOCKET, SO_RCVBUF, &flag, sizeof(flag));

		flag = 1;
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

		if (connect(s, res0->ai_addr, res0->ai_addrlen) == 0)
			break;

		close(s);
		s = -1;
	} while ((res0 = res0->ai_next) != NULL);

	freeaddrinfo(res);

	if (s < 0)
		return;

	ctx->fd_data = s;

	printk(KERN_INFO "vTuner: Connected, fd=%d (data)\n", s);
}

static int
vtunerc_fd_read(int fd, u8 * ptr, int len)
{
	int off = 0;
	int err;

	while (off < len) {
		err = read(fd, ptr + off, len - off);
		if (err <= 0)
			return (err);
		off += err;
	}
	return (off);
}

static int
vtunerc_fd_write(int fd, const u8 * ptr, int len)
{
	int off = 0;
	int err;

	while (off < len) {
		err = write(fd, ptr + off, len - off);
		if (err <= 0)
			return (err);
		off += err;
	}
	return (off);
}

static void
vtunerc_do_message(struct vtunerc_ctx *ctx,
    struct vtuner_message *msg, int mtype, int rx_size, int tx_size, int *pret)
{
	int len;

	/* if an error is already set, just return */

	if (*pret != 0)
		return;

	down(&ctx->xchange_sem);

	/* stamp the byte order and version */

	msg->hdr.magic = VTUNER_MAGIC;
	msg->hdr.mtype = mtype;
	msg->hdr.rx_size = rx_size;
	msg->hdr.tx_size = tx_size;
	msg->hdr.error = 0;
	msg->hdr.padding = 0;

retry:
	if (ctx->fd_control < 0) {
		vtunerc_connect_control(ctx);
		if (ctx->fd_control < 0) {
			up(&ctx->xchange_sem);
			*pret = -ENXIO;
			return;
		}
	}
	len = rx_size;
	if (len < 0)
		goto tx_error;

	len += sizeof(msg->hdr);

	printk(KERN_INFO "vTuner: Doing message mt=%d rxs=%d txs=%d len=%d\n",
	    mtype, rx_size, tx_size, len);

	if (vtunerc_fd_write(ctx->fd_control, (u8 *) msg, len) != len)
		goto tx_error;

	len = tx_size;
	if (len < 0)
		goto tx_error;

	len += sizeof(msg->hdr);

	if (vtunerc_fd_read(ctx->fd_control, (u8 *) msg, len) != len) {
tx_error:
		printk(KERN_INFO "vTuner: Do message failed\n");
		close(ctx->fd_control);
		ctx->fd_control = -1;
		goto retry;
	}
	*pret = (s16) msg->hdr.error;

	printk(KERN_INFO "vTuner: Result %d, len=%d\n", *pret, len);

	up(&ctx->xchange_sem);
}

static void *
vtuner_reader_thread(void *arg)
{
	struct vtunerc_ctx *ctx = arg;
	int len;
	int type;

	while (1) {

		while (ctx->fd_data < 0) {
			if (ctx->fd_control > -1)
				vtunerc_connect_data(ctx);
			if (ctx->fd_data < 0)
				usleep(1000000);
		}

		wait_event(ctx->rd_queue, ctx->buffer_rem == 0);

		if (vtunerc_fd_read(ctx->fd_data, (u8 *) & ctx->buffer_hdr,
		    sizeof(ctx->buffer_hdr)) != sizeof(ctx->buffer_hdr))
			goto rx_error;

		if (ctx->buffer_hdr.magic != VTUNER_MAGIC) {
			vtuner_data_hdr_byteswap(&ctx->buffer_hdr);
			if (ctx->buffer_hdr.magic != VTUNER_MAGIC)
				goto rx_error;
		}
		len = VTUNER_GET_LEN(ctx->buffer_hdr.length);
		type = VTUNER_GET_TYPE(ctx->buffer_hdr.length);

		if (len < 0 || len > VTUNER_BUFFER_MAX)
			goto rx_error;

		if (vtunerc_fd_read(ctx->fd_data, (u8 *) ctx->buffer, len) != len) {
	rx_error:

			down(&ctx->rd_sem);
			ctx->buffer_rem = 0;
			ctx->buffer_off = 0;
			up(&ctx->rd_sem);

			close(ctx->fd_data);
			ctx->fd_data = -1;
		} else {

			switch (type) {
			case 0:
				if (!ctx->dmx_opened)
					continue;
				break;
			case 1:
				if (!ctx->dvr_opened)
					continue;
				break;
			default:
				continue;
			}

			down(&ctx->rd_sem);
			ctx->buffer_typ = type;
			ctx->buffer_rem = len;
			ctx->buffer_off = 0;
			up(&ctx->rd_sem);

			wake_up_all(&ctx->rd_queue);
			cuse_poll_wakeup();
		}

	}
	return (NULL);
}

static int
vtunerc_process_ioctl(struct vtunerc_ctx *ctx, unsigned int cmd, union vtuner_dvb_message *dvb)
{
	struct {
		struct dtv_properties dtv_properties;
	}      dummy;
	int ret = 0;
	u32 i;
	u32 max;

	switch (cmd) {
	case DMX_START:
		vtunerc_do_message(ctx, &ctx->msgbuf, MSG_DMX_START, 0, 0, &ret);
		break;
	case DMX_STOP:
		vtunerc_do_message(ctx, &ctx->msgbuf, MSG_DMX_STOP, 0, 0, &ret);
		break;
	case DMX_SET_FILTER:
		VTUNER_LOCAL_MEMSET(&ctx->msgbuf.body.dmx_sct_filter_params, 0, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_sct_filter_params.pid, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_sct_filter_params.filter, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_sct_filter_params.timeout, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_sct_filter_params.flags, &ret);
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_SET_FILTER,
		    sizeof(ctx->msgbuf.body.dmx_sct_filter_params), 0, &ret);
		break;
	case DMX_SET_PES_FILTER:
		VTUNER_LOCAL_MEMSET(&ctx->msgbuf.body.dmx_pes_filter_params, 0,
		    &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_pes_filter_params.pid, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_pes_filter_params.input, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_pes_filter_params.output, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_pes_filter_params.pes_type, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_pes_filter_params.flags, &ret);
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_SET_PES_FILTER,
		    sizeof(ctx->msgbuf.body.dmx_pes_filter_params), 0, &ret);
		break;
	case DMX_SET_BUFFER_SIZE:
		ctx->msgbuf.body.value32 = (long)dvb;

		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_SET_BUFFER_SIZE,
		    sizeof(u32), 0, &ret);
		break;
	case DMX_GET_PES_PIDS:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_GET_PES_PIDS,
		    0, sizeof(ctx->msgbuf.body.dmx_pes_pid), &ret);

		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, dmx_pes_pid.pids, &ret);
		break;
	case DMX_GET_CAPS:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_GET_CAPS,
		    0, sizeof(ctx->msgbuf.body.dmx_caps), &ret);

		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, dmx_caps.caps, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, dmx_caps.num_decoders, &ret);
		break;
	case DMX_SET_SOURCE:
		ctx->msgbuf.body.value32 = dvb->value32;

		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_SET_SOURCE,
		    sizeof(u32),
		    0, &ret);
		break;
	case DMX_GET_STC:
		VTUNER_LOCAL_MEMSET(&ctx->msgbuf.body.dmx_stc, 0,
		    &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_stc.num, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_stc.base, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dmx_stc.stc, &ret);
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_GET_STC,
		    sizeof(ctx->msgbuf.body.dmx_stc),
		    sizeof(ctx->msgbuf.body.dmx_stc), &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, dmx_stc.num, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, dmx_stc.base, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, dmx_stc.stc, &ret);
		break;
	case DMX_ADD_PID:
		ctx->msgbuf.body.value16 = dvb->value16;
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_ADD_PID,
		    sizeof(u16), 0, &ret);
		break;
	case DMX_REMOVE_PID:
		ctx->msgbuf.body.value16 = dvb->value16;
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_REMOVE_PID,
		    sizeof(u16), 0, &ret);
		break;

	case FE_SET_PROPERTY:
	case FE_GET_PROPERTY:
		VTUNER_LOCAL_MEMSET(&ctx->msgbuf.body.dtv_properties, 0,
		    &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dtv_properties.num, &ret);
		VTUNER_LOCAL_MEMCPY(&dummy, &(*dvb),
		    dtv_properties.props, &ret);

		max = ctx->msgbuf.body.dtv_properties.num;
		if (max > VTUNER_PROP_MAX) {
			ret |= -ENOMEM;
			break;
		}
		for (i = 0; i != max; i++) {
			VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &dummy,
			    dtv_properties.props[i].cmd, &ret);
			VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &dummy,
			    dtv_properties.props[i].reserved[0], &ret);
			VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &dummy,
			    dtv_properties.props[i].reserved[1], &ret);
			VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &dummy,
			    dtv_properties.props[i].reserved[2], &ret);

			if (ctx->msgbuf.body.dtv_properties.props[i].cmd != DTV_DISEQC_MASTER &&
			    ctx->msgbuf.body.dtv_properties.props[i].cmd != DTV_DISEQC_SLAVE_REPLY) {
				VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &dummy,
				    dtv_properties.props[i].u.data, &ret);
			} else {
				VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &dummy,
				    dtv_properties.props[i].u.buffer.len, &ret);
				VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &dummy,
				    dtv_properties.props[i].u.buffer.data, &ret);
			}
		}

		if (cmd == FE_SET_PROPERTY) {
			vtunerc_do_message(ctx, &ctx->msgbuf,
			    MSG_FE_SET_PROPERTY,
			    (u8 *) & ctx->msgbuf.body.dtv_properties.props[max] - (u8 *) & ctx->msgbuf.body,
			    0, &ret);
			break;
		}
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_GET_PROPERTY,
		    (u8 *) & ctx->msgbuf.body.dtv_properties.props[max] - (u8 *) & ctx->msgbuf.body,
		    (u8 *) & ctx->msgbuf.body.dtv_properties.props[max] - (u8 *) & ctx->msgbuf.body, &ret);

		for (i = 0; i != max; i++) {
			VTUNER_PEER_MEMSET(&dummy.dtv_properties.props[i], 0, &ret);
			VTUNER_PEER_MEMCPY(&dummy, &ctx->msgbuf.body, dtv_properties.props[i].cmd, &ret);
			VTUNER_PEER_MEMCPY(&dummy, &ctx->msgbuf.body, dtv_properties.props[i].reserved[0], &ret);
			VTUNER_PEER_MEMCPY(&dummy, &ctx->msgbuf.body, dtv_properties.props[i].reserved[1], &ret);
			VTUNER_PEER_MEMCPY(&dummy, &ctx->msgbuf.body, dtv_properties.props[i].reserved[2], &ret);

			if (ctx->msgbuf.body.dtv_properties.props[i].cmd != DTV_DISEQC_MASTER &&
			    ctx->msgbuf.body.dtv_properties.props[i].cmd != DTV_DISEQC_SLAVE_REPLY) {
				VTUNER_PEER_MEMCPY(&dummy, &ctx->msgbuf.body, dtv_properties.props[i].u.data, &ret);
			} else {
				VTUNER_PEER_MEMCPY(&dummy, &ctx->msgbuf.body, dtv_properties.props[i].u.buffer.len, &ret);
				VTUNER_PEER_MEMCPY(&dummy, &ctx->msgbuf.body, dtv_properties.props[i].u.buffer.data, &ret);
			}
		}
		break;

	case FE_GET_INFO:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_GET_INFO,
		    0,
		    sizeof(ctx->msgbuf.body.dvb_frontend_info), &ret);
		VTUNER_PEER_MEMSET(&(*dvb).dvb_frontend_info, 0, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.name, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.type, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.frequency_min, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.frequency_max, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.frequency_stepsize, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.frequency_tolerance, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.symbol_rate_min, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.symbol_rate_max, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.symbol_rate_tolerance, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.notifier_delay, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_info.caps, &ret);
		break;
	case FE_DISEQC_RESET_OVERLOAD:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_DISEQC_RESET_OVERLOAD, 0, 0, &ret);
		break;
	case FE_DISEQC_SEND_MASTER_CMD:
		VTUNER_LOCAL_MEMSET(&ctx->msgbuf.body.dvb_diseqc_master_cmd, 0,
		    &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb), dvb_diseqc_master_cmd.msg, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb), dvb_diseqc_master_cmd.msg_len, &ret);
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_DISEQC_SEND_MASTER_CMD,
		    sizeof(ctx->msgbuf.body.dvb_diseqc_master_cmd), 0, &ret);
		break;
	case FE_DISEQC_RECV_SLAVE_REPLY:
		VTUNER_LOCAL_MEMSET(&ctx->msgbuf.body.dvb_diseqc_slave_reply, 0,
		    &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_diseqc_slave_reply.timeout, &ret);
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_DISEQC_RECV_SLAVE_REPLY,
		    sizeof(ctx->msgbuf.body.dvb_diseqc_slave_reply),
		    sizeof(ctx->msgbuf.body.dvb_diseqc_slave_reply), &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, dvb_diseqc_slave_reply.msg, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, dvb_diseqc_slave_reply.msg_len, &ret);
		break;
	case FE_DISEQC_SEND_BURST:
		ctx->msgbuf.body.value32 = (long)dvb;
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_DISEQC_SEND_BURST,
		    sizeof(u32),
		    0, &ret);
		break;
	case FE_SET_TONE:
		ctx->msgbuf.body.value32 = (long)dvb;
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_SET_TONE,
		    sizeof(u32), 0, &ret);
		break;
	case FE_SET_VOLTAGE:
		ctx->msgbuf.body.value32 = (long)dvb;
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_SET_VOLTAGE,
		    sizeof(u32), 0, &ret);
		break;
	case FE_ENABLE_HIGH_LNB_VOLTAGE:
		ctx->msgbuf.body.value32 = (long)dvb;
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_ENABLE_HIGH_LNB_VOLTAGE,
		    sizeof(u32), 0, &ret);
		break;
	case FE_READ_STATUS:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_READ_STATUS,
		    0, sizeof(u32), &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, value32, &ret);
		break;
	case FE_READ_BER:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_READ_BER,
		    0, sizeof(u32), &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, value32, &ret);
		break;
	case FE_READ_SIGNAL_STRENGTH:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_READ_SIGNAL_STRENGTH,
		    0, sizeof(u16), &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, value16, &ret);
		break;
	case FE_READ_SNR:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_READ_SNR,
		    0, sizeof(u16), &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, value16, &ret);
		break;
	case FE_READ_UNCORRECTED_BLOCKS:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_READ_UNCORRECTED_BLOCKS,
		    0, sizeof(u32), &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body, value32, &ret);
		break;
	case FE_SET_FRONTEND:
		VTUNER_LOCAL_MEMSET(&ctx->msgbuf.body.dvb_frontend_parameters, 0,
		    &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_frontend_parameters.frequency, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_frontend_parameters.inversion, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_frontend_parameters.u.ofdm.bandwidth, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_frontend_parameters.u.ofdm.code_rate_HP, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_frontend_parameters.u.ofdm.code_rate_LP, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_frontend_parameters.u.ofdm.constellation, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_frontend_parameters.u.ofdm.transmission_mode, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_frontend_parameters.u.ofdm.guard_interval, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dvb_frontend_parameters.u.ofdm.hierarchy_information, &ret);
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_SET_FRONTEND,
		    sizeof(ctx->msgbuf.body.dvb_frontend_parameters), 0, &ret);
		break;
	case FE_GET_FRONTEND:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_GET_FRONTEND,
		    0, sizeof(ctx->msgbuf.body.dvb_frontend_parameters), &ret);
		VTUNER_PEER_MEMSET(&(*dvb).dvb_frontend_parameters, 0, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_parameters.frequency, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_parameters.inversion, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_parameters.u.ofdm.bandwidth, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_parameters.u.ofdm.code_rate_HP, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_parameters.u.ofdm.code_rate_LP, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_parameters.u.ofdm.constellation, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_parameters.u.ofdm.transmission_mode, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_parameters.u.ofdm.guard_interval, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_parameters.u.ofdm.hierarchy_information, &ret);
		break;
	case FE_SET_FRONTEND_TUNE_MODE:
		ctx->msgbuf.body.value32 = (long)dvb;
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_SET_FRONTEND_TUNE_MODE,
		    sizeof(u32),
		    0, &ret);
		break;
	case FE_GET_EVENT:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_GET_EVENT,
		    0, sizeof(ctx->msgbuf.body.dvb_frontend_event), &ret);
		VTUNER_PEER_MEMSET(&(*dvb).dvb_frontend_event, 0, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.status, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.parameters.frequency, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.parameters.inversion, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.parameters.u.ofdm.bandwidth, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.parameters.u.ofdm.code_rate_HP, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.parameters.u.ofdm.code_rate_LP, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.parameters.u.ofdm.constellation, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.parameters.u.ofdm.transmission_mode, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.parameters.u.ofdm.guard_interval, &ret);
		VTUNER_PEER_MEMCPY(&(*dvb), &ctx->msgbuf.body,
		    dvb_frontend_event.parameters.u.ofdm.hierarchy_information, &ret);
		break;
	case FE_DISHNETWORK_SEND_LEGACY_CMD:
		ctx->msgbuf.body.value32 = (long)dvb;
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_DISHNETWORK_SEND_LEGACY_CMD,
		    sizeof(u32), 0, &ret);
		break;
	default:
		ret |= -1U;
		break;
	}
	return (ret);
}

/*------------------------------------------------------------------------*
 * vTuner client Cuse4BSD interface
 *------------------------------------------------------------------------*/
static void
vtunerc_work_exec_hup(int dummy)
{

}

static void *
vtunerc_work(void *arg)
{
	signal(SIGHUP, vtunerc_work_exec_hup);

	while (1) {
		if (cuse_wait_and_process() != 0)
			break;
	}

	exit(0);			/* we are done */

	return (NULL);
}

static int
vtunerc_convert_error(int error)
{
	;				/* indent fix */
	if (error < 0) {
		switch (error) {
		case -EBUSY:
			error = CUSE_ERR_BUSY;
			break;
		case -EWOULDBLOCK:
			error = CUSE_ERR_WOULDBLOCK;
			break;
		case -EINVAL:
			error = CUSE_ERR_INVALID;
			break;
		case -ENOMEM:
			error = CUSE_ERR_NO_MEMORY;
			break;
		case -EFAULT:
			error = CUSE_ERR_FAULT;
			break;
		case -EINTR:
			error = CUSE_ERR_SIGNAL;
			break;
		default:
			error = CUSE_ERR_OTHER;
			break;
		}
	}
	return (error);
}

static int
vtunerc_open(struct cuse_dev *cdev, int fflags)
{
	struct vtunerc_ctx *ctx;
	long devtype;

	ctx = cuse_dev_get_priv0(cdev);
	devtype = (long)cuse_dev_get_priv1(cdev);

	switch (devtype) {
	case VTUNERC_DT_FRONTEND:
		if (ctx->frontend_opened)
			return (CUSE_ERR_BUSY);
		ctx->frontend_opened = 1;
		break;
	case VTUNERC_DT_DMX:
		if (ctx->dmx_opened)
			return (CUSE_ERR_BUSY);
		ctx->dmx_opened = 1;
		break;
	case VTUNERC_DT_DVR:
		if (ctx->dvr_opened)
			return (CUSE_ERR_BUSY);
		ctx->dvr_opened = 1;
		break;
	default:
		return (CUSE_ERR_INVALID);
	}

	return (0);
}

static int
vtunerc_close(struct cuse_dev *cdev, int fflags)
{
	struct vtunerc_ctx *ctx;
	long devtype;

	ctx = cuse_dev_get_priv0(cdev);
	devtype = (long)cuse_dev_get_priv1(cdev);

	switch (devtype) {
	case VTUNERC_DT_FRONTEND:
		ctx->frontend_opened = 0;
		break;
	case VTUNERC_DT_DMX:
		ctx->dmx_opened = 0;

		down(&ctx->rd_sem);
		if (ctx->buffer_rem != 0 && ctx->buffer_typ == 0) {
			ctx->buffer_rem = 0;
			wake_up_all(&ctx->rd_queue);
			cuse_poll_wakeup();
		}
		up(&ctx->rd_sem);
		break;
	case VTUNERC_DT_DVR:
		ctx->dvr_opened = 0;

		down(&ctx->rd_sem);
		if (ctx->buffer_rem != 0 && ctx->buffer_typ == 1) {
			ctx->buffer_rem = 0;
			wake_up_all(&ctx->rd_queue);
			cuse_poll_wakeup();
		}
		up(&ctx->rd_sem);
		break;
	default:
		break;
	}
	return (0);
}

static int
vtunerc_read(struct cuse_dev *cdev, int fflags,
    void *peer_ptr, int len)
{
	struct vtunerc_ctx *ctx;
	long devtype;
	u8 type;
	int delta;
	int off;

	ctx = cuse_dev_get_priv0(cdev);
	devtype = (long)cuse_dev_get_priv1(cdev);

	switch (devtype) {
	case VTUNERC_DT_DVR:
		type = 1;
		break;
	case VTUNERC_DT_DMX:
		type = 0;
		break;
	default:
		return (CUSE_ERR_INVALID);
	}

	off = 0;

repeat:
	delta = len;

	down(&ctx->rd_sem);
	if (ctx->buffer_typ == type) {
		if ((u32) delta > ctx->buffer_rem)
			delta = ctx->buffer_rem;
	} else {
		delta = 0;
	}

	if (delta != 0) {

		if (copy_to_user(((u8 *) peer_ptr) + off, ((u8 *) ctx->buffer) +
		    ctx->buffer_off, delta) != 0)
			return (CUSE_ERR_FAULT);

		ctx->buffer_off += delta;
		ctx->buffer_rem -= delta;

		len -= delta;
		off += delta;

		wake_up_all(&ctx->rd_queue);
	}
	up(&ctx->rd_sem);

	if (len) {
		if (fflags & CUSE_FFLAG_NONBLOCK) {
			delta = wait_event_interruptible(ctx->rd_queue,
			    ctx->buffer_rem != 0 && ctx->buffer_typ == type);
			if (delta)
				return (CUSE_ERR_SIGNAL);
			else
				goto repeat;
		} else {
			if (delta == 0)
				return (CUSE_ERR_WOULDBLOCK);
		}
	}
	return (off);
}

static int
vtunerc_write(struct cuse_dev *cdev, int fflags,
    const void *peer_ptr, int len)
{
	return (CUSE_ERR_INVALID);
}

static int
vtunerc_ioctl(struct cuse_dev *cdev, int fflags,
    unsigned long cmd, void *peer_data)
{
	struct vtunerc_ctx *ctx;
	long devtype;
	int error;

	ctx = cuse_dev_get_priv0(cdev);
	devtype = (long)cuse_dev_get_priv1(cdev);

	/* we support blocking/non-blocking I/O */
	if (cmd == FIONBIO || cmd == FIOASYNC)
		return (0);

	down(&ctx->ioctl_sem);
	error = vtunerc_process_ioctl(ctx, cmd, peer_data);
	up(&ctx->ioctl_sem);

	return (vtunerc_convert_error(error));
}

static int
vtunerc_poll(struct cuse_dev *cdev, int fflags, int events)
{
	struct vtunerc_ctx *ctx;
	long devtype;
	int revents;

	ctx = cuse_dev_get_priv0(cdev);
	devtype = (long)cuse_dev_get_priv1(cdev);

	revents = 0;

	switch (devtype) {
	case VTUNERC_DT_DVR:
		down(&ctx->rd_sem);
		if (ctx->buffer_rem != 0 && ctx->buffer_typ == 1)
			revents |= events & CUSE_POLL_READ;
		up(&ctx->rd_sem);
		break;
	case VTUNERC_DT_DMX:
		down(&ctx->rd_sem);
		if (ctx->buffer_rem != 0 && ctx->buffer_typ == 0)
			revents |= events & CUSE_POLL_READ;
		up(&ctx->rd_sem);
		break;
	default:
		return (0);
	}

#if 0
	if (error & (POLLOUT | POLLWRNORM))
		revents |= events & CUSE_POLL_WRITE;

	/* currently we mask away any poll errors */
	if (error & (POLLHUP | POLLNVAL | POLLERR))
		revents |= events & CUSE_POLL_ERROR;
#endif
	return (revents);
}

/*------------------------------------------------------------------------*
 * vTuner client init and exit
 *------------------------------------------------------------------------*/

static int __init
vtunerc_init(void)
{
	struct vtunerc_ctx *ctx = NULL;
	char buf[64];
	int u;
	int p;

	if (vtuner_max_unit < 0 || vtuner_max_unit > CONFIG_DVB_MAX_ADAPTERS)
		vtuner_max_unit = CONFIG_DVB_MAX_ADAPTERS;

	printk(KERN_INFO "virtual DVB client adapter driver, version "
	    VTUNER_MODULE_VERSION ", (c) 2011 Hans Petter Selasky\n");

	for (u = 0; u < vtuner_max_unit; u++) {
		ctx = kzalloc(sizeof(struct vtunerc_ctx), GFP_KERNEL);
		if (!ctx)
			return -ENOMEM;

		vtunerc_tbl[u] = ctx;

		init_waitqueue_head(&ctx->rd_queue);

		sema_init(&ctx->rd_sem, 1);
		sema_init(&ctx->xchange_sem, 1);
		sema_init(&ctx->ioctl_sem, 1);

		ctx->fd_control = -1;
		ctx->fd_data = -1;

		snprintf(ctx->cport, sizeof(ctx->cport),
		    "%u", atoi(vtuner_cport) + (2 * u));

		snprintf(ctx->dport, sizeof(ctx->dport),
		    "%u", atoi(vtuner_cport) + (2 * u) + 1);

		/* create reader thread */
		pthread_create(&ctx->reader_thread, NULL, vtuner_reader_thread, ctx);

		for (p = 0; p != (3 * 4); p++) {
			pthread_t dummy;

			if (pthread_create(&dummy, NULL, vtunerc_work, NULL))
				printk(KERN_INFO "Failed creating vTuncer client process\n");
		}

		snprintf(buf, sizeof(buf), "dvb/adapter%d/frontend0", webcamd_unit + u);

		printf("Creating /dev/%s (vTuner client)\n", buf);

		cuse_dev_create(&vtunerc_methods, ctx, (void *)VTUNERC_DT_FRONTEND,
		    v4b_get_uid(), v4b_get_gid(), v4b_get_perm(), "%s", buf);

		if (webcamd_hal_register)
			hal_add_device(buf);

		snprintf(buf, sizeof(buf), "dvb/adapter%d/dvr0", webcamd_unit + u);

		printf("Creating /dev/%s (vTuner client)\n", buf);

		cuse_dev_create(&vtunerc_methods, ctx, (void *)VTUNERC_DT_DVR,
		    v4b_get_uid(), v4b_get_gid(), v4b_get_perm(), "%s", buf);

		if (webcamd_hal_register)
			hal_add_device(buf);

		snprintf(buf, sizeof(buf), "dvb/adapter%d/demux0", webcamd_unit + u);

		printf("Creating /dev/%s (vTuner client)\n", buf);

		cuse_dev_create(&vtunerc_methods, ctx, (void *)VTUNERC_DT_DMX,
		    v4b_get_uid(), v4b_get_gid(), v4b_get_perm(), "%s", buf);

		if (webcamd_hal_register)
			hal_add_device(buf);
	}
	return 0;
}

static void __exit
vtunerc_exit(void)
{
#if 0
	struct dvb_demux *dvbdemux;
	struct dmx_demux *dmx;
	int u;

	for (u = 0; u < vtuner_max_unit; u++) {
		struct vtunerc_ctx *ctx = vtunerc_tbl[u];

		if (!ctx)
			continue;
		vtunerc_tbl[u] = NULL;

		XXX TODO;

		kfree(ctx);
	}

	printk(KERN_NOTICE "vtunerc: unloaded successfully\n");
#endif
}

module_init(vtunerc_init);
module_exit(vtunerc_exit);

module_param_named(devices, vtuner_max_unit, int, 0644);
MODULE_PARM_DESC(devices, "Number of clients (default is 0, disabled)");

module_param_string(host, vtuner_host, sizeof(vtuner_host), 0644);
MODULE_PARM_DESC(host, "Destination host (default is 127.0.0.1)");

module_param_string(cport, vtuner_cport, sizeof(vtuner_cport), 0644);
MODULE_PARM_DESC(cport, "Destination port (default is 5100)");

MODULE_AUTHOR("Hans Petter Selasky");
MODULE_DESCRIPTION("Virtual DVB device server");
MODULE_LICENSE("BSD");
MODULE_VERSION(VTUNER_MODULE_VERSION);
