#ifndef _LINUX_FUNC_H_
#define	_LINUX_FUNC_H_

uint16_t le16_to_cpu(uint16_t x);
uint16_t be16_to_cpu(uint16_t x);

uint16_t cpu_to_le16(uint16_t x);
uint16_t cpu_to_be16(uint16_t x);

uint32_t le32_to_cpu(uint32_t x);
uint32_t be32_to_cpu(uint32_t x);

uint32_t cpu_to_le32(uint32_t x);
uint32_t cpu_to_be32(uint32_t x);

uint16_t le16_to_cpup(uint16_t *p);
uint16_t be16_to_cpup(uint16_t *p);

uint16_t cpu_to_le16p(uint16_t *p);
uint16_t cpu_to_be16p(uint16_t *p);

uint32_t le32_to_cpup(uint32_t *p);
uint32_t be32_to_cpup(uint32_t *p);

uint32_t cpu_to_le32p(uint32_t *p);
uint32_t cpu_to_be32p(uint32_t *p);

void	put_unaligned_le32(uint32_t, void *);
void	put_unaligned_le16(uint16_t, void *);

uint32_t get_unaligned_le32(const void *);
uint16_t get_unaligned_le16(const void *);

void   *dev_get_drvdata(struct device *dev);
void	dev_set_drvdata(struct device *dev, void *data);
const char *dev_name(struct device *dev);
int vm_insert_page(struct vm_area_struct *vma,   unsigned long start, struct page *page);
int	remap_pfn_range(struct vm_area_struct *, unsigned long addr, unsigned long pfn, unsigned long size, pgprot_t);
void	vfree(void *);
void   *page_address(struct page *page);
unsigned long copy_to_user(void *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void *from, unsigned long n);
void	tasklet_schedule(struct tasklet_struct *t);
void	tasklet_init(struct tasklet_struct *t, void (*func) (unsigned long), unsigned long data);
void	schedule(void);
int	atomic_inc(atomic_t *v);
int	atomic_dec(atomic_t *v);
int	atomic_add(int i, atomic_t *v);
void	atomic_set(atomic_t *v, int i);
int	atomic_read(const atomic_t *v);
int	test_bit(int nr, const void *addr);
int	test_and_set_bit(int nr, volatile unsigned long *addr);
int	test_and_clear_bit(int nr, volatile unsigned long *addr);
void	set_bit(int nr, volatile unsigned long *addr);
void	clear_bit(int nr, volatile unsigned long *addr);
void	atomic_lock(void);
void	atomic_unlock(void);
pthread_mutex_t *atomic_get_lock();
struct cdev *cdev_alloc(void);
void	cdev_del(struct cdev *);
int	cdev_add(struct cdev *cdev, dev_t mm, unsigned count);
unsigned int hweight8(unsigned int w);
unsigned long copy_to_user(void *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void *from, unsigned long n);
void	kref_get(struct kref *kref);
int	kref_put(struct kref *kref, void (*release) (struct kref *kref));
void	kref_init(struct kref *kref);
struct device *get_device(struct device *dev);
void	put_device(struct device *dev);
int	device_add(struct device *dev);
void	device_del(struct device *dev);
int	device_register(struct device *dev);
void	device_unregister(struct device *dev);
struct device *device_create_vargs(struct class *class, struct device *parent, dev_t devt, void *drvdata, const char *fmt, va_list args);
struct device *device_create(struct class *class, struct device *parent, dev_t devt, void *drvdata, const char *fmt,...);
void	device_destroy(struct class *class, dev_t devt);
void	module_put(struct module *module);
int	try_module_get(struct module *module);
void	module_get(struct module *module);
void   *ERR_PTR(long error);
long	PTR_ERR(const void *ptr);
long	IS_ERR(const void *ptr);
long	ffs(long x);
long	ffz(long x);
unsigned long find_next_bit(const unsigned long *addr, unsigned long size, unsigned long offset);
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size, unsigned long offset);
void	sort(void *base, size_t num, size_t size, int (*cmp) (const void *, const void *), void (*swap) (void *, void *, int size));
u32	crc32_le(u32 crc, unsigned char const *p, size_t len);
u32	crc32_be(u32 crc, unsigned char const *p, size_t len);
void   *vmalloc(size_t size);
struct class *class_get(struct class *class);
struct class *class_put(struct class *class);
int	class_register(struct class *class);
void	class_unregister(struct class *class);
void	class_destroy(struct class *class);
int	register_chrdev_region(dev_t from, unsigned count, const char *name);
void	unregister_chrdev_region(dev_t from, unsigned count);
int	remap_vmalloc_range(struct vm_area_struct *vma, void *addr, unsigned long pgoff);
void	jiffies_to_timeval(uint64_t j, struct timeval *tv);
int	do_gettimeofday(struct timeval *tp);
void	poll_initwait(struct poll_wqueues *pwq);
void	poll_freewait(struct poll_wqueues *pwq);
void	poll_schedule(struct poll_wqueues *pwq, int flag);
void	bitmap_zero(unsigned long *dst, int nbits);

#endif					/* _LINUX_FUNC_H_ */
