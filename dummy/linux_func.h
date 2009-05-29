#ifndef _LINUX_FUNC_H_
#define	_LINUX_FUNC_H_

uint16_t le16_to_cpu(uint16_t x);
uint16_t cpu_to_le16(uint16_t x);
uint32_t le32_to_cpu(uint32_t x);
uint32_t cpu_to_le32(uint32_t x);
uint16_t le16_to_cpup(uint16_t *p);
uint16_t cpu_to_le16p(uint16_t *p);
uint32_t le32_to_cpup(uint32_t *p);
uint32_t cpu_to_le32p(uint32_t *p);

void	put_unaligned_le32(uint32_t, void *);
void	put_unaligned_le16(uint16_t, void *);
uint32_t get_unaligned_le32(void *);
uint16_t get_unaligned_le16(void *);

void   *dev_get_drvdata(struct device *dev);
void	dev_set_drvdata(struct device *dev, void *data);
int	remap_pfn_range(struct vm_area_struct *, unsigned long addr, unsigned long pfn, unsigned long size, pgprot_t);
void   *vmalloc_32(unsigned long size);
void	vfree(void *);
struct page *vmalloc_to_page(void *addr);
void   *page_address(struct page *page);
unsigned long copy_to_user(void __user * to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void __user * from, unsigned long n);
void	tasklet_schedule(struct tasklet_struct *t);
void	tasklet_init(struct tasklet_struct *t, void (*func) (unsigned long), unsigned long data);
uint64_t get_jiffies_64(void);
int	jiffies_to_msecs(unsigned long);
void	schedule(void);
int	atomic_inc(atomic_t *v);
int	atomic_dec(atomic_t *v);
int	atomic_add(int i, atomic_t *v);
void	atomic_set(atomic_t *v, int i);
int	atomic_read(const atomic_t *v);
int	test_bit(int nr, const void *addr);
int	test_and_set_bit(int nr, volatile unsigned long *addr);
int	test_and_clear_bit(int nr, volatile unsigned long *addr);
void	atomic_lock(void);
void	atomic_unlock(void);
struct cdev *cdev_alloc(void);
void	cdev_del(struct cdev *);
void   *dma_alloc_coherent(struct device *dev, uint32_t size, dma_addr_t *hdl, uint32_t flag);
pgprot_t pgprot_noncached(pgprot_t);
unsigned int hweight8(unsigned int w);

#endif					/* _LINUX_FUNC_H_ */
