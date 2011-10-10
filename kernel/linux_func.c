/*-
 * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.
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

/* NOTE: Some functions in this file derive directly from the Linux kernel sources. */

#include <media/v4l2-dev.h>

#include <linux/major.h>

#include <dvbdev.h>

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
put_unaligned_le32(uint32_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[0] = val & 0xFF;
	val >>= 8;
	ptr[1] = val & 0xFF;
	val >>= 8;
	ptr[2] = val & 0xFF;
	val >>= 8;
	ptr[3] = val & 0xFF;
}

void
put_unaligned_be16(uint16_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[0] = (val >> 8) & 0xFF;
	ptr[1] = val & 0xFF;
}

void
put_unaligned_le16(uint16_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[0] = val & 0xFF;
	val >>= 8;
	ptr[1] = val & 0xFF;
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
dev_name(struct device *dev)
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

#define	LIRC_MAJOR 14
#define	EVDEV_MINOR_BASE 64

static struct cdev *cdev_registry[F_V4B_MAX][F_V4B_SUBDEV_MAX];
static uint32_t cdev_mm[F_V4B_MAX][F_V4B_SUBDEV_MAX];

static void
cdev_set_device(dev_t mm, struct cdev *cdev)
{
	uint8_t subdev;
	uint8_t id;

	switch (mm & 0xFFFF0000U) {
	case MKDEV(INPUT_MAJOR, 0):
		switch (mm & 0xFFE0) {
		case EVDEV_MINOR_BASE:
			subdev = mm & 0x1F;
			if (subdev >= F_V4B_SUBDEV_MAX)
				break;
			cdev_registry[F_V4B_EVDEV][subdev] = cdev;
			cdev_mm[F_V4B_EVDEV][subdev] = mm;
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
		if (id != 0)
			return;

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

	subunit = f_v4b % F_V4B_SUBDEV_MAX;

	f_v4b /= F_V4B_SUBDEV_MAX;

	if (f_v4b >= F_V4B_MAX)
		return (NULL);		/* should not happen */

	return (cdev_registry[f_v4b][subunit]);
}

uint32_t
cdev_get_mm(unsigned int f_v4b)
{
	unsigned int subunit;

	subunit = f_v4b % F_V4B_SUBDEV_MAX;

	f_v4b /= F_V4B_SUBDEV_MAX;

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
device_add(struct device *dev)
{
	/* TODO */
	get_device(dev);
	return (0);
}

void
device_del(struct device *dev)
{
	/* TODO */
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
	put_device(dev);
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
		snprintf(dev->name, sizeof(dev->name), "webcamd%d.%s", webcamd_unit, class->name);

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

void
bitmap_zero(unsigned long *dst, int nbits)
{
	int len = (nbits + ((-nbits) & (BITS_PER_LONG - 1))) / 8;

	memset(dst, 0, len);
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
    void (*swap) (void *, void *, int size))
{
	/* pre-scale counters for performance */
	int i = (num / 2 - 1) * size, n = num * size, c, r;

	if (!swap)
		swap = (size == 4 ? u32_swap : generic_swap);

	/* heapify */
	for (; i >= 0; i -= size) {
		for (r = i; r * 2 + size < n; r = c) {
			c = r * 2 + size;
			if (c < n - size && cmp(base + c, base + c + size) < 0)
				c += size;
			if (cmp(base + r, base + c) >= 0)
				break;
			swap(base + r, base + c, size);
		}
	}

	/* sort */
	for (i = n - size; i > 0; i -= size) {
		swap(base, base + i, size);
		for (r = 0; r * 2 + size < i; r = c) {
			c = r * 2 + size;
			if (c < i - size && cmp(base + c, base + c + size) < 0)
				c += size;
			if (cmp(base + r, base + c) >= 0)
				break;
			swap(base + r, base + c, size);
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
ktime_get(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME_FAST, &ts);

	return (ts);
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

uint64_t
div_u64(uint64_t rem, uint32_t div)
{
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
