#ifndef _LINUX_FUNC_H_
#define	_LINUX_FUNC_H_

#include <sys/endian.h>

int	zero_nop(void);

#define	device_create_file(...) zero_nop()
#define	device_remove_file(...) zero_nop()
#define	device_create_bin_file(...) zero_nop()
#define	device_remove_bin_file(...) zero_nop()
#define	device_set_wakeup_enable(...) zero_nop()
#define	device_set_wakeup_capable(...) zero_nop()
#define	device_set_wakeup_disable(...) zero_nop()

#define	driver_create_file(...) zero_nop()
#define	driver_remove_file(...) zero_nop()
#define	driver_attach(...) zero_nop()

#define	platform_device_register_data(...) ERR_PTR(-EOPNOTSUPP)
#define	platform_device_unregister(...) __nop
#define	platform_get_device_id(...) NULL
#define	platform_get_drvdata(...) NULL
#define	platform_set_drvdata(...) __nop
#define	module_platform_driver(...)

int	driver_register(struct device_driver *);
int	driver_unregister(struct device_driver *);

int	printk_nop(void);

#define	le16_to_cpu(x) le16toh(x)
#define	be16_to_cpu(x) be16toh(x)

#define	cpu_to_le16(x) htole16(x)
#define	cpu_to_be16(x) htobe16(x)

#define	le32_to_cpu(x) le32toh(x)
#define	be32_to_cpu(x) be32toh(x)

#define	le64_to_cpu(x) le64toh(x)
#define	be64_to_cpu(x) be64toh(x)

#define	cpu_to_le32(x) htole32(x)
#define	cpu_to_be32(x) htobe32(x)

#define	cpu_to_le64(x) htole64(x)
#define	cpu_to_be64(x) htobe64(x)

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
void	le64_to_cpus(uint64_t *p);

void	put_unaligned_le64(uint64_t, void *);
void	put_unaligned_be64(uint64_t, void *);
void	put_unaligned_le32(uint32_t, void *);
void	put_unaligned_be32(uint32_t, void *);
void	put_unaligned_be16(uint16_t, void *);
void	put_unaligned_le16(uint16_t, void *);

uint64_t get_unaligned_le64(const void *);
uint64_t get_unaligned_be64(const void *);
uint32_t get_unaligned_le32(const void *);
uint32_t get_unaligned_be32(const void *);
uint16_t get_unaligned_be16(const void *);
uint16_t get_unaligned_le16(const void *);

void   *devm_kcalloc(struct device *dev, size_t n, size_t size, gfp_t gfp);
void   *devm_kzalloc(struct device *dev, size_t size, gfp_t gfp);
void   *devm_kmalloc(struct device *dev, size_t size, gfp_t gfp);
void   *devm_kmalloc_array(struct device *, size_t, size_t, gfp_t);
void   *devm_kmemdup(struct device *, const void *, size_t, gfp_t);
void	devm_kfree(struct device *dev, void *ptr);
int	devm_add_action(struct device *, void (*)(void *), void *);
int	devm_add_action_or_reset(struct device *, void (*action)(void *), void *);
struct clk *devm_clk_get(struct device *, const char *);
void   *dev_get_drvdata(const struct device *dev);
void	dev_set_drvdata(struct device *dev, void *data);
const char *dev_name(const struct device *dev);
int	vm_insert_page(struct vm_area_struct *vma, unsigned long start, struct page *page);
int	remap_pfn_range(struct vm_area_struct *, unsigned long addr, unsigned long pfn, unsigned long size, int);
void	vfree(void *);
unsigned long copy_to_user(void *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void *from, unsigned long n);
unsigned long clear_user(void *to, unsigned long n);
void	schedule(void);
int	atomic_inc(atomic_t *);
int	atomic_dec(atomic_t *);
int	atomic_add(int i, atomic_t *);
int	atomic_add_unless(atomic_t *, int, int);
void	atomic_set(atomic_t *, int);
int	atomic_read(const atomic_t *);
int	atomic_dec_and_test(atomic_t *);
int	atomic_cmpxchg(atomic_t *, int, int);

uint64_t atomic64_read(atomic64_t *);
void atomic64_or(uint64_t, atomic64_t *);
void atomic64_xor(uint64_t, atomic64_t *);
void atomic64_and(uint64_t, atomic64_t *);
void atomic64_andnot(uint64_t, atomic64_t *);

/* Bit-operations */
int	test_bit(int nr, const void *addr);

#define	__test_bit(a,b) test_bit(a,b)
int	test_and_set_bit(int nr, volatile unsigned long *addr);

#define	__test_and_set_bit(a,b) test_and_set_bit(a,b)
int	test_and_clear_bit(int nr, volatile unsigned long *addr);

#define	__test_and_clear_bit(a,b) test_and_clear_bit(a,b)
void	set_bit(int nr, volatile unsigned long *addr);

#define	__set_bit(a,b) set_bit(a,b)
void	clear_bit(int nr, volatile unsigned long *addr);

#define	__clear_bit(a,b) clear_bit(a,b)
void	change_bit(int nr, volatile unsigned long *addr);

#define	__change_bit(a,b) change_bit(a,b)

struct cdev *cdev_alloc(void);
void	cdev_del(struct cdev *);
int	cdev_add(struct cdev *cdev, dev_t mm, unsigned count);
void	cdev_set_parent(struct cdev *, struct kobject *);
int	cdev_device_add(struct cdev *, struct device *);
void	cdev_device_del(struct cdev *, struct device *);

int	register_chrdev(dev_t mm, const char *desc, const struct file_operations *fops);
int	unregister_chrdev(dev_t mm, const char *desc);

uint8_t	bitrev8(uint8_t a);
uint16_t bitrev16(uint16_t a);
size_t	memweight(const void *, size_t);
unsigned int hweight8(unsigned int w);
unsigned int hweight16(unsigned int w);
unsigned int hweight32(unsigned int w);
unsigned long hweight64(uint64_t w);
unsigned long copy_to_user(void *to, const void *from, unsigned long n);
unsigned long copy_from_user(void *to, const void *from, unsigned long n);
void	kref_get(struct kref *kref);
int	kref_get_unless_zero(struct kref *kref);
int	kref_put(struct kref *kref, void (*release) (struct kref *kref));
void	kref_init(struct kref *kref);
struct device *get_device(struct device *dev);
void	put_device(struct device *dev);
int	device_move(struct device *dev, struct device *new_parent, int how);
int	bus_register(struct bus_type *);
int	bus_unregister(struct bus_type *);
int	device_add(struct device *dev);
void	device_del(struct device *dev);
int	device_register(struct device *dev);
void	device_unregister(struct device *dev);
struct device *device_create_vargs(struct class *class, struct device *parent, dev_t devt, void *drvdata, const char *fmt, va_list args);
struct device *device_create(struct class *class, struct device *parent, dev_t devt, void *drvdata, const char *fmt,...);
int	device_enable_async_suspend(struct device *);
void	device_destroy(struct class *class, dev_t devt);
void	module_put(struct module *module);
int	try_module_get(struct module *module);

#define	__module_get module_get
void	module_get(struct module *module);
void   *ERR_PTR(long error);
long	PTR_ERR(const void *ptr);
long	IS_ERR(const void *ptr);

#define	TK_OFFS_REAL 0
#define	TK_OFFS_BOOT 1

int	__ffs(int x);

#define	ffz(x) __ffz(x)
int	__ffz(int x);

#define	__fls(x) fls(x)
int	fls (int mask);
unsigned long find_next_bit(const unsigned long *addr, unsigned long size, unsigned long offset);
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size, unsigned long offset);
void	sort(void *base, size_t num, size_t size, int (*cmp) (const void *, const void *), void (*swap) (void *, void *, int size));
u32	crc32_le(u32 crc, unsigned char const *p, size_t len);
u32	crc32_be(u32 crc, unsigned char const *p, size_t len);
void   *vmalloc(size_t size);
void   *vzalloc(size_t size);
void   *kcalloc(size_t n, size_t size, int flags);

#define	kmalloc_array(...) kcalloc(__VA_ARGS__)
long	__get_free_page(int);
void	free_page(long);
struct class *class_get(struct class *class);
struct class *class_put(struct class *class);
int	class_register(struct class *class);
void	class_unregister(struct class *class);
void	class_destroy(struct class *class);
int	alloc_chrdev_region(dev_t *, unsigned, unsigned, const char *);
int	register_chrdev_region(dev_t from, unsigned count, const char *name);
void	unregister_chrdev_region(dev_t from, unsigned count);
int	remap_vmalloc_range(struct vm_area_struct *vma, void *addr, unsigned long pgoff);
void	jiffies_to_timeval(uint64_t j, struct timeval *tv);
uint64_t round_jiffies_relative(uint64_t j);
uint64_t sched_clock(void);
int	do_gettimeofday(struct timeval *tp);
void	poll_initwait(struct poll_wqueues *pwq);
void	poll_freewait(struct poll_wqueues *pwq);
void	poll_schedule(struct poll_wqueues *pwq, int flag);

#define	poll_requested_events(...) 0xFFFFFFFFU

unsigned long *bitmap_alloc(unsigned int, gfp_t);
unsigned long *bitmap_zalloc(unsigned int, gfp_t);
void	bitmap_free(unsigned long *);
void	bitmap_copy(unsigned long *, const unsigned long *, unsigned int);
int	bitmap_weight(const unsigned long *, unsigned int);
int	bitmap_andnot(unsigned long *, const unsigned long *, const unsigned long *, int);
int	bitmap_and(unsigned long *, const unsigned long *, const unsigned long *, int);
void	bitmap_or(unsigned long *, const unsigned long *, const unsigned long *, int);
void	bitmap_xor(unsigned long *, const unsigned long *, const unsigned long *, int);
void	bitmap_zero(unsigned long *, unsigned int);
void	bitmap_fill(unsigned long *, unsigned int);
int	bitmap_subset(const unsigned long *, const unsigned long *, int);
int	bitmap_full(const unsigned long *, int);
void	bitmap_clear(unsigned long *, int, int);
void	bitmap_shift_right(unsigned long *, const unsigned long *, int, int);
void	bitmap_shift_left(unsigned long *, const unsigned long *, int, int);
int	bitmap_equal(const unsigned long *, const unsigned long *, unsigned);
int	bitmap_empty(const unsigned long *, unsigned);
int	bitmap_intersects(const unsigned long *, const unsigned long *, unsigned);
int32_t	div_round_closest_s32(int32_t rem, int32_t div);
uint32_t div_round_closest_u32(uint32_t rem, uint32_t div);
int64_t	div_round_closest_s64(int64_t rem, int64_t div);
uint64_t div_round_closest_u64(uint64_t rem, uint64_t div);
#define	ktime_to_timespec64(ktime) ktime
struct timespec ktime_mono_to_real(struct timespec);
struct timespec ktime_get_boottime(void);
struct timespec ktime_get_real(void);
struct timespec ktime_get(void);
int64_t ktime_get_ns(void);
struct timespec ktime_mono_to_any(struct timespec, int);
struct timeval ktime_to_timeval(const struct timespec);
void	ktime_get_ts(struct timespec *);
void	ktime_get_real_ts(struct timespec *);
int64_t	ktime_to_ns(const struct timespec ts);
struct timespec ktime_sub(const struct timespec a, const struct timespec b);
struct timespec ktime_add(const struct timespec a, const struct timespec b);
struct timespec ktime_get_monotonic_offset(void);
struct timespec ktime_add_us(const struct timespec, const uint64_t);
int64_t	ktime_us_delta(const struct timespec, const struct timespec);
int64_t	ktime_to_us(const struct timespec);
int64_t	ktime_to_ms(const struct timespec);
struct timespec ktime_set(const s64, const unsigned long);
int64_t ktime_ms_delta(const ktime_t, const ktime_t);
int	ktime_compare(const ktime_t, const ktime_t);
u64	timeval_to_ns(const struct timeval *);
struct timeval	ns_to_timeval(u64);
void	msleep(uint32_t ms);
void	ssleep(uint32_t s);
uint32_t msleep_interruptible(uint32_t ms);
int	request_module(const char *fmt,...);
int	request_module_nowait(const char *fmt,...);
int	device_can_wakeup(struct device *dev);
void	device_init_wakeup(struct device *dev, int flags);
void	device_initialize(struct device *dev);
int	dmi_check_system(const struct dmi_system_id *list);
void	swab16s(uint16_t *ptr);
uint16_t swab16(uint16_t temp);
void	swab32s(uint32_t *ptr);
uint32_t swab32(uint32_t temp);
int	scnprintf(char *buf, size_t size, const char *fmt,...);
#define	vscnprintf(...) \
	scnprintf(__VA_ARGS__)
char   *devm_kasprintf(struct device *, gfp_t, const char *,...);
struct timespec current_kernel_time(void);
int64_t	timespec_to_ns(const struct timespec *);
struct timespec ns_to_timespec(const int64_t);
#define	ns_to_timespec64(x) ns_to_timespec(x)
struct timespec timespec_add(struct timespec, struct timespec);
struct timespec timespec_sub(struct timespec, struct timespec);
uint32_t do_div(uint64_t *rem, uint32_t div);
uint64_t div64_u64(uint64_t, uint64_t);
int64_t	div64_s64(int64_t, int64_t);
uint64_t div_u64_rem(uint64_t, uint32_t, uint32_t *);
int64_t	div_s64_rem(int64_t, int32_t, int32_t *);
uint64_t div_u64(uint64_t, uint32_t);
int64_t	div_s64(int64_t, int32_t);

#define	do_div(r,d) do_div(&(r),(d))
int	sysfs_create_group(struct kobject *, const struct attribute_group *);
void	sysfs_remove_group(struct kobject *, const struct attribute_group *);
int	sysfs_create_bin_file(struct kobject *, struct bin_attribute *);
int	sysfs_remove_bin_file(struct kobject *, struct bin_attribute *);
void   *pci_alloc_consistent(struct pci_dev *hwdev, size_t size, dma_addr_t *dma_addr);
void   *pci_zalloc_consistent(struct pci_dev *hwdev, size_t size, dma_addr_t *dma_addr);
void	pci_free_consistent(struct pci_dev *hwdev, size_t size, void *vaddr, dma_addr_t dma_handle);
int	add_uevent_var(struct kobj_uevent_env *env, const char *format,...);
struct class *class_create(struct module *owner, const char *name);
void	cdev_init(struct cdev *cdev, const struct file_operations *fops);
struct cdev *cdev_get_device(unsigned int f_v4b);
uint32_t cdev_get_mm(unsigned int f_v4b);
int	usb_register_dev(struct usb_interface *, struct usb_class_driver *);
void	usb_deregister_dev(struct usb_interface *, struct usb_class_driver *);
struct usb_interface *usb_find_interface(struct usb_driver *, int);

int	is_vmalloc_addr(void *);
void   *malloc_vm(size_t);
void	free_vm(void *);
int	pidfile_create(int bus, int addr, int index);

void   *kmemdup(const void *src, size_t len, gfp_t gfp);
void   *memdup_user(const void *src, size_t len);

unsigned long rounddown_pow_of_two(unsigned long);
unsigned long roundup_pow_of_two(unsigned long);
const char *skip_spaces(const char *str);

int	kobject_set_name(struct kobject *kobj, const char *fmt,...);

int	nonseekable_open(struct inode *inode, struct file *filp);
int	stream_open(struct inode *inode, struct file *filp);

int	kstrtos16(const char *, unsigned int, int16_t *);
int	kstrtou16(const char *, unsigned int, uint16_t *);
int	kstrtos8(const char *, unsigned int, int8_t *);
int	kstrtou8(const char *, unsigned int, uint8_t *);
int	kstrtoul(const char *, unsigned int, unsigned long *);
int	kstrtouint(const char *, unsigned int, unsigned int *);
int	kstrtoint(const char *, unsigned int, int *);

ssize_t	simple_write_to_buffer(void *, size_t, loff_t *, const void __user *, size_t);
ssize_t	simple_read_from_buffer(void __user *, size_t, loff_t *, const void *, size_t);

uint64_t int_sqrt(uint64_t a);

void   *devres_alloc(dr_release_t, size_t, gfp_t);
void	devres_free(void *);
void	devres_add(struct device *, void *);
int	devres_destroy(struct device *, dr_release_t, dr_match_t, void *);
void   *devres_open_group(struct device *, void *, gfp_t);
void	devres_close_group(struct device *, void *);
int	devres_release_group(struct device *, void *);

int	dma_buf_fd(struct dma_buf *, int);
struct dma_buf *dma_buf_get(int);
void	dma_buf_put(struct dma_buf *);
void   *dma_buf_vmap(struct dma_buf *);
void	dma_buf_vunmap(struct dma_buf *, void *);

uint32_t ror32(uint32_t, uint8_t);
unsigned long gcd(unsigned long, unsigned long);
void	get_random_bytes(void *, int);
u32	prandom_u32_max(u32);

const char *dev_driver_string(const struct device *);

s32	sign_extend32(u32, int);

void	eth_zero_addr(u8 *addr);

struct device *kobj_to_dev(struct kobject *);

void	*memscan(void *, int, size_t);

int	refcount_read(refcount_t *);
bool	refcount_dec_and_test(refcount_t *);
void	refcount_set(refcount_t *, int);
void	refcount_inc(refcount_t *);

size_t	array_size(size_t, size_t);
size_t	array3_size(size_t, size_t, size_t);
size_t	struct_size_sub(size_t, size_t, size_t);
ssize_t	strscpy(char *, const char *, size_t);

#define	struct_size(p, member, n)				\
	struct_size_sub(n, sizeof(*(p)->member), sizeof(*(p)))

ssize_t	memory_read_from_buffer(void *, size_t, loff_t *, const void *, size_t);

#endif					/* _LINUX_FUNC_H_ */
