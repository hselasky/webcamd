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

#include <sched.h>

int
printk_nop()
{
	return (1);
}

uint16_t
le16_to_cpu(uint16_t x)
{
	uint8_t *p = (uint8_t *)&x;

	return (p[0] | (p[1] << 8));
}

uint16_t
be16_to_cpu(uint16_t x)
{
	uint8_t *p = (uint8_t *)&x;

	return (p[1] | (p[0] << 8));
}

uint32_t
le32_to_cpu(uint32_t x)
{
	uint8_t *p = (uint8_t *)&x;

	return (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}

uint32_t
be32_to_cpu(uint32_t x)
{
	uint8_t *p = (uint8_t *)&x;

	return (p[3] | (p[2] << 8) | (p[1] << 16) | (p[0] << 24));
}

uint16_t
cpu_to_le16(uint16_t x)
{
	uint8_t p[2];

	p[0] = x & 0xFF;
	p[1] = x >> 8;

	return (*(uint16_t *)p);
}

uint16_t
cpu_to_be16(uint16_t x)
{
	uint8_t p[2];

	p[1] = x & 0xFF;
	p[0] = x >> 8;

	return (*(uint16_t *)p);
}

uint32_t
cpu_to_le32(uint32_t x)
{
	uint8_t p[4];

	p[0] = x;
	p[1] = x >> 8;
	p[2] = x >> 16;
	p[3] = x >> 24;

	return (*(uint32_t *)p);
}

uint32_t
cpu_to_be32(uint32_t x)
{
	uint8_t p[4];

	p[3] = x;
	p[2] = x >> 8;
	p[1] = x >> 16;
	p[0] = x >> 24;

	return (*(uint32_t *)p);
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
put_unaligned_le16(uint16_t val, void *_ptr)
{
	uint8_t *ptr = _ptr;

	ptr[0] = val & 0xFF;
	val >>= 8;
	ptr[1] = val & 0xFF;
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
dev_get_drvdata(struct device *dev)
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
	return (dev->name);
}

int
remap_pfn_range(struct vm_area_struct *vma, unsigned long addr,
    unsigned long pfn, unsigned long size, pgprot_t pgr)
{
	return -EINVAL;
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

unsigned int
hweight8(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x55);

	res = (res & 0x33) + ((res >> 2) & 0x33);
	return (res + (res >> 4)) & 0x0F;
}

#ifndef HAVE_WEBCAMD
unsigned long
copy_to_user(void *to, const void *from, unsigned long n)
{
	memcpy(to, from, n);
	return (0);
}

unsigned long
copy_from_user(void *to, const void *from, unsigned long n)
{
	memcpy(to, from, n);
	return (0);
}

#endif

struct cdev *
cdev_alloc(void)
{
	struct cdev *cdev;

	cdev = malloc(sizeof(*cdev));
	if (cdev == NULL)
		goto done;

	/* initialise cdev */
	memset(cdev, 0, sizeof(*cdev));

	cdev->fixed_dentry.d_inode = &cdev->fixed_inode;
	cdev->fixed_file.f_path.dentry = &cdev->fixed_dentry;

done:
	return (cdev);
}

int
cdev_add(struct cdev *cdev, dev_t mm, unsigned count)
{
	cdev->fixed_inode.d_inode = mm;
	cdev->fixed_file.f_op = cdev->ops;

	/* XXX hack */

	usb_linux_set_cdev(cdev);

	return (0);
}

void
cdev_del(struct cdev *cdev)
{
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
	printf("KrefGet: %p = %u\n", kref, kref->refcount.counter);
}

int
kref_put(struct kref *kref, void (*release) (struct kref *kref))
{
	printf("KrefPut: %p = %u\n", kref, kref->refcount.counter);

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
device_add(struct device *dev)
{
	/* TODO */
	printf("Added device %p\n", dev);
	get_device(dev);
	return (0);
}

void
device_del(struct device *dev)
{
	/* TODO */
	printf("Deleted device %p\n", dev);
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
	dev->devt = devt;
	dev->class = class;
	dev->parent = parent;
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
__fls(int x)
{
	return (fls(x));
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
vmalloc(size_t size)
{
	return (malloc(size));
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
dvb_net_release(struct dvb_net *dvbnet)
{
}

int
dvb_net_init(struct dvb_adapter *adap, struct dvb_net *dvbnet, struct dmx_demux *dmx)
{
	/* not supported */
	return 0;
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

void
pthread_set_kernel_prio(void)
{
	struct sched_param parm;

	memset(&parm, 0, sizeof(parm));

	parm.sched_priority = sched_get_priority_max(SCHED_FIFO);

	pthread_setschedparam(pthread_self(), SCHED_FIFO, &parm);
}

void
ktime_get_ts(struct timespec *ts)
{
	clock_gettime(CLOCK_MONOTONIC, ts);
}

void
ktime_get_real_ts(struct timespec *ts)
{
	clock_gettime(CLOCK_REALTIME, ts);
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
