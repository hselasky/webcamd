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

/* NOTE: This code derives from i2c-core.c in Linux */

int
i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	unsigned long orig_jiffies;
	int ret;
	int try;

	if (adap->algo->master_xfer) {

		mutex_lock(&adap->bus_lock);

		orig_jiffies = jiffies;
		for (ret = 0, try = 0; try <= adap->retries; try++) {
			ret = adap->algo->master_xfer(adap, msgs, num);
			if (ret != -EAGAIN)
				break;
			if (time_after(jiffies, orig_jiffies + adap->timeout))
				break;
		}
		mutex_unlock(&adap->bus_lock);

		return (ret);
	} else {
		return (-EOPNOTSUPP);
	}
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
	return (NULL);
}

struct i2c_client *
i2c_new_probed_device(struct i2c_adapter *adapt,
    struct i2c_board_info *info,
    unsigned short const *addr_list)
{
	return (i2c_new_device(adapt, info));
}

void
i2c_unregister_device(struct i2c_client *client)
{
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
