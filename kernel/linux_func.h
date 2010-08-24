#ifndef _LINUX_FUNC_H_
#define	_LINUX_FUNC_H_

#define	device_create_file(...) 0
#define	device_remove_file(...) __nop

int	printk_nop(void);

uint16_t le16_to_cpu(uint16_t x);
uint16_t be16_to_cpu(uint16_t x);

uint16_t cpu_to_le16(uint16_t x);
uint16_t cpu_to_be16(uint16_t x);

uint32_t le32_to_cpu(uint32_t x);
uint32_t be32_to_cpu(uint32_t x);

uint64_t le64_to_cpu(uint64_t x);
uint64_t be64_to_cpu(uint64_t x);

uint32_t cpu_to_le32(uint32_t x);
uint32_t cpu_to_be32(uint32_t x);

uint64_t cpu_to_le64(uint64_t x);
uint64_t cpu_to_be64(uint64_t x);

uint16_t le16_to_cpup(uint16_t *p);
uint16_t be16_to_cpup(uint16_t *p);

uint16_t cpu_to_le16p(uint16_t *p);
uint16_t cpu_to_be16p(uint16_t *p);

uint32_t le32_to_cpup(uint32_t *p);
uint32_t be32_to_cpup(uint32_t *p);

uint32_t cpu_to_le32p(uint32_t *p);
uint32_t cpu_to_be32p(uint32_t *p);

uint64_t le64_to_cpup(uint64_t *p);
uint64_t be64_to_cpup(uint64_t *p);

uint64_t cpu_to_le64p(uint64_t *p);
uint64_t cpu_to_be64p(uint64_t *p);

void	le16_to_cpus(uint16_t *p);
void	le32_to_cpus(uint32_t *p);

void	put_unaligned_le32(uint32_t, void *);
void	put_unaligned_le16(uint16_t, void *);

uint64_t get_unaligned_le64(const void *);
uint32_t get_unaligned_le32(const void *);
uint16_t get_unaligned_le16(const void *);

void   *dev_get_drvdata(const struct device *dev);
void	dev_set_drvdata(struct device *dev, void *data);
const char *dev_name(struct device *dev);
int	vm_insert_page(struct vm_area_struct *vma, unsigned long start, struct page *page);
int	remap_pfn_range(struct vm_area_struct *, unsigned long addr, unsigned long pfn, unsigned long size, int);
void	vfree(void *);
unsigned long copy_to_user(void *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void *from, unsigned long n);
unsigned long clear_user(void *to, unsigned long n);
void	schedule(void);
int	atomic_inc(atomic_t *v);
int	atomic_dec(atomic_t *v);
int	atomic_add(int i, atomic_t *v);
void	atomic_set(atomic_t *v, int i);
int	atomic_read(const atomic_t *v);
int	atomic_dec_and_test(atomic_t *v);
int	test_bit(int nr, const void *addr);
int	test_and_set_bit(int nr, volatile unsigned long *addr);
int	test_and_clear_bit(int nr, volatile unsigned long *addr);
void	set_bit(int nr, volatile unsigned long *addr);
void	clear_bit(int nr, volatile unsigned long *addr);
struct cdev *cdev_alloc(void);
void	cdev_del(struct cdev *);
int	cdev_add(struct cdev *cdev, dev_t mm, unsigned count);
unsigned int hweight8(unsigned int w);
unsigned int hweight16(unsigned int w);
unsigned int hweight32(unsigned int w);
unsigned long hweight64(uint64_t w);
unsigned long copy_to_user(void *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void *from, unsigned long n);
void	kref_get(struct kref *kref);
int	kref_put(struct kref *kref, void (*release) (struct kref *kref));
void	kref_init(struct kref *kref);
struct device *get_device(struct device *dev);
void	put_device(struct device *dev);
int	device_move(struct device *dev, struct device *new_parent);
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
int	__ffs(int x);
int	__ffz(int x);

#define	__fls(x) fls(x)
int	fls (int mask);
unsigned long find_next_bit(const unsigned long *addr, unsigned long size, unsigned long offset);
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size, unsigned long offset);
void	sort(void *base, size_t num, size_t size, int (*cmp) (const void *, const void *), void (*swap) (void *, void *, int size));
u32	crc32_le(u32 crc, unsigned char const *p, size_t len);
u32	crc32_be(u32 crc, unsigned char const *p, size_t len);
void   *vmalloc(size_t size);
void   *kcalloc(size_t n, size_t size, int flags);
long	__get_free_page(int);
void	free_page(long);
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
int32_t	div_round_closest_s32(int32_t rem, int32_t div);
uint32_t div_round_closest_u32(uint32_t rem, uint32_t div);
int64_t	div_round_closest_s64(int64_t rem, int64_t div);
uint64_t div_round_closest_u64(uint64_t rem, uint64_t div);
struct timespec ktime_get(void);
struct timeval ktime_to_timeval(const struct timespec ts);
void	ktime_get_ts(struct timespec *ts);
void	ktime_get_real_ts(struct timespec *ts);
void	msleep(uint32_t ms);
uint32_t msleep_interruptible(uint32_t ms);
void	request_module(const char *ptr);
int	device_can_wakeup(struct device *dev);
void	device_init_wakeup(struct device *dev, int flags);
int	dmi_check_system(const struct dmi_system_id *list);
void	swab16s(uint16_t *ptr);
uint16_t swab16(uint16_t temp);
void	swab32s(uint32_t *ptr);
uint32_t swab32(uint32_t temp);
int	scnprintf(char *buf, size_t size, const char *fmt,...);
struct timespec current_kernel_time(void);
int64_t	timespec_to_ns(const struct timespec *ts);
struct timespec timespec_add(struct timespec, struct timespec);
struct timespec timespec_sub(struct timespec, struct timespec);
uint64_t div64_u64(uint64_t, uint64_t);
uint32_t do_div(uint64_t *rem, uint32_t div);

#define	do_div(r,d) do_div(&(r),(d))
int	sysfs_create_group(struct kobject *kobj, const struct attribute_group *grp);
void	sysfs_remove_group(struct kobject *kobj, const struct attribute_group *grp);
void   *pci_alloc_consistent(struct pci_dev *hwdev, size_t size, dma_addr_t *dma_addr);
void	pci_free_consistent(struct pci_dev *hwdev, size_t size, void *vaddr, dma_addr_t dma_handle);
int	add_uevent_var(struct kobj_uevent_env *env, const char *format,...);
struct class *class_create(struct module *owner, const char *name);
void	cdev_init(struct cdev *cdev, const struct file_operations *fops);
struct cdev *cdev_get_device(unsigned int f_v4b);
uint32_t cdev_get_mm(unsigned int f_v4b);
int	usb_register_dev(struct usb_interface *, struct usb_class_driver *);
void	usb_deregister_dev(struct usb_interface *, struct usb_class_driver *);
struct usb_interface *usb_find_interface(struct usb_driver *, int);

#ifdef HAVE_WEBCAMD
void   *malloc_vm(size_t);
void	free_vm(void *);
int	pidfile_create(int bus, int addr, int index);

#else
#define	malloc_vm(x) malloc(x)
#define	free_vm(x) free(x)
#endif

void   *kmemdup(const void *src, size_t len, gfp_t gfp);
void   *memdup_user(const void *src, size_t len);

#endif					/* _LINUX_FUNC_H_ */
