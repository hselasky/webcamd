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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>

#include "demux.h"
#include "dmxdev.h"
#include "dvb_demux.h"
#include "dvb_frontend.h"
#include "dvb_net.h"
#include "dvbdev.h"

#include <vtuner/vtuner.h>
#include <vtuner/vtuner_server.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>

#include <cuse4bsd.h>

#define	VTUNER_BSWAP16(x) x = bswap16(x)
#define	VTUNER_BSWAP32(x) x = bswap32(x)
#define	VTUNER_BSWAP64(x) x = bswap64(x)

#define	VTUNER_MODULE_VERSION "1.0-hps"

#define	VTUNER_MEMSET(a,b)			\
  memset(a,b,sizeof(*(a)))

#define	VTUNER_MEMCPY(a,b,c) do {					\
  extern int dummy[(sizeof(*(a.c)) != sizeof(*(b.c))) ? -1 : 1];	\
  memcpy(a.c, b.c, sizeof(*(a.c)));					\
} while (0)

static struct vtuners_ctx *vtuners_tbl[CONFIG_DVB_MAX_ADAPTERS];
static int vtuner_max_unit = 0;
static char vtuner_host[64] = {"127.0.0.1"};
static char vtuner_cport[16] = {VTUNER_DEFAULT_PORT};

static int
vtuners_struct_size(int type)
{
	switch (type) {
	case MSG_STRUCT_DMX_FILTER:
		return (sizeof(((struct vtuner_message *) 0)->body.dmx_filter));
	case MSG_STRUCT_DMX_SCT_FILTER_PARAMS:
		return (sizeof(((struct vtuner_message *) 0)->body.dmx_sct_filter_params));
	case MSG_STRUCT_DMX_PES_FILTER_PARAMS:
		return (sizeof(((struct vtuner_message *) 0)->body.dmx_pes_filter_params));
	case MSG_STRUCT_DMX_PES_PID:
		return (sizeof(((struct vtuner_message *) 0)->body.dmx_pes_pid));
	case MSG_STRUCT_DMX_CAPS:
		return (sizeof(((struct vtuner_message *) 0)->body.dmx_caps));
	case MSG_STRUCT_DMX_STC:
		return (sizeof(((struct vtuner_message *) 0)->body.dmx_stc));
	case MSG_STRUCT_DVB_FRONTEND_INFO:
		return (sizeof(((struct vtuner_message *) 0)->body.dvb_frontend_info));
	case MSG_STRUCT_DVB_DISEQC_MASTER_CMD:
		return (sizeof(((struct vtuner_message *) 0)->body.dvb_diseqc_master_cmd));
	case MSG_STRUCT_DVB_DISEQC_SLAVE_REPLY:
		return (sizeof(((struct vtuner_message *) 0)->body.dvb_diseqc_slave_reply));
	case MSG_STRUCT_DVB_FRONTEND_PARAMETERS:
		return (sizeof(((struct vtuner_message *) 0)->body.dvb_frontend_parameters));
	case MSG_STRUCT_DVB_FRONTEND_EVENT:
		return (sizeof(((struct vtuner_message *) 0)->body.dvb_frontend_event));
	case MSG_STRUCT_DTV_CMDS_H:
		return (sizeof(((struct vtuner_message *) 0)->body.dtv_cmds_h));
	case MSG_STRUCT_DTV_PROPERTIES:
		return (sizeof(((struct vtuner_message *) 0)->body.dtv_properties));
	case MSG_STRUCT_U32:
		return (sizeof(((struct vtuner_message *) 0)->body.value32));
	case MSG_STRUCT_U16:
		return (sizeof(((struct vtuner_message *) 0)->body.value16));
	case MSG_STRUCT_NULL:
		return (0);
	default:
		return (-1);
	}
}

static void
vtuners_hdr_byteswap(struct vtuner_message *msg)
{
	VTUNER_BSWAP32(msg->hdr.mtype);
	VTUNER_BSWAP32(msg->hdr.magic);
	VTUNER_BSWAP32(msg->hdr.rx_struct);
	VTUNER_BSWAP32(msg->hdr.tx_struct);
	VTUNER_BSWAP32(msg->hdr.error);
}

static void
vtuners_body_byteswap(struct vtuner_message *msg, int type)
{
	int i;

	switch (type) {
	case MSG_STRUCT_DMX_FILTER:
		break;
	case MSG_STRUCT_DMX_SCT_FILTER_PARAMS:
		VTUNER_BSWAP16(msg->body.dmx_sct_filter_params.pid);
		VTUNER_BSWAP32(msg->body.dmx_sct_filter_params.timeout);
		VTUNER_BSWAP32(msg->body.dmx_sct_filter_params.flags);
		break;
	case MSG_STRUCT_DMX_PES_FILTER_PARAMS:
		VTUNER_BSWAP16(msg->body.dmx_pes_filter_params.pid);
		VTUNER_BSWAP32(msg->body.dmx_pes_filter_params.input);
		VTUNER_BSWAP32(msg->body.dmx_pes_filter_params.output);
		VTUNER_BSWAP32(msg->body.dmx_pes_filter_params.pes_type);
		VTUNER_BSWAP32(msg->body.dmx_pes_filter_params.flags);
		break;
	case MSG_STRUCT_DMX_PES_PID:
		for (i = 0; i != 5; i++)
			VTUNER_BSWAP16(msg->body.dmx_pes_pid.pids[i]);
		break;
	case MSG_STRUCT_DMX_CAPS:
		VTUNER_BSWAP32(msg->body.dmx_caps.caps);
		VTUNER_BSWAP32(msg->body.dmx_caps.num_decoders);
		break;
	case MSG_STRUCT_DMX_STC:
		VTUNER_BSWAP32(msg->body.dmx_stc.num);
		VTUNER_BSWAP32(msg->body.dmx_stc.base);
		VTUNER_BSWAP64(msg->body.dmx_stc.stc);
		break;
	case MSG_STRUCT_DVB_FRONTEND_INFO:
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.type);
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.frequency_min);
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.frequency_max);
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.frequency_stepsize);
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.frequency_tolerance);
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.symbol_rate_min);
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.symbol_rate_max);
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.symbol_rate_tolerance);
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.notifier_delay);
		VTUNER_BSWAP32(msg->body.dvb_frontend_info.caps);
		break;
	case MSG_STRUCT_DVB_DISEQC_MASTER_CMD:
		break;
	case MSG_STRUCT_DVB_DISEQC_SLAVE_REPLY:
		VTUNER_BSWAP32(msg->body.dvb_diseqc_slave_reply.timeout);
		break;
	case MSG_STRUCT_DVB_FRONTEND_PARAMETERS:
		VTUNER_BSWAP32(msg->body.dvb_frontend_parameters.frequency);
		VTUNER_BSWAP32(msg->body.dvb_frontend_parameters.inversion);
		VTUNER_BSWAP32(msg->body.dvb_frontend_parameters.u.ofdm.bandwidth);
		VTUNER_BSWAP32(msg->body.dvb_frontend_parameters.u.ofdm.code_rate_HP);
		VTUNER_BSWAP32(msg->body.dvb_frontend_parameters.u.ofdm.code_rate_LP);
		VTUNER_BSWAP32(msg->body.dvb_frontend_parameters.u.ofdm.constellation);
		VTUNER_BSWAP32(msg->body.dvb_frontend_parameters.u.ofdm.transmission_mode);
		VTUNER_BSWAP32(msg->body.dvb_frontend_parameters.u.ofdm.guard_interval);
		VTUNER_BSWAP32(msg->body.dvb_frontend_parameters.u.ofdm.hierarchy_information);
		break;
	case MSG_STRUCT_DVB_FRONTEND_EVENT:
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.status);
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.parameters.frequency);
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.parameters.inversion);
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.parameters.u.ofdm.bandwidth);
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.parameters.u.ofdm.code_rate_HP);
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.parameters.u.ofdm.code_rate_LP);
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.parameters.u.ofdm.constellation);
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.parameters.u.ofdm.transmission_mode);
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.parameters.u.ofdm.guard_interval);
		VTUNER_BSWAP32(msg->body.dvb_frontend_event.parameters.u.ofdm.hierarchy_information);
		break;
	case MSG_STRUCT_DTV_CMDS_H:
		VTUNER_BSWAP32(msg->body.dtv_cmds_h.cmd);
		break;
	case MSG_STRUCT_DTV_PROPERTIES:
		VTUNER_BSWAP32(msg->body.dtv_properties.num);
		for (i = 0; i != VTUNER_MAX_PROP; i++) {
			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].cmd);
			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].reserved[0]);
			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].reserved[1]);
			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].reserved[2]);
			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].u.data);
			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].u.buffer.len);
		}
		break;
	case MSG_STRUCT_U32:
		VTUNER_BSWAP32(msg->body.value32);
		break;
	case MSG_STRUCT_U16:
		VTUNER_BSWAP16(msg->body.value16);
		break;
	default:
		break;
	}
}

static int
vtuners_process_msg(struct vtuners_ctx *ctx, struct vtuner_message *msg)
{
	int ret = -1;
	int i;

	switch (msg->hdr.mtype) {
	case MSG_DMX_START:
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_START, NULL);
		break;
	case MSG_DMX_STOP:
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_STOP, NULL);
		break;
	case MSG_DMX_SET_FILTER:
		VTUNER_MEMSET(&ctx->dvb.dmx_sct_filter_params, 0);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_sct_filter_params.pid);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_sct_filter_params.filter);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_sct_filter_params.timeout);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_sct_filter_params.flags);
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_SET_FILTER, &ctx->dvb.dmx_sct_filter_params);
		break;
	case MSG_DMX_SET_PES_FILTER:
		VTUNER_MEMSET(&ctx->dvb.dmx_pes_filter_params, 0);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_pes_filter_params.pid);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_pes_filter_params.input);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_pes_filter_params.output);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_pes_filter_params.pes_type);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_pes_filter_params.flags);
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_SET_PES_FILTER, &ctx->dvb.dmx_pes_filter_params);
		break;
	case MSG_DMX_SET_BUFFER_SIZE:
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_SET_BUFFER_SIZE, (void *)(long)msg->body.value32);
		break;
	case MSG_DMX_GET_PES_PIDS:
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_GET_PES_PIDS, msg->body.dmx_pes_pid.pids);
		break;
	case MSG_DMX_GET_CAPS:
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_GET_CAPS, &msg->body.value32);
		break;
	case MSG_DMX_SET_SOURCE:
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_SET_SOURCE, &msg->body.value32);
		break;
	case MSG_DMX_GET_STC:
		VTUNER_MEMSET(&ctx->dvb.dmx_stc, 0);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_stc.num);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_stc.base);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dmx_stc.stc);
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_GET_STC, &ctx->dvb.dmx_stc);
		VTUNER_MEMCPY(&msg->body, &ctx->dvb, dmx_stc.num);
		VTUNER_MEMCPY(&msg->body, &ctx->dvb, dmx_stc.base);
		VTUNER_MEMCPY(&msg->body, &ctx->dvb, dmx_stc.stc);
		break;
	case MSG_DMX_ADD_PID:
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_ADD_PID, &msg->body.value16);
		break;
	case MSG_DMX_REMOVE_PID:
		ret = linux_ioctl(ctx->demux_fd, CUSE_FFLAG_NONBLOCK, DMX_REMOVE_PID, &msg->body.value16);
		break;

	case MSG_FE_SET_PROPERTY:
		if (msg->body.dtv_properties.num > VTUNER_MAX_PROP)
			msg->body.dtv_properties.num = VTUNER_MAX_PROP;

		VTUNER_MEMSET(&ctx->dvb.dtv_properties, 0);
		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dtv_properties.num);
		ctx->dvb.dtv_properties.props = ctx->dtv_props;

		for (i = 0; i != msg->body.dtv_properties.num; i++) {
			VTUNER_MEMSET(&ctx->dtv_props[i], 0);
			VTUNER_MEMCPY(&ctx->dtv_props[i], &msg->body.dtv_properties.props[i], cmd);
			VTUNER_MEMCPY(&ctx->dtv_props[i], &msg->body.dtv_properties.props[i], reserved[0]);
			VTUNER_MEMCPY(&ctx->dtv_props[i], &msg->body.dtv_properties.props[i], reserved[1]);
			VTUNER_MEMCPY(&ctx->dtv_props[i], &msg->body.dtv_properties.props[i], reserved[2]);
			if (msg->body.dtv_properties.props[i].u.buffer.len == 0 ||
			    msg->body.dtv_properties.props[i].u.buffer.len > 32) {
				VTUNER_MEMCPY(&ctx->dtv_props[i], &msg->body.dtv_properties.props[i], u.data);
			} else {
				VTUNER_MEMCPY(&ctx->dtv_props[i], &msg->body.dtv_properties.props[i], u.buffer.len);
				VTUNER_MEMCPY(&ctx->dtv_props[i], &msg->body.dtv_properties.props[i], u.buffer.data);
			}
		}

		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_SET_PROPERTY, &ctx->dvb.dtv_properties);
		break;

	case MSG_FE_GET_PROPERTY:
		if (msg->body.dtv_properties.num > VTUNER_MAX_PROP)
			msg->body.dtv_properties.num = VTUNER_MAX_PROP;

		VTUNER_MEMSET(&msg->body.dtv_properties.props, 0);

		VTUNER_MEMCPY(&ctx->dvb, &msg->body, dtv_properties.num);
		ctx->dvb.dtv_properties.props = ctx->dtv_props;

		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_GET_PROPERTY, &ctx->dvb.dtv_properties);

		for (i = 0; i != msg->body.dtv_properties.num; i++) {
			VTUNER_MEMSET(&ctx->dtv_props[i], 0);
			VTUNER_MEMCPY(&msg->body.dtv_properties.props[i], &ctx->dtv_props[i], cmd);
			VTUNER_MEMCPY(&msg->body.dtv_properties.props[i], &ctx->dtv_props[i], reserved[0]);
			VTUNER_MEMCPY(&msg->body.dtv_properties.props[i], &ctx->dtv_props[i], reserved[1]);
			VTUNER_MEMCPY(&msg->body.dtv_properties.props[i], &ctx->dtv_props[i], reserved[2]);
			if (ctx->dtv_props[i].u.buffer.len == 0 ||
			    ctx->dtv_props[i].u.buffer.len > 32) {
				VTUNER_MEMCPY(&msg->body.dtv_properties.props[i], &ctx->dtv_props[i], u.data);
			} else {
				VTUNER_MEMCPY(&msg->body.dtv_properties.props[i], &ctx->dtv_props[i], u.buffer.len);
				VTUNER_MEMCPY(&msg->body.dtv_properties.props[i], &ctx->dtv_props[i], u.buffer.data);
			}
		}
		break;
	case MSG_FE_GET_INFO:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_GET_INFO, &ctx->dvb.dvb_frontend_info);
		VTUNER_MEMSET(&msg->body.dvb_frontend_info, 0);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, name);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, type);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, frequency_min);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, frequency_max);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, frequency_stepsize);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, frequency_tolerance);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, symbol_rate_min);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, symbol_rate_max);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, symbol_rate_tolerance);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, notifier_delay);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_info, &ctx->dvb.dvb_frontend_info, caps);
		break;
	case MSG_FE_DISEQC_RESET_OVERLOAD:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_DISEQC_RESET_OVERLOAD, 0);
		break;
	case MSG_FE_DISEQC_SEND_MASTER_CMD:
		VTUNER_MEMSET(&ctx->dvb.dvb_diseqc_master_cmd, 0);
		VTUNER_MEMCPY(&ctx->dvb.dvb_diseqc_master_cmd, &msg->body.dvb_diseqc_master_cmd, msg);
		VTUNER_MEMCPY(&ctx->dvb.dvb_diseqc_master_cmd, &msg->body.dvb_diseqc_master_cmd, msg_len);
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_DISEQC_SEND_MASTER_CMD, &ctx->dvb.dvb_diseqc_master_cmd);
		break;
	case MSG_FE_DISEQC_RECV_SLAVE_REPLY:
		VTUNER_MEMSET(&ctx->dvb.dvb_diseqc_slave_reply, 0);
		VTUNER_MEMCPY(&ctx->dvb.dvb_diseqc_slave_reply, &msg->body.dvb_diseqc_slave_reply, timeout);
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_DISEQC_RECV_SLAVE_REPLY, &ctx->dvb.dvb_diseqc_slave_reply);
		VTUNER_MEMCPY(&msg->body.dvb_diseqc_slave_reply, &ctx->dvb.dvb_diseqc_slave_reply, msg);
		VTUNER_MEMCPY(&msg->body.dvb_diseqc_slave_reply, &ctx->dvb.dvb_diseqc_slave_reply, msg_len);
		break;
	case MSG_FE_DISEQC_SEND_BURST:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_DISEQC_SEND_BURST, (void *)(long)msg->body.value32);
		break;
	case MSG_FE_SET_TONE:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_SET_TONE, (void *)(long)msg->body.value32);
		break;
	case MSG_FE_SET_VOLTAGE:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_SET_VOLTAGE, (void *)(long)msg->body.value32);
		break;
	case MSG_FE_ENABLE_HIGH_LNB_VOLTAGE:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_ENABLE_HIGH_LNB_VOLTAGE, (void *)(long)msg->body.value32);
		break;
	case MSG_FE_READ_STATUS:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_READ_STATUS, &msg->body.value32);
		break;
	case MSG_FE_READ_BER:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_READ_BER, &msg->body.value32);
		break;
	case MSG_FE_READ_SIGNAL_STRENGTH:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_READ_SIGNAL_STRENGTH, &msg->body.value16);
		break;
	case MSG_FE_READ_SNR:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_READ_SNR, &msg->body.value16);
		break;
	case MSG_FE_READ_UNCORRECTED_BLOCKS:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_READ_UNCORRECTED_BLOCKS, &msg->body.value32);
		break;
	case MSG_FE_SET_FRONTEND:

		VTUNER_MEMSET(&ctx->dvb.dvb_frontend_parameters, 0);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters, &msg->body.dvb_frontend_parameters, frequency);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters, &msg->body.dvb_frontend_parameters, inversion);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters, &msg->body.dvb_frontend_parameters, u.ofdm.bandwidth);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters, &msg->body.dvb_frontend_parameters, u.ofdm.code_rate_HP);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters, &msg->body.dvb_frontend_parameters, u.ofdm.code_rate_LP);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters, &msg->body.dvb_frontend_parameters, u.ofdm.constellation);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters, &msg->body.dvb_frontend_parameters, u.ofdm.transmission_mode);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters, &msg->body.dvb_frontend_parameters, u.ofdm.guard_interval);
		VTUNER_MEMCPY(&ctx->dvb.dvb_frontend_parameters, &msg->body.dvb_frontend_parameters, u.ofdm.hierarchy_information);

		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_SET_FRONTEND, &ctx->dvb.dvb_frontend_parameters);

		break;
	case MSG_FE_GET_FRONTEND:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_GET_FRONTEND, &ctx->dvb.dvb_frontend_parameters);

		VTUNER_MEMSET(&msg->body.dvb_frontend_parameters, 0);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters, &ctx->dvb.dvb_frontend_parameters, frequency);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters, &ctx->dvb.dvb_frontend_parameters, inversion);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters, &ctx->dvb.dvb_frontend_parameters, u.ofdm.bandwidth);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters, &ctx->dvb.dvb_frontend_parameters, u.ofdm.code_rate_HP);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters, &ctx->dvb.dvb_frontend_parameters, u.ofdm.code_rate_LP);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters, &ctx->dvb.dvb_frontend_parameters, u.ofdm.constellation);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters, &ctx->dvb.dvb_frontend_parameters, u.ofdm.transmission_mode);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters, &ctx->dvb.dvb_frontend_parameters, u.ofdm.guard_interval);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_parameters, &ctx->dvb.dvb_frontend_parameters, u.ofdm.hierarchy_information);
		break;

	case MSG_FE_SET_FRONTEND_TUNE_MODE:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_SET_FRONTEND_TUNE_MODE, (void *)(long)msg->body.value32);
		break;

	case MSG_FE_GET_EVENT:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_GET_EVENT, &ctx->dvb.dvb_frontend_event);
		VTUNER_MEMSET(&msg->body.dvb_frontend_event, 0);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, status);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, parameters.frequency);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, parameters.inversion);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.bandwidth);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.code_rate_HP);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.code_rate_LP);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.constellation);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.transmission_mode);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.guard_interval);
		VTUNER_MEMCPY(&msg->body.dvb_frontend_event, &ctx->dvb.dvb_frontend_event, parameters.u.ofdm.hierarchy_information);
		break;
	case MSG_FE_DISHNETWORK_SEND_LEGACY_CMD:
		ret = linux_ioctl(ctx->frontend_fd, CUSE_FFLAG_NONBLOCK, FE_DISHNETWORK_SEND_LEGACY_CMD, (void *)(long)msg->body.value32);
		break;
	default:
		break;
	}
	return (ret);
}

static void
vtuners_close(struct vtuners_ctx *ctx)
{
	if (ctx->frontend_fd != NULL) {
		linux_close(ctx->frontend_fd);
		ctx->frontend_fd = NULL;
	}
	if (ctx->streaming_fd != NULL) {
		linux_close(ctx->streaming_fd);
		ctx->streaming_fd = NULL;
	}
	down(&ctx->writer_sem);
	if (ctx->demux_fd != NULL) {
		linux_close(ctx->demux_fd);
		ctx->demux_fd = NULL;
	}
	up(&ctx->writer_sem);
}

static int
vtuners_open(struct vtuners_ctx *ctx)
{
	int adapter;

	/* reset some fields to attach default */
	VTUNER_MEMSET(&ctx->dvb, 0);
	VTUNER_MEMSET(&ctx->dtv_props, 0);
	VTUNER_MEMSET(&ctx->msgbuf, 0);
	VTUNER_MEMSET(&ctx->buffer, 0);

	adapter = ctx->unit;

	ctx->frontend_fd = linux_open((F_V4B_SUBDEV_MAX *
	    F_V4B_DVB_FRONTEND) + adapter, O_RDWR);

	if (ctx->frontend_fd == NULL)
		goto error;

	ctx->streaming_fd = linux_open(
	    (F_V4B_SUBDEV_MAX * F_V4B_DVB_DVR) + adapter, O_RDONLY);

	if (ctx->streaming_fd == NULL)
		goto error;

	down(&ctx->writer_sem);
	ctx->demux_fd = linux_open(
	    (F_V4B_SUBDEV_MAX * F_V4B_DVB_DEMUX) + adapter, O_RDWR | O_NONBLOCK);
	if (ctx->demux_fd == NULL) {
		up(&ctx->writer_sem);
		goto error;
	}
	up(&ctx->writer_sem);

	return 0;

error:
	vtuners_close(ctx);
	return -1;
}

static int
vtuners_read(int fd, u8 * ptr, int len)
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
vtuners_write(int fd, const u8 * ptr, int len)
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
vtuners_connect_control(struct vtuners_ctx *ctx)
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

	printk(KERN_INFO "vTuner: Listening to %s:%s (control)\n",
	    vtuner_host, ctx->cport);

	if ((error = getaddrinfo(vtuner_host, ctx->cport, &hints, &res)))
		return;

	res0 = res;

	do {
		if ((s = socket(res0->ai_family, res0->ai_socktype,
		    res0->ai_protocol)) < 0)
			continue;

		flag = 1;
		setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &flag, (int)sizeof(flag));

		if (bind(s, res0->ai_addr, res0->ai_addrlen) == 0) {
			if (listen(s, 1) == 0)
				break;
		}
		close(s);
		s = -1;
	} while ((res0 = res0->ai_next) != NULL);

	freeaddrinfo(res);

	if (s < 0)
		return;

	ctx->fd_control = accept(s, NULL, NULL);

	printk(KERN_INFO "vTuner accepted %d (control)\n", ctx->fd_control);

	close(s);

	s = ctx->fd_control;

	if (s < 0)
		return;

	flag = 2 * sizeof(struct vtuner_message);
	setsockopt(s, SOL_SOCKET, SO_SNDBUF, &flag, (int)sizeof(flag));
	setsockopt(s, SOL_SOCKET, SO_RCVBUF, &flag, (int)sizeof(flag));

	flag = 1;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

static void
vtuners_connect_data(struct vtuners_ctx *ctx)
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

	printk(KERN_INFO "vTuner: Listening to %s:%s (data)\n",
	    vtuner_host, ctx->dport);

	if ((error = getaddrinfo(vtuner_host, ctx->dport, &hints, &res)))
		return;

	res0 = res;

	do {
		if ((s = socket(res0->ai_family, res0->ai_socktype,
		    res0->ai_protocol)) < 0)
			continue;

		flag = 1;
		setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));

		if (bind(s, res0->ai_addr, res0->ai_addrlen) == 0) {
			if (listen(s, 1) == 0)
				break;
		}
		close(s);
		s = -1;
	} while ((res0 = res0->ai_next) != NULL);

	freeaddrinfo(res);

	if (s < 0)
		return;

	ctx->fd_data = accept(s, NULL, NULL);

	printk(KERN_INFO "vTuner accepted %d (data)\n", ctx->fd_data);

	close(s);

	s = ctx->fd_data;

	if (s < 0)
		return;

	flag = 2 * sizeof(ctx->buffer);
	setsockopt(s, SOL_SOCKET, SO_SNDBUF, &flag, sizeof(flag));

	flag = 1;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

static void *
vtuners_writer_thread(void *arg)
{
	struct vtuners_ctx *ctx = arg;
	int len;

	while (1) {

		while (ctx->fd_data < 0) {
			down(&ctx->writer_sem);
			len = (ctx->demux_fd != NULL);
			up(&ctx->writer_sem);

			if (len != 0)
				vtuners_connect_data(ctx);
			if (ctx->fd_data < 0) {
				usleep(1000000);
				continue;
			}
		}

		down(&ctx->writer_sem);
		if (ctx->demux_fd == NULL) {
			len = -EWOULDBLOCK;
		} else {
			len = linux_read(ctx->demux_fd, CUSE_FFLAG_NONBLOCK,
			    (u8 *) ctx->buffer, sizeof(ctx->buffer));
		}
		up(&ctx->writer_sem);

		if (len == -EWOULDBLOCK) {
			usleep(2000);
			continue;
		} else if (len <= 0) {
			close(ctx->fd_data);
			ctx->fd_data = -1;
			continue;
		}
		if (vtuners_write(ctx->fd_data,
		    (u8 *) ctx->buffer, len) != len) {
			close(ctx->fd_data);
			ctx->fd_data = -1;
			continue;
		}
	}
	return (NULL);
}

static void *
vtuners_control_thread(void *arg)
{
	struct vtuners_ctx *ctx = arg;
	int len;
	int swapped;

	while (1) {

		while (ctx->fd_control < 0) {
			vtuners_connect_control(ctx);
			if (ctx->fd_control < 0) {
				usleep(1000000);
				continue;
			}
			if (vtuners_open(ctx) < 0) {
		rx_error:
				close(ctx->fd_control);
				ctx->fd_control = -1;
				vtuners_close(ctx);
				usleep(1000000);
				continue;
			}
		}

		len = sizeof(ctx->msgbuf.hdr);

		if (vtuners_read(ctx->fd_control, (u8 *) & ctx->msgbuf.hdr, len) != len)
			goto rx_error;

		if (ctx->msgbuf.hdr.magic != VTUNER_MAGIC) {
			vtuners_hdr_byteswap(&ctx->msgbuf);
			if (ctx->msgbuf.hdr.magic != VTUNER_MAGIC)
				goto rx_error;
			swapped = 1;
		} else {
			swapped = 0;
		}

		len = vtuners_struct_size(ctx->msgbuf.hdr.rx_struct);
		if (len < 0)
			goto rx_error;

		if (vtuners_read(ctx->fd_control, (u8 *) & ctx->msgbuf.body, len) != len)
			goto rx_error;

		if (swapped)
			vtuners_body_byteswap(&ctx->msgbuf, ctx->msgbuf.hdr.rx_struct);

		ctx->msgbuf.hdr.error =
		    vtuners_process_msg(ctx, &ctx->msgbuf);

		len = vtuners_struct_size(ctx->msgbuf.hdr.tx_struct);
		if (len < 0)
			continue;

		len += sizeof(ctx->msgbuf.hdr);

		if (swapped) {
			vtuners_body_byteswap(&ctx->msgbuf, ctx->msgbuf.hdr.tx_struct);
			vtuners_hdr_byteswap(&ctx->msgbuf);
		}
		if (vtuners_write(ctx->fd_control, (u8 *) & ctx->msgbuf, len) != len)
			goto rx_error;
	}
	return (NULL);
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
		ctx = kzalloc(sizeof(struct vtuners_ctx), GFP_KERNEL);
		if (!ctx)
			return -ENOMEM;

		vtuners_tbl[u] = ctx;

		ctx->fd_data = -1;
		ctx->fd_control = -1;
		ctx->unit = u;

		sema_init(&ctx->writer_sem, 1);

		snprintf(ctx->cport, sizeof(ctx->cport),
		    "%u", atoi(vtuner_cport) + (2 * u));

		snprintf(ctx->dport, sizeof(ctx->dport),
		    "%u", atoi(vtuner_cport) + (2 * u) + 1);

		/* create writer thread */
		pthread_create(&ctx->writer_thread, NULL, vtuners_writer_thread, ctx);

		/* create control thread */
		pthread_create(&ctx->control_thread, NULL, vtuners_control_thread, ctx);
	}
	return (0);
}

static void __exit
vtuners_exit(void)
{
	struct vtuners_ctx *ctx;
	int u;

	for (u = 0; u < vtuner_max_unit; u++) {
		ctx = vtuners_tbl[u];
		if (ctx == NULL)
			continue;

		vtuners_close(ctx);
		kfree(ctx);
	}
}

module_init(vtuners_init);
module_exit(vtuners_exit);

module_param_named(devices, vtuner_max_unit, int, 0644);
MODULE_PARM_DESC(devices, "Number of servers (default is 0, disabled)");

module_param_string(host, vtuner_host, sizeof(vtuner_host), 0644);
MODULE_PARM_DESC(host, "Hostname at which to bind (default is 127.0.0.1)");

module_param_string(cport, vtuner_cport, sizeof(vtuner_cport), 0644);
MODULE_PARM_DESC(cport, "Control port at host (default is 5100)");

MODULE_AUTHOR("Hans Petter Selasky");
MODULE_DESCRIPTION("Virtual DVB device server");
MODULE_LICENSE("BSD");
MODULE_VERSION(VTUNERS_MODULE_VERSION);
