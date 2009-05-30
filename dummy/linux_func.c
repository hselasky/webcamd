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
	/* TODO */
}

void
atomic_unlock(void)
{
	/* TODO */
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

void
cdev_del(struct cdev *cdev)
{
	free(cdev);
}

uint64_t
get_jiffies_64(void)
{
	/* TODO */
	return (0);
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
