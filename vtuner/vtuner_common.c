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

#define	__DVB_CORE__

#include <vtuner/vtuner.h>
#include <vtuner/vtuner_common.h>

#define	VTUNER_BSWAP16(x) do {				\
	extern int dummy[(sizeof(x) != 2) ? -1 : 1];	\
	x = bswap16(x);					\
} while (0)

#define	VTUNER_BSWAP32(x) do {				\
	extern int dummy[(sizeof(x) != 4) ? -1 : 1];	\
	x = bswap32(x);					\
} while (0)

#define	VTUNER_BSWAP64(x) do {				\
	extern int dummy[(sizeof(x) != 8) ? -1 : 1];	\
	x = bswap64(x);					\
} while (0)

#define	NMSG ((struct vtuner_message *)0)

void
vtuner_data_hdr_byteswap(u32 * ptr)
{
	VTUNER_BSWAP32(ptr[0]);
	VTUNER_BSWAP32(ptr[1]);
}

void
vtuner_hdr_byteswap(struct vtuner_message *msg)
{
	VTUNER_BSWAP32(msg->hdr.mtype);
	VTUNER_BSWAP32(msg->hdr.magic);
	VTUNER_BSWAP16(msg->hdr.rx_size);
	VTUNER_BSWAP16(msg->hdr.tx_size);
	VTUNER_BSWAP16(msg->hdr.error);
	VTUNER_BSWAP16(msg->hdr.padding);
}

void
vtuner_body_byteswap(struct vtuner_message *msg, int type)
{
	int i;

	switch (type) {
	case DMX_SET_FILTER:
		VTUNER_BSWAP16(msg->body.dmx_sct_filter_params.pid);
		VTUNER_BSWAP32(msg->body.dmx_sct_filter_params.timeout);
		VTUNER_BSWAP32(msg->body.dmx_sct_filter_params.flags);
		break;
	case DMX_SET_PES_FILTER:
		VTUNER_BSWAP16(msg->body.dmx_pes_filter_params.pid);
		VTUNER_BSWAP32(msg->body.dmx_pes_filter_params.input);
		VTUNER_BSWAP32(msg->body.dmx_pes_filter_params.output);
		VTUNER_BSWAP32(msg->body.dmx_pes_filter_params.pes_type);
		VTUNER_BSWAP32(msg->body.dmx_pes_filter_params.flags);
		break;
	case DMX_GET_PES_PIDS:
		for (i = 0; i != 5; i++)
			VTUNER_BSWAP16(msg->body.dmx_pes_pid.pids[i]);
		break;
	case DMX_GET_CAPS:
		VTUNER_BSWAP32(msg->body.dmx_caps.caps);
		VTUNER_BSWAP32(msg->body.dmx_caps.num_decoders);
		break;
	case DMX_GET_STC:
		VTUNER_BSWAP32(msg->body.dmx_stc.num);
		VTUNER_BSWAP32(msg->body.dmx_stc.base);
		VTUNER_BSWAP64(msg->body.dmx_stc.stc);
		break;
	case FE_GET_INFO:
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
	case FE_DISEQC_RECV_SLAVE_REPLY:
		VTUNER_BSWAP32(msg->body.dvb_diseqc_slave_reply.timeout);
		break;
	case FE_SET_FRONTEND:
	case FE_GET_FRONTEND:
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
	case FE_GET_EVENT:
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
	case FE_SET_PROPERTY:
	case FE_GET_PROPERTY:
		VTUNER_BSWAP32(msg->body.dtv_properties.num);
		for (i = 0; i != VTUNER_PROP_MAX; i++) {
			int has_buf = 0;

			if (msg->body.dtv_properties.props[i].cmd == DTV_DISEQC_MASTER ||
			    msg->body.dtv_properties.props[i].cmd == DTV_DISEQC_SLAVE_REPLY)
				has_buf = 1;

			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].cmd);

			if (msg->body.dtv_properties.props[i].cmd == DTV_DISEQC_MASTER ||
			    msg->body.dtv_properties.props[i].cmd == DTV_DISEQC_SLAVE_REPLY)
				has_buf = 1;

			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].reserved[0]);
			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].reserved[1]);
			VTUNER_BSWAP32(msg->body.dtv_properties.props[i].reserved[2]);

			if (has_buf)
				VTUNER_BSWAP32(msg->body.dtv_properties.props[i].u.buffer.len);
			else
				VTUNER_BSWAP32(msg->body.dtv_properties.props[i].u.data);
		}
		break;
	case DMX_SET_SOURCE:
	case DMX_SET_BUFFER_SIZE:
	case FE_DISEQC_SEND_BURST:
	case FE_SET_TONE:
	case FE_SET_VOLTAGE:
	case FE_ENABLE_HIGH_LNB_VOLTAGE:
	case FE_READ_STATUS:
	case FE_READ_BER:
	case FE_READ_UNCORRECTED_BLOCKS:
	case FE_SET_FRONTEND_TUNE_MODE:
	case FE_DISHNETWORK_SEND_LEGACY_CMD:
		VTUNER_BSWAP32(msg->body.value32);
		break;
	case FE_READ_SIGNAL_STRENGTH:
	case DMX_ADD_PID:
	case DMX_REMOVE_PID:
	case FE_READ_SNR:
		VTUNER_BSWAP16(msg->body.value16);
		break;
	default:
		break;
	}
}
