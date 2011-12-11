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
 * BSD vTuner API
 *
 * Inspired by code written by:
 * Honza Petrous <jpetrous@smartimp.cz>
 */

#ifndef _VTUNER_H_
#define	_VTUNER_H_

#include <linux/dvb/version.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

#define	VTUNER_VERSION_00010001 0x00010001
#define	VTUNER_VERSION VTUNER_VERSION_00010001
#define	VTUNER_MAGIC 0x5654554EU	/* 'VTUN' */
#define	VTUNER_TS_ALIGN 188
#define	VTUNER_DEFAULT_PORT "5100"
#define	VTUNER_BUFFER_MAX (2 * 65536)
#define	VTUNER_PROP_MAX 64
#define	VTUNER_GET_LEN(x) (0xFFFFFF & (x))
#define	VTUNER_GET_TYPE(x) (0xFF & ((x) >> 24))
#define	VTUNER_SET_LEN(x) (0xFFFFFF & (x))
#define	VTUNER_SET_TYPE(x) (0xFF000000 & ((x) << 24))

enum {
	MSG_UNDEFINED = 0,

	MSG_DMX_START = 16,
	MSG_DMX_STOP,
	MSG_DMX_SET_FILTER,
	MSG_DMX_SET_PES_FILTER,
	MSG_DMX_SET_BUFFER_SIZE,
	MSG_DMX_GET_PES_PIDS,
	MSG_DMX_GET_CAPS,
	MSG_DMX_SET_SOURCE,
	MSG_DMX_GET_STC,
	MSG_DMX_ADD_PID,
	MSG_DMX_REMOVE_PID,

	MSG_FE_SET_PROPERTY = 32,
	MSG_FE_GET_PROPERTY,
	MSG_FE_GET_INFO,
	MSG_FE_DISEQC_RESET_OVERLOAD,
	MSG_FE_DISEQC_SEND_MASTER_CMD,
	MSG_FE_DISEQC_RECV_SLAVE_REPLY,
	MSG_FE_DISEQC_SEND_BURST,
	MSG_FE_SET_TONE,
	MSG_FE_SET_VOLTAGE,
	MSG_FE_ENABLE_HIGH_LNB_VOLTAGE,

	MSG_FE_READ_STATUS,
	MSG_FE_READ_BER,
	MSG_FE_READ_SIGNAL_STRENGTH,
	MSG_FE_READ_SNR,
	MSG_FE_READ_UNCORRECTED_BLOCKS,

	MSG_FE_SET_FRONTEND,
	MSG_FE_GET_FRONTEND,
	MSG_FE_SET_FRONTEND_TUNE_MODE,
	MSG_FE_GET_EVENT,
	MSG_FE_DISHNETWORK_SEND_LEGACY_CMD,
};

/* define platform independant types */

typedef uint8_t v8 __aligned(1);
typedef uint16_t v16 __aligned(2);
typedef uint32_t v32 __aligned(4);
typedef uint64_t v64 __aligned(8);

/*
 * ==== DMX data header ====
 */

struct vtuner_data_hdr {
	v32	magic;			/* VTUNER_MAGIC */
	v32	length;			/* must be <= VTUNER_BUFFER_MAX */
};

/*
 * ==== DMX device structures ====
 */

struct vtuner_dmx_filter {
	v8	filter[DMX_FILTER_SIZE];
	v8	mask [DMX_FILTER_SIZE];
	v8	mode [DMX_FILTER_SIZE];
};

struct vtuner_dmx_sct_filter_params {
	v16	pid;
	struct vtuner_dmx_filter filter;
	v32	timeout;
	v32	flags;
};

struct vtuner_dmx_pes_filter_params {
	v16	pid;
	v32	input;
	v32	output;
	v32	pes_type;
	v32	flags;
};

struct vtuner_dmx_pes_pid {
	uint16_t pids[5];
};

struct vtuner_dmx_caps {
	v32	caps;
	v32	num_decoders;
};

struct vtuner_dmx_stc {
	v32	num;
	v32	base;
	v64	stc;
};

/*
 * ==== Frontend device structures ====
 */

struct vtuner_dvb_frontend_info {
	v8	name [128];
	v32	type;
	v32	frequency_min;
	v32	frequency_max;
	v32	frequency_stepsize;
	v32	frequency_tolerance;
	v32	symbol_rate_min;
	v32	symbol_rate_max;
	v32	symbol_rate_tolerance;
	v32	notifier_delay;
	v32	caps;
};

struct vtuner_dvb_diseqc_master_cmd {
	v8	msg  [6];
	v8	msg_len;
};

struct vtuner_dvb_diseqc_slave_reply {
	v8	msg  [4];
	v8	msg_len;
	v32	timeout;
};

struct vtuner_dvb_qpsk_parameters {
	v32	symbol_rate;
	v32	fec_inner;
};

struct vtuner_dvb_qam_parameters {
	v32	symbol_rate;
	v32	fec_inner;
	v32	modulation;
};

struct vtuner_dvb_vsb_parameters {
	v32	modulation;
};

struct vtuner_dvb_ofdm_parameters {
	v32	bandwidth;
	v32	code_rate_HP;
	v32	code_rate_LP;
	v32	constellation;
	v32	transmission_mode;
	v32	guard_interval;
	v32	hierarchy_information;
};

struct vtuner_dvb_frontend_parameters {
	v32	frequency;
	v32	inversion;
	union {
		struct vtuner_dvb_qpsk_parameters qpsk;
		struct vtuner_dvb_qam_parameters qam;
		struct vtuner_dvb_ofdm_parameters ofdm;
		struct vtuner_dvb_vsb_parameters vsb;
	}	u;
};

struct vtuner_dvb_frontend_event {
	v32	status;
	struct vtuner_dvb_frontend_parameters parameters;
};

struct vtuner_dtv_property {
	v32	cmd;
	v32	reserved[3];
	union {
		v32	data;
		struct {
			v8	data [32];
			v32	len;
		}	buffer;
	}	u;
	int	result;
};

struct vtuner_dtv_properties {
	v32	num;
	struct vtuner_dtv_property props[VTUNER_PROP_MAX];
};

struct vtuner_message {
	struct {
		v32	magic;		/* VTUNER_MAGIC */
		v32	mtype;
		v16	rx_size;
		v16	tx_size;
		v16	error;
		v16	padding;
	}	hdr;
	union {
		v32	value32;
		v16	value16;
		struct vtuner_dmx_sct_filter_params dmx_sct_filter_params;
		struct vtuner_dmx_pes_filter_params dmx_pes_filter_params;
		struct vtuner_dmx_pes_pid dmx_pes_pid;
		struct vtuner_dmx_caps dmx_caps;
		struct vtuner_dmx_stc dmx_stc;
		struct vtuner_dvb_frontend_info dvb_frontend_info;
		struct vtuner_dvb_diseqc_master_cmd dvb_diseqc_master_cmd;
		struct vtuner_dvb_diseqc_slave_reply dvb_diseqc_slave_reply;
		struct vtuner_dvb_frontend_parameters dvb_frontend_parameters;
		struct vtuner_dvb_frontend_event dvb_frontend_event;
		struct vtuner_dtv_properties dtv_properties;
	}	body;
};

union vtuner_dvb_message {
	u32	value32;
	u16	value16;
	struct {
		u16	pids[5];
	}	dmx_pes_pid;
	struct dmx_sct_filter_params dmx_sct_filter_params;
	struct dmx_pes_filter_params dmx_pes_filter_params;
	struct dmx_caps dmx_caps;
	struct dmx_stc dmx_stc;
	struct dvb_frontend_info dvb_frontend_info;
	struct dvb_diseqc_master_cmd dvb_diseqc_master_cmd;
	struct dvb_diseqc_slave_reply dvb_diseqc_slave_reply;
	struct dvb_frontend_parameters dvb_frontend_parameters;
	struct dvb_frontend_event dvb_frontend_event;
	struct dtv_properties dtv_properties;
};

#endif					/* _VTUNER_H_ */
