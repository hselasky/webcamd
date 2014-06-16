/* -------------------------------------------------------------------------
 *   Copyright (C) 1995-99 Simon G. Vogl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ------------------------------------------------------------------------- */

/* With some changes from Ky<C3><B6>sti M<C3><A4>lkki <kmalkki@cc.hut.fi>.
   All SMBus-related things are written by Frodo Looijaard <frodol@dds.nl>
   SMBus 2.0 support by Mark Studebaker <mdsxyz123@yahoo.com> and
   Jean Delvare <khali@linux-fr.org>
   Mux support by Rodolfo Giometti <giometti@enneenne.com> and
   Michael Lawnick <michael.lawnick.ext@nsn.com> */

/* NOTE: This code derives from i2c-core.c in Linux */

#include <linux/i2c.h>
#include <linux/idr.h>

static	DEFINE_IDR(i2c_adapter_idr);
static TAILQ_HEAD(, i2c_driver) i2c_driver_head = TAILQ_HEAD_INITIALIZER(i2c_driver_head);

int
i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	unsigned long end_jiffies;
	int ret;
	int try;

	if (adap->algo->master_xfer) {

		mutex_lock(&adap->bus_lock);

		end_jiffies = jiffies + adap->timeout;
		for (ret = 0, try = 0; try <= adap->retries; try++) {
			ret = adap->algo->master_xfer(adap, msgs, num);
			if (ret != -EAGAIN)
				break;
			if (time_after(jiffies, end_jiffies))
				break;
		}
		mutex_unlock(&adap->bus_lock);

		return (ret);
	} else {
		return (-EOPNOTSUPP);
	}
}

int
__i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	unsigned long end_jiffies;
	int ret;
	int try;

	if (adap->algo->master_xfer) {

		end_jiffies = jiffies + adap->timeout;
		for (ret = 0, try = 0; try <= adap->retries; try++) {
			ret = adap->algo->master_xfer(adap, msgs, num);
			if (ret != -EAGAIN)
				break;
			if (time_after(jiffies, end_jiffies))
				break;
		}
		return (ret);
	} else {
		return (-EOPNOTSUPP);
	}
}

int
i2c_register_driver(struct module *mod, struct i2c_driver *drv)
{
	TAILQ_INSERT_TAIL(&i2c_driver_head, drv, entry);
	return (0);
}

void
i2c_del_driver(struct i2c_driver *drv)
{
	TAILQ_REMOVE(&i2c_driver_head, drv, entry);
}

static int
i2c_register_adapter(struct i2c_adapter *adap)
{
	int res;

	mutex_init(&adap->bus_lock);
	if (adap->timeout == 0)
		adap->timeout = HZ;

	dev_set_name(&adap->dev, "i2c-%d", adap->nr);
	res = device_register(&adap->dev);

	return (res);
}

static int
i2c_add_numbered_adapter_sub(struct i2c_adapter *adap)
{
	int id;

	atomic_lock();
	id = idr_alloc(&i2c_adapter_idr, adap, adap->nr, adap->nr + 1,
	    GFP_KERNEL);
	atomic_unlock();
	if (id < 0)
		return ((id == -ENOSPC) ? -EBUSY : id);

	return (i2c_register_adapter(adap));
}

int
i2c_add_numbered_adapter(struct i2c_adapter *adap)
{
	if (adap->nr == -1)		/* -1 means dynamically assign bus id */
		return (i2c_add_adapter(adap));

	return (i2c_add_numbered_adapter_sub(adap));
}

int
i2c_add_adapter(struct i2c_adapter *adapt)
{
	static int nr = 128;

	atomic_lock();
	adapt->nr = ++nr;		/* dummy */
	atomic_unlock();

	return (i2c_register_adapter(adapt));
}

int
i2c_del_adapter(struct i2c_adapter *adapt)
{
	return (0);
}

struct i2c_client *
i2c_new_device(struct i2c_adapter *adapt, struct i2c_board_info const *info)
{
	struct i2c_client *client;
	struct i2c_driver *drv;
	int status = -1;

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (client == NULL)
		return (NULL);

	client->adapter = adapt;

	client->dev.platform_data = info->platform_data;

	client->flags = info->flags;
	client->addr = info->addr;
	client->irq = info->irq;

	strlcpy(client->name, info->type, sizeof(client->name));

	client->dev.parent = &client->adapter->dev;

	/* For 10-bit clients, add an arbitrary offset to avoid collisions */
	dev_set_name(&client->dev, "%d-%04x", i2c_adapter_id(adapt),
	    client->addr | ((client->flags & I2C_CLIENT_TEN)
	    ? 0xa000 : 0));

	TAILQ_FOREACH(drv, &i2c_driver_head, entry) {
		uint32_t i;

		for (i = 0; drv->id_table != NULL &&
		    drv->id_table[i].name[0] != 0; i++) {
			if (strcmp(drv->id_table[i].name, client->name) != 0)
				continue;
			client->driver = drv;
			client->dev.driver = &drv->driver;

			if (drv->probe == NULL) {
				status = 0;
				break;
			}
			status = drv->probe(client, drv->id_table + i);
			if (status == 0)
				break;
			client->driver = NULL;
			client->dev.driver = NULL;
		}
	}
	if (status != 0) {
		kfree(client);
		return (NULL);
	}
	return (client);
}

struct i2c_client *
i2c_new_probed_device(struct i2c_adapter *adapt,
    struct i2c_board_info *info,
    const unsigned short *probe_attrs,
    unsigned short const *addr_list)
{
	return (i2c_new_device(adapt, info));
}

void
i2c_unregister_device(struct i2c_client *client)
{
	if (client == NULL)
		return;

	if (client->driver != NULL && client->driver->remove != NULL)
		client->driver->remove(client);

	kfree(client);
}

int
i2c_master_send(struct i2c_client *client, const char *buf, int count)
{
	int ret;
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.len = count;
	msg.buf = (char *)buf;

	ret = i2c_transfer(adap, &msg, 1);

	return ((ret == 1) ? count : ret);
}

int
i2c_master_recv(struct i2c_client *client, char *buf, int count)
{
	struct i2c_adapter *adap = client->adapter;
	struct i2c_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = buf;

	ret = i2c_transfer(adap, &msg, 1);

	return ((ret == 1) ? count : ret);
}

void
i2c_clients_command(struct i2c_adapter *adap, unsigned int cmd, void *arg)
{
	printf("WARNING: i2c_clients_command() is not implemented\n");
}

#define	POLY (0x1070U << 3)

static	u8
crc8(u16 data)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (data & 0x8000)
			data = data ^ POLY;
		data = data << 1;
	}
	return (u8) (data >> 8);
}

static	u8
i2c_smbus_pec(u8 crc, u8 * p, size_t count)
{
	int i;

	for (i = 0; i < count; i++)
		crc = crc8((crc ^ p[i]) << 8);
	return crc;
}

static	u8
i2c_smbus_msg_pec(u8 pec, struct i2c_msg *msg)
{
	u8 addr = (msg->addr << 1) | !!(msg->flags & I2C_M_RD);

	pec = i2c_smbus_pec(pec, &addr, 1);

	return i2c_smbus_pec(pec, msg->buf, msg->len);
}

static inline void
i2c_smbus_add_pec(struct i2c_msg *msg)
{
	msg->buf[msg->len] = i2c_smbus_msg_pec(0, msg);
	msg->len++;
}

static int
i2c_smbus_check_pec(u8 cpec, struct i2c_msg *msg)
{
	u8 rpec = msg->buf[--msg->len];

	cpec = i2c_smbus_msg_pec(cpec, msg);

	if (rpec != cpec) {
		pr_debug("i2c-core: Bad PEC 0x%02x vs. 0x%02x\n",
		    rpec, cpec);
		return -EBADMSG;
	}
	return 0;
}

int
i2c_smbus_read_byte(struct i2c_client *client)
{
	union i2c_smbus_data data;
	int status;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_READ, 0,
	    I2C_SMBUS_BYTE, &data);
	return (status < 0) ? status : data.byte;
}

int
i2c_smbus_write_byte(struct i2c_client *client, u8 value)
{
	return i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_WRITE, value, I2C_SMBUS_BYTE, NULL);
}

int
i2c_smbus_read_byte_data(struct i2c_client *client, u8 command)
{
	union i2c_smbus_data data;
	int status;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_READ, command,
	    I2C_SMBUS_BYTE_DATA, &data);
	return (status < 0) ? status : data.byte;
}

int
i2c_smbus_write_byte_data(struct i2c_client *client, u8 command,
    u8 value)
{
	union i2c_smbus_data data;

	data.byte = value;
	return i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_WRITE, command,
	    I2C_SMBUS_BYTE_DATA, &data);
}

int
i2c_smbus_read_word_data(struct i2c_client *client, u8 command)
{
	union i2c_smbus_data data;
	int status;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_READ, command,
	    I2C_SMBUS_WORD_DATA, &data);
	return (status < 0) ? status : data.word;
}

int
i2c_smbus_write_word_data(struct i2c_client *client, u8 command,
    u16 value)
{
	union i2c_smbus_data data;

	data.word = value;
	return i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_WRITE, command,
	    I2C_SMBUS_WORD_DATA, &data);
}

int
i2c_smbus_process_call(struct i2c_client *client, u8 command,
    u16 value)
{
	union i2c_smbus_data data;
	int status;

	data.word = value;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_WRITE, command,
	    I2C_SMBUS_PROC_CALL, &data);
	return (status < 0) ? status : data.word;
}

int
i2c_smbus_read_block_data(struct i2c_client *client, u8 command,
    u8 * values)
{
	union i2c_smbus_data data;
	int status;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_READ, command,
	    I2C_SMBUS_BLOCK_DATA, &data);
	if (status)
		return status;

	memcpy(values, &data.block[1], data.block[0]);
	return data.block[0];
}

int
i2c_smbus_write_block_data(struct i2c_client *client, u8 command,
    u8 length, const u8 * values)
{
	union i2c_smbus_data data;

	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	data.block[0] = length;
	memcpy(&data.block[1], values, length);
	return i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_WRITE, command,
	    I2C_SMBUS_BLOCK_DATA, &data);
}

int
i2c_smbus_read_i2c_block_data(struct i2c_client *client, u8 command,
    u8 length, u8 * values)
{
	union i2c_smbus_data data;
	int status;

	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	data.block[0] = length;
	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_READ, command,
	    I2C_SMBUS_I2C_BLOCK_DATA, &data);
	if (status < 0)
		return status;

	memcpy(values, &data.block[1], data.block[0]);
	return data.block[0];
}

int
i2c_smbus_write_i2c_block_data(struct i2c_client *client, u8 command,
    u8 length, const u8 * values)
{
	union i2c_smbus_data data;

	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	data.block[0] = length;
	memcpy(data.block + 1, values, length);
	return i2c_smbus_xfer(client->adapter, client->addr, client->flags,
	    I2C_SMBUS_WRITE, command,
	    I2C_SMBUS_I2C_BLOCK_DATA, &data);
}

static int
i2c_smbus_xfer_emulated(struct i2c_adapter *adapter, u16 addr,
    unsigned short flags,
    char read_write, u8 command, int size,
    union i2c_smbus_data *data)
{
	unsigned char msgbuf0[I2C_SMBUS_BLOCK_MAX + 3];
	unsigned char msgbuf1[I2C_SMBUS_BLOCK_MAX + 2];
	int num = read_write == I2C_SMBUS_READ ? 2 : 1;
	struct i2c_msg msg[2] = {{addr, flags, 1, msgbuf0},
	{addr, flags | I2C_M_RD, 0, msgbuf1}
	};
	int i;
	u8 partial_pec = 0;
	int status;

	msgbuf0[0] = command;
	switch (size) {
	case I2C_SMBUS_QUICK:
		msg[0].len = 0;
		msg[0].flags = flags | (read_write == I2C_SMBUS_READ ?
		    I2C_M_RD : 0);
		num = 1;
		break;
	case I2C_SMBUS_BYTE:
		if (read_write == I2C_SMBUS_READ) {
			msg[0].flags = I2C_M_RD | flags;
			num = 1;
		}
		break;
	case I2C_SMBUS_BYTE_DATA:
		if (read_write == I2C_SMBUS_READ)
			msg[1].len = 1;
		else {
			msg[0].len = 2;
			msgbuf0[1] = data->byte;
		}
		break;
	case I2C_SMBUS_WORD_DATA:
		if (read_write == I2C_SMBUS_READ)
			msg[1].len = 2;
		else {
			msg[0].len = 3;
			msgbuf0[1] = data->word & 0xff;
			msgbuf0[2] = data->word >> 8;
		}
		break;
	case I2C_SMBUS_PROC_CALL:
		num = 2;
		read_write = I2C_SMBUS_READ;
		msg[0].len = 3;
		msg[1].len = 2;
		msgbuf0[1] = data->word & 0xff;
		msgbuf0[2] = data->word >> 8;
		break;
	case I2C_SMBUS_BLOCK_DATA:
		if (read_write == I2C_SMBUS_READ) {
			msg[1].flags |= I2C_M_RECV_LEN;
			msg[1].len = 1;
		} else {
			msg[0].len = data->block[0] + 2;
			if (msg[0].len > I2C_SMBUS_BLOCK_MAX + 2) {
				dev_err(&adapter->dev,
				    "Invalid block write size %d\n",
				    data->block[0]);
				return -EINVAL;
			}
			for (i = 1; i < msg[0].len; i++)
				msgbuf0[i] = data->block[i - 1];
		}
		break;
	case I2C_SMBUS_BLOCK_PROC_CALL:
		num = 2;
		read_write = I2C_SMBUS_READ;
		if (data->block[0] > I2C_SMBUS_BLOCK_MAX) {
			dev_err(&adapter->dev,
			    "Invalid block write size %d\n",
			    data->block[0]);
			return -EINVAL;
		}
		msg[0].len = data->block[0] + 2;
		for (i = 1; i < msg[0].len; i++)
			msgbuf0[i] = data->block[i - 1];
		msg[1].flags |= I2C_M_RECV_LEN;
		msg[1].len = 1;
		break;
	case I2C_SMBUS_I2C_BLOCK_DATA:
		if (read_write == I2C_SMBUS_READ) {
			msg[1].len = data->block[0];
		} else {
			msg[0].len = data->block[0] + 1;
			if (msg[0].len > I2C_SMBUS_BLOCK_MAX + 1) {
				dev_err(&adapter->dev,
				    "Invalid block write size %d\n",
				    data->block[0]);
				return -EINVAL;
			}
			for (i = 1; i <= data->block[0]; i++)
				msgbuf0[i] = data->block[i];
		}
		break;
	default:
		dev_err(&adapter->dev, "Unsupported transaction %d\n", size);
		return -EOPNOTSUPP;
	}

	i = ((flags & I2C_CLIENT_PEC) && size != I2C_SMBUS_QUICK
	    && size != I2C_SMBUS_I2C_BLOCK_DATA);
	if (i) {
		if (!(msg[0].flags & I2C_M_RD)) {
			if (num == 1)
				i2c_smbus_add_pec(&msg[0]);
			else
				partial_pec = i2c_smbus_msg_pec(0, &msg[0]);
		}
		if (msg[num - 1].flags & I2C_M_RD)
			msg[num - 1].len++;
	}
	status = i2c_transfer(adapter, msg, num);
	if (status < 0)
		return status;

	if (i && (msg[num - 1].flags & I2C_M_RD)) {
		status = i2c_smbus_check_pec(partial_pec, &msg[num - 1]);
		if (status < 0)
			return status;
	}
	if (read_write == I2C_SMBUS_READ)
		switch (size) {
		case I2C_SMBUS_BYTE:
			data->byte = msgbuf0[0];
			break;
		case I2C_SMBUS_BYTE_DATA:
			data->byte = msgbuf1[0];
			break;
		case I2C_SMBUS_WORD_DATA:
		case I2C_SMBUS_PROC_CALL:
			data->word = msgbuf1[0] | (msgbuf1[1] << 8);
			break;
		case I2C_SMBUS_I2C_BLOCK_DATA:
			for (i = 0; i < data->block[0]; i++)
				data->block[i + 1] = msgbuf1[i];
			break;
		case I2C_SMBUS_BLOCK_DATA:
		case I2C_SMBUS_BLOCK_PROC_CALL:
			for (i = 0; i < msgbuf1[0] + 1; i++)
				data->block[i] = msgbuf1[i];
			break;
		}
	return 0;
}

int
i2c_smbus_xfer(struct i2c_adapter *adapter, u16 addr, unsigned short flags,
    char read_write, u8 command, int protocol,
    union i2c_smbus_data *data)
{
	unsigned long orig_jiffies;
	int try;
	int res;

	flags &= I2C_M_TEN | I2C_CLIENT_PEC;

	if (adapter->algo->smbus_xfer) {

		mutex_lock(&adapter->bus_lock);

		orig_jiffies = jiffies;
		for (res = 0, try = 0; try <= adapter->retries; try++) {
			res = adapter->algo->smbus_xfer(adapter, addr, flags,
			    read_write, command,
			    protocol, data);
			if (res != -EAGAIN)
				break;
			if (time_after(jiffies,
			    orig_jiffies + adapter->timeout))
				break;
		}

		mutex_unlock(&adapter->bus_lock);
	} else
		res = i2c_smbus_xfer_emulated(adapter, addr, flags, read_write,
		    command, protocol, data);

	return res;
}

struct i2c_client *
i2c_verify_client(struct device *dev)
{
	return NULL;			/* NOT supported */
}
