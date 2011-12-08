/*
 * vtunerc: Virtual adapter client driver
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
#include <vtuner/vtuner_client.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>

#define	VTUNERC_MODULE_VERSION "1.2p1-hps"

DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

#define	DRIVER_NAME		"vTuner proxy"

static struct vtunerc_ctx *vtunerc_tbl[CONFIG_DVB_MAX_ADAPTERS];
static int vtuner_max_unit = 0;
static int vtuner_type = 0;
static char vtuner_host[64] = {"127.0.0.1"};
static char vtuner_cport[16] = {"5100"};

static int
vtunerc_do_message(struct vtunerc_ctx *ctx,
    struct vtuner_message *msg, int do_wait);

/*------------------------------------------------------------------------*
 * PID table management
 *------------------------------------------------------------------------*/
static int
pidtab_find_index(u16 * pidtab, int pid)
{
	int i;

	for (i = 0; i < (VTUNER_MAX_PID - 1); i++) {
		if (pidtab[i] == pid)
			return (i);
	}

	return -1;
}

static int
pidtab_add_pid(u16 * pidtab, int pid)
{
	int i;

	for (i = 0; i < (VTUNER_MAX_PID - 1); i++)
		if (pidtab[i] == PID_UNKNOWN ||
		    pidtab[i] == pid) {
			pidtab[i] = pid;
			return 0;
		}
	return -1;
}

static int
pidtab_del_pid(u16 * pidtab, int pid)
{
	int i;

	for (i = 0; i < (VTUNER_MAX_PID - 1); i++) {
		if (pidtab[i] == pid) {
			pidtab[i] = PID_UNKNOWN;
			return 0;
		}
	}

	return -1;
}

static void
pidtab_copy_to_msg(struct vtunerc_ctx *ctx,
    struct vtuner_message *msg)
{
	int i;

	for (i = 0; i < (VTUNER_MAX_PID - 1); i++)
		msg->body.pidlist[i] = ctx->pidtab[i];
	msg->body.pidlist[i] = 0;
}

static int
vtunerc_start_feed(struct dvb_demux_feed *feed)
{
	struct dvb_demux *demux = feed->demux;
	struct vtunerc_ctx *ctx = demux->priv;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	switch (feed->type) {
	case DMX_TYPE_TS:
		break;
	case DMX_TYPE_SEC:
		break;
	case DMX_TYPE_PES:
		printk(KERN_ERR "vtunerc: feed type PES is not supported\n");
		return -EINVAL;
	default:
		printk(KERN_ERR "vtunerc: feed type %d is not supported\n",
		    feed->type);
		return -EINVAL;
	}

	/* organize PID list table */

	if (pidtab_find_index(ctx->pidtab, feed->pid) < 0) {
		pidtab_add_pid(ctx->pidtab, feed->pid);

		pidtab_copy_to_msg(ctx, &msg);

		msg.msg_type = MSG_PIDLIST;

		if (vtunerc_do_message(ctx, &msg, 0) < 0)
			return -ENXIO;
	}
	return 0;
}

static int
vtunerc_stop_feed(struct dvb_demux_feed *feed)
{
	struct dvb_demux *demux = feed->demux;
	struct vtunerc_ctx *ctx = demux->priv;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	/* organize PID list table */

	if (pidtab_find_index(ctx->pidtab, feed->pid) > -1) {
		pidtab_del_pid(ctx->pidtab, feed->pid);

		pidtab_copy_to_msg(ctx, &msg);

		msg.msg_type = MSG_PIDLIST;

		if (vtunerc_do_message(ctx, &msg, 0) < 0)
			return -ENXIO;
	}
	return 0;
}

/*------------------------------------------------------------------------*
 * DVB frontend proxy
 *------------------------------------------------------------------------*/

struct dvb_proxyfe_state {
	struct dvb_frontend frontend;
	struct vtunerc_ctx *ctx;
};

static void
vtunerc_tryconnect(struct vtunerc_ctx *ctx)
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

	if ((error = getaddrinfo(vtuner_host, ctx->cport, &hints, &res)))
		return;

	res0 = res;

	do {
		if ((s = socket(res0->ai_family, res0->ai_socktype,
		    res0->ai_protocol)) < 0)
			continue;

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

	ctx->fd_control = s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((error = getaddrinfo(vtuner_host, ctx->cport, &hints, &res))) {
		close(ctx->fd_control);
		ctx->fd_control = -1;
		return;
	}
	res0 = res;

	do {
		if ((s = socket(res0->ai_family, res0->ai_socktype,
		    res0->ai_protocol)) < 0)
			continue;

		flag = sizeof(ctx->buffer);
		setsockopt(s, SOL_SOCKET, SO_RCVBUF, &flag, sizeof(flag));

		flag = 1;
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

		if (connect(s, res0->ai_addr, res0->ai_addrlen) == 0)
			break;

		close(s);
		s = -1;
	} while ((res0 = res0->ai_next) != NULL);

	freeaddrinfo(res);

	if (s < 0) {
		close(ctx->fd_control);
		ctx->fd_control = -1;
		return;
	}
	ctx->fd_data = s;
}

static int
vtunerc_do_message(struct vtunerc_ctx *ctx,
    struct vtuner_message *msg, int do_wait)
{
	int ret = 0;

	down(&ctx->xchange_sem);

	/* stamp the byte order and version */
	msg->msg_magic = VTUNER_MAGIC;
	msg->msg_version = VTUNER_VERSION;
	msg->msg_error = do_wait ? -1U : 0U;

retry:
	if (ctx->fd_control < 0 || ctx->fd_data < 0) {
		up(&ctx->xchange_sem);
		return -ENXIO;
	}
	if (write(ctx->fd_control, msg, sizeof(struct vtuner_message)) !=
	    sizeof(struct vtuner_message)) {
		close(ctx->fd_control);
		ctx->fd_control = -1;
		pthread_kill(ctx->reader_thread, SIGURG);
		goto retry;
	}
	if (do_wait) {
		if (read(ctx->fd_control, msg, sizeof(struct vtuner_message)) !=
		    sizeof(struct vtuner_message)) {
			close(ctx->fd_control);
			ctx->fd_control = -1;
			pthread_kill(ctx->reader_thread, SIGURG);
			goto retry;
		}
		ret = msg->msg_error;
	}
	up(&ctx->xchange_sem);
	return ret;
}

static void *
vtuner_reader_thread(void *arg)
{
	struct vtunerc_ctx *ctx = arg;
	int len;

	while (1) {

		while (ctx->fd_control < 0 || ctx->fd_data < 0) {
			down(&ctx->xchange_sem);
			if (ctx->fd_control >= 0) {
				close(ctx->fd_control);
				ctx->fd_control = -1;
			}
			if (ctx->fd_data >= 0) {
				close(ctx->fd_data);
				ctx->fd_data = -1;
			}
			vtunerc_tryconnect(ctx);

			if (ctx->fd_control < 0 || ctx->fd_data < 0)
				usleep(1000000);

			up(&ctx->xchange_sem);
		}

		if (ctx->trailsize != 0)
			memcpy(ctx->buffer, ctx->trailbuf, ctx->trailsize);

		len = read(ctx->fd_data, ((u8 *) ctx->buffer) + ctx->trailsize,
		    sizeof(ctx->buffer) - ctx->trailsize);
		if (len <= 0) {
			close(ctx->fd_data);
			ctx->fd_data = -1;
			ctx->trailsize = 0;
		} else {
			int actlen;
			int rem;

			len += ctx->trailsize;

			rem = len % VTUNER_TS_ALIGN;
			actlen = len - rem;

			if (rem != 0) {
				memcpy(ctx->trailbuf, ((u8 *) ctx->buffer) + actlen, rem);
				ctx->trailsize = rem;
			} else {
				ctx->trailsize = 0;
			}

			if (actlen >= VTUNER_TS_ALIGN) {
				dvb_dmx_swfilter_packets(&ctx->demux,
				    (u8 *) ctx->buffer, actlen / VTUNER_TS_ALIGN);
			}
		}
	}
	return (NULL);
}

static int
dvb_proxyfe_read_status(struct dvb_frontend *fe, fe_status_t *status)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.msg_type = MSG_READ_STATUS;

	if (vtunerc_do_message(ctx, &msg, 1) < 0) {
		*status = 0;
		return -ENXIO;
	}
	*status = msg.body.status;
	return 0;
}

static int
dvb_proxyfe_read_ber(struct dvb_frontend *fe, u32 * ber)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.msg_type = MSG_READ_BER;

	if (vtunerc_do_message(ctx, &msg, 1) < 0) {
		*ber = 0;
		return -ENXIO;
	}
	*ber = msg.body.ber;
	return 0;
}

static int
dvb_proxyfe_read_signal_strength(struct dvb_frontend *fe,
    u16 * strength)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.msg_type = MSG_READ_SIGNAL_STRENGTH;

	if (vtunerc_do_message(ctx, &msg, 1) < 0) {
		*strength = 0;
		return -ENXIO;
	}
	*strength = msg.body.ss;
	return 0;
}

static int
dvb_proxyfe_read_snr(struct dvb_frontend *fe, u16 * snr)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.msg_type = MSG_READ_SNR;

	if (vtunerc_do_message(ctx, &msg, 1) < 0) {
		*snr = 0;
		return -ENXIO;
	}
	*snr = msg.body.snr;
	return 0;
}

static int
dvb_proxyfe_read_ucblocks(struct dvb_frontend *fe, u32 * ucblocks)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.msg_type = MSG_READ_UCBLOCKS;

	if (vtunerc_do_message(ctx, &msg, 1) < 0) {
		*ucblocks = 0;
		return -ENXIO;
	}
	*ucblocks = msg.body.ucb;
	return 0;
}

static int
dvb_proxyfe_get_frontend(struct dvb_frontend *fe,
    struct dvb_frontend_parameters *p)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.msg_type = MSG_GET_FRONTEND;

	if (vtunerc_do_message(ctx, &msg, 1) < 0)
		return -ENXIO;

	switch (ctx->vtype) {
	case VT_S:
	case VT_S2:
		{
			struct dvb_qpsk_parameters *op = &p->u.qpsk;

			op->symbol_rate = msg.body.fe_params.u.qpsk.symbol_rate;
			op->fec_inner = msg.body.fe_params.u.qpsk.fec_inner;
		}
		break;
	case VT_T:
		{
			struct dvb_ofdm_parameters *op = &p->u.ofdm;

			op->bandwidth = msg.body.fe_params.u.ofdm.bandwidth;
			op->code_rate_HP = msg.body.fe_params.u.ofdm.code_rate_HP;
			op->code_rate_LP = msg.body.fe_params.u.ofdm.code_rate_LP;
			op->constellation = msg.body.fe_params.u.ofdm.constellation;
			op->transmission_mode = msg.body.fe_params.u.ofdm.transmission_mode;
			op->guard_interval = msg.body.fe_params.u.ofdm.guard_interval;
			op->hierarchy_information = msg.body.fe_params.u.ofdm.hierarchy_information;
		}
		break;
	case VT_C:
		{
			struct dvb_qam_parameters *op = &p->u.qam;

			op->symbol_rate = msg.body.fe_params.u.qam.symbol_rate;
			op->fec_inner = msg.body.fe_params.u.qam.fec_inner;
			op->modulation = msg.body.fe_params.u.qam.modulation;
		}
		break;
	default:
		printk(KERN_ERR "vtunerc unrecognized tuner vtype = %d\n",
		    ctx->vtype);
		return -EINVAL;
	}
	p->frequency = msg.body.fe_params.frequency;
	p->inversion = msg.body.fe_params.inversion;
	return 0;
}

static int
dvb_proxyfe_set_frontend(struct dvb_frontend *fe,
    struct dvb_frontend_parameters *p)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.body.fe_params.frequency = p->frequency;
	msg.body.fe_params.inversion = p->inversion;

	switch (ctx->vtype) {
	case VT_S:
	case VT_S2:
		{
			struct dvb_qpsk_parameters *op = &p->u.qpsk;
			struct dtv_frontend_properties *props = &fe->dtv_property_cache;

			msg.body.fe_params.u.qpsk.symbol_rate = op->symbol_rate;
			msg.body.fe_params.u.qpsk.fec_inner = op->fec_inner;

			if (ctx->vtype == VT_S2 && props->delivery_system == SYS_DVBS2) {
				/* DELIVERY SYSTEM: S2 delsys in use */
				msg.body.fe_params.u.qpsk.fec_inner = 9;

				/* MODULATION */
				if (props->modulation == PSK_8)
					/* signal PSK_8 modulation used */
					msg.body.fe_params.u.qpsk.fec_inner += 9;

				/* FEC */
				switch (props->fec_inner) {
				case FEC_1_2:
					msg.body.fe_params.u.qpsk.fec_inner += 1;
					break;
				case FEC_2_3:
					msg.body.fe_params.u.qpsk.fec_inner += 2;
					break;
				case FEC_3_4:
					msg.body.fe_params.u.qpsk.fec_inner += 3;
					break;
				case FEC_4_5:
					msg.body.fe_params.u.qpsk.fec_inner += 8;
					break;
				case FEC_5_6:
					msg.body.fe_params.u.qpsk.fec_inner += 4;
					break;
					/*
					 * case FEC_6_7: // undefined
					 * msg.body.fe_params.u.qpsk.fec_inne
					 * r += 2; break;
					 */
				case FEC_7_8:
					msg.body.fe_params.u.qpsk.fec_inner += 5;
					break;
				case FEC_8_9:
					msg.body.fe_params.u.qpsk.fec_inner += 6;
					break;
					/*
					 * case FEC_AUTO: // undefined
					 * msg.body.fe_params.u.qpsk.fec_inne
					 * r += 2; break;
					 */
				case FEC_3_5:
					msg.body.fe_params.u.qpsk.fec_inner += 7;
					break;
				case FEC_9_10:
					msg.body.fe_params.u.qpsk.fec_inner += 9;
					break;
				default:
					;	/* FIXME: what now? */
					break;
				}

				/* ROLLOFF */
				switch (props->rolloff) {
				case ROLLOFF_20:
					msg.body.fe_params.inversion |= 0x08;
					break;
				case ROLLOFF_25:
					msg.body.fe_params.inversion |= 0x04;
					break;
				case ROLLOFF_35:
				default:
					break;
				}

				/* PILOT */
				switch (props->pilot) {
				case PILOT_ON:
					msg.body.fe_params.inversion |= 0x10;
					break;
				case PILOT_AUTO:
					msg.body.fe_params.inversion |= 0x20;
					break;
				case PILOT_OFF:
				default:
					break;
				}
			}
		}
		break;
	case VT_T:
		{
			struct dvb_ofdm_parameters *op = &p->u.ofdm;

			msg.body.fe_params.u.ofdm.bandwidth = op->bandwidth;
			msg.body.fe_params.u.ofdm.code_rate_HP = op->code_rate_HP;
			msg.body.fe_params.u.ofdm.code_rate_LP = op->code_rate_LP;
			msg.body.fe_params.u.ofdm.constellation = op->constellation;
			msg.body.fe_params.u.ofdm.transmission_mode = op->transmission_mode;
			msg.body.fe_params.u.ofdm.guard_interval = op->guard_interval;
			msg.body.fe_params.u.ofdm.hierarchy_information = op->hierarchy_information;
		}
		break;
	case VT_C:
		{
			struct dvb_qam_parameters *op = &p->u.qam;

			msg.body.fe_params.u.qam.symbol_rate = op->symbol_rate;
			msg.body.fe_params.u.qam.fec_inner = op->fec_inner;
			msg.body.fe_params.u.qam.modulation = op->modulation;
		}
		break;
	default:
		printk(KERN_ERR "vtunerc: unrecognized tuner vtype = %d\n",
		    ctx->vtype);
		return -EINVAL;
	}

	msg.msg_type = MSG_SET_FRONTEND;

	if (vtunerc_do_message(ctx, &msg, 1) < 0)
		return -ENXIO;

	return 0;
}

static int
dvb_proxyfe_get_property(struct dvb_frontend *fe, struct dtv_property *tvp)
{
	return 0;
}

static enum dvbfe_algo
dvb_proxyfe_get_frontend_algo(struct dvb_frontend *fe)
{
	return DVBFE_ALGO_SW;
}

static int
dvb_proxyfe_sleep(struct dvb_frontend *fe)
{
	return 0;
}

static int
dvb_proxyfe_init(struct dvb_frontend *fe)
{
	return 0;
}

static int
dvb_proxyfe_set_tone(struct dvb_frontend *fe, fe_sec_tone_mode_t tone)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.body.tone = tone;
	msg.msg_type = MSG_SET_TONE;

	if (vtunerc_do_message(ctx, &msg, 1) < 0)
		return -ENXIO;

	return 0;
}

static int
dvb_proxyfe_set_voltage(struct dvb_frontend *fe, fe_sec_voltage_t voltage)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.body.voltage = voltage;
	msg.msg_type = MSG_SET_VOLTAGE;

	if (vtunerc_do_message(ctx, &msg, 1) < 0)
		return -ENXIO;

	return 0;
}

static int
dvb_proxyfe_send_diseqc_msg(struct dvb_frontend *fe, struct dvb_diseqc_master_cmd *cmd)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	memcpy(&msg.body.diseqc_master_cmd, cmd, sizeof(struct dvb_diseqc_master_cmd));
	msg.msg_type = MSG_SEND_DISEQC_MSG;

	if (vtunerc_do_message(ctx, &msg, 1) < 0)
		return -ENXIO;

	return 0;
}

static int
dvb_proxyfe_send_diseqc_burst(struct dvb_frontend *fe, fe_sec_mini_cmd_t burst)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;
	struct vtunerc_ctx *ctx = state->ctx;
	struct vtuner_message msg;

	memset(&msg, 0, sizeof(msg));

	msg.body.burst = burst;
	msg.msg_type = MSG_SEND_DISEQC_BURST;

	if (vtunerc_do_message(ctx, &msg, 1) < 0)
		return -ENXIO;

	return 0;
}

static void
dvb_proxyfe_release(struct dvb_frontend *fe)
{
	struct dvb_proxyfe_state *state = fe->demodulator_priv;

	kfree(state);
}

static struct dvb_frontend_ops dvb_proxyfe_ofdm_ops;

static struct dvb_frontend *
dvb_proxyfe_ofdm_attach(struct vtunerc_ctx *ctx)
{
	struct dvb_frontend *fe = ctx->fe;

	if (!fe) {
		struct dvb_proxyfe_state *state = NULL;

		/* allocate memory for the internal state */
		state = kmalloc(sizeof(struct dvb_proxyfe_state), GFP_KERNEL);
		if (state == NULL) {
			return NULL;
		}
		fe = &state->frontend;
		fe->demodulator_priv = state;
		state->ctx = ctx;
	}
	memcpy(&fe->ops, &dvb_proxyfe_ofdm_ops, sizeof(struct dvb_frontend_ops));

	return fe;
}

static struct dvb_frontend_ops dvb_proxyfe_qpsk_ops;

static struct dvb_frontend *
dvb_proxyfe_qpsk_attach(struct vtunerc_ctx *ctx, int can_2g_modulation)
{
	struct dvb_frontend *fe = ctx->fe;

	if (!fe) {
		struct dvb_proxyfe_state *state = NULL;

		/* allocate memory for the internal state */
		state = kmalloc(sizeof(struct dvb_proxyfe_state), GFP_KERNEL);
		if (state == NULL) {
			return NULL;
		}
		fe = &state->frontend;
		fe->demodulator_priv = state;
		state->ctx = ctx;
	}
	memcpy(&fe->ops, &dvb_proxyfe_qpsk_ops, sizeof(struct dvb_frontend_ops));
	if (can_2g_modulation) {
		fe->ops.info.caps |= FE_CAN_2G_MODULATION;
		strcpy(fe->ops.info.name, "vTuner proxyFE DVB-S2");
	}
	return fe;
}

static struct dvb_frontend_ops dvb_proxyfe_qam_ops;

static struct dvb_frontend *
dvb_proxyfe_qam_attach(struct vtunerc_ctx *ctx)
{
	struct dvb_frontend *fe = ctx->fe;

	if (!fe) {
		struct dvb_proxyfe_state *state = NULL;

		/* allocate memory for the internal state */
		state = kmalloc(sizeof(struct dvb_proxyfe_state), GFP_KERNEL);
		if (state == NULL) {
			return NULL;
		}
		fe = &state->frontend;
		fe->demodulator_priv = state;
		state->ctx = ctx;
	}
	memcpy(&fe->ops, &dvb_proxyfe_qam_ops, sizeof(struct dvb_frontend_ops));

	return fe;
}

static struct dvb_frontend_ops dvb_proxyfe_ofdm_ops = {

	.info = {
		.name = "vTuner proxyFE DVB-T",
		.type = FE_OFDM,
		.frequency_min = 51000000,
		.frequency_max = 863250000,
		.frequency_stepsize = 62500,
		.caps = FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
		FE_CAN_FEC_4_5 | FE_CAN_FEC_5_6 | FE_CAN_FEC_6_7 |
		FE_CAN_FEC_7_8 | FE_CAN_FEC_8_9 | FE_CAN_FEC_AUTO |
		FE_CAN_QAM_16 | FE_CAN_QAM_64 | FE_CAN_QAM_AUTO |
		FE_CAN_TRANSMISSION_MODE_AUTO |
		FE_CAN_GUARD_INTERVAL_AUTO |
		FE_CAN_HIERARCHY_AUTO,
	},

	.release = dvb_proxyfe_release,

	.init = dvb_proxyfe_init,
	.sleep = dvb_proxyfe_sleep,

	.set_frontend = dvb_proxyfe_set_frontend,
	.get_frontend = dvb_proxyfe_get_frontend,

	.read_status = dvb_proxyfe_read_status,
	.read_ber = dvb_proxyfe_read_ber,
	.read_signal_strength = dvb_proxyfe_read_signal_strength,
	.read_snr = dvb_proxyfe_read_snr,
	.read_ucblocks = dvb_proxyfe_read_ucblocks,
};

static struct dvb_frontend_ops dvb_proxyfe_qam_ops = {

	.info = {
		.name = "vTuner proxyFE DVB-C",
		.type = FE_QAM,
		.frequency_stepsize = 62500,
		.frequency_min = 51000000,
		.frequency_max = 858000000,
		/* SACLK/64 == (XIN/2)/64 */
		.symbol_rate_min = (57840000 / 2) / 64,
		/* SACLK/4 */
		.symbol_rate_max = (57840000 / 2) / 4,
		.caps = FE_CAN_QAM_16 | FE_CAN_QAM_32 | FE_CAN_QAM_64 |
		FE_CAN_QAM_128 | FE_CAN_QAM_256 |
		FE_CAN_FEC_AUTO | FE_CAN_INVERSION_AUTO
	},

	.release = dvb_proxyfe_release,

	.init = dvb_proxyfe_init,
	.sleep = dvb_proxyfe_sleep,

	.set_frontend = dvb_proxyfe_set_frontend,
	.get_frontend = dvb_proxyfe_get_frontend,

	.read_status = dvb_proxyfe_read_status,
	.read_ber = dvb_proxyfe_read_ber,
	.read_signal_strength = dvb_proxyfe_read_signal_strength,
	.read_snr = dvb_proxyfe_read_snr,
	.read_ucblocks = dvb_proxyfe_read_ucblocks,
};

static struct dvb_frontend_ops dvb_proxyfe_qpsk_ops = {

	.info = {
		.name = "vTuner proxyFE DVB-S",
		.type = FE_QPSK,
		.frequency_min = 950000,
		.frequency_max = 2150000,
		/* kHz for QPSK frontends */
		.frequency_stepsize = 250,
		.frequency_tolerance = 29500,
		.symbol_rate_min = 1000000,
		.symbol_rate_max = 45000000,
		.caps = FE_CAN_INVERSION_AUTO |
		FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
		FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
		FE_CAN_QPSK
	},

	.release = dvb_proxyfe_release,

	.init = dvb_proxyfe_init,
	.sleep = dvb_proxyfe_sleep,

	.get_frontend = dvb_proxyfe_get_frontend,
	.get_property = dvb_proxyfe_get_property,
	.get_frontend_algo = dvb_proxyfe_get_frontend_algo,
	.set_frontend = dvb_proxyfe_set_frontend,

	.read_status = dvb_proxyfe_read_status,
	.read_ber = dvb_proxyfe_read_ber,
	.read_signal_strength = dvb_proxyfe_read_signal_strength,
	.read_snr = dvb_proxyfe_read_snr,
	.read_ucblocks = dvb_proxyfe_read_ucblocks,

	.set_voltage = dvb_proxyfe_set_voltage,
	.set_tone = dvb_proxyfe_set_tone,

	.diseqc_send_master_cmd = dvb_proxyfe_send_diseqc_msg,
	.diseqc_send_burst = dvb_proxyfe_send_diseqc_burst,

};

static int
vtunerc_frontend_init(struct vtunerc_ctx *ctx, int vtype)
{
	int ret = 0;

	if (ctx->fe && vtype == ctx->vtype) {
		printk(KERN_NOTICE "vtunerc: frontend already "
		    "initialized as type=%d\n",
		    ctx->vtype);
		return 0;
	}
	switch (vtype) {
	case VT_S:
		ctx->fe = dvb_proxyfe_qpsk_attach(ctx, 0);
		break;
	case VT_S2:
		ctx->fe = dvb_proxyfe_qpsk_attach(ctx, 1);
		break;
	case VT_T:
		ctx->fe = dvb_proxyfe_ofdm_attach(ctx);
		break;
	case VT_C:
		ctx->fe = dvb_proxyfe_qam_attach(ctx);
		break;
	default:
		printk(KERN_ERR "vtunerc: unregognized tuner vtype = %d\n",
		    ctx->vtype);
		return -EINVAL;
	}

	/* register frontend if vtype is zero */
	if (ctx->vtype == VT_NULL)
		ret = dvb_register_frontend(&ctx->dvb_adapter, ctx->fe);

	ctx->vtype = vtype;

	return ret;
}

static int
vtunerc_frontend_clear(struct vtunerc_ctx *ctx)
{
	return ctx->fe ? dvb_unregister_frontend(ctx->fe) : 0;
}

/*------------------------------------------------------------------------*
 * VTuner init and uninit
 *------------------------------------------------------------------------*/

static int __init
vtunerc_init(void)
{
	struct vtunerc_ctx *ctx = NULL;
	struct dvb_demux *dvbdemux;
	struct dmx_demux *dmx;
	int ret = -EINVAL;
	int i;
	int u;

	printk(KERN_INFO "virtual DVB adapter driver, version "
	    VTUNERC_MODULE_VERSION
	    ", (c) 2010-11 Honza Petrous, SmartImp.cz\n");

	if (vtuner_max_unit > CONFIG_DVB_MAX_ADAPTERS)
		vtuner_max_unit = CONFIG_DVB_MAX_ADAPTERS;
	else if (vtuner_max_unit < 0)
		vtuner_max_unit = 0;

	for (u = 0; u < vtuner_max_unit; u++) {
		ctx = kzalloc(sizeof(struct vtunerc_ctx), GFP_KERNEL);
		if (!ctx)
			return -ENOMEM;

		vtunerc_tbl[u] = ctx;

		ctx->fd_control = -1;
		ctx->fd_data = -1;

		snprintf(ctx->cport, sizeof(ctx->cport),
		    "%u", atoi(vtuner_cport) + (2 * u));

		snprintf(ctx->dport, sizeof(ctx->dport),
		    "%u", atoi(vtuner_cport) + (2 * u) + 1);

		/* DVB */

		/* create new adapter */
		ret = dvb_register_adapter(&ctx->dvb_adapter, DRIVER_NAME,
		    THIS_MODULE, NULL, adapter_nr);
		if (ret < 0)
			goto err_kfree;

		ctx->dvb_adapter.priv = ctx;

		memset(&ctx->demux, 0, sizeof(ctx->demux));
		dvbdemux = &ctx->demux;
		dvbdemux->priv = ctx;
		dvbdemux->filternum = VTUNER_MAX_PID;
		dvbdemux->feednum = VTUNER_MAX_PID;
		dvbdemux->start_feed = vtunerc_start_feed;
		dvbdemux->stop_feed = vtunerc_stop_feed;
		dvbdemux->dmx.capabilities = 0;
		ret = dvb_dmx_init(dvbdemux);
		if (ret < 0)
			goto err_dvb_unregister_adapter;

		dmx = &dvbdemux->dmx;

		ctx->hw_frontend.source = DMX_FRONTEND_0;
		ctx->mem_frontend.source = DMX_MEMORY_FE;
		ctx->dmxdev.filternum = VTUNER_MAX_PID;
		ctx->dmxdev.demux = dmx;

		ret = dvb_dmxdev_init(&ctx->dmxdev, &ctx->dvb_adapter);
		if (ret < 0)
			goto err_dvb_dmx_release;

		ret = dmx->add_frontend(dmx, &ctx->hw_frontend);
		if (ret < 0)
			goto err_dvb_dmxdev_release;

		ret = dmx->add_frontend(dmx, &ctx->mem_frontend);
		if (ret < 0)
			goto err_remove_hw_frontend;

		ret = dmx->connect_frontend(dmx, &ctx->hw_frontend);
		if (ret < 0)
			goto err_remove_mem_frontend;

		sema_init(&ctx->xchange_sem, 1);

		/* init pid table */
		for (i = 0; i < VTUNER_MAX_PID; i++)
			ctx->pidtab[i] = PID_UNKNOWN;

		/* setup frontend */
		if (vtunerc_frontend_init(ctx, vtuner_type) != 0)
			ctx->vtype = VT_NULL;

		/* create reader thread */
		pthread_create(&ctx->reader_thread, NULL, vtuner_reader_thread, ctx);
	}

out:
	return ret;

	dmx->disconnect_frontend(dmx);
err_remove_mem_frontend:
	dmx->remove_frontend(dmx, &ctx->mem_frontend);
err_remove_hw_frontend:
	dmx->remove_frontend(dmx, &ctx->hw_frontend);
err_dvb_dmxdev_release:
	dvb_dmxdev_release(&ctx->dmxdev);
err_dvb_dmx_release:
	dvb_dmx_release(dvbdemux);
err_dvb_unregister_adapter:
	dvb_unregister_adapter(&ctx->dvb_adapter);
err_kfree:
	kfree(ctx);
	goto out;
}

static void __exit
vtunerc_exit(void)
{
	struct dvb_demux *dvbdemux;
	struct dmx_demux *dmx;
	int u;

	for (u = 0; u < vtuner_max_unit; u++) {
		struct vtunerc_ctx *ctx = vtunerc_tbl[u];

		if (!ctx)
			continue;
		vtunerc_tbl[u] = NULL;

		vtunerc_frontend_clear(ctx);

		dvbdemux = &ctx->demux;
		dmx = &dvbdemux->dmx;

		dmx->disconnect_frontend(dmx);
		dmx->remove_frontend(dmx, &ctx->mem_frontend);
		dmx->remove_frontend(dmx, &ctx->hw_frontend);
		dvb_dmxdev_release(&ctx->dmxdev);
		dvb_dmx_release(dvbdemux);
		dvb_unregister_adapter(&ctx->dvb_adapter);
		kfree(ctx);
	}

	printk(KERN_NOTICE "vtunerc: unloaded successfully\n");
}

module_init(vtunerc_init);
module_exit(vtunerc_exit);

module_param_named(devices, vtuner_max_unit, int, 0644);
MODULE_PARM_DESC(devices, "Number of clients (default is 0, disabled)");

module_param_named(type, vtuner_type, int, 0644);
MODULE_PARM_DESC(type, "Type of adapters (S=1,C=2,T=4,S2=8, default is 0)");

module_param_string(host, vtuner_host, sizeof(vtuner_host), 0644);
MODULE_PARM_DESC(host, "Hostname at which to connect (default is 127.0.0.1)");

module_param_string(cport, vtuner_cport, sizeof(vtuner_cport), 0644);
MODULE_PARM_DESC(cport, "Control port at host (default is 5100)");

MODULE_AUTHOR("Honza Petrous");
MODULE_DESCRIPTION("Virtual DVB device client");
MODULE_LICENSE("GPL");
MODULE_VERSION(VTUNERC_MODULE_VERSION);
