#ifndef _LINUX_FUNC_H_
#define	_LINUX_FUNC_H_

uint16_t le16_to_cpu(uint16_t x);
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
int	atomic_add(int i, atomic_t *v);
void	atomic_set(atomic_t *v, int i);
int	atomic_read(const atomic_t *v);
struct cdev *cdev_alloc(void);
void	cdev_del(struct cdev *);

#endif					/* _LINUX_FUNC_H_ */
