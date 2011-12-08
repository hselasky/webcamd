/*
 * vtuners: Virtual adapter server driver
 *
 * Copyright (C) 2010-11 Honza Petrous <jpetrous@smartimp.cz>
 * [Created 2010-03-23]
 * Sponsored by Smartimp s.r.o. for its NessieDVB.com box
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

#define	VTUNER_BSWAP32(x) x = bswap32(x)
#define	VTUNER_BSWAP16(x) x = bswap16(x)
#define	VTUNERC_MODULE_VERSION "1.2p1-hps"

static struct vtuners_ctx *vtuners_tbl[CONFIG_DVB_MAX_ADAPTERS];
static int vtuner_max_unit = 0;
static char vtuner_host[64] = {"127.0.0.1"};
static char vtuner_cport[16] = {VTUNER_DEFAULT_PORT};

static int
hw_frontend_ioctl(struct vtuners_ctx *hw, unsigned int cmd, void *arg, const char *debug)
{
	int ret;

	ret = linux_ioctl(hw->frontend_fd, 0, cmd, arg);

	if (ret != 0) {
		WARN(MSG_NET "vTuner frontend IOCTL "
		    "0x%08x(%s) failed with reason %d\n",
		    cmd, debug, ret);
	}
	return (ret);
}

#define	hw_frontend_ioctl(hw,cmd,arg)	\
	hw_frontend_ioctl(hw,cmd,arg,#cmd)

static void
hw_free(struct vtuners_ctx *hw)
{

	if (hw->frontend_fd != NULL) {
		linux_close(hw->frontend_fd);
		hw->frontend_fd = NULL;
	}
	if (hw->streaming_fd != NULL) {
		linux_close(hw->streaming_fd);
		hw->streaming_fd = NULL;
	}
	down(&hw->writer_sem);
	if (hw->demux_fd != NULL) {
		linux_close(hw->demux_fd);
		hw->demux_fd = NULL;
	}
	up(&hw->writer_sem);
}

static int
hw_init(struct vtuners_ctx *hw)
{
	int adapter;

	/* reset some fields to attach default */
	memset(&hw->fe_info, 0, sizeof(hw->fe_info));
	memset(&hw->fe_params, 0, sizeof(hw->fe_params));
	memset(&hw->msgbuf, 0, sizeof(hw->msgbuf));
	memset(&hw->buffer, 0, sizeof(hw->buffer));
	memset(&hw->pids, 0, sizeof(hw->pids));
	memset(&hw->props, 0, sizeof(hw->props));

	hw->skip_set_frontend = 0;
	hw->num_props = 0;

	adapter = hw->unit;

	hw->frontend_fd = linux_open((F_V4B_SUBDEV_MAX *
	    F_V4B_DVB_FRONTEND) + adapter, O_RDWR);

	if (hw->frontend_fd == NULL)
		goto error;

	if (hw_frontend_ioctl(hw, FE_GET_INFO, &hw->fe_info) != 0)
		goto error;

	switch (hw->fe_info.type) {
	case FE_QPSK:
		if (hw->fe_info.caps & (FE_HAS_EXTENDED_CAPS |
		    FE_CAN_2G_MODULATION)) {
			hw->type = VT_S2;
		} else {
			hw->type = VT_S;
		}
		break;
	case FE_QAM:
		hw->type = VT_C;
		break;
	case FE_OFDM:
		hw->type = VT_T;
		break;
	default:
		printk(KERN_INFO "Unknown frontend type %d\n", hw->fe_info.type);
		goto error;
	}

	printk(KERN_INFO "FE_GET_INFO dvb-type:%d vtuner-type:%d\n", hw->fe_info.type, hw->type);

	hw->streaming_fd = linux_open(
	    (F_V4B_SUBDEV_MAX * F_V4B_DVB_DVR) + adapter, O_RDONLY);

	if (hw->streaming_fd == NULL)
		goto error;

	down(&hw->writer_sem);
	hw->demux_fd = linux_open(
	    (F_V4B_SUBDEV_MAX * F_V4B_DVB_DEMUX) + adapter, O_RDWR | O_NONBLOCK);
	if (hw->demux_fd == NULL) {
		up(&hw->writer_sem);
		goto error;
	}
	if (linux_ioctl(hw->demux_fd, 0, DMX_SET_BUFFER_SIZE,
	    (void *)sizeof(hw->buffer)) != 0) {
		up(&hw->writer_sem);
		goto error;
	}
	up(&hw->writer_sem);

	return 0;

error:
	hw_free(hw);
	return -1;
}

static int
hw_get_frontend(struct vtuners_ctx *hw, struct dvb_frontend_parameters *fe_params)
{
	return hw_frontend_ioctl(hw, FE_GET_FRONTEND, fe_params);
}

int
hw_set_frontend(struct vtuners_ctx *hw, struct dvb_frontend_parameters *fe_params)
{
	struct dtv_properties cmdseq = {
		.num = 0,
		.props = NULL
	};

	struct dtv_property CLEAR[] = {
		{.cmd = DTV_CLEAR},
	};

	struct dtv_property S[] = {
		{.cmd = DTV_DELIVERY_SYSTEM,.u.data = SYS_DVBS},
		{.cmd = DTV_FREQUENCY,.u.data = fe_params->frequency},
		{.cmd = DTV_MODULATION,.u.data = QPSK},
		{.cmd = DTV_SYMBOL_RATE,.u.data = fe_params->u.qpsk.symbol_rate},
		{.cmd = DTV_INNER_FEC,.u.data = fe_params->u.qpsk.fec_inner},
		{.cmd = DTV_INVERSION,.u.data = INVERSION_AUTO},
		{.cmd = DTV_ROLLOFF,.u.data = ROLLOFF_AUTO},
		{.cmd = DTV_PILOT,.u.data = PILOT_AUTO},
		{.cmd = DTV_TUNE},
	};

	int ret;

	cmdseq.num = 1;
	cmdseq.props = CLEAR;

	hw_frontend_ioctl(hw, FE_SET_PROPERTY, &cmdseq);

	switch (hw->type) {
	case VT_S:
	case VT_S2:
		cmdseq.num = 9;
		cmdseq.props = S;
		if ((hw->type == VT_S || hw->type == VT_S2) &&
		    (fe_params->u.qpsk.fec_inner > FEC_AUTO)) {
			cmdseq.props[0].u.data = SYS_DVBS2;
			switch (fe_params->u.qpsk.fec_inner) {
			case 19:
				cmdseq.props[2].u.data = PSK_8;
			case 10:
				cmdseq.props[4].u.data = FEC_1_2;
				break;
			case 20:
				cmdseq.props[2].u.data = PSK_8;
			case 11:
				cmdseq.props[4].u.data = FEC_2_3;
				break;
			case 21:
				cmdseq.props[2].u.data = PSK_8;
			case 12:
				cmdseq.props[4].u.data = FEC_3_4;
				break;
			case 22:
				cmdseq.props[2].u.data = PSK_8;
			case 13:
				cmdseq.props[4].u.data = FEC_5_6;
				break;
			case 23:
				cmdseq.props[2].u.data = PSK_8;
			case 14:
				cmdseq.props[4].u.data = FEC_7_8;
				break;
			case 24:
				cmdseq.props[2].u.data = PSK_8;
			case 15:
				cmdseq.props[4].u.data = FEC_8_9;
				break;
			case 25:
				cmdseq.props[2].u.data = PSK_8;
			case 16:
				cmdseq.props[4].u.data = FEC_3_5;
				break;
			case 26:
				cmdseq.props[2].u.data = PSK_8;
			case 17:
				cmdseq.props[4].u.data = FEC_4_5;
				break;
			case 27:
				cmdseq.props[2].u.data = PSK_8;
			case 18:
				cmdseq.props[4].u.data = FEC_9_10;
				break;
			default:
				WARN(MSG_NET, "FEC unknown\n");
				break;
			}
			switch (fe_params->inversion & 0x0c) {
			case 0:
				cmdseq.props[6].u.data = ROLLOFF_35;
				break;
			case 4:
				cmdseq.props[6].u.data = ROLLOFF_25;
				break;
			case 8:
				cmdseq.props[6].u.data = ROLLOFF_20;
				break;
			default:
				WARN(MSG_NET, "ROLLOFF unknnown\n");
			}
			switch (fe_params->inversion & 0x30) {
			case 0:
				cmdseq.props[7].u.data = PILOT_OFF;
				break;
			case 0x10:
				cmdseq.props[7].u.data = PILOT_ON;
				break;
			case 0x20:
				cmdseq.props[7].u.data = PILOT_AUTO;
				break;
			default:
				WARN(MSG_NET, "PILOT unknown\n");
				break;
			}
			cmdseq.props[5].u.data &= 0x04;
		}
#if 0
		printk(KERN_INFO "S2API tuning SYS:%d MOD:%d FEC:%d INV:%d ROLLOFF:%d PILOT:%d\n",
		    cmdseq.props[0].u.data, cmdseq.props[2].u.data, cmdseq.props[4].u.data,
		    cmdseq.props[5].u.data, cmdseq.props[6].u.data, cmdseq.props[7].u.data);
#endif
		ret = hw_frontend_ioctl(hw, FE_SET_PROPERTY, &cmdseq);
		break;
	case VT_C:
	case VT_T:
		/* Even If we would have S2API, the old is sufficent to tune */
		ret = hw_frontend_ioctl(hw, FE_SET_FRONTEND, fe_params);
		break;
	default:
		WARN(MSG_NET, "tuning not implemented for HW-type:%d (S:%d, S2:%d C:%d T:%d)\n",
		    hw->type, VT_S, VT_S2, VT_C, VT_T);
		ret = -EINVAL;
		break;
	}
	if (ret != 0)
		WARN(MSG_NET, "FE_SET_FRONTEND failed %s - %m\n", msg);
	return ret;
}

int
hw_get_property(struct vtuners_ctx *hw, struct dtv_property *prop)
{
	WARN(MSG_NET, "FE_GET_PROPERTY: not implemented %d\n", prop->cmd);
	return 0;
}

int
hw_set_property(struct vtuners_ctx *hw, struct dtv_property *prop)
{
	struct dtv_properties cmdseq;
	int ret = 0;

	memset(&cmdseq, 0, sizeof(cmdseq));

	switch (prop->cmd) {
	case DTV_UNDEFINED:
		break;
	case DTV_CLEAR:
		hw->num_props = 0;
		break;
	case DTV_TUNE:
		if (hw->num_props < DTV_IOCTL_MAX_MSGS) {
			hw->props[hw->num_props].cmd = prop->cmd;
			hw->props[hw->num_props].u.data = prop->u.data;
			hw->num_props++;

			cmdseq.num = hw->num_props;
			cmdseq.props = hw->props;

			ret = hw_frontend_ioctl(hw, FE_SET_PROPERTY, &cmdseq);
		} else {
			ret = -1;
		}
		break;

	case DTV_FREQUENCY:
	case DTV_MODULATION:
	case DTV_BANDWIDTH_HZ:
	case DTV_INVERSION:
	case DTV_DISEQC_MASTER:
	case DTV_SYMBOL_RATE:
	case DTV_INNER_FEC:
	case DTV_VOLTAGE:
	case DTV_TONE:
	case DTV_PILOT:
	case DTV_ROLLOFF:
	case DTV_DISEQC_SLAVE_REPLY:
	case DTV_FE_CAPABILITY_COUNT:
	case DTV_FE_CAPABILITY:
	case DTV_DELIVERY_SYSTEM:
		if (hw->num_props < DTV_IOCTL_MAX_MSGS) {
			hw->props[hw->num_props].cmd = prop->cmd;
			hw->props[hw->num_props].u.data = prop->u.data;
			hw->num_props++;
		} else {
			ret = -1;
		} break;
	default:
		WARN(MSG_NET, "FE_SET_PROPERTY unknown property %d\n", prop->cmd);
		ret = -1;
		break;
	}
	return ret;
}

int
hw_read_status(struct vtuners_ctx *hw, u32 * status)
{
	return (hw_frontend_ioctl(hw, FE_READ_STATUS, status));
}

int
hw_set_tone(struct vtuners_ctx *hw, u8 tone)
{
	return (hw_frontend_ioctl(hw, FE_SET_TONE, (void *)(long)tone));
}

int
hw_set_voltage(struct vtuners_ctx *hw, u8 voltage)
{
	return (hw_frontend_ioctl(hw, FE_SET_VOLTAGE, (void *)(long)voltage));
}

int
hw_send_diseq_msg(struct vtuners_ctx *hw, struct diseqc_master_cmd *cmd)
{
	return (hw_frontend_ioctl(hw, FE_DISEQC_SEND_MASTER_CMD, cmd));
}

int
hw_send_diseq_burst(struct vtuners_ctx *hw, u8 burst)
{
	return (hw_frontend_ioctl(hw, FE_DISEQC_SEND_BURST, (void *)(long)burst));
}

int
hw_pidlist(struct vtuners_ctx *hw, u16 * pidlist)
{
	struct dmx_pes_filter_params flt;
	int i;

	memset(&flt, 0, sizeof(flt));

	/*
	 * Make some assumptions about how the PID list is organized
	 * at the client to avoid expensive sort and compare:
	 */

	for (i = 0; i < VTUNER_MAX_PID; i++) {
		if (hw->pids[i] != pidlist[i]) {
			if (hw->pids[i] != VTUNER_PID_UNKNOWN) {
				/*
				 * Need to stop the demuxer:
				 * A PID was changed.
				 */
				linux_ioctl(hw->demux_fd, 0, DMX_STOP, 0);
				break;
			}
		}
	}

	for (i = 0; i < VTUNER_MAX_PID; i++) {
		if (hw->pids[i] != pidlist[i]) {
			if (pidlist[i] != VTUNER_PID_UNKNOWN) {
				flt.pid = pidlist[i];
				flt.input = DMX_IN_FRONTEND;
				flt.pes_type = DMX_PES_OTHER;
				flt.output = DMX_OUT_TS_TAP;
				flt.flags = DMX_IMMEDIATE_START;
				linux_ioctl(hw->demux_fd, 0, DMX_SET_PES_FILTER, &flt);
			}
			hw->pids[i] = pidlist[i];
		}
	}
	return 0;
}

void
vtuners_get_dvb_fe_param(struct dvb_frontend_parameters *hfe, struct vtuner_message *msg, int type)
{
	memset(hfe, 0, sizeof(hfe));

	hfe->frequency = msg->body.fe_params.frequency;
	hfe->inversion = msg->body.fe_params.inversion;

	switch (type) {
	case VT_S:
	case VT_S2:
		hfe->u.qpsk.symbol_rate = msg->body.fe_params.u.qpsk.symbol_rate;
		hfe->u.qpsk.fec_inner = msg->body.fe_params.u.qpsk.fec_inner;
		break;
	case VT_C:
		hfe->u.qam.symbol_rate = msg->body.fe_params.u.qam.symbol_rate;
		hfe->u.qam.fec_inner = msg->body.fe_params.u.qam.fec_inner;
		hfe->u.qam.modulation = msg->body.fe_params.u.qam.modulation;
		break;
	case VT_T:
		hfe->u.ofdm.bandwidth = msg->body.fe_params.u.ofdm.bandwidth;
		hfe->u.ofdm.code_rate_HP = msg->body.fe_params.u.ofdm.code_rate_HP;
		hfe->u.ofdm.code_rate_LP = msg->body.fe_params.u.ofdm.code_rate_LP;
		hfe->u.ofdm.constellation = msg->body.fe_params.u.ofdm.constellation;
		hfe->u.ofdm.transmission_mode = msg->body.fe_params.u.ofdm.transmission_mode;
		hfe->u.ofdm.guard_interval = msg->body.fe_params.u.ofdm.guard_interval;
		hfe->u.ofdm.hierarchy_information = msg->body.fe_params.u.ofdm.hierarchy_information;
		break;
	default:
		break;
	}
}

void
vtuners_set_dvb_fe_param(struct vtuner_message *msg, struct dvb_frontend_parameters *hfe, int type)
{
	msg->body.fe_params.frequency = hfe->frequency;
	msg->body.fe_params.inversion = hfe->inversion;

	switch (type) {
	case VT_S:
	case VT_S2:
		msg->body.fe_params.u.qpsk.symbol_rate = hfe->u.qpsk.symbol_rate;
		msg->body.fe_params.u.qpsk.fec_inner = hfe->u.qpsk.fec_inner;
		break;
	case VT_C:
		msg->body.fe_params.u.qam.symbol_rate = hfe->u.qam.symbol_rate;
		msg->body.fe_params.u.qam.fec_inner = hfe->u.qam.fec_inner;
		msg->body.fe_params.u.qam.modulation = hfe->u.qam.modulation;
		break;
	case VT_T:
		msg->body.fe_params.u.ofdm.bandwidth = hfe->u.ofdm.bandwidth;
		msg->body.fe_params.u.ofdm.code_rate_HP = hfe->u.ofdm.code_rate_HP;
		msg->body.fe_params.u.ofdm.code_rate_LP = hfe->u.ofdm.code_rate_LP;
		msg->body.fe_params.u.ofdm.constellation = hfe->u.ofdm.constellation;
		msg->body.fe_params.u.ofdm.transmission_mode = hfe->u.ofdm.transmission_mode;
		msg->body.fe_params.u.ofdm.guard_interval = hfe->u.ofdm.guard_interval;
		msg->body.fe_params.u.ofdm.hierarchy_information = hfe->u.ofdm.hierarchy_information;
		break;
	default:
		break;
	}
}

static void
vtuners_byteswap(struct vtuner_message *msg, int type)
{
	int i;

	VTUNER_BSWAP32(msg->msg_type);
	VTUNER_BSWAP32(msg->msg_magic);
	VTUNER_BSWAP32(msg->msg_version);
	VTUNER_BSWAP32(msg->msg_error);

	switch (msg->msg_type) {
	case MSG_GET_FRONTEND:
	case MSG_SET_FRONTEND:
		VTUNER_BSWAP32(msg->body.fe_params.frequency);
		switch (type) {
		case VT_S:
		case VT_S2:
		case VT_S | VT_S2:
			VTUNER_BSWAP32(msg->body.fe_params.u.qpsk.symbol_rate);
			VTUNER_BSWAP32(msg->body.fe_params.u.qpsk.fec_inner);
			break;
		case VT_C:
			VTUNER_BSWAP32(msg->body.fe_params.u.qam.symbol_rate);
			VTUNER_BSWAP32(msg->body.fe_params.u.qam.fec_inner);
			VTUNER_BSWAP32(msg->body.fe_params.u.qam.modulation);
			break;
		case VT_T:
			VTUNER_BSWAP32(msg->body.fe_params.u.ofdm.bandwidth);
			VTUNER_BSWAP32(msg->body.fe_params.u.ofdm.code_rate_HP);
			VTUNER_BSWAP32(msg->body.fe_params.u.ofdm.code_rate_LP);
			VTUNER_BSWAP32(msg->body.fe_params.u.ofdm.constellation);
			VTUNER_BSWAP32(msg->body.fe_params.u.ofdm.transmission_mode);
			VTUNER_BSWAP32(msg->body.fe_params.u.ofdm.guard_interval);
			VTUNER_BSWAP32(msg->body.fe_params.u.ofdm.hierarchy_information);
			break;
		default:
			WARN(MSG_NET, "unkown frontend type %d (known types are "
			    "%d,%d,%d,%d)\n", type, VT_S, VT_C, VT_T, VT_S2);
			break;
		}
		break;
	case MSG_READ_STATUS:
		VTUNER_BSWAP32(msg->body.status);
		break;
	case MSG_READ_BER:
		VTUNER_BSWAP32(msg->body.ber);
		break;
	case MSG_READ_SIGNAL_STRENGTH:
		VTUNER_BSWAP16(msg->body.ss);
		break;
	case MSG_READ_SNR:
		VTUNER_BSWAP16(msg->body.snr);
		break;
	case MSG_READ_UCBLOCKS:
		VTUNER_BSWAP32(msg->body.ucb);
		break;
	case MSG_SET_PROPERTY:
	case MSG_GET_PROPERTY:
		VTUNER_BSWAP32(msg->body.prop.cmd);
		VTUNER_BSWAP32(msg->body.prop.u.data);
		break;
	case MSG_PIDLIST:
		for (i = 0; i < VTUNER_MAX_PID; i++)
			VTUNER_BSWAP16(msg->body.pidlist[i]);
		break;
	default:
		WARN(MSG_NET, "Unkown message type %d\n", msg->msg_type);
		break;
	}
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

static int
vtuners_process_msg(struct vtuners_ctx *hw, struct vtuner_message *msg)
{
	int swapped;
	int ret;

	if (msg->msg_magic != VTUNER_MAGIC) {
		vtuners_byteswap(msg, hw->type);
		if (msg->msg_magic != VTUNER_MAGIC)
			return (-1);
		swapped = 1;
	} else {
		swapped = 0;
	}

	switch (msg->msg_type) {
	case MSG_SET_FRONTEND:
		memset(&hw->fe_params, 0, sizeof(hw->fe_params));
		vtuners_get_dvb_fe_param(&hw->fe_params, msg, hw->type);
		if (hw->skip_set_frontend) {
			ret = 0;
		} else {
			ret = hw_set_frontend(hw, &hw->fe_params);
		}
		break;
	case MSG_GET_FRONTEND:
		memset(&hw->fe_params, 0, sizeof(hw->fe_params));
		ret = hw_get_frontend(hw, &hw->fe_params);
		vtuners_set_dvb_fe_param(msg, &hw->fe_params, hw->type);
		break;
	case MSG_READ_STATUS:
		ret = hw_read_status(hw, &msg->body.status);
		break;
	case MSG_READ_BER:
		ret = hw_frontend_ioctl(hw, FE_READ_BER, &msg->body.ber);
		break;
	case MSG_READ_SIGNAL_STRENGTH:
		ret = hw_frontend_ioctl(hw, FE_READ_SIGNAL_STRENGTH, &msg->body.ss);
		break;
	case MSG_READ_SNR:
		ret = hw_frontend_ioctl(hw, FE_READ_SNR, &msg->body.snr);
		break;
	case MSG_READ_UCBLOCKS:
		ret = hw_frontend_ioctl(hw, FE_READ_UNCORRECTED_BLOCKS, &msg->body.ucb);
		break;
	case MSG_SET_TONE:
		ret = hw_set_tone(hw, msg->body.tone);
		break;
	case MSG_SET_VOLTAGE:
		ret = hw_set_voltage(hw, msg->body.voltage);
		break;
	case MSG_ENABLE_HIGH_VOLTAGE:
		ret = 0;
		break;
	case MSG_SEND_DISEQC_MSG:
		ret = hw_send_diseq_msg(hw, &msg->body.diseqc_master_cmd);
		break;
	case MSG_SEND_DISEQC_BURST:
		ret = hw_send_diseq_burst(hw, msg->body.burst);
		break;
	case MSG_SET_PROPERTY:
		ret = hw_set_property(hw, &msg->body.prop);
		/*
		 * In case the call was successful, we have to skip
		 * the next call to SET_FRONTEND
		 */
		hw->skip_set_frontend = (ret == 0);
		break;
	case MSG_GET_PROPERTY:
		ret = hw_get_property(hw, &msg->body.prop);
		break;
	case MSG_PIDLIST:
		ret = hw_pidlist(hw, msg->body.pidlist);
		break;
	default:
		WARN(MSG_NET, "Unknown vtuner message %d\n", msg->msg_type);
		ret = -1;
		break;
	}

	if (msg->msg_error != 0) {
		msg->msg_error = ret;
		if (swapped)
			vtuners_byteswap(msg, hw->type);
		if ((ret = vtuners_write(hw->fd_control,
		    (u8 *) msg, sizeof(*msg))) != sizeof(*msg)) {
			printk(KERN_INFO "vTuner Write error %d\n", ret);
			return (-1);
		}
	}
	return (0);
}

static void
vtuners_connect_control(struct vtuners_ctx *hw)
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
	    vtuner_host, hw->cport);

	if ((error = getaddrinfo(vtuner_host, hw->cport, &hints, &res)))
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

	hw->fd_control = accept(s, NULL, NULL);

	printk(KERN_INFO "vTuner accepted %d (control)\n", hw->fd_control);

	close(s);

	s = hw->fd_control;

	if (s < 0)
		return;

	flag = 2 * sizeof(struct vtuner_message);
	setsockopt(s, SOL_SOCKET, SO_SNDBUF, &flag, (int)sizeof(flag));
	setsockopt(s, SOL_SOCKET, SO_RCVBUF, &flag, (int)sizeof(flag));

	flag = 1;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

static void
vtuners_connect_data(struct vtuners_ctx *hw)
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
	    vtuner_host, hw->dport);

	if ((error = getaddrinfo(vtuner_host, hw->dport, &hints, &res)))
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

	hw->fd_data = accept(s, NULL, NULL);

	printk(KERN_INFO "vTuner accepted %d (data)\n", hw->fd_data);

	close(s);

	s = hw->fd_data;

	if (s < 0)
		return;

	flag = 2 * sizeof(hw->buffer);
	setsockopt(s, SOL_SOCKET, SO_SNDBUF, &flag, sizeof(flag));

	flag = 1;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
}

static void *
vtuners_writer_thread(void *arg)
{
	struct vtuners_ctx *hw = arg;
	int len;

	while (1) {

		while (hw->fd_data < 0) {
			down(&hw->writer_sem);
			len = (hw->demux_fd != NULL);
			up(&hw->writer_sem);

			if (len != 0)
				vtuners_connect_data(hw);
			if (hw->fd_data < 0) {
				usleep(1000000);
				continue;
			}
		}

		down(&hw->writer_sem);
		if (hw->demux_fd == NULL) {
			len = -EWOULDBLOCK;
		} else {
			len = linux_read(hw->demux_fd, -1,
			    (u8 *) hw->buffer, sizeof(hw->buffer));
		}
		up(&hw->writer_sem);

		if (len == -EWOULDBLOCK) {
			usleep(2000);
			continue;
		} else if (len <= 0) {
			close(hw->fd_data);
			hw->fd_data = -1;
			continue;
		}
		if (vtuners_write(hw->fd_data,
		    (u8 *) hw->buffer, len) != len) {
			close(hw->fd_data);
			hw->fd_data = -1;
			continue;
		}
	}
	return (NULL);
}

static void *
vtuners_control_thread(void *arg)
{
	struct vtuners_ctx *hw = arg;
	int len;

	while (1) {

		while (hw->fd_control < 0) {
			vtuners_connect_control(hw);
			if (hw->fd_control < 0) {
				usleep(1000000);
				continue;
			}
			if (hw_init(hw) < 0) {
				printk(KERN_INFO "vTuner hardware init failed\n");
				close(hw->fd_control);
				hw->fd_control = -1;
				usleep(1000000);
				continue;
			}
		}

		len = sizeof(hw->msgbuf);

		if (vtuners_read(hw->fd_control, (u8 *) & hw->msgbuf, len) != len) {
			printk(KERN_INFO "vTuner read error %d\n", len);
			close(hw->fd_control);
			hw->fd_control = -1;
			hw_free(hw);
		} else {
			if (vtuners_process_msg(hw, &hw->msgbuf) < 0) {
				printk(KERN_INFO "vTuner process error\n");
				close(hw->fd_control);
				hw->fd_control = -1;
				hw_free(hw);
			}
		}
	}
	return (NULL);
}

static int __init
vtuners_init(void)
{
	struct vtuners_ctx *hw = NULL;
	int u;

	if (vtuner_max_unit < 0 || vtuner_max_unit > CONFIG_DVB_MAX_ADAPTERS)
		vtuner_max_unit = CONFIG_DVB_MAX_ADAPTERS;

	printk(KERN_INFO "virtual DVB server adapter driver, version "
	    VTUNERC_MODULE_VERSION
	    ", (c) 2010-11 Honza Petrous, SmartImp.cz\n");

	for (u = 0; u < vtuner_max_unit; u++) {
		hw = kzalloc(sizeof(struct vtuners_ctx), GFP_KERNEL);
		if (!hw)
			return -ENOMEM;

		vtuners_tbl[u] = hw;

		hw->fd_data = -1;
		hw->fd_control = -1;
		hw->unit = u;

		sema_init(&hw->writer_sem, 1);

		snprintf(hw->cport, sizeof(hw->cport),
		    "%u", atoi(vtuner_cport) + (2 * u));

		snprintf(hw->dport, sizeof(hw->dport),
		    "%u", atoi(vtuner_cport) + (2 * u) + 1);

		/* create writer thread */
		pthread_create(&hw->writer_thread, NULL, vtuners_writer_thread, hw);

		/* create control thread */
		pthread_create(&hw->control_thread, NULL, vtuners_control_thread, hw);
	}

	return (0);
}

static void __exit
vtuners_exit(void)
{
	struct vtuners_ctx *hw;
	int u;

	for (u = 0; u < vtuner_max_unit; u++) {
		hw = vtuners_tbl[u];
		if (hw == NULL)
			continue;

		hw_free(hw);
		kfree(hw);
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

MODULE_AUTHOR("Honza Petrous");
MODULE_DESCRIPTION("Virtual DVB device server");
MODULE_LICENSE("GPL");
MODULE_VERSION(VTUNERS_MODULE_VERSION);
