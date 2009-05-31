
static pthread_mutex_t atomic_mutex;

uint16_t
le16_to_cpu(uint16_t x)
{
	uint8_t *p = (uint8_t *)&x;

	return (p[0] | (p[1] << 8));
}

uint32_t
le32_to_cpu(uint32_t x)
{
	uint8_t *p = (uint8_t *)&x;

	return (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}

uint16_t
cpu_to_le16(uint16_t x)
{
	uint8_t p[2];

	p[0] = x & 0xFF;
	p[1] = x >> 8;

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
test_bit(int nr, const void *addr)
{
	const uint32_t *ptr = (const uint32_t *)addr;
	int i;

	atomic_lock();
	i = (((1UL << (nr & 31)) & (ptr[nr >> 5])) != 0);
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
atomic_lock(void)
{
	pthread_mutex_lock(&atomic_mutex);
}

void
atomic_unlock(void)
{
	pthread_mutex_unlock(&atomic_mutex);
}

pthread_mutex_t *
atomic_get_lock(void)
{
	return (&atomic_mutex);
}

unsigned int
hweight8(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x55);

	res = (res & 0x33) + ((res >> 2) & 0x33);
	return (res + (res >> 4)) & 0x0F;
}

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

struct cdev *
cdev_alloc(void)
{
	struct cdev *cdev;

	cdev = malloc(sizeof(*cdev));
	if (cdev)
		memset(cdev, 0, sizeof(*cdev));
	return (cdev);
}

int
cdev_add(struct cdev *cdev, dev_t mm, unsigned count)
{
	/* TODO */
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
	/* TODO */
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

	return (0);
}

void
device_del(struct device *dev)
{
	/* TODO */
	printf("Deleted device %p\n", dev);
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

void
module_put(struct module *module)
{

}

int
try_module_get(struct module *module)
{
	return (0);
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

long
ffs(long x)
{
	return (~(x - 1) & x);
}

long
ffz(long x)
{
	return ((x + 1) & ~x);
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

static int
func_init(void)
{
	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&atomic_mutex, &attr);
	return (0);
}

module_init(func_init);
