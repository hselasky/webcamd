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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define	__DVB_CORE__

#include "dvb_demux.h"
#include "dvb_frontend.h"

#include <vtuner/vtuner.h>
#include <vtuner/vtuner_server.h>
#include <vtuner/vtuner_common.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>

#ifdef HAVE_CUSE
#include <cuse.h>
#else
#include <cuse4bsd.h>
#endif

#define	VTUNER_MODULE_VERSION "1.0-hps"

#define	VTUNER_MEMSET(a,b)			\
  memset(a,b,sizeof(*(a)))

#define	VTUNER_MEMCPY(a,b,c) do {					\
  extern int dummy[(sizeof(*(a.c)) != sizeof(*(b.c))) ? -1 : 1];	\
  memcpy(a.c, b.c, sizeof(*(a.c)));					\
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

static void
vtuners_work_exec_hup(int dummy)
{
	DPRINTF("\n");
}

static int
vtuners_process_msg(struct vtuners_ctx *ctx, struct vtuner_message *msg)
{
	int ret = -1;
	int max;
	int i;

	DPRINTF("\n");

	switch (msg->hdr.mtype) {
	case MSG_DMX_START:
		ret = linux_ioctl(ctx->proxy_fd,
		    CUSE_FFLAG_NONBLOCK, DMX_START, NULL);
		break;
	case MSG_DMX_STOP:
		ret = linux_ioctl(ctx->proxy_fd,
		    CUSE_FFLAG_NONBLOCK, DMX_STOP, NULL);
		break;
	case MSG_DMX_SET_FILTER:
		VTUNER_MEMSET(&ctx->dvb.dmx_sct_filter_params, 0);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_sct_filter_params.pid);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_sct_filter_params.filter);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_sct_filter_params.timeout);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_sct_filter_params.flags);
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    DMX_SET_FILTER, &ctx->dvb.dmx_sct_filter_params);
		break;
	case MSG_DMX_SET_PES_FILTER:
		VTUNER_MEMSET(&ctx->dvb.dmx_pes_filter_params, 0);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_pes_filter_params.pid);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_pes_filter_params.input);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_pes_filter_params.output);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_pes_filter_params.pes_type);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_pes_filter_params.flags);
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    DMX_SET_PES_FILTER, &ctx->dvb.dmx_pes_filter_params);
		break;
	case MSG_DMX_SET_BUFFER_SIZE:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    DMX_SET_BUFFER_SIZE, (void *)(long)msg->body.value32);
		break;
	case MSG_DMX_GET_PES_PIDS:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    DMX_GET_PES_PIDS, msg->body.dmx_pes_pid.pids);
		break;
	case MSG_DMX_GET_CAPS:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    DMX_GET_CAPS, &ctx->dvb.dmx_caps);

		VTUNER_MEMCPY(&msg->body, &ctx->dvb, dmx_caps.caps);
		VTUNER_MEMCPY(&msg->body, &ctx->dvb, dmx_caps.num_decoders);
		break;
	case MSG_DMX_SET_SOURCE:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    DMX_SET_SOURCE, &msg->body.value32);
		break;
	case MSG_DMX_GET_STC:
		VTUNER_MEMSET(&ctx->dvb.dmx_stc, 0);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_stc.num);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_stc.base);
		VTUNER_MEMCPY(&ctx->dvb,
		    &msg->body, dmx_stc.stc);
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    DMX_GET_STC, &ctx->dvb.dmx_stc);
		VTUNER_MEMCPY(&msg->body,
		    &ctx->dvb, dmx_stc.num);
		VTUNER_MEMCPY(&msg->body,
		    &ctx->dvb, dmx_stc.base);
		VTUNER_MEMCPY(&msg->body,
		    &ctx->dvb, dmx_stc.stc);
		break;
	case MSG_DMX_ADD_PID:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    DMX_ADD_PID, &msg->body.value16);
		break;
	case MSG_DMX_REMOVE_PID:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    DMX_REMOVE_PID, &msg->body.value16);
		break;

	case MSG_FE_GET_PROPERTY:
	case MSG_FE_SET_PROPERTY:

		max = msg->body.dtv_properties.num;

		if (max > VTUNER_PROP_MAX)
			msg->body.dtv_properties.num = max = VTUNER_PROP_MAX;

		VTUNER_MEMSET(&ctx->dvb.dtv_properties, 0);
		ctx->dvb.dtv_properties.num = max;
		ctx->dvb.dtv_properties.props = ctx->dtv_props;

		for (i = 0; i != max; i++) {
			VTUNER_MEMSET(&ctx->dtv_props[i], 0);
			VTUNER_MEMCPY(&ctx->dtv_props[i],
			    &msg->body.dtv_properties.props[i], cmd);
			VTUNER_MEMCPY(&ctx->dtv_props[i],
			    &msg->body.dtv_properties.props[i], reserved[0]);
			VTUNER_MEMCPY(&ctx->dtv_props[i],
			    &msg->body.dtv_properties.props[i], reserved[1]);
			VTUNER_MEMCPY(&ctx->dtv_props[i],
			    &msg->body.dtv_properties.props[i], reserved[2]);

			if (msg->body.dtv_properties.props[i].cmd != DTV_DISEQC_MASTER &&
			    msg->body.dtv_properties.props[i].cmd != DTV_DISEQC_SLAVE_REPLY) {
				VTUNER_MEMCPY(&ctx->dtv_props[i],
				    &msg->body.dtv_properties.props[i], u.data);
			} else {
				VTUNER_MEMCPY(&ctx->dtv_props[i],
				    &msg->body.dtv_properties.props[i], u.buffer.len);
				VTUNER_MEMCPY(&ctx->dtv_props[i],
				    &msg->body.dtv_properties.props[i], u.buffer.data);
			}
		}

		if (msg->hdr.mtype == MSG_FE_SET_PROPERTY) {
			ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
			    FE_SET_PROPERTY, &ctx->dvb.dtv_properties);
			break;
		}
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_GET_PROPERTY, &ctx->dvb.dtv_properties);

		for (i = 0; i != max; i++) {
			VTUNER_MEMSET(&msg->body.dtv_properties.props[i], 0);
			VTUNER_MEMCPY(&msg->body.dtv_properties.props[i],
			    &ctx->dtv_props[i], cmd);
			VTUNER_MEMCPY(&msg->body.dtv_properties.props[i],
			    &ctx->dtv_props[i], reserved[0]);
			VTUNER_MEMCPY(&msg->body.dtv_properties.props[i],
			    &ctx->dtv_props[i], reserved[1]);
			VTUNER_MEMCPY(&msg->body.dtv_properties.props[i],
			    &ctx->dtv_props[i], reserved[2]);
			if (ctx->dtv_props[i].cmd != DTV_DISEQC_MASTER &&
			    ctx->dtv_props[i].cmd != DTV_DISEQC_SLAVE_REPLY) {
				VTUNER_MEMCPY(&msg->body.dtv_properties.props[i],
				    &ctx->dtv_props[i], u.data);
			} else {
				VTUNER_MEMCPY(&msg->body.dtv_properties.props[i],
				    &ctx->dtv_props[i], u.buffer.len);
				VTUNER_MEMCPY(&msg->body.dtv_properties.props[i],
				    &ctx->dtv_props[i], u.buffer.data);
			}
		}
		break;
	case MSG_FE_GET_INFO:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_GET_INFO, &ctx->dvb.dvb_frontend_info);
		VTUNER_MEMSET(&msg->body.dvb_frontend_info, 0);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, name);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, type);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, frequency_min);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, frequency_max);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, frequency_stepsize);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, frequency_tolerance);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, symbol_rate_min);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, symbol_rate_max);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, symbol_rate_tolerance);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, notifier_delay);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info,
		    &ctx->dvb.dvb_frontend_info, caps);
		break;
	case MSG_FE_DISEQC_RESET_OVERLOAD:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_DISEQC_RESET_OVERLOAD, 0);
		break;
	case MSG_FE_DISEQC_SEND_MASTER_CMD:
		VTUNER_MEMSET(&ctx->dvb.dvb_diseqc_master_cmd, 0);
		VTUNER_MEMCPY(&ctx->dvb.dvb_diseqc_master_cmd,
		    &msg->body.dvb_diseqc_master_cmd, msg);
		VTUNER_MEMCPY(&ctx->dvb.dvb_diseqc_master_cmd,
		    &msg->body.dvb_diseqc_master_cmd, msg_len);
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_DISEQC_SEND_MASTER_CMD, &ctx->dvb.dvb_diseqc_master_cmd);
		break;
	case MSG_FE_DISEQC_RECV_SLAVE_REPLY:
		VTUNER_MEMSET(&ctx->dvb.dvb_diseqc_slave_reply, 0);
		VTUNER_MEMCPY(&ctx->dvb.dvb_diseqc_slave_reply,
		    &msg->body.dvb_diseqc_slave_reply, timeout);
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_DISEQC_RECV_SLAVE_REPLY, &ctx->dvb.dvb_diseqc_slave_reply);
		VTUNER_MEMCPY(&msg->body.dvb_diseqc_slave_reply,
		    &ctx->dvb.dvb_diseqc_slave_reply, msg);
		VTUNER_MEMCPY(&msg->body.dvb_diseqc_slave_reply,
		    &ctx->dvb.dvb_diseqc_slave_reply, msg_len);
		break;
	case MSG_FE_DISEQC_SEND_BURST:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_DISEQC_SEND_BURST, (void *)(long)msg->body.value32);
		break;
	case MSG_FE_SET_TONE:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_SET_TONE, (void *)(long)msg->body.value32);
		break;
	case MSG_FE_SET_VOLTAGE:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_SET_VOLTAGE, (void *)(long)msg->body.value32);
		break;
	case MSG_FE_ENABLE_HIGH_LNB_VOLTAGE:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_ENABLE_HIGH_LNB_VOLTAGE, (void *)(long)msg->body.value32);
		break;
	case MSG_FE_READ_STATUS:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_READ_STATUS, &msg->body.value32);
		break;
	case MSG_FE_READ_BER:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_READ_BER, &msg->body.value32);
		break;
	case MSG_FE_READ_SIGNAL_STRENGTH:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_READ_SIGNAL_STRENGTH, &msg->body.value16);
		break;
	case MSG_FE_READ_SNR:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_READ_SNR, &msg->body.value16);
		break;
	case MSG_FE_READ_UNCORRECTED_BLOCKS:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_READ_UNCORRECTED_BLOCKS, &msg->body.value32);
		break;
	case MSG_FE_SET_FRONTEND:
		VTUNER_MEMSET(&ctx->dvb.dvb_frontend_parameters, 0);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters,
		    &msg->body.dvb_frontend_parameters, frequency);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters,
		    &msg->body.dvb_frontend_parameters, inversion);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters,
		    &msg->body.dvb_frontend_parameters, u.ofdm.bandwidth);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters,
		    &msg->body.dvb_frontend_parameters, u.ofdm.code_rate_HP);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters,
		    &msg->body.dvb_frontend_parameters, u.ofdm.code_rate_LP);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters,
		    &msg->body.dvb_frontend_parameters, u.ofdm.constellation);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters,
		    &msg->body.dvb_frontend_parameters, u.ofdm.transmission_mode);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters,
		    &msg->body.dvb_frontend_parameters, u.ofdm.guard_interval);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters,
		    &msg->body.dvb_frontend_parameters, u.ofdm.hierarchy_information);
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_SET_FRONTEND, &ctx->dvb.dvb_frontend_parameters);
		break;
	case MSG_FE_GET_FRONTEND:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_GET_FRONTEND, &ctx->dvb.dvb_frontend_parameters);
		VTUNER_MEMSET(&msg->body.dvb_frontend_parameters, 0);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters,
		    &ctx->dvb.dvb_frontend_parameters, frequency);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters,
		    &ctx->dvb.dvb_frontend_parameters, inversion);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters,
		    &ctx->dvb.dvb_frontend_parameters, u.ofdm.bandwidth);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters,
		    &ctx->dvb.dvb_frontend_parameters, u.ofdm.code_rate_HP);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters,
		    &ctx->dvb.dvb_frontend_parameters, u.ofdm.code_rate_LP);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters,
		    &ctx->dvb.dvb_frontend_parameters, u.ofdm.constellation);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters,
		    &ctx->dvb.dvb_frontend_parameters, u.ofdm.transmission_mode);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters,
		    &ctx->dvb.dvb_frontend_parameters, u.ofdm.guard_interval);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters,
		    &ctx->dvb.dvb_frontend_parameters, u.ofdm.hierarchy_information);
		break;
	case MSG_FE_SET_FRONTEND_TUNE_MODE:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_SET_FRONTEND_TUNE_MODE, (void *)(long)msg->body.value32);
		break;
	case MSG_FE_GET_EVENT:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_GET_EVENT, &ctx->dvb.dvb_frontend_event);
		VTUNER_MEMSET(&msg->body.dvb_frontend_event, 0);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, status);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, parameters.frequency);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, parameters.inversion);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.bandwidth);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.code_rate_HP);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.code_rate_LP);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.constellation);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.transmission_mode);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.guard_interval);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event,
		    &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.hierarchy_information);
		break;
	case MSG_FE_DISHNETWORK_SEND_LEGACY_CMD:
		ret = linux_ioctl(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    FE_DISHNETWORK_SEND_LEGACY_CMD, (void *)(long)msg->body.value32);
		break;
	default:
		break;
	}
	return (ret);
}

static int
vtuners_read(int fd, u8 * ptr, int len)
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
vtuners_write(int fd, const u8 * ptr, int len)
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

static int
vtuners_listen(const char *host, const char *port, int buffer)
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
	hints.ai_flags |= AI_NUMERICHOST;

	DPRINTF("Listening to %s:%s\n", host, port);

	if ((error = getaddrinfo(host, port, &hints, &res)))
		return (-1);

	res0 = res;

	do {
		if ((s = socket(res0->ai_family, res0->ai_socktype,
		    res0->ai_protocol)) < 0)
			continue;

		flag = 1;
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, (int)sizeof(flag));
		setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &flag, (int)sizeof(flag));

		setsockopt(s, SOL_SOCKET, SO_SNDBUF, &buffer, (int)sizeof(buffer));
		setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffer, (int)sizeof(buffer));

		if (bind(s, res0->ai_addr, res0->ai_addrlen) == 0) {
			if (listen(s, 1) == 0)
				break;
		}
		close(s);
		s = -1;
	} while ((res0 = res0->ai_next) != NULL);

	freeaddrinfo(res);

	printk(KERN_INFO "vTuner: Listen result fd=%d\n", s);

	return (s);
}

static void *
vtuners_writer_thread(void *arg)
{
	struct vtuners_ctx *ctx = arg;
	int len;

	signal(SIGHUP, vtuners_work_exec_hup);

	while (ctx->fd_control > -1) {

		len = linux_read(ctx->proxy_fd, CUSE_FFLAG_NONBLOCK,
		    ((u8 *) ctx->buffer) + 8, sizeof(ctx->buffer) - 8);

		if (len <= 0) {
			usleep(2000);
			continue;
		}
		DPRINTF("len = %d\n", len);

		ctx->buffer[0] = VTUNER_MAGIC;
		ctx->buffer[1] = len;

		len += 8;

		if (vtuners_write(ctx->fd_data,
		    (u8 *) ctx->buffer, len) != len) {
			DPRINTF("Could not write %d bytes\n", len);
			break;
		}
	}

	down(&ctx->writer_sem);
	close(ctx->fd_data);
	ctx->fd_data = -1;
	len = (ctx->fd_control == -1);
	up(&ctx->writer_sem);

	if (len) {
		DPRINTF("Closing %p\n", ctx->proxy_fd);
		linux_close(ctx->proxy_fd);
		kfree(ctx);
	}
	return (NULL);
}

static void *
vtuners_control_thread(void *arg)
{
	struct vtuners_ctx *ctx = arg;
	int len;
	int swapped;

	signal(SIGHUP, vtuners_work_exec_hup);

	while (1) {

		len = sizeof(ctx->msgbuf.hdr);

		if (vtuners_read(ctx->fd_control,
		    (u8 *) & ctx->msgbuf.hdr, len) != len) {
			DPRINTF("Bad read of len %d\n", len);
			break;
		}
		if (ctx->msgbuf.hdr.magic != VTUNER_MAGIC) {
			vtuner_hdr_byteswap(&ctx->msgbuf);
			if (ctx->msgbuf.hdr.magic != VTUNER_MAGIC) {
				DPRINTF("Bad magic 0x%08x\n", ctx->msgbuf.hdr.magic);
				break;
			}
			swapped = 1;
		} else {
			swapped = 0;
		}

		len = ctx->msgbuf.hdr.rx_size;
		if (len < 0 || len > sizeof(ctx->msgbuf.body)) {
			DPRINTF("Bad rx_size = %d\n", len);
			break;
		}
		if (len != 0) {
			if (vtuners_read(ctx->fd_control,
			    (u8 *) & ctx->msgbuf.body, len) != len) {
				DPRINTF("Bad read of len %d\n", len);
				break;
			}
		}
		if (swapped) {
			vtuner_body_byteswap(&ctx->msgbuf,
			    ctx->msgbuf.hdr.mtype);
		}
		ctx->msgbuf.hdr.error =
		    vtuners_process_msg(ctx, &ctx->msgbuf);

		len = ctx->msgbuf.hdr.tx_size;
		if (len < 0 || len > sizeof(ctx->msgbuf.body)) {
			DPRINTF("Bad write length %d\n", len);
			break;
		}
		len += sizeof(ctx->msgbuf.hdr);

		if (swapped) {
			vtuner_body_byteswap(&ctx->msgbuf, ctx->msgbuf.hdr.mtype);
			vtuner_hdr_byteswap(&ctx->msgbuf);
		}
		if (vtuners_write(ctx->fd_control,
		    (u8 *) & ctx->msgbuf, len) != len) {
			DPRINTF("Could not write %d bytes\n", len);
			break;
		}
	}

	down(&ctx->writer_sem);
	close(ctx->fd_control);
	ctx->fd_control = -1;
	swapped = (ctx->fd_data == -1);
	up(&ctx->writer_sem);

	if (swapped) {
		DPRINTF("Closing %p\n", ctx->proxy_fd);
		linux_close(ctx->proxy_fd);
		kfree(ctx);
	}
	return (NULL);
}

static void *
vtuners_listen_worker(void *arg)
{
	struct vtuners_config *cfg = arg;
	struct vtuners_ctx *ctx;
	int f_ctrl;
	int f_data;

	signal(SIGHUP, vtuners_work_exec_hup);

	if (cfg == NULL)
		return (NULL);

	while (1) {
		f_ctrl = accept(cfg->c_fd, NULL, NULL);

		if (f_ctrl > -1) {
			alarm(4);
			f_data = accept(cfg->d_fd, NULL, NULL);
			alarm(0);
			if (f_data > -1) {

				DPRINTF("New connection %d,%d\n", f_ctrl, f_data);

				ctx = kzalloc(sizeof(struct vtuners_ctx), GFP_KERNEL);
				if (!ctx) {
					close(f_data);
					close(f_ctrl);
					continue;
				}
				ctx->proxy_fd = linux_open(cfg->unit, cfg->mode);
				if (ctx->proxy_fd == NULL) {
					close(f_data);
					close(f_ctrl);
					continue;
				}
				ctx->fd_control = f_ctrl;
				ctx->fd_data = f_data;

				sema_init(&ctx->writer_sem, 1);

				/* create writer thread */
				pthread_create(&ctx->writer_thread, NULL, vtuners_writer_thread, ctx);

				/* create control thread */
				pthread_create(&ctx->control_thread, NULL, vtuners_control_thread, ctx);
			} else {
				close(f_ctrl);
			}
		}
		usleep(100000);
	}
	return (NULL);
}

/*------------------------------------------------------------------------*
 * vTuner server init and exit
 *------------------------------------------------------------------------*/

static struct vtuners_config *
vtuners_make_config(int off, int unit, int mode)
{
	struct vtuners_config *cfg;

	cfg = kzalloc(sizeof(struct vtuners_config), GFP_KERNEL);
	if (cfg == NULL)
		return (NULL);

	cfg->unit = unit;
	cfg->host = vtuner_host;
	cfg->mode = mode;
	snprintf(cfg->cport, sizeof(cfg->cport), "%u", atoi(vtuner_port) + off);
	snprintf(cfg->dport, sizeof(cfg->dport), "%u", atoi(vtuner_port) + off + 1);

	cfg->c_fd = vtuners_listen(cfg->host, cfg->cport, 4096);
	if (cfg->c_fd < 0) {
		kfree(cfg);
		return (NULL);
	}
	cfg->d_fd = vtuners_listen(cfg->host, cfg->dport, 2 * VTUNER_BUFFER_MAX);
	if (cfg->d_fd < 0) {
		close(cfg->c_fd);
		kfree(cfg);
		return (NULL);
	}
	return (cfg);
}

static int __init
vtuners_init(void)
{
	struct vtuners_ctx *ctx = NULL;
	int u;

	if (vtuner_max_unit < 0 || vtuner_max_unit > CONFIG_DVB_MAX_ADAPTERS)
		vtuner_max_unit = CONFIG_DVB_MAX_ADAPTERS;

	printk(KERN_INFO "virtual DVB server adapter driver, version "
	    VTUNER_MODULE_VERSION ", (c) 2011 Hans Petter Selasky\n");

	for (u = 0; u < vtuner_max_unit; u++) {

		pthread_t dummy;

		pthread_create(&dummy, NULL, &vtuners_listen_worker,
		    vtuners_make_config(0 + (8 * u), (F_V4B_SUBDEV_MAX *
		    F_V4B_DVB_FRONTEND) + u, O_RDWR));

		pthread_create(&dummy, NULL, &vtuners_listen_worker,
		    vtuners_make_config(2 + (8 * u), (F_V4B_SUBDEV_MAX *
		    F_V4B_DVB_DVR) + u, O_RDONLY));

		pthread_create(&dummy, NULL, &vtuners_listen_worker,
		    vtuners_make_config(4 + (8 * u), (F_V4B_SUBDEV_MAX *
		    F_V4B_DVB_DEMUX) + u, O_RDWR));
	}
	return (0);
}

module_init(vtuners_init);

module_param_named(debug, vtuner_debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable debugging (default is 0, disabled)");

module_param_named(devices, vtuner_max_unit, int, 0644);
MODULE_PARM_DESC(devices, "Number of servers (default is 0, disabled)");

module_param_string(host, vtuner_host, sizeof(vtuner_host), 0644);
MODULE_PARM_DESC(host, "Listen host (default is 127.0.0.1)");

module_param_string(port, vtuner_port, sizeof(vtuner_port), 0644);
MODULE_PARM_DESC(port, "Listen port (default is 5100)");

MODULE_AUTHOR("Hans Petter Selasky");
MODULE_DESCRIPTION("Virtual DVB device server");
MODULE_LICENSE("BSD");
MODULE_VERSION(VTUNER_MODULE_VERSION);
