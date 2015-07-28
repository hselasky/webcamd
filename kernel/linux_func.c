/*-
 * Copyright (c) 2009-2012 Hans Petter Selasky. All rights reserved.
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
 * NOTE: Some functions in this file derive directly from the Linux kernel
 * sources and are covered by the GPLv2.
 */

#include <media/v4l2-dev.h>

#include <linux/leds.h>
#include <linux/major.h>
#include <linux/power_supply.h>
#include <linux/dma-buf.h>

#include <dvbdev.h>

static struct timespec ktime_mono_to_real_offset;
static struct timespec ktime_mono_to_uptime_offset;

int
printk_nop()
{
	return (1);
}

void
le16_to_cpus(uint16_t *p)
{
	uint16_t temp;

	/* assuming that the pointer is correctly aligned */

	temp = ((uint8_t *)p)[0] | (((uint8_t *)p)[1] << 8);

	*p = temp;
}

void
le32_to_cpus(uint32_t *p)
{
	uint32_t temp;

	/* assuming that the pointer is correctly aligned */

	temp = (((uint8_t *)p)[0] | (((uint8_t *)p)[1] << 8) |
	    (((uint8_t *)p)[2] << 16) | (((uint8_t *)p)[3] << 24));

	*p = temp;
}

void
le64_to_cpus(uint64_t *p)
{
	uint64_t temp;

	/* assuming that the pointer is correctly aligned */
	temp = (((uint8_t *)p)[4] | (((uint8_t *)p)[5] << 8) |
	    (((uint8_t *)p)[6] << 16) | (((uint8_t *)p)[7] << 24));
	temp <<= 32;
	temp |= (((uint8_t *)p)[0] | (((uint8_t *)p)[1] << 8) |
	    (((uint8_t *)p)[2] << 16) | (((uint8_t *)p)[3] << 24));

	*p = temp;
}

uint16_t
be16_to_cpup(uint16_t *p)
{
	return be16_to_cpu(*p);
}

uint16_t
cpu_to_be16p(uint16_t *p)
{
	return cpu_to_be16(*p);
}


uint16_t
le16_to_cpup(uint16_t *p)
{
	return le16_to_cpu(*p);
}

uint16_t
cpu_to_le16p(uint16_t *p)
{
	return cpu_to_le16(*p);
}

uint32_t
le32_to_cpup(uint32_t *p)
{
	return le32_to_cpu(*p);
}

uint32_t
cpu_to_le32p(uint32_t *p)
{
	return cpu_to_le32(*p);
}

uint32_t
be32_to_cpup(uint32_t *p)
{
	return be32_to_cpu(*p);
}

uint32_t
cpu_to_be32p(uint32_t *p)
{
	return cpu_to_be32(*p);
}

uint64_t
le64_to_cpup(uint64_t *p)
{
	return le64_to_cpu(*p);
}

uint64_t
cpu_to_le64p(uint64_t *p)
{
	return cpu_to_le64(*p);
}

uint64_t
be64_to_cpup(uint64_t *p)
{
	return be64_to_cpu(*p);
}

uint64_t
cpu_to_be64p(uint64_t *p)
{
	return cpu_to_be64(*p);
}

void
put_unaligned_le64(uint64_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[0] = val;
	val >>= 8;
	ptr[1] = val;
	val >>= 8;
	ptr[2] = val;
	val >>= 8;
	ptr[3] = val;
	val >>= 8;
	ptr[4] = val;
	val >>= 8;
	ptr[5] = val;
	val >>= 8;
	ptr[6] = val;
	val >>= 8;
	ptr[7] = val;
}

void
put_unaligned_be64(uint64_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[7] = val;
	val >>= 8;
	ptr[6] = val;
	val >>= 8;
	ptr[5] = val;
	val >>= 8;
	ptr[4] = val;
	val >>= 8;
	ptr[3] = val;
	val >>= 8;
	ptr[2] = val;
	val >>= 8;
	ptr[1] = val;
	val >>= 8;
	ptr[0] = val;
}

void
put_unaligned_le32(uint32_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[0] = val;
	val >>= 8;
	ptr[1] = val;
	val >>= 8;
	ptr[2] = val;
	val >>= 8;
	ptr[3] = val;
}

void
put_unaligned_be32(uint32_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[3] = val;
	val >>= 8;
	ptr[2] = val;
	val >>= 8;
	ptr[1] = val;
	val >>= 8;
	ptr[0] = val;
}

void
put_unaligned_be16(uint16_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[0] = (val >> 8);
	ptr[1] = val;
}

void
put_unaligned_le16(uint16_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[0] = val;
	val >>= 8;
	ptr[1] = val;
}

uint64_t
get_unaligned_le64(const void *_ptr)
{
	const uint8_t *ptr = _ptr;
	uint64_t val;

	val = ptr[7];
	val <<= 8;
	val |= ptr[6];
	val <<= 8;
	val |= ptr[5];
	val <<= 8;
	val |= ptr[4];
	val <<= 8;
	val |= ptr[3];
	val <<= 8;
	val |= ptr[2];
	val <<= 8;
	val |= ptr[1];
	val <<= 8;
	val |= ptr[0];
	return (val);
}

uint64_t
get_unaligned_be64(const void *_ptr)
{
	const uint8_t *ptr = _ptr;
	uint64_t val;

	val = ptr[0];
	val <<= 8;
	val |= ptr[1];
	val <<= 8;
	val |= ptr[2];
	val <<= 8;
	val |= ptr[3];
	val <<= 8;
	val |= ptr[4];
	val <<= 8;
	val |= ptr[5];
	val <<= 8;
	val |= ptr[6];
	val <<= 8;
	val |= ptr[7];
	return (val);
}

uint32_t
get_unaligned_le32(const void *_ptr)
{
	const uint8_t *ptr = _ptr;
	uint32_t val;

	val = ptr[3];
	val <<= 8;
	val |= ptr[2];
	val <<= 8;
	val |= ptr[1];
	val <<= 8;
	val |= ptr[0];
	return (val);
}

uint32_t
get_unaligned_be32(const void *_ptr)
{
	const uint8_t *ptr = _ptr;
	uint32_t val;

	val = ptr[0];
	val <<= 8;
	val |= ptr[1];
	val <<= 8;
	val |= ptr[2];
	val <<= 8;
	val |= ptr[3];
	return (val);
}

uint16_t
get_unaligned_be16(const void *_ptr)
{
	const uint8_t *ptr = _ptr;
	uint16_t val;

	val = ptr[0];
	val <<= 8;
	val |= ptr[1];
	return (val);
}

uint16_t
get_unaligned_le16(const void *_ptr)
{
	const uint8_t *ptr = _ptr;
	uint16_t val;

	val = ptr[1];
	val <<= 8;
	val |= ptr[0];
	return (val);
}

void   *
devm_kzalloc(struct device *dev, size_t size, gfp_t gfp)
{
	void *ptr;

	/*
	 * TODO: Register data so that it gets freed
	 * when the device is freed.
	 */
	ptr = malloc(size);
	if (ptr != NULL)
		memset(ptr, 0, size);
	return (ptr);
}

void   *
devm_kmalloc(struct device *dev, size_t size, gfp_t gfp)
{
	return (malloc(size));
}

void   *
devm_kmalloc_array(struct device *dev,
    size_t n, size_t size, gfp_t flags)
{
	size_t total = n * size;

	if (size != 0 && total / size != n)
		return (NULL);		/* overflow */
	return (malloc(total));
}

void
devm_kfree(struct device *dev, void *ptr)
{
	free(ptr);
}

void   *
dev_get_drvdata(const struct device *dev)
{
	return (dev->driver_data);
}

void
dev_set_drvdata(struct device *dev, void *data)
{
	dev->driver_data = data;
}

const char *
dev_name(const struct device *dev)
{
	if (dev == NULL)
		return ("NULL");
	return (dev->name);
}

int
atomic_add(int i, atomic_t *v)
{
	atomic_lock();
	v->counter += i;
	i = v->counter;
	atomic_unlock();

	return (i);
}

int
atomic_add_unless(atomic_t *v, int a, int u)
{
	int c;

	atomic_lock();
	c = v->counter;
	if (c != u)
		v->counter += a;
	atomic_unlock();

	return (c != u);
}

int
atomic_inc(atomic_t *v)
{
	int i;

	atomic_lock();
	v->counter++;
	i = v->counter;
	atomic_unlock();

	return (i);
}

int
atomic_dec(atomic_t *v)
{
	int i;

	atomic_lock();
	v->counter--;
	i = v->counter;
	atomic_unlock();

	return (i);
}

void
atomic_set(atomic_t *v, int i)
{
	atomic_lock();
	v->counter = i;
	atomic_unlock();
}

int
atomic_read(const atomic_t *v)
{
	int i;

	atomic_lock();
	i = v->counter;
	atomic_unlock();
	return (i);
}

int
atomic_dec_and_test(atomic_t *v)
{
	int i;

	atomic_lock();
	v->counter--;
	i = v->counter;
	atomic_unlock();

	return (i == 0);
}

int
atomic_cmpxchg(atomic_t *v, int old, int new)
{
	int prev;

	atomic_lock();
	prev = v->counter;
	if (prev == old)
		v->counter = new;
	atomic_unlock();
	return (prev);
}

int
test_bit(int nr, const void *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	int i;

	atomic_lock();
	i = (*p & mask) ? 1 : 0;
	atomic_unlock();

	return (i);
}

int
test_and_set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old;

	atomic_lock();
	old = *p;
	*p = old | mask;
	atomic_unlock();
	return (old & mask) != 0;
}

int
test_and_clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old;

	atomic_lock();
	old = *p;
	*p = old & ~mask;
	atomic_unlock();

	return (old & mask) != 0;
}

void
set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	atomic_lock();
	*p |= mask;
	atomic_unlock();
}

void
clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	atomic_lock();
	*p &= ~mask;
	atomic_unlock();
}

void
change_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	atomic_lock();
	*p ^= mask;
	atomic_unlock();
}

uint8_t
bitrev8(uint8_t a)
{
	a = ((a & 0x55) << 1) | ((a & 0xAA) >> 1);
	a = ((a & 0x33) << 2) | ((a & 0xCC) >> 2);
	a = ((a & 0x0F) << 4) | ((a & 0xF0) >> 4);
	return (a);
}

uint16_t
bitrev16(uint16_t a)
{
	a = ((a & 0x5555) << 1) | ((a & 0xAAAA) >> 1);
	a = ((a & 0x3333) << 2) | ((a & 0xCCCC) >> 2);
	a = ((a & 0x0F0F) << 4) | ((a & 0xF0F0) >> 4);
	a = ((a & 0x00FF) << 8) | ((a & 0xFF00) >> 8);
	return (a);
}

size_t
memweight(const void *ptr, size_t bytes)
{
	size_t x;
	size_t y;

	for (x = y = 0; x != bytes; x++) {
		y += hweight8(((uint8_t *)ptr)[x]);
	}
	return (y);
}

unsigned int
hweight8(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x55);

	res = (res & 0x33) + ((res >> 2) & 0x33);
	return (res + (res >> 4)) & 0x0F;
}

unsigned int
hweight16(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x5555);

	res = (res & 0x3333) + ((res >> 2) & 0x3333);
	res = (res + (res >> 4)) & 0x0F0F;
	return (res + (res >> 8)) & 0x00FF;
}

unsigned int
hweight32(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x55555555);

	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
	res = (res + (res >> 4)) & 0x0F0F0F0F;
	res = res + (res >> 8);
	return (res + (res >> 16)) & 0x000000FF;
}

unsigned long
hweight64(uint64_t w)
{
	if (sizeof(long) == 4) {
		return (hweight32((unsigned int)(w >> 32)) + hweight32((unsigned int)w));
	} else {
		uint64_t res = w - ((w >> 1) & 0x5555555555555555ul);

		res = (res & 0x3333333333333333ul) + ((res >> 2) & 0x3333333333333333ul);
		res = (res + (res >> 4)) & 0x0F0F0F0F0F0F0F0Ful;
		res = res + (res >> 8);
		res = res + (res >> 16);
		return (res + (res >> 32)) & 0x00000000000000FFul;
	}
}

struct cdev *
cdev_alloc(void)
{
	struct cdev *cdev;

	cdev = malloc(sizeof(*cdev));
	if (cdev == NULL)
		goto done;

	/* initialise cdev */
	cdev_init(cdev, NULL);

	cdev->is_alloced = 1;

done:
	return (cdev);
}

#define	ROCCAT_MAJOR 35
#define	LIRC_MAJOR 14
#define	EVDEV_MINOR_BASE 64
#define	JOYDEV_MINOR_BASE 0

#define	SUB_MAX (F_V4B_SUBDEV_MAX * F_V4B_SUBSUBDEV_MAX)

static struct cdev *cdev_registry[F_V4B_MAX][SUB_MAX];
static uint32_t cdev_mm[F_V4B_MAX][SUB_MAX];

static int dvb_swap_fe;

static TAILQ_HEAD(, bus_type) bus_type_head = TAILQ_HEAD_INITIALIZER(bus_type_head);
static TAILQ_HEAD(, device_driver) device_driver_head = TAILQ_HEAD_INITIALIZER(device_driver_head);

module_param_named(dvb_swap_fe, dvb_swap_fe, int, 0644);
MODULE_PARM_DESC(dvb_swap_fe, "swap default DVB frontend, 0..3");

static void
cdev_set_device(dev_t mm, struct cdev *cdev)
{
	uint8_t subdev;
	uint8_t id;

	switch (mm & 0xFFFF0000U) {
	case MKDEV(INPUT_MAJOR, 0):
		switch (mm & 0xFFC0) {
		case EVDEV_MINOR_BASE:
			subdev = mm & 0x3F;
			if (subdev >= F_V4B_SUBDEV_MAX)
				break;
			cdev_registry[F_V4B_EVDEV][subdev] = cdev;
			cdev_mm[F_V4B_EVDEV][subdev] = mm;
			break;

		case JOYDEV_MINOR_BASE:
			subdev = mm & 0x3F;
			if (subdev >= F_V4B_SUBDEV_MAX)
				break;
			cdev_registry[F_V4B_JOYDEV][subdev] = cdev;
			cdev_mm[F_V4B_JOYDEV][subdev] = mm;
			break;
		default:
			subdev = 0;
			goto error;
		}
		break;

	case MKDEV(LIRC_MAJOR, 0):
		subdev = mm & 0xFF;
		if (subdev >= F_V4B_SUBDEV_MAX)
			break;
		cdev_registry[F_V4B_LIRC][subdev] = cdev;
		cdev_mm[F_V4B_LIRC][subdev] = mm;
		break;

	case MKDEV(ROCCAT_MAJOR, 0):
		subdev = mm & 0xFF;
		if (subdev >= F_V4B_SUBDEV_MAX)
			break;
		cdev_registry[F_V4B_ROCCAT][subdev] = cdev;
		cdev_mm[F_V4B_ROCCAT][subdev] = mm;
		break;

	case MKDEV(VIDEO_MAJOR, 0):
		subdev = mm & 0xFF;
		if (subdev >= F_V4B_SUBDEV_MAX)
			goto error;
		cdev_registry[F_V4B_VIDEO][subdev] = cdev;
		cdev_mm[F_V4B_VIDEO][subdev] = mm;
		break;

	case MKDEV(DVB_MAJOR, 0):
		subdev = (mm >> 6) & 0x3FF;
		if (subdev >= F_V4B_SUBDEV_MAX)
			return;

		id = (mm >> 4) & 0x03;

		switch (mm & 0xFFFF000FU) {
		case MKDEV(DVB_MAJOR, DVB_DEVICE_FRONTEND):
			id = (id ^ dvb_swap_fe) & 0x03;
			break;
		default:
			break;
		}

		subdev += F_V4B_SUBDEV_MAX * id;

		switch (mm & 0xFFFF000FU) {
		case MKDEV(DVB_MAJOR, DVB_DEVICE_AUDIO):
			cdev_registry[F_V4B_DVB_AUDIO][subdev] = cdev;
			cdev_mm[F_V4B_DVB_AUDIO][subdev] = mm;
			break;
		case MKDEV(DVB_MAJOR, DVB_DEVICE_CA):
			cdev_registry[F_V4B_DVB_CA][subdev] = cdev;
			cdev_mm[F_V4B_DVB_CA][subdev] = mm;
			break;
		case MKDEV(DVB_MAJOR, DVB_DEVICE_DEMUX):
			cdev_registry[F_V4B_DVB_DEMUX][subdev] = cdev;
			cdev_mm[F_V4B_DVB_DEMUX][subdev] = mm;
			break;
		case MKDEV(DVB_MAJOR, DVB_DEVICE_DVR):
			cdev_registry[F_V4B_DVB_DVR][subdev] = cdev;
			cdev_mm[F_V4B_DVB_DVR][subdev] = mm;
			break;
		case MKDEV(DVB_MAJOR, DVB_DEVICE_FRONTEND):
			cdev_registry[F_V4B_DVB_FRONTEND][subdev] = cdev;
			cdev_mm[F_V4B_DVB_FRONTEND][subdev] = mm;
			break;
		case MKDEV(DVB_MAJOR, DVB_DEVICE_OSD):
			cdev_registry[F_V4B_DVB_OSD][subdev] = cdev;
			cdev_mm[F_V4B_DVB_OSD][subdev] = mm;
			break;
		case MKDEV(DVB_MAJOR, DVB_DEVICE_SEC):
			cdev_registry[F_V4B_DVB_SEC][subdev] = cdev;
			cdev_mm[F_V4B_DVB_SEC][subdev] = mm;
			break;
		case MKDEV(DVB_MAJOR, DVB_DEVICE_VIDEO):
			cdev_registry[F_V4B_DVB_VIDEO][subdev] = cdev;
			cdev_mm[F_V4B_DVB_VIDEO][subdev] = mm;
			break;
		default:
			break;		/* silently ignore */
		}
		break;
	default:
		subdev = 0;
		goto error;
	}
	return;

error:
	printf("Trying to register "
	    "unknown device(0x%08x) "
	    "or subdevice(%d) too big.\n",
	    mm, (int)subdev);
	return;
}

struct cdev *
cdev_get_device(unsigned int f_v4b)
{
	unsigned int subunit;

	subunit = f_v4b % SUB_MAX;

	f_v4b /= SUB_MAX;

	if (f_v4b >= F_V4B_MAX)
		return (NULL);		/* should not happen */

	return (cdev_registry[f_v4b][subunit]);
}

uint32_t
cdev_get_mm(unsigned int f_v4b)
{
	unsigned int subunit;

	subunit = f_v4b % SUB_MAX;

	f_v4b /= SUB_MAX;

	if (f_v4b >= F_V4B_MAX)
		return (0);		/* should not happen */

	return (cdev_mm[f_v4b][subunit]);
}

void
cdev_init(struct cdev *cdev, const struct file_operations *fops)
{
	memset(cdev, 0, sizeof(*cdev));

	cdev->ops = fops;
}

int
cdev_add(struct cdev *cdev, dev_t mm, unsigned count)
{
	cdev->mm_start = mm;
	cdev->mm_end = mm + count;

	while (mm != cdev->mm_end) {
		cdev_set_device(mm, cdev);
		mm++;
	}

	return (0);
}

int
register_chrdev(dev_t mm, const char *desc, const struct file_operations *fops)
{
	struct cdev *cdev;

	switch (mm) {
	case INPUT_MAJOR:
		cdev = cdev_alloc();
		if (cdev == NULL)
			goto error;
		cdev->ops = fops;
		cdev_add(cdev, MKDEV(mm, EVDEV_MINOR_BASE), 32);
		break;
	default:
		goto error;
	}
	return (0);

error:
	printf("Cannot register character "
	    "device mm=0x%08x and desc='%s'.\n", mm, desc);
	return (-1);
}

int
unregister_chrdev(dev_t mm, const char *desc)
{
	printf("Cannot unregister character "
	    "device mm=0x%08x and desc='%s'.\n", mm, desc);
	return (-1);
}

void
cdev_del(struct cdev *cdev)
{
	dev_t mm;

	if (cdev == NULL)
		return;

	mm = cdev->mm_start;

	while (mm != cdev->mm_end) {
		cdev_set_device(mm, NULL);
		mm++;
	}

	if (cdev->is_alloced)
		free(cdev);
}

void
kref_init(struct kref *kref)
{
	kref->refcount.counter = 0;
}

void
kref_get(struct kref *kref)
{
	atomic_inc(&kref->refcount);
}

int
kref_get_unless_zero(struct kref *kref)
{
	return (atomic_add_unless(&kref->refcount, 1, 0));
}

int
kref_put(struct kref *kref, void (*release) (struct kref *kref))
{
	if (atomic_dec(&kref->refcount) == 0) {
		release(kref);
		return 1;
	}
	return 0;
}

struct device *
get_device(struct device *dev)
{
	if (dev)
		kref_get(&dev->refcount);
	return (dev);
}

static void
dev_release(struct kref *kref)
{
#if 0
	struct device *dev =
	container_of(kref, struct device, refcount);

	/* TODO */

	free(dev);
#endif
}

void
put_device(struct device *dev)
{
	if (dev)
		kref_put(&dev->refcount, &dev_release);
}

int
device_move(struct device *dev, struct device *new_parent, int how)
{
	if (dev->parent != NULL)
		put_device(dev->parent);

	dev->parent = new_parent;

	if (dev->parent != NULL)
		get_device(dev->parent);

	return (0);
}

int
driver_register(struct device_driver *drv)
{
	TAILQ_INSERT_TAIL(&device_driver_head, drv, entry);
	return (0);
}

int
driver_unregister(struct device_driver *drv)
{
	if (drv->entry.tqe_prev == NULL)
		return (-EINVAL);
	TAILQ_REMOVE(&device_driver_head, drv, entry);
	drv->entry.tqe_prev = NULL;
	return (0);
}

int
bus_register(struct bus_type *bus)
{
	TAILQ_INSERT_TAIL(&bus_type_head, bus, entry);
	return (0);
}

int
bus_unregister(struct bus_type *bus)
{
	if (bus->entry.tqe_prev == NULL)
		return (-EINVAL);
	TAILQ_REMOVE(&bus_type_head, bus, entry);
	bus->entry.tqe_prev = NULL;
	return (0);
}

int
device_add(struct device *dev)
{
	struct device_driver *drv;

	if (dev->bus == NULL) {
		get_device(dev);
		return (0);
	}
	TAILQ_FOREACH(drv, &device_driver_head, entry) {
		if (drv->bus != dev->bus)
			continue;

		dev->driver = drv;

		if (dev->bus->match != NULL) {
			if (dev->bus->match(dev, drv) == 0)
				continue;
		}
		if (dev->bus->probe != NULL) {
			if (dev->bus->probe(dev))
				continue;
		}
		get_device(dev);
		return (0);
	}

	dev->driver = NULL;
	return (-ENXIO);
}

void
device_del(struct device *dev)
{
	if (dev->bus != NULL && dev->bus->remove != NULL) {
		dev->bus->remove(dev);
	}
	dev->driver = NULL;
	put_device(dev);
}

int
device_register(struct device *dev)
{
	return (device_add(dev));
}

void
device_unregister(struct device *dev)
{
	device_del(dev);
}

struct device *
device_create_vargs(struct class *class, struct device *parent,
    dev_t devt, void *drvdata, const char *fmt, va_list args)
{
	struct device *dev = NULL;
	int retval = -ENODEV;

	if (class == NULL || IS_ERR(class))
		goto error;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		retval = -ENOMEM;
		goto error;
	}
	dev->driver_static.name = "webcamd";

	/* set a default device name */
	if (class != NULL && class->name != NULL)
		snprintf(dev->name, sizeof(dev->name), "webcamd.%s", class->name);

	dev->devt = devt;
	dev->class = class;
	dev->parent = parent;
	dev->driver = &dev->driver_static;
	dev_set_drvdata(dev, drvdata);

	vsnprintf(dev->bus_id, BUS_ID_SIZE, fmt, args);
	retval = device_register(dev);
	if (retval)
		goto error;

	return dev;

error:
	put_device(dev);
	return ERR_PTR(retval);
}

struct device *
device_create(struct class *class, struct device *parent,
    dev_t devt, void *drvdata, const char *fmt,...)
{
	va_list vargs;
	struct device *dev;

	va_start(vargs, fmt);
	dev = device_create_vargs(class, parent, devt, drvdata, fmt, vargs);
	va_end(vargs);
	return dev;
}

void
device_destroy(struct class *class, dev_t devt)
{

}

void
module_put(struct module *module)
{

}

int
try_module_get(struct module *module)
{
	return (1);
}

void
module_get(struct module *module)
{

}

void   *
ERR_PTR(long error)
{
	return ((void *)error);
}

long
PTR_ERR(const void *ptr)
{
	return ((long)ptr);
}

long
IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

int
__ffs(int x)
{
	return (~(x - 1) & x);
}

int
__ffz(int x)
{
	return ((x + 1) & ~x);
}

int
fls(int mask)
{
	int bit;

	if (mask == 0)
		return (0);
	bit = 1;
	if (mask & 0xFFFF0000U) {
		bit += 16;
		mask = (unsigned int)mask >> 16;
	}
	if (mask & 0xFF00U) {
		bit += 8;
		mask = (unsigned int)mask >> 8;
	}
	if (mask & 0xF0U) {
		bit += 4;
		mask = (unsigned int)mask >> 4;
	}
	if (mask & 0xCU) {
		bit += 2;
		mask = (unsigned int)mask >> 2;
	}
	if (mask & 0x2U) {
		bit += 1;
		mask = (unsigned int)mask >> 1;
	}
	return (bit);
}

static unsigned long
__flsl(unsigned long mask)
{
	int bit;

	if (mask == 0)
		return (0);
	bit = 1;
#if (BITS_PER_LONG > 32)
	if (mask & 0xFFFFFFFF00000000UL) {
		bit += 32;
		mask = (unsigned long)mask >> 32;
	}
#endif
	if (mask & 0xFFFF0000UL) {
		bit += 16;
		mask = (unsigned long)mask >> 16;
	}
	if (mask & 0xFF00UL) {
		bit += 8;
		mask = (unsigned long)mask >> 8;
	}
	if (mask & 0xF0UL) {
		bit += 4;
		mask = (unsigned long)mask >> 4;
	}
	if (mask & 0xCUL) {
		bit += 2;
		mask = (unsigned long)mask >> 2;
	}
	if (mask & 0x2UL) {
		bit += 1;
		mask = (unsigned long)mask >> 1;
	}
	return (bit);
}

unsigned long
find_next_bit(const unsigned long *addr, unsigned long size,
    unsigned long offset)
{
	const unsigned long *p;

	while (offset < size) {

		p = addr + BIT_WORD(offset);

		if (*p == 0) {
			if (offset & (BITS_PER_LONG - 1))
				offset += (-offset) & (BITS_PER_LONG - 1);
			else
				offset += BITS_PER_LONG;
		} else {
			if (*p & (1 << (offset & (BITS_PER_LONG - 1))))
				break;
			offset++;
		}
	}
	if (offset > size)
		offset = size;
	return (offset);
}

unsigned long
find_next_zero_bit(const unsigned long *addr, unsigned long size,
    unsigned long offset)
{
	const unsigned long *p;

	while (offset < size) {

		p = addr + BIT_WORD(offset);

		if (*p == (long)-1) {
			if (offset & (BITS_PER_LONG - 1))
				offset += (-offset) & (BITS_PER_LONG - 1);
			else
				offset += BITS_PER_LONG;
		} else {
			if (!((*p) & (1 << (offset & (BITS_PER_LONG - 1)))))
				break;
			offset++;
		}
	}
	if (offset > size)
		offset = size;
	return (offset);
}

int
bitmap_weight(const unsigned long *src, unsigned nbits)
{
	unsigned x;
	unsigned y;
	for (x = y = 0; x != nbits; x++) {
		if (src[x / BITS_PER_LONG] & BIT_MASK(x))
			y++;
	}
	return (y);
}

int
bitmap_andnot(unsigned long *dst, const unsigned long *b1,
    const unsigned long *b2, int nbits)
{
	int len = (nbits + BITS_PER_LONG - 1) / BITS_PER_LONG;
	long retval = 0;
	long temp;
	int n;

	for (n = 0; n != len; n++) {
		temp = b1[n] & ~b2[n];
		dst[n] = temp;
		retval |= temp;
	}
	return (retval != 0);
}

int
bitmap_and(unsigned long *dst, const unsigned long *b1,
    const unsigned long *b2, int nbits)
{
	int len = (nbits + BITS_PER_LONG - 1) / BITS_PER_LONG;
	long retval = 0;
	long temp;
	int n;

	for (n = 0; n != len; n++) {
		temp = b1[n] & b2[n];
		dst[n] = temp;
		retval |= temp;
	}
	return (retval != 0);
}

void
bitmap_or(unsigned long *dst, const unsigned long *b1,
    const unsigned long *b2, int nbits)
{
	int len = (nbits + BITS_PER_LONG - 1) / BITS_PER_LONG;
	long temp;
	int n;

	for (n = 0; n != len; n++) {
		temp = b1[n] | b2[n];
		dst[n] = temp;
	}
}

void
bitmap_xor(unsigned long *dst, const unsigned long *b1,
    const unsigned long *b2, int nbits)
{
	int len = (nbits + BITS_PER_LONG - 1) / BITS_PER_LONG;
	long temp;
	int n;

	for (n = 0; n != len; n++) {
		temp = b1[n] | b2[n];
		dst[n] = temp;
	}
}

void
bitmap_zero(unsigned long *dst, int nbits)
{
	int len = (nbits + ((-nbits) & (BITS_PER_LONG - 1))) / 8;

	memset(dst, 0, len);
}

int
bitmap_subset(const unsigned long *pa, const unsigned long *pb, int nbits)
{
	int end = nbits / BITS_PER_LONG;
	int x;

	for (x = 0; x != end; x++) {
		if (pa[x] & ~pb[x])
			return (0);
	}

	x = nbits % BITS_PER_LONG;
	if (x) {
		if (pa[end] & ~pb[end] & ((1ULL << x) - 1ULL))
			return (0);
	}
	return (1);
}

int
bitmap_full(const unsigned long *bitmap, int bits)
{
	int k;
	int lim = bits / BITS_PER_LONG;

	for (k = 0; k < lim; ++k)
		if (~bitmap[k])
			return (0);

	lim = bits % BITS_PER_LONG;
	if (lim) {
		if ((~bitmap[k]) & ((1ULL << lim) - 1ULL))
			return (0);
	}
	return (1);
}

void
bitmap_clear(unsigned long *map, int start, int nr)
{
	unsigned long *p = map + (start / BITS_PER_LONG);
	int size = start + nr;
	int rem = start % BITS_PER_LONG;
	int bits_to_clear = BITS_PER_LONG - rem;
	unsigned long mask_to_clear = ((1ULL << rem) - 1ULL);

	while (nr - bits_to_clear >= 0) {
		*p &= ~mask_to_clear;
		nr -= bits_to_clear;
		bits_to_clear = BITS_PER_LONG;
		mask_to_clear = ~0UL;
		p++;
	}
	if (nr) {
		size = size % BITS_PER_LONG;
		mask_to_clear &= ((1ULL << size) - 1ULL);
		*p &= ~mask_to_clear;
	}
}

void
bitmap_shift_right(unsigned long *dst, const unsigned long *src, int n, int nbits)
{
	int x;
	int y;

	for (x = 0; x < (nbits - n); x++) {
		y = x + n;
		if (src[y / BITS_PER_LONG] & BIT_MASK(y))
			dst[x / BITS_PER_LONG] |= BIT_MASK(x);
		else
			dst[x / BITS_PER_LONG] &= ~BIT_MASK(x);
	}
	for (; x < nbits; x++)
		dst[x / BITS_PER_LONG] &= ~BIT_MASK(x);
}

void
bitmap_shift_left(unsigned long *dst, const unsigned long *src, int n, int nbits)
{
	int x;
	int y;

	for (x = 0; x != n; x++)
		dst[x / BITS_PER_LONG] &= ~BIT_MASK(x);

	for (; x < nbits; x++) {
		y = x - n;
		if (src[y / BITS_PER_LONG] & BIT_MASK(y))
			dst[x / BITS_PER_LONG] |= BIT_MASK(x);
		else
			dst[x / BITS_PER_LONG] &= ~BIT_MASK(x);
	}
}

int
bitmap_equal(const unsigned long *pa,
    const unsigned long *pb, unsigned bits)
{
	unsigned k;
	unsigned lim = bits / BITS_PER_LONG;

	for (k = 0; k != lim; k++)
		if (pa[k] != pb[k])
			return (0);

	bits %= BITS_PER_LONG;
	for (lim = 0; lim != bits; lim++) {
		if ((pa[k] ^ pb[k]) & BIT_MASK(lim))
			return (0);
	}
	return (1);
}

/*
 * A fast, small, non-recursive O(nlog n) sort for the Linux kernel
 *
 * Jan 23 2005  Matt Mackall <mpm@selenic.com>
 */

static void
u32_swap(void *a, void *b, int size)
{
	u32 t = *(u32 *) a;

	*(u32 *) a = *(u32 *) b;
	*(u32 *) b = t;
}

static void
generic_swap(void *a, void *b, int size)
{
	char t;

	do {
		t = *(char *)a;
		*(char *)a++ = *(char *)b;
		*(char *)b++ = t;
	} while (--size > 0);
}

void
sort(void *base, size_t num, size_t size,
    int (*cmp) (const void *, const void *),
    void (*swap_fn) (void *, void *, int size))
{
	/* pre-scale counters for performance */
	int i = (num / 2 - 1) * size, n = num * size, c, r;

	if (!swap_fn)
		swap_fn = (size == 4 ? u32_swap : generic_swap);

	/* heapify */
	for (; i >= 0; i -= size) {
		for (r = i; r * 2 + size < n; r = c) {
			c = r * 2 + size;
			if (c < n - size && cmp(base + c, base + c + size) < 0)
				c += size;
			if (cmp(base + r, base + c) >= 0)
				break;
			swap_fn(base + r, base + c, size);
		}
	}

	/* sort */
	for (i = n - size; i > 0; i -= size) {
		swap_fn(base, base + i, size);
		for (r = 0; r * 2 + size < i; r = c) {
			c = r * 2 + size;
			if (c < i - size && cmp(base + c, base + c + size) < 0)
				c += size;
			if (cmp(base + r, base + c) >= 0)
				break;
			swap_fn(base + r, base + c, size);
		}
	}
}

/* standard CRC computation */

u32
crc32_le(u32 crc, unsigned char const *p, size_t len)
{
	uint8_t i;

	while (len--) {
		crc ^= *p++;
		for (i = 0; i != 8; i++)
			crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LE : 0);
	}
	return crc;
}

u32
crc32_be(u32 crc, unsigned char const *p, size_t len)
{
	uint8_t i;

	while (len--) {
		crc ^= *p++ << 24;
		for (i = 0; i != 8; i++)
			crc =
			    (crc << 1) ^ ((crc & 0x80000000) ? CRCPOLY_BE : 0);
	}
	return crc;
}

void   *
kcalloc(size_t n, size_t size, int flags)
{
	void *ptr;

	ptr = malloc(size * n);
	if (ptr != NULL)
		memset(ptr, 0, size * n);

	return (ptr);
}

void   *
vmalloc(size_t size)
{
	return (malloc_vm(size));
}

void   *
vzalloc(size_t size)
{
	void *ptr = malloc_vm(size);

	if (ptr != NULL)
		memset(ptr, 0, size);

	return (ptr);
}

long
__get_free_page(int flags)
{
	return ((long)malloc(PAGE_SIZE));
}

void
free_page(long ptr)
{
	free((void *)ptr);
}

struct class *
class_get(struct class *class)
{
	if (class)
		kref_get(&class->refcount);
	return (class);
}

static void
class_release(struct kref *kref)
{
	/* TODO */

}

struct class *
class_put(struct class *class)
{
	if (class)
		kref_put(&class->refcount, class_release);
	return (class);
}

int
class_register(struct class *class)
{
	return (0);
}

void
class_unregister(struct class *class)
{

}

void
class_destroy(struct class *class)
{
	if ((class == NULL) || (IS_ERR(class)))
		return;
	class_unregister(class);
}

int
alloc_chrdev_region(dev_t *pdev, unsigned basemin, unsigned count, const char *name)
{
	if (strcmp(name, "BaseRemoteCtl") == 0) {
		*pdev = MKDEV(LIRC_MAJOR, basemin);
		return (0);
	} else if (strcmp(name, "roccat") == 0) {
		*pdev = MKDEV(ROCCAT_MAJOR, basemin);
		return (0);
	}
	printf("alloc_chrdev_region: Unknown region name: '%s'\n", name);
	return (-ENOMEM);
}

int
register_chrdev_region(dev_t from, unsigned count, const char *name)
{
	return (0);
}

void
unregister_chrdev_region(dev_t from, unsigned count)
{
	return;
}

int
remap_pfn_range(struct vm_area_struct *vma, unsigned long start,
    unsigned long page, unsigned long size, int prot)
{
	/* assuming that pages are virtually contiguous */
	if (start == vma->vm_start)
		vma->vm_buffer_address = (void *)page;

	return (0);
}

int
vm_insert_page(struct vm_area_struct *vma,
    unsigned long start, struct page *page)
{
	/* assuming that pages are virtually contiguous */
	if (start == vma->vm_start)
		vma->vm_buffer_address = (void *)page;

	return (0);
}

int
remap_vmalloc_range(struct vm_area_struct *vma,
    void *addr, unsigned long pgoff)
{
	addr = (uint8_t *)addr + (pgoff << PAGE_SHIFT);
	vma->vm_buffer_address = addr;

	return (0);
}

void
jiffies_to_timeval(uint64_t j, struct timeval *tv)
{
	tv->tv_usec = ((j % 1000ULL) * 1000ULL);
	tv->tv_sec = j / 1000ULL;
}

uint64_t
round_jiffies_relative(uint64_t j)
{
	return (j);
}

int
do_gettimeofday(struct timeval *tp)
{
	return (gettimeofday(tp, NULL));
}

void
poll_initwait(struct poll_wqueues *pwq)
{
	memset(pwq, 0, sizeof(*pwq));
}

void
poll_freewait(struct poll_wqueues *pwq)
{

}

void
poll_schedule(struct poll_wqueues *pwq, int flag)
{
	schedule();
}

int32_t
div_round_closest_s32(int32_t rem, int32_t div)
{
	return ((rem + (div / 2)) / div);
}

uint32_t
div_round_closest_u32(uint32_t rem, uint32_t div)
{
	return ((rem + (div / 2)) / div);
}

int64_t
div_round_closest_s64(int64_t rem, int64_t div)
{
	return ((rem + (div / 2)) / div);
}

uint64_t
div_round_closest_u64(uint64_t rem, uint64_t div)
{
	return ((rem + (div / 2)) / div);
}

struct timespec
ktime_mono_to_real(struct timespec arg)
{
	return (ktime_add(arg, ktime_mono_to_real_offset));
}

struct timespec
ktime_get_boottime(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_UPTIME_FAST, &ts);

	return (ts);
}

struct timespec
ktime_get_real(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME_FAST, &ts);

	return (ts);
}

struct timespec
ktime_get(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME_FAST, &ts);

	return (ts);
}

struct timespec
ktime_mono_to_any(struct timespec arg, int off)
{
	switch (off) {
	case TK_OFFS_REAL:
		return (ktime_add(arg, ktime_mono_to_real_offset));
	case TK_OFFS_BOOT:
		return (ktime_add(arg, ktime_mono_to_uptime_offset));
	default:
		printf("Unknown clock conversion\n");
		return (arg);
	}
}

struct timeval
ktime_to_timeval(const struct timespec ts)
{
	struct timeval tv;

	tv.tv_sec = ts.tv_sec;
	tv.tv_usec = ts.tv_nsec / 1000;

	return (tv);
}

void
ktime_get_ts(struct timespec *ts)
{
	clock_gettime(CLOCK_MONOTONIC_FAST, ts);
}

void
ktime_get_real_ts(struct timespec *ts)
{
	clock_gettime(CLOCK_REALTIME_FAST, ts);
}

int64_t
ktime_to_ns(const struct timespec ts)
{
	return ((((int64_t)ts.tv_sec) *
	    (int64_t)1000000000L) + (int64_t)ts.tv_nsec);
}

struct timespec
ktime_sub(const struct timespec a, const struct timespec b)
{
	struct timespec r;

	/* do subtraction */
	r.tv_sec = a.tv_sec - b.tv_sec;
	r.tv_nsec = a.tv_nsec - b.tv_nsec;

	/* carry */
	if (r.tv_nsec < 0) {
		r.tv_nsec += 1000000000LL;
		r.tv_sec--;
	}
	return (r);
}

struct timespec
ktime_add(const struct timespec a, const struct timespec b)
{
	struct timespec r;

	/* do subtraction */
	r.tv_sec = a.tv_sec + b.tv_sec;
	r.tv_nsec = a.tv_nsec + b.tv_nsec;

	/* carry */
	if (r.tv_nsec >= 1000000000LL) {
		r.tv_nsec -= 1000000000LL;
		r.tv_sec++;
	}
	return (r);
}

static int
ktime_monotonic_offset_init(void)
{
	struct timespec ta;
	struct timespec tb;
	struct timespec tc;

	clock_gettime(CLOCK_MONOTONIC, &ta);
	clock_gettime(CLOCK_REALTIME, &tb);
	clock_gettime(CLOCK_UPTIME, &tc);

	ktime_mono_to_real_offset = ktime_sub(tb, ta);
	ktime_mono_to_uptime_offset = ktime_sub(tc, ta);

	return (0);
}

module_init(ktime_monotonic_offset_init);

struct timespec
ktime_get_monotonic_offset(void)
{
	return (ktime_mono_to_real_offset);
}

struct timespec
ktime_add_us(const struct timespec t, const uint64_t usec)
{
	struct timespec temp;

	temp.tv_nsec = 1000 * (usec % 1000000ULL);
	temp.tv_sec = usec / 1000000ULL;

	return (timespec_add(t, temp));
}

int64_t
ktime_us_delta(const struct timespec last, const struct timespec first)
{
	return (ktime_to_us(ktime_sub(last, first)));
}

int64_t
ktime_to_us(const struct timespec t)
{
	struct timeval tv = ktime_to_timeval(t);

	return (((int64_t)tv.tv_sec * 1000000LL) + tv.tv_usec);
}

struct timespec
current_kernel_time(void)
{
	struct timespec ts;

	ktime_get_real_ts(&ts);
	return (ts);
}

int64_t
timespec_to_ns(const struct timespec *ts)
{
	return ((ts->tv_sec * 1000000000L) + ts->tv_nsec);
}

struct timespec
timespec_add(struct timespec vvp, struct timespec uvp)
{
	vvp.tv_sec += uvp.tv_sec;
	vvp.tv_nsec += uvp.tv_nsec;
	if (vvp.tv_nsec >= 1000000000L) {
		vvp.tv_sec++;
		vvp.tv_nsec -= 1000000000L;
	}
	return (vvp);
}

struct timespec
timespec_sub(struct timespec vvp, struct timespec uvp)
{
	vvp.tv_sec -= uvp.tv_sec;
	vvp.tv_nsec -= uvp.tv_nsec;
	if (vvp.tv_nsec < 0) {
		vvp.tv_sec--;
		vvp.tv_nsec += 1000000000L;
	}
	return (vvp);
}

void
msleep(uint32_t ms)
{
	uint32_t drops;

	atomic_lock();
	drops = atomic_drop();
	atomic_unlock();

	usleep(ms * 1000);

	atomic_lock();
	atomic_pickup(drops);
	atomic_unlock();
}

void
ssleep(uint32_t s)
{
	msleep(s * 1000);
}

uint32_t
msleep_interruptible(uint32_t ms)
{
	msleep(ms);
	return (0);
}

void
request_module(const char *ptr,...)
{
}

int
device_can_wakeup(struct device *dev)
{
	return (-EINVAL);
}

void
device_init_wakeup(struct device *dev, int flags)
{
}

void
device_initialize(struct device *dev)
{
}

int
dmi_check_system(const struct dmi_system_id *list)
{
	return (0);
}

unsigned long
clear_user(void *to, unsigned long size)
{
	static uint8_t buf[256];

	uint8_t *ptr = to;

	while (size > sizeof(buf)) {
		if (copy_to_user(ptr, buf, sizeof(buf)))
			return (size);

		ptr += sizeof(buf);
		size -= sizeof(buf);
	}

	if (size > 0)
		return (copy_to_user(ptr, buf, size));

	return (0);
}

void
swab16s(uint16_t *ptr)
{
	*ptr = bswap16(*ptr);
}

uint16_t
swab16(uint16_t temp)
{
	return (bswap16(temp));
}

void
swab32s(uint32_t *ptr)
{
	*ptr = bswap32(*ptr);
}

uint32_t
swab32(uint32_t temp)
{
	return (bswap32(temp));
}

int
scnprintf(char *buf, size_t size, const char *fmt,...)
{
	va_list args;
	int retval;

	va_start(args, fmt);
	retval = vsnprintf(buf, size, fmt, args);
	va_end(args);

	return ((retval >= size) ? (size - 1) : retval);
}

#undef do_div

uint32_t
do_div(uint64_t *rem, uint32_t div)
{
	uint64_t val = *rem;

	*rem = val / div;

	return (val % div);
}

int
sysfs_create_group(struct kobject *kobj,
    const struct attribute_group *grp)
{
	return (0);
}

void
sysfs_remove_group(struct kobject *kobj,
    const struct attribute_group *grp)
{
}

int
sysfs_create_bin_file(struct kobject *kobj, struct bin_attribute *attr)
{
	return (0);
}

int
sysfs_remove_bin_file(struct kobject *kobj, struct bin_attribute *attr)
{
	return (0);
}

void   *
pci_zalloc_consistent(struct pci_dev *hwdev, size_t size,
    dma_addr_t *dma_addr)
{
	return (pci_alloc_consistent(hwdev, size, dma_addr));
}

void   *
pci_alloc_consistent(struct pci_dev *hwdev, size_t size,
    dma_addr_t *dma_addr)
{
	void *ptr;

	if (dma_addr)
		*dma_addr = 0;
	ptr = malloc(size);
	if (ptr)
		memset(ptr, 0, size);
	return (ptr);
}

void
pci_free_consistent(struct pci_dev *hwdev, size_t size,
    void *vaddr, dma_addr_t dma_handle)
{
	free(vaddr);
}

int
add_uevent_var(struct kobj_uevent_env *env, const char *format,...)
{
	return (0);
}

struct class *
class_create(struct module *owner, const char *name)
{
	struct class *class;

	class = malloc(sizeof(*class));

	if (class == NULL)
		return (NULL);

	memset(class, 0, sizeof(*class));

	class->name = name;

	return (class);
}

int
usb_register_dev(struct usb_interface *iface, struct usb_class_driver *info)
{
	return (0);
}

void
usb_deregister_dev(struct usb_interface *iface, struct usb_class_driver *info)
{

}

struct usb_interface *
usb_find_interface(struct usb_driver *drv, int minor)
{
	return (NULL);			/* not supported */
}

void   *
kmemdup(const void *src, size_t len, gfp_t gfp)
{
	void *p;

	p = malloc(len);
	if (p)
		memcpy(p, src, len);

	return (p);
}

void   *
memdup_user(const void *src, size_t len)
{
	void *p;

	p = malloc(len);
	if (p == NULL)
		return (ERR_PTR(-ENOMEM));

	if (copy_from_user(p, src, len)) {
		free(p);
		return ERR_PTR(-EFAULT);
	}
	return (p);
}

unsigned long
rounddown_pow_of_two(unsigned long x)
{
	if (x == 0)
		return (0);
	else
		return (1UL << (__flsl(x) - 1));
}

unsigned long
roundup_pow_of_two(unsigned long x)
{
	if (x == 0)
		return (0);
	else
		return (1UL << __flsl((x) - 1));
}

const char *
skip_spaces(const char *str)
{
	while (isspace(*str))
		str++;
	return ((const char *)str);
}

uint64_t
div64_u64(uint64_t rem, uint64_t div)
{
	return (rem / div);
}

int64_t
div64_s64(int64_t rem, int64_t div)
{
	return (rem / div);
}

int64_t
div_s64(int64_t rem, int32_t div)
{
	return (rem / (int64_t)div);
}

uint64_t
div_u64(uint64_t rem, uint32_t div)
{
	return (rem / (uint64_t)div);
}

uint64_t
div_u64_rem(uint64_t rem, uint32_t div, uint32_t *prem)
{
	*prem = rem % (uint64_t)div;
	return (rem / (uint64_t)div);
}

int
nonseekable_open(struct inode *inode, struct file *file)
{
	return (0);
}

int
kobject_set_name(struct kobject *kobj, const char *fmt,...)
{
	return (0);
}

int
zero_nop(void)
{
	return (0);
}

int
kstrtos16(const char *nptr, unsigned int base, int16_t *res)
{
	long long temp;
	char *pp = NULL;

	*res = 0;

	if (base < 2 || base > 35)
		return (-EINVAL);
	temp = strtoll(nptr, &pp, base);
	if (pp && pp[0])
		return (-EINVAL);
	if (temp != (long long)(int16_t)temp)
		return (-ERANGE);

	*res = temp;
	return (0);
}

int
kstrtou16(const char *nptr, unsigned int base, uint16_t *res)
{
	unsigned long long temp;
	char *pp = NULL;

	*res = 0;

	if (base < 2 || base > 35)
		return (-EINVAL);
	temp = strtoull(nptr, &pp, base);
	if (pp && pp[0])
		return (-EINVAL);
	if (temp != (unsigned long long)(uint16_t)temp)
		return (-ERANGE);

	*res = temp;
	return (0);
}

int
kstrtos8(const char *nptr, unsigned int base, int8_t *res)
{
	long long temp;
	char *pp = NULL;

	*res = 0;

	if (base < 2 || base > 35)
		return (-EINVAL);
	temp = strtoll(nptr, &pp, base);
	if (pp && pp[0])
		return (-EINVAL);
	if (temp != (long long)(int8_t)temp)
		return (-ERANGE);

	*res = temp;
	return (0);
}

int
kstrtou8(const char *nptr, unsigned int base, uint8_t *res)
{
	unsigned long long temp;
	char *pp = NULL;

	*res = 0;

	if (base < 2 || base > 35)
		return (-EINVAL);
	temp = strtoull(nptr, &pp, base);
	if (pp && pp[0])
		return (-EINVAL);
	if (temp != (unsigned long long)(uint8_t)temp)
		return (-ERANGE);

	*res = temp;
	return (0);
}

int
kstrtouint(const char *nptr, unsigned int base, unsigned int *res)
{
	unsigned long long temp;
	char *pp = NULL;

	*res = 0;

	if (base < 2 || base > 35)
		return (-EINVAL);
	temp = strtoull(nptr, &pp, base);
	if (pp && pp[0])
		return (-EINVAL);
	if (temp != (unsigned long long)(unsigned int)temp)
		return (-ERANGE);

	*res = temp;
	return (0);
}

int
kstrtoint(const char *nptr, unsigned int base, int *res)
{
	long long temp;
	char *pp = NULL;

	*res = 0;

	if (base < 2 || base > 35)
		return (-EINVAL);
	temp = strtoll(nptr, &pp, base);
	if (pp && pp[0])
		return (-EINVAL);
	if (temp != (long long)(int)temp)
		return (-ERANGE);

	*res = temp;
	return (0);
}

int
kstrtoul(const char *nptr, unsigned int base, unsigned long *res)
{
	long long temp;
	char *pp = NULL;

	*res = 0;

	if (base < 2 || base > 35)
		return (-EINVAL);
	temp = strtoul(nptr, &pp, base);
	if (pp && pp[0])
		return (-EINVAL);
	*res = temp;
	return (0);
}

/* The following function was copied from the Linux Kernel sources, fs/libfs.c */

ssize_t
simple_read_from_buffer(void __user * to, size_t count, loff_t *ppos,
    const void *from, size_t available)
{
	loff_t pos = *ppos;
	size_t ret;

	if (pos < 0)
		return (-EINVAL);
	if (pos >= available || count == 0)
		return (0);
	if (count > available - pos)
		count = available - pos;
	ret = copy_to_user(to, from + pos, count);
	if (ret == count)
		return (-EFAULT);
	count -= ret;
	*ppos = pos + count;
	return (count);
}

/* The following function was copied from the Linux Kernel sources, fs/libfs.c */

ssize_t
simple_write_to_buffer(void *to, size_t available,
    loff_t *ppos, const void __user * from, size_t count)
{
	loff_t pos = *ppos;
	size_t ret;

	if (pos < 0)
		return (-EINVAL);
	if (pos >= available || count == 0)
		return (0);
	if (count > available - pos)
		count = available - pos;
	ret = copy_from_user(to + pos, from, count);
	if (ret == count)
		return (-EFAULT);
	count -= ret;
	*ppos = pos + count;
	return (count);
}

struct power_supply *
power_supply_register(struct device *parent,
    const struct power_supply_desc *desc,
    const struct power_supply_config *cfg)
{
	return (NULL);
}

void
power_supply_unregister(struct power_supply *psy)
{

}

int
power_supply_powers(struct power_supply *psy, struct device *dev)
{
	return (0);
}

void
power_supply_changed(struct power_supply *psy)
{
}

void *
power_supply_get_drvdata(struct power_supply *psy)
{
	return (NULL);
}

int
led_classdev_register(struct device *parent, struct led_classdev *led_cdev)
{
	return (0);
}

void
led_classdev_unregister(struct led_classdev *led_cdev)
{

}

void
led_classdev_suspend(struct led_classdev *led_cdev)
{

}

void
led_classdev_resume(struct led_classdev *led_cdev)
{

}

/* "int_sqrt" was copied from HPS's libmbin */

uint64_t
int_sqrt(uint64_t a)
{
	uint64_t b = 0x4000000000000000ULL;

	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00000ULL;
	} else {
		b >>= 1;
		b ^= 0xc00000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700000ULL;
	} else {
		b >>= 1;
		b ^= 0x300000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0000ULL;
	} else {
		b >>= 1;
		b ^= 0xc0000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70000ULL;
	} else {
		b >>= 1;
		b ^= 0x30000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c000ULL;
	} else {
		b >>= 1;
		b ^= 0xc000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7000ULL;
	} else {
		b >>= 1;
		b ^= 0x3000ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c00ULL;
	} else {
		b >>= 1;
		b ^= 0xc00ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x700ULL;
	} else {
		b >>= 1;
		b ^= 0x300ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1c0ULL;
	} else {
		b >>= 1;
		b ^= 0xc0ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x70ULL;
	} else {
		b >>= 1;
		b ^= 0x30ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1cULL;
	} else {
		b >>= 1;
		b ^= 0xcULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x7ULL;
	} else {
		b >>= 1;
		b ^= 0x3ULL;
	}
	if (a >= b) {
		a -= b;
		b >>= 1;
		b ^= 0x1ULL;
	} else {
		b >>= 1;
	}
	return (b);
}

void   *
devres_alloc(dr_release_t release, size_t size, gfp_t gfp)
{
	void *ptr;

	ptr = malloc(size);
	if (ptr != NULL)
		memset(ptr, 0, size);
	return (ptr);
}

void
devres_free(void *res)
{
	free(res);
}

void
devres_add(struct device *dev, void *res)
{
	/* NOP */
}

int
devres_destroy(struct device *dev, dr_release_t release,
    dr_match_t match, void *match_data)
{
	printf("TODO: Implement devres_destroy()\n");
	return (0);
}

int
dma_buf_fd(struct dma_buf *dmabuf, int flags)
{
	return (-1);
}

struct dma_buf *
dma_buf_get(int fd)
{
	return (NULL);
}

void
dma_buf_put(struct dma_buf *dmabuf)
{

}

void   *
dma_buf_vmap(struct dma_buf *buf)
{
	return (NULL);
}

void
dma_buf_vunmap(struct dma_buf *buf, void *vaddr)
{

}

uint32_t
ror32(uint32_t x, uint8_t n)
{
	n &= 0x1f;
	if (n == 0)
		return (x);
	return ((x >> n) | (x << (32 - n)));
}

unsigned long
gcd(unsigned long a, unsigned long b)
{
	unsigned long r;

	if (a < b)
		swap(a, b);
	if (!b)
		return (a);
	while ((r = (a % b)) != 0) {
		a = b;
		b = r;
	}
	return (b);
}

void
get_random_bytes(void *buf, int nbytes)
{
	while (nbytes--)
		*((char *)buf + nbytes) = rand();
}

const char *
dev_driver_string(const struct device *dev)
{
	struct device_driver *drv;

	drv = dev->driver;
	return (drv ? drv->name :
	    (dev->bus ? dev->bus->name :
	    (dev->class ? dev->class->name : "")));
}

s32
sign_extend32(u32 value, int index)
{
	u8 shift = 31 - index;

	return ((s32) (value << shift) >> shift);
}

char   *
devm_kasprintf(struct device *dev, gfp_t gfp, const char *fmt,...)
{
	va_list ap;
	char *ptr = NULL;

	va_start(ap, fmt);
	vasprintf(&ptr, fmt, ap);
	va_end(ap);
	return (ptr);
}
