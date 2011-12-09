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

#include <vtuner/vtuner.h>
#include <vtuner/vtuner_common.h>

#define	VTUNER_BSWAP16(x) x = bswap16(x)
#define	VTUNER_BSWAP32(x) x = bswap32(x)
#define	VTUNER_BSWAP64(x) x = bswap64(x)

#define	NMSG ((struct vtuner_message *)0)

int
vtuner_struct_size(int type)
{
	switch (type) {
	case MSG_STRUCT_DMX_SCT_FILTER_PARAMS:
		return (sizeof(NMSG->body.dmx_sct_filter_params));
	case MSG_STRUCT_DMX_PES_FILTER_PARAMS:
		return (sizeof(NMSG->body.dmx_pes_filter_params));
	case MSG_STRUCT_DMX_PES_PID:
		return (sizeof(NMSG->body.dmx_pes_pid));
	case MSG_STRUCT_DMX_CAPS:
		return (sizeof(NMSG->body.dmx_caps));
	case MSG_STRUCT_DMX_STC:
		return (sizeof(NMSG->body.dmx_stc));
	case MSG_STRUCT_DVB_FRONTEND_INFO:
		return (sizeof(NMSG->body.dvb_frontend_info));
	case MSG_STRUCT_DVB_DISEQC_MASTER_CMD:
		return (sizeof(NMSG->body.dvb_diseqc_master_cmd));
	case MSG_STRUCT_DVB_DISEQC_SLAVE_REPLY:
		return (sizeof(NMSG->body.dvb_diseqc_slave_reply));
	case MSG_STRUCT_DVB_FRONTEND_PARAMETERS:
		return (sizeof(NMSG->body.dvb_frontend_parameters));
	case MSG_STRUCT_DVB_FRONTEND_EVENT:
		return (sizeof(NMSG->body.dvb_frontend_event));
	case MSG_STRUCT_DTV_CMDS_H:
		return (sizeof(NMSG->body.dtv_cmds_h));
	case MSG_STRUCT_DTV_PROPERTIES:
		return (sizeof(NMSG->body.dtv_properties));
	case MSG_STRUCT_U32:
		return (sizeof(NMSG->body.value32));
	case MSG_STRUCT_U16:
		return (sizeof(NMSG->body.value16));
	case MSG_STRUCT_NULL:
		return (0);
	default:
		return (-1);
	}
}

void
vtuner_hdr_byteswap(struct vtuner_message *msg)
{
	VTUNER_BSWAP32(msg->hdr.mtype);
	VTUNER_BSWAP32(msg->hdr.magic);
	VTUNER_BSWAP32(msg->hdr.rx_struct);
	VTUNER_BSWAP32(msg->hdr.tx_struct);
	VTUNER_BSWAP32(msg->hdr.error);
}

void
vtuner_body_byteswap(struct vtuner_message *msg, int type)
{
	int i;

	switch (type) {
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
