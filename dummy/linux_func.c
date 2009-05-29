#include <machine/atomic.h>

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
get_unaligned_le32(void *_ptr)
{
	uint8_t *ptr = _ptr;
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
get_unaligned_le16(void *_ptr)
{
	uint8_t *ptr = _ptr;
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
	return i + atomic_fetchadd_int(&v->counter, i);
}

void
atomic_set(atomic_t *v, int i)
{
	v->counter = i;
}

int
atomic_read(const atomic_t *v)
{
	return v->counter;
}
