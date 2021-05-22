/*-
 * Copyright (c) 2011-2020 Hans Petter Selasky. All rights reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define	__DVB_CORE__

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

#include <cuse.h>

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

#define	DPRINTF(fmt,...) do {				\
	if (vtuner_debug) {				\
		printk(KERN_INFO "%s:%s:%d: " fmt,	\
		    __FILE__, __FUNCTION__,		\
		    __LINE__,## __VA_ARGS__);		\
	}						\
} while (0)

static int vtuner_max_unit;
static int vtuner_debug;
static char vtuner_host[64] = {"127.0.0.1"};
static char vtuner_port[16] = {VTUNER_DEFAULT_PORT};

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

static int
vtunerc_connect(const char *host, const char *port, int buffer)
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

	DPRINTF("Trying to connect to %s:%s\n", host, port);

	if ((error = getaddrinfo(host, port, &hints, &res)))
		return (-1);

	res0 = res;

	do {
		if ((s = socket(res0->ai_family, res0->ai_socktype,
		    res0->ai_protocol)) < 0)
			continue;

		flag = 1;
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, (int)sizeof(flag));

		setsockopt(s, SOL_SOCKET, SO_SNDBUF, &buffer, (int)sizeof(buffer));
		setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffer, (int)sizeof(buffer));

		if (connect(s, res0->ai_addr, res0->ai_addrlen) == 0)
			break;

		close(s);
		s = -1;
	} while ((res0 = res0->ai_next) != NULL);

	freeaddrinfo(res);

	DPRINTF("Connected, fd=%d\n", s);

	return (s);
}

static int
vtunerc_fd_read(int fd, u8 * ptr, int len)
{
	int off = 0;
	int err;

	DPRINTF("\n");

	while (off < len) {
		err = read(fd, ptr + off, len - off);
		if (err <= 0) {
			DPRINTF("Read error %d\n", err);
			return (err);
		}
		off += err;
	}
	return (off);
}

static int
vtunerc_fd_write(int fd, const u8 * ptr, int len)
{
	int off = 0;
	int err;

	DPRINTF("\n");

	while (off < len) {
		err = write(fd, ptr + off, len - off);
		if (err <= 0) {
			DPRINTF("Write error %d\n", err);
			return (err);
		}
		off += err;
	}
	return (off);
}

static void
vtunerc_do_message(struct vtunerc_ctx *ctx,
    struct vtuner_message *msg, int mtype,
    int rx_size, int tx_size, int *pret)
{
	int len;

	/* if an error is already set, just return */

	if (*pret != 0)
		return;

	/* stamp the byte order and version */

	msg->hdr.magic = VTUNER_MAGIC;
	msg->hdr.mtype = mtype;
	msg->hdr.rx_size = rx_size;
	msg->hdr.tx_size = tx_size;
	msg->hdr.error = 0;
	msg->hdr.padding = 0;

	len = rx_size;
	if (len < 0) {
		DPRINTF("Bad RX size %d\n", rx_size);
		goto error;
	}
	len += sizeof(msg->hdr);

	DPRINTF("Doing message mt=%d rxs=%d txs=%d len=%d\n",
	    mtype, rx_size, tx_size, len);

	if (vtunerc_fd_write(ctx->fd_ctrl_peer, (u8 *) msg, len) != len) {
		DPRINTF("Bad write of length %d\n", len);
		goto error;
	}
	len = tx_size;
	if (len < 0) {
		DPRINTF("Bad TX size %d\n", tx_size);
		goto error;
	}
	len += sizeof(msg->hdr);

	if (vtunerc_fd_read(ctx->fd_ctrl_peer, (u8 *) msg, len) != len) {
		DPRINTF("Bad read of length %d\n", len);
		goto error;
	}
	*pret = (s16) msg->hdr.error;

	DPRINTF("Result %d, len=%d\n", *pret, len);
	return;

error:
	*pret = -ENXIO;
	return;
}

static int
vtunerc_do_peer_read(struct vtunerc_ctx *ctx)
{
	int len;

	DPRINTF("\n");

	if (ctx->buffer_rem != 0)
		return (0);

	len = vtunerc_fd_read(ctx->fd_data_peer,
	    (u8 *) & ctx->buffer, 8);

	if (len != 8) {
		DPRINTF("Bad peer read of 8 bytes\n");
		return (-1);;
	}
	if (ctx->buffer[0] != VTUNER_MAGIC) {
		vtuner_data_hdr_byteswap(ctx->buffer);
		if (ctx->buffer[0] != VTUNER_MAGIC) {
			DPRINTF("Bad magic 0x%08x != "
			    "0x%08x\n", ctx->buffer[0], VTUNER_MAGIC);
			return (-1);
		}
	}
	len = ctx->buffer[1];

	if (len < 0 || len > VTUNER_BUFFER_MAX) {
		DPRINTF("Bad receive length (%d)\n", len);
		return (-1);;
	}
	if (vtunerc_fd_read(ctx->fd_data_peer,
	    (u8 *) ctx->buffer, len) != len) {
		DPRINTF("Bad peer read of %d bytes\n", len);
		return (-1);;
	}
	ctx->buffer_rem = len;
	ctx->buffer_off = 0;

	if (ctx->rd_message == 1) {
		ctx->rd_message = 0;
		pthread_kill(ctx->poll_thread, SIGURG);
	}
	return (0);
}

static void *
vtunerc_do_peer_poll(void *arg)
{
	struct vtunerc_ctx *ctx = arg;
	struct pollfd fds[1];

	DPRINTF("\n");

	while (!ctx->closing) {
		fds[0].fd = ctx->fd_data_peer;
		fds[0].revents = 0;
		fds[0].events = (POLLRDNORM | POLLIN);

		poll(fds, 1, 250);

		if (fds[0].revents) {
			if (ctx->rd_message == 0)
				ctx->rd_message = 1;
			cuse_poll_wakeup();
			usleep(250000);
		}
	}
	ctx->rd_message = -1;
	return (NULL);
}

static int
vtunerc_process_ioctl(struct vtunerc_ctx *ctx, unsigned int cmd, union vtuner_dvb_message *dvb)
{
	int ret = 0;
	u32 i;
	u32 max;

	DPRINTF("\n");

	switch (cmd) {
	case DMX_START:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_START, 0, 0, &ret);
		break;
	case DMX_STOP:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_STOP, 0, 0, &ret);
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
		    MSG_DMX_ADD_PID, sizeof(u16), 0, &ret);
		break;
	case DMX_REMOVE_PID:
		ctx->msgbuf.body.value16 = dvb->value16;
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_DMX_REMOVE_PID, sizeof(u16), 0, &ret);
		break;

	case FE_SET_PROPERTY:
	case FE_GET_PROPERTY:
		VTUNER_LOCAL_MEMSET(&ctx->msgbuf.body.dtv_properties, 0,
		    &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &(*dvb),
		    dtv_properties.num, &ret);
		VTUNER_LOCAL_MEMCPY(&ctx->msgdummy, &(*dvb),
		    dtv_properties.props, &ret);

		max = ctx->msgbuf.body.dtv_properties.num;
		if (max > VTUNER_PROP_MAX) {
			ret |= -ENOMEM;
			break;
		}
		for (i = 0; i != max; i++) {
			VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &ctx->msgdummy,
			    dtv_properties.props[i].cmd, &ret);
			VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &ctx->msgdummy,
			    dtv_properties.props[i].reserved[0], &ret);
			VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &ctx->msgdummy,
			    dtv_properties.props[i].reserved[1], &ret);
			VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &ctx->msgdummy,
			    dtv_properties.props[i].reserved[2], &ret);

			if (ctx->msgbuf.body.dtv_properties.props[i].cmd != DTV_DISEQC_MASTER &&
			    ctx->msgbuf.body.dtv_properties.props[i].cmd != DTV_DISEQC_SLAVE_REPLY) {
				VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &ctx->msgdummy,
				    dtv_properties.props[i].u.data, &ret);
			} else {
				VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &ctx->msgdummy,
				    dtv_properties.props[i].u.buffer.len, &ret);
				VTUNER_LOCAL_MEMCPY(&ctx->msgbuf.body, &ctx->msgdummy,
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
			VTUNER_PEER_MEMSET(&ctx->msgdummy.dtv_properties.props[i], 0, &ret);
			VTUNER_PEER_MEMCPY(&ctx->msgdummy, &ctx->msgbuf.body, dtv_properties.props[i].cmd, &ret);
			VTUNER_PEER_MEMCPY(&ctx->msgdummy, &ctx->msgbuf.body, dtv_properties.props[i].reserved[0], &ret);
			VTUNER_PEER_MEMCPY(&ctx->msgdummy, &ctx->msgbuf.body, dtv_properties.props[i].reserved[1], &ret);
			VTUNER_PEER_MEMCPY(&ctx->msgdummy, &ctx->msgbuf.body, dtv_properties.props[i].reserved[2], &ret);

			if (ctx->msgbuf.body.dtv_properties.props[i].cmd != DTV_DISEQC_MASTER &&
			    ctx->msgbuf.body.dtv_properties.props[i].cmd != DTV_DISEQC_SLAVE_REPLY) {
				VTUNER_PEER_MEMCPY(&ctx->msgdummy, &ctx->msgbuf.body, dtv_properties.props[i].u.data, &ret);
			} else {
				VTUNER_PEER_MEMCPY(&ctx->msgdummy, &ctx->msgbuf.body, dtv_properties.props[i].u.buffer.len, &ret);
				VTUNER_PEER_MEMCPY(&ctx->msgdummy, &ctx->msgbuf.body, dtv_properties.props[i].u.buffer.data, &ret);
			}
		}
		break;

	case FE_GET_INFO:
		vtunerc_do_message(ctx, &ctx->msgbuf,
		    MSG_FE_GET_INFO,
		    0, sizeof(ctx->msgbuf.body.dvb_frontend_info), &ret);
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
		    sizeof(u32), 0, &ret);
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
		    sizeof(u32), 0, &ret);
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
vtunerc_cuse_thread_hup(int dummy)
{
	DPRINTF("\n");
}

static void *
vtunerc_cuse_thread(void *arg)
{
	signal(SIGHUP, vtunerc_cuse_thread_hup);

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
	struct vtunerc_config *cfg;

	DPRINTF("\n");

	cfg = cuse_dev_get_priv0(cdev);
	if (cfg == NULL)
		return (CUSE_ERR_INVALID);

	ctx = kzalloc(sizeof(struct vtunerc_ctx), GFP_KERNEL);
	if (ctx == NULL)
		return (CUSE_ERR_NO_MEMORY);

	ctx->fd_ctrl_peer = vtunerc_connect(cfg->host,
	    cfg->cport, 4096);
	if (ctx->fd_ctrl_peer < 0) {
		kfree(ctx);
		return (CUSE_ERR_OTHER);
	}
	ctx->fd_data_peer = vtunerc_connect(cfg->host,
	    cfg->dport, 2 * VTUNER_BUFFER_MAX);
	if (ctx->fd_data_peer < 0) {
		close(ctx->fd_ctrl_peer);
		kfree(ctx);
		return (CUSE_ERR_OTHER);
	}
	pthread_create(&ctx->poll_thread, NULL,
	    &vtunerc_do_peer_poll, ctx);

	cuse_dev_set_per_file_handle(cdev, ctx);

	return (0);
}

static int
vtunerc_close(struct cuse_dev *cdev, int fflags)
{
	struct vtunerc_ctx *ctx;

	DPRINTF("\n");

	ctx = cuse_dev_get_per_file_handle(cdev);
	cuse_dev_set_per_file_handle(cdev, NULL);

	close(ctx->fd_ctrl_peer);
	close(ctx->fd_data_peer);

	ctx->closing = 1;

	while (ctx->rd_message != -1)
		usleep(250000);

	kfree(ctx);

	return (0);
}

static int
vtunerc_read(struct cuse_dev *cdev, int fflags,
    void *peer_ptr, int len)
{
	struct vtunerc_ctx *ctx;
	int delta;
	int off;

	ctx = cuse_dev_get_per_file_handle(cdev);

	off = 0;

	DPRINTF("\n");

repeat:
	if (cuse_got_peer_signal() == 0)
		return (CUSE_ERR_SIGNAL);

	if ((fflags & CUSE_FFLAG_NONBLOCK) == 0 || ctx->rd_message == 1) {
		if (vtunerc_do_peer_read(ctx) < 0)
			return (CUSE_ERR_OTHER);
	}
	delta = len;

	if ((u32) delta > ctx->buffer_rem)
		delta = ctx->buffer_rem;

	if (delta != 0) {
		if (copy_to_user(((u8 *) peer_ptr) + off, ((u8 *) ctx->buffer) +
		    ctx->buffer_off, delta) != 0) {
			return (CUSE_ERR_FAULT);
		}
		ctx->buffer_off += delta;
		ctx->buffer_rem -= delta;

		len -= delta;
		off += delta;
	}
	if ((fflags & CUSE_FFLAG_NONBLOCK) == 0) {

		if (off) {
			DPRINTF("Return %d\n", off);
			return (off);
		}
		goto repeat;
	}
	if (off == 0)
		off = CUSE_ERR_WOULDBLOCK;

	DPRINTF("Return %d\n", off);

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
	int error;

	ctx = cuse_dev_get_per_file_handle(cdev);

	DPRINTF("\n");

	/* we support blocking/non-blocking I/O */
	if (cmd == FIONBIO || cmd == FIOASYNC)
		return (0);

	error = vtunerc_process_ioctl(ctx, cmd, peer_data);

	return (vtunerc_convert_error(error));
}

static int
vtunerc_poll(struct cuse_dev *cdev, int fflags, int events)
{
	struct vtunerc_ctx *ctx;
	int revents;

	ctx = cuse_dev_get_per_file_handle(cdev);

	if (ctx->rd_message == 1) {
		revents = events & CUSE_POLL_READ;
	} else {
		revents = 0;
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

static struct vtunerc_config *
vtunerc_make_config(int off)
{
	struct vtunerc_config *cfg;

	cfg = kzalloc(sizeof(struct vtunerc_config), GFP_KERNEL);
	if (cfg == NULL)
		return (NULL);

	cfg->host = vtuner_host;
	snprintf(cfg->cport, sizeof(cfg->cport), "%u", atoi(vtuner_port) + off);
	snprintf(cfg->dport, sizeof(cfg->dport), "%u", atoi(vtuner_port) + off + 1);
	return (cfg);
}

static int __init
vtunerc_init(void)
{
	struct vtunerc_config *cfg = NULL;
	char buf[64];
	int u;

	if (vtuner_max_unit < 0 || vtuner_max_unit > CONFIG_DVB_MAX_ADAPTERS)
		vtuner_max_unit = CONFIG_DVB_MAX_ADAPTERS;

	DPRINTF("virtual DVB client adapter driver, version "
	    VTUNER_MODULE_VERSION ", (c) 2011 Hans Petter Selasky\n");

	for (u = 0; u != (4 * vtuner_max_unit); u++) {
		pthread_t dummy;

		if (pthread_create(&dummy, NULL, vtunerc_cuse_thread, NULL))
			DPRINTF("Failed creating vTuncer client process\n");
	}

	for (u = 0; u != vtuner_max_unit; u++) {

		int unit = -1;
		int id = webcamd_devnames[F_V4B_DVB_FRONTEND][0] - 'A';

		if (cuse_alloc_unit_number_by_id(&unit,
		    CUSE_ID_WEBCAMD(id)) != 0) {
			printf("Could not allocate unit number\n");
			continue;
		}

		snprintf(buf, sizeof(buf), webcamd_devnames[F_V4B_DVB_FRONTEND] + 1, unit, 0);

		printf("Creating /dev/%s (vTuner client)\n", buf);

		cuse_dev_create(&vtunerc_methods, vtunerc_make_config(0 + (8 * u)), NULL,
		    v4b_get_uid(), v4b_get_gid(), v4b_get_perm(), "%s", buf);

		snprintf(buf, sizeof(buf), webcamd_devnames[F_V4B_DVB_DVR] + 1, unit, 0);

		printf("Creating /dev/%s (vTuner client)\n", buf);

		cuse_dev_create(&vtunerc_methods, vtunerc_make_config(2 + (8 * u)), NULL,
		    v4b_get_uid(), v4b_get_gid(), v4b_get_perm(), "%s", buf);

		snprintf(buf, sizeof(buf), webcamd_devnames[F_V4B_DVB_DEMUX] + 1, unit, 0);

		printf("Creating /dev/%s (vTuner client)\n", buf);

		cuse_dev_create(&vtunerc_methods, vtunerc_make_config(4 + (8 * u)), NULL,
		    v4b_get_uid(), v4b_get_gid(), v4b_get_perm(), "%s", buf);
	}
	return 0;
}

module_init(vtunerc_init);

module_param_named(debug, vtuner_debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable debugging (default is 0, disabled)");

module_param_named(devices, vtuner_max_unit, int, 0644);
MODULE_PARM_DESC(devices, "Number of clients (default is 0, disabled)");

module_param_string(host, vtuner_host, sizeof(vtuner_host), 0644);
MODULE_PARM_DESC(host, "Destination host (default is 127.0.0.1)");

module_param_string(port, vtuner_port, sizeof(vtuner_port), 0644);
MODULE_PARM_DESC(port, "Destination port (default is 5100)");

MODULE_AUTHOR("Hans Petter Selasky");
MODULE_DESCRIPTION("Virtual DVB device server");
MODULE_LICENSE("BSD");
MODULE_VERSION(VTUNER_MODULE_VERSION);
