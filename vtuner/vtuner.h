/*
 * vtuner API
 *
 * Copyright (C) 2010-11 Honza Petrous <jpetrous@smartimp.cz>
 * [based on dreamtuner userland code by Roland Mieslinger]
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

#ifndef _VTUNER_H_
#define	_VTUNER_H_

#include <linux/dvb/version.h>
#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

#define	VTUNER_VERSION_00030001 0x00030001
#define	VTUNER_VERSION VTUNER_VERSION_00030001
#define	VTUNER_MAGIC 0x76543210U
#define	VTUNER_MAX_PID	30
#define	VTUNER_TS_ALIGN 188
#define	VTUNER_PID_UNKNOWN 0xFFFFU
#define	VTUNER_DEFAULT_PORT "5100"

#define	VT_NULL 0x00
#define	VT_S   0x01
#define	VT_C   0x02
#define	VT_T   0x04
#define	VT_S2  0x08

#define	MSG_SET_FRONTEND		1
#define	MSG_GET_FRONTEND		2
#define	MSG_READ_STATUS			3
#define	MSG_READ_BER			4
#define	MSG_READ_SIGNAL_STRENGTH	5
#define	MSG_READ_SNR			6
#define	MSG_READ_UCBLOCKS		7
#define	MSG_SET_TONE			8
#define	MSG_SET_VOLTAGE			9
#define	MSG_ENABLE_HIGH_VOLTAGE		10
#define	MSG_SEND_DISEQC_MSG		11
#define	MSG_SEND_DISEQC_BURST		13
#define	MSG_PIDLIST			14
#define	MSG_TYPE_CHANGED		15
#define	MSG_SET_PROPERTY		16
#define	MSG_GET_PROPERTY		17

#define	MSG_NULL			1024
#define	MSG_DISCOVER			1025
#define	MSG_UPDATE       		1026

/* define platform independant types */

typedef uint8_t v8 __aligned(1);
typedef uint16_t v16 __aligned(2);
typedef uint32_t v32 __aligned(4);

typedef int8_t t8 __aligned(1);
typedef int16_t t16 __aligned(2);
typedef int32_t t32 __aligned(4);

/* define master message types */

struct diseqc_master_cmd {
	v8	msg  [6];
	v8	msg_len;
};

struct vtuner_property {
	v32	cmd;
	union {
		v32	data;
		struct {
			v8	data [32];
			v32	len;
		}	buffer;
	}	u;
	t32	result;
};

struct vtuner_message {
	t32	msg_type;
	u32	msg_magic;
	u32	msg_version;
	u32	msg_error;
	union {
		struct {
			v32	frequency;
			v8	inversion;
			union {
				struct {
					v32	symbol_rate;
					v32	fec_inner;
				}	qpsk;
				struct {
					v32	symbol_rate;
					v32	fec_inner;
					v32	modulation;
				}	qam;
				struct {
					v32	bandwidth;
					v32	code_rate_HP;
					v32	code_rate_LP;
					v32	constellation;
					v32	transmission_mode;
					v32	guard_interval;
					v32	hierarchy_information;
				}	ofdm;
				struct {
					v32	modulation;
				}	vsb;
			}	u;
		}	fe_params;
		struct vtuner_property prop;
		v32	status;
		v32	ber;
		v16	ss;
		v16	snr;
		v32	ucb;
		v8	tone;
		v8	voltage;
		struct diseqc_master_cmd diseqc_master_cmd;
		v8	burst;
		v16	pidlist[VTUNER_MAX_PID];
		v8	pad  [72];
		v32	type_changed;
	}	body;
};

#endif					/* _VTUNER_H_ */
