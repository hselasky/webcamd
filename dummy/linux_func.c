#include <machine/atomic.h>

uint16_t
le16_to_cpu(uint16_t x)
{
	uint8_t *p = (uint8_t *)&x;

	return (p[0] + (p[1] << 8));
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
