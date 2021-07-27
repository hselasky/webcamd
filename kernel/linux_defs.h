/*-
 * Copyright (c) 2009-2021 Hans Petter Selasky. All rights reserved.
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

#ifndef _LINUX_DEFS_H_
#define	_LINUX_DEFS_H_

/*
 * Linux kernel defines from various header files
 */

#define	CONFIG_BASE_SMALL	1

#define	DIV_ROUND_CLOSEST(rem, div) \
((sizeof(typeof(div)) > 4) ?	    \
 ((((typeof(div))-1) <= 0) ?	    \
  div_round_closest_s64(rem,div) :  \
  div_round_closest_u64(rem,div)) : \
 ((((typeof(div))-1) <= 0) ?	    \
  div_round_closest_s32(rem,div) :  \
  div_round_closest_u32(rem,div)))

#define	DIV_ROUND_CLOSEST_ULL(rem, div) \
  div_round_closest_u64(rem,div)

#define	mult_frac(x, numer, denom)			\
({							\
	typeof(x) quot = (x) / (denom);                 \
	typeof(x) rem  = (x) % (denom);                 \
        (quot * (numer)) + ((rem * (numer)) / (denom)); \
})

#ifndef howmany
#define	howmany(x, y)   (((x)+((y)-1))/(y))
#endif

#define	is_power_of_2(x) (((-(x)) & (x)) == (x))
#define	__nop do {} while (0)
#define	__user
#define	__kernel
#define	__safe
#define	__deprecated
#define	__force
#define	__nocast
#define	__iomem
#define	__read_mostly
#define	__maybe_unused
#define	__must_check
#define	__must_hold(...)
#define	__chk_user_ptr(x) __nop
#define	__chk_io_ptr(x) __nop
#define	__builtin_warning(x, ...) (1)
#define	__acquires(x)
#define	__releases(x)
#define	__acquire(x) __nop
#define	__release(x) __nop
#define	__cond_lock(x,c) (c)
#define	__pgprot(x)     ((pgprot_t)(x))
#define	__rcu
#define	__percpu
#define	fallthrough __nop
#define	local_irq_save(x) __nop
#define	local_irq_restore(x) __nop
#define	preempt_enable() __nop
#define	preempt_disable() __nop
#define	__this_cpu_write(a,b) do { (a) = (b); } while (0)
#define	__this_cpu_read(a) (a)
#define	__this_cpu_inc(a) do { (a)++; } while (0)
#define	__this_cpu_dec(a) do { (a)--; } while (0)
#define	irqs_disabled(...) (0)
#define	SetPageReserved(...)   __nop
#define	ClearPageReserved(...) __nop
#define	ETH_ALEN 6
#define	_PAGE_PRESENT   0
#define	_PAGE_RW        0
#define	_PAGE_USER      0
#define	_PAGE_ACCESSED  0
#define	PAGE_SHARED 0
#define	PAGE_ALIGN(addr)        (((addr)+PAGE_SIZE-1)&(~(PAGE_SIZE - 1)))
#undef ALIGN
#define	ALIGN(x,a)		(((x)+(a)-1)&(~((a)-1)))
#define	PAGE_OFFSET	0
#define	__pa(x)                 ((unsigned long)(x)-PAGE_OFFSET)
#define	S_IRWXG 00070
#define	S_IRWXU 00700
#define	S_IRUSR 00400
#define	S_IWUSR 00200
#define	S_IXUSR 00100
#define	S_IRGRP 00040
#define	S_IWGRP 00020
#define	S_IXGRP 00010
#define	S_IROTH 00004
#define	S_IWOTH 00002
#define	S_IXOTH 00001
#define	S_IRUGO (S_IRUSR|S_IRGRP|S_IROTH)
#define	S_IWUGO (S_IWUSR|S_IWGRP|S_IWOTH)
#define	offsetof(t, m) ((uint8_t *)&((t *)0)->m - (uint8_t *)0)
#define	container_of(ptr, t, m) ((t *)((uint8_t *)(ptr) - offsetof(t, m)))
#define	__devinitdata
#define	__init
#define	__exit
#define	___stringify(...)	#__VA_ARGS__
#define	__stringify(...)	___stringify(__VA_ARGS__)
#define	ERESTARTSYS     512
#define	ENOIOCTLCMD     513
#define	EMEDIUMTYPE	514
#define	ENODATA		515
#define	EPROBE_DEFER	516
#define	EBADR		517
#define	symbol_request(x) (&(x))
#define	symbol_get(x) __nop
#define	symbol_put(x) __nop
#define	EXPORT_SYMBOL(...)
#define	EXPORT_SYMBOL_GPL(...)
#define	MODULE_INFO(...)
#define	MODULE_AUTHOR(...)
#define	MODULE_DESCRIPTION(...)
#define	MODULE_LICENSE(...)
#define	MODULE_DEVICE_TABLE(...)
#define	MODULE_VERSION(...)
#define	MODULE_ALIAS(...)
#define	MODULE_ALIAS_CHARDEV_MAJOR(...)
#define	MODULE_SUPPORTED_DEVICE(...)
#define	MODULE_FIRMWARE(...)
#define	IS_MODULE(x) x##_IS_MODULE
#define	IS_BUILTIN(x) IS_ENABLED(x)
#define	IS_ENABLED(x) x##_IS_ENABLED
#define	IS_REACHABLE(x,...) defined(x##__VA_ARGS__)
#define	IS_ALIGNED(x, a) (((x) & ((typeof(x))(a) - 1)) == 0)
#define	__aligned_u64 uint64_t __aligned(8)
#define	THIS_MODULE (NULL)
#define	DECLARE_EVENT_CLASS(...)
#define	DECLARE_EVENT(...)
#define	DECLARE_PER_CPU(...)
#define	DEFINE_EVENT(...)
#define	TRACE_EVENT(...)
#define	TRACE_DEFINE_ENUM(...)
#define	trace_regcache_drop_region(...) __nop
#define	trace_regcache_sync(...) __nop
#define	trace_regmap_async_complete_done(...) __nop
#define	trace_regmap_async_complete_start(...) __nop
#define	trace_regmap_async_io_complete(...) __nop
#define	trace_regmap_async_write_start(...) __nop
#define	trace_regmap_cache_bypass(...) __nop
#define	trace_regmap_cache_only(...) __nop
#define	trace_regmap_hw_read_done(...) __nop
#define	trace_regmap_hw_read_start(...) __nop
#define	trace_regmap_hw_write_done(...) __nop
#define	trace_regmap_hw_write_start(...) __nop
#define	trace_regmap_reg_read(...) __nop
#define	trace_regmap_reg_read_cache(...) __nop
#define	trace_regmap_reg_write(...) __nop
#define	trace_v4l2_dqbuf(...) __nop
#define	trace_v4l2_qbuf(...) __nop
#define	trace_vb2_buf_done(...) __nop
#define	trace_vb2_qbuf(...) __nop
#define	trace_vb2_buf_queue(...) __nop
#define	trace_vb2_dqbuf(...) __nop
#define	trace_pwc_handler_enter(...) __nop
#define	trace_pwc_handler_exit(...) __nop
#define	print_hex_dump_debug(...) __nop
#define	__compiletime_warning(...)
#define	__compiletime_error(...)
#ifdef HAVE_DEBUG
#define	printk(...) do {		\
	syslog(LOG_DEBUG, __VA_ARGS__);	\
} while (0)
#define	printk_once(...) printk(__VA_ARGS__)
#define	pr_warn_once(...) printk(__VA_ARGS__)
#define	dev_warn_once(dev, ...) printk(__VA_ARGS__)
#else
#define	printk(...) printk_nop()
#define	printk_once(...) printk_nop()
#define	pr_warn_once(...) printk_nop()
#define	dev_warn_once(...) printk_nop()
#endif
#define	no_printk(...) printk_nop()
#define	print_hex_dump_bytes(...) printk_nop()
#define	printk_ratelimit(...) printk_nop()
#define	printk_timed_ratelimit(...) printk_nop()
#ifdef HAVE_DEBUG
#define	pr_fmt(...) __VA_ARGS__
#endif
#define	printk_ratelimited(...) printk_nop()
#define	pr_err_ratelimited(...) __nop
#define	pr_warn_ratelimited(...) __nop
#define	pr_cont(...) __nop
#define	pr_err(...) __nop
#define	pr_info(...) __nop
#define	pr_dbg(...) __nop
#define	pr_debug(...) __nop
#define	pr_warn(...) __nop
#define	pr_warning(...) __nop
#define	pr_emerg(...) __nop
#define	dev_dbg(dev, fmt, ...) printk("DBG: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_debug(dev, fmt, ...) printk("DBG: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_dbg_ratelimited(dev, fmt, ...) printk("DBG: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_err(dev, fmt, ...) printk("ERR: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_err_ratelimited(dev, fmt, ...) printk("ERR: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_info(dev, fmt, ...) printk("INFO: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_info_ratelimited(dev, fmt, ...) printk("INFO: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_warn(dev, fmt, ...) printk("WARN: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_warn_ratelimited(dev, fmt, ...) printk("WARN: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_notice(dev, fmt, ...) printk("NOTICE: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_notice_ratelimited(dev, fmt, ...) printk("NOTICE: %s: " fmt "\n", dev_name(dev),## __VA_ARGS__)
#define	dev_printk(info, dev, fmt, ...) printk("DBG: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	info(fmt, ...) printk("INFO: " fmt "\n",## __VA_ARGS__)
#define	warn(fmt, ...) printk("WARN: " fmt "\n",## __VA_ARGS__)
#define	dbg(fmt, ...) printk("DBG: " fmt "\n",## __VA_ARGS__)
#define	err(fmt, ...) printk("ERR: " fmt "\n",## __VA_ARGS__)
#define	notice(fmt, ...) printk("NOTICE: " fmt "\n",## __VA_ARGS__)
#define	kmem_cache_create(desc,size,align,arg,fn) ((struct kmem_cache *)(size))
#define	kmem_cache_destroy(...) __nop
#define	kmem_cache_free(ref,ptr) free(GP_DECONST(ptr))
#define	kmem_cache_alloc(ref,g) malloc((long)(ref))
#define	kmem_cache_zalloc(ref,g) calloc(1, (long)(ref))
#define	kvmalloc(size,flags) malloc(size)
#define	kvmalloc_array(n,s,flags) malloc((n) * (s))
#define	kvzalloc(s,flags) calloc(1, s)
#define	kvfree(ptr) free(ptr)
#define	kmalloc(s,opt) malloc(s)
#define	kzalloc(s,opt) calloc(1, (s))
#define	krealloc(p,s,opt) realloc(p,(s))
#define	dma_alloc_coherent(d,s,h,g) calloc(1,(s))
#define	dma_free_coherent(d,s,v,h) free(GP_DECONST(v))
#define	dma_map_single(...) 0
#define	dma_unmap_single(...) __nop
#define	dma_mapping_error(...) 0
#define	dma_sync_single_for_device(...) __nop
#define	vmalloc_32_user(s) malloc_vm(s)
#define	vmalloc_user(s) malloc_vm(s)
#define	vmalloc_32(s) malloc_vm(s)
#define	vmalloc_to_page(x) ((struct page *)(x))	/* HACK */
#define	vmalloc_to_pfn(x) ((unsigned long)(x))	/* HACK */
#define	alloc_page(...) malloc_vm(PAGE_SIZE)
#define	page_address(x) ((void *)(x))	/* HACK */
#define	virt_to_page(x) ((struct page *)(((uintptr_t)(x)) & ~(PAGE_SIZE-1)))	/* HACK */
#define	offset_in_page(x) (((uintptr_t)(x)) & (PAGE_SIZE - 1))
#define	page_to_phys(x) ((uintptr_t)(x))
#define	page_cache_release(...) __nop
#define	clear_user_highpage(...) __nop
#define	sg_set_page(...) __nop
#define	sg_next(...) NULL
#define	of_match_ptr(...) NULL
#define	is_of_node(...) 0
#define	sysfs_attr_init(x) __nop
#define	sysfs_create_files(...) (0)
#define	sysfs_streq(a,b) (strcmp(a,b) == 0)
#define	kobject_create_and_add(...) ((void *)1)
#define	kobject_get_path(...) strdup("webcamd")
#define	kobject_put(...) __nop
#define	kobject_get(...) __nop
#define	kobject_uevent(...) __nop
#define	kobject_uevent_env(...) (int)0
#define	vfree(ptr) free_vm(ptr)
#define	kfree(ptr) free(GP_DECONST(ptr))
#define	kfree_const(ptr) free(GP_DECONST(ptr))
#define	kstrdup(a,b) strdup(a)
#define	kstrdup_const(a,b) strdup(a)
#define	might_sleep(x) __nop
#define	might_sleep_if(x) __nop
#define	fsleep(d) usleep(d)
#define	ndelay(d) usleep(((d) + 1000ULL - 1ULL)/1000UL)
#define	udelay(d) usleep(d)
#define	mdelay(d) usleep((d) * 1000)
#define	swap(a, b) \
    do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)
#define	usleep_range(_min,_max) usleep(_min)
#define	__GFP_WAIT 0
#define	__GFP_HIGH 0
#define	__GFP_IO 0
#define	__GFP_FS 0
#define	__GFP_COLD 0
#define	__GFP_NOWARN 0
#define	__GFP_REPEAT 0
#define	__GFP_NOFAIL 0
#define	__GFP_NORETRY 0
#define	__GFP_NO_GROW 0
#define	__GFP_COMP 0
#define	__GFP_ZERO 0
#define	__GFP_NOMEMALLOC 0
#define	__GFP_HARDWALL 0
#define	__GFP_DMA32 0
#define	__GFP_BITS_SHIFT 1
#define	GFP_NOWAIT 0
#define	GFP_ATOMIC 0
#define	GFP_NOIO 0
#define	GFP_NOFS 0
#define	GFP_KERNEL 0
#define	GFP_USER 0
#define	GFP_HIGHUSER 0
#define	TASK_RUNNING            0
#define	TASK_INTERRUPTIBLE      1
#define	TASK_UNINTERRUPTIBLE    2
#define	TASK_STOPPED            4
#define	TASK_TRACED             8
#define	EXIT_ZOMBIE             16
#define	EXIT_DEAD               32
#define	TASK_NONINTERACTIVE     64
#define	TASK_COMM_LEN		16
#define	no_llseek	NULL
#define	default_llseek	NULL
#define	GENMASK(hi, lo) (((2UL << ((hi) - (lo))) - 1UL) << (lo))
#define	GENMASK_ULL(hi, lo) (((2ULL << ((hi) - (lo))) - 1ULL) << (lo))
#define	BITS_PER_LONG_LONG 64
#ifdef __LP64__
#define	BITS_PER_LONG 64
#else
#define	BITS_PER_LONG 32
#endif
#define	BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#define	BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#define	BITS_PER_BYTE 8
#define	for_each_set_bit(b, addr, size) \
    for ((b) = 0; (b) < (size); (b)++) \
	if ((addr)[(b) / BITS_PER_LONG] & BIT((b) % BITS_PER_LONG))
#define	for_each_set_bit_from(b, addr, size) \
    for ( ; (b) < (size); (b)++) \
	if ((addr)[(b) / BITS_PER_LONG] & BIT((b) % BITS_PER_LONG))
#define	BITS_TO_LONGS(n) (((n) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define	BIT(n) (1UL << (n))
#define	BIT_ULL(n) (1ULL << (n))
#define	KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define	LINUX_VERSION_CODE KERNEL_VERSION(2, 6, 38)
#define	BUS_ID_SIZE 32
#define	DECLARE_BITMAP(n, max) unsigned long n[((max)+BITS_PER_LONG-1)/BITS_PER_LONG]
#define	MKDEV(maj,min) ((dev_t)(uint32_t)((((maj) & 0xFFFFUL) << 16)|((min) & 0xFFFFUL)))
#define	MAJOR(dev) (((dev) >> 16) & 0xFFFFUL)
#define	MINOR(dev) ((dev) & 0xFFFFUL)
#define	dev_set_name(d, ...) \
	snprintf((d)->name, sizeof((d)->name), __VA_ARGS__)
#define	kasprintf(mem,...) ({			\
	char *temp_ptr;				\
	asprintf(&temp_ptr, __VA_ARGS__);	\
	temp_ptr;				\
})
#define	print_hex_dump(...) __nop
#define	__printf(a,b)
#define	DEFAULT_POLLMASK POLLNVAL
#define	POLL_ERR POLLERR
#define	EPOLLRDNORM POLLRDNORM
#define	EPOLLWRNORM POLLWRNORM
#define	EPOLLIN	POLLIN
#define	EPOLLOUT POLLOUT
#define	EPOLLERR POLLERR
#define	EPOLLPRI POLLPRI
#define	EPOLLHUP POLLHUP
#define	IOCSIZE_MASK (_IOC_SIZEMASK << _IOC_SIZESHIFT)
#define	_IOC_TYPE(cmd) IOCGROUP(cmd)
#define	_IOC_SIZE(cmd) IOCPARM_LEN(cmd)
#define	_IOC_NR(cmd) ((cmd) & 0xFF)
#define	_IOC_DIR(cmd) ((cmd) & IOC_DIRMASK)
#define	_IOC_NONE IOC_VOID
#define	_IOC_READ IOC_OUT
#define	_IOC_WRITE IOC_IN
#define	_IOC_SIZEMASK IOCPARM_MASK
#define	_IOC_SIZESHIFT 16
#define	__OLD_VIDIOC_
#define	down_read(...) __nop
#define	up_read(...) __nop
#define	VM_WRITE 0x0001
#define	VM_READ 0x0002
#define	VM_SHARED 0x0004
#define	VM_DONTEXPAND 0x0008
#define	VM_FAULT_OOM 0x0010
#define	VM_RESERVED 0x0020
#define	VM_IO 0x0040
#define	VM_DONTDUMP 0x0080
#define	DMA_FROM_DEVICE 0x01
#define	DMA_TO_DEVICE 0x02
#define	DMA_BIDIRECTIONAL 0x03
#define	sizeof_field(t, m) sizeof((((t *)0)->m))
#define	ARRAY_SIZE(ptr) (sizeof(ptr) / sizeof((ptr)[0]))
#define	array_index_nospec(index, size) (((index) >= (size)) ? 0 : (index))
#define	__KERNEL__
#define	capable(...) 1
#define	uninitialized_var(...) __VA_ARGS__
#define	HZ 1000
#define	NSEC_PER_SEC 1000000000LL
#define	USEC_PER_SEC 1000000LL
#define	USEC_PER_MSEC 1000LL
#define	MSEC_PER_SEC 1000LL
#define	jiffies get_jiffies_64()
#define	msecs_to_jiffies(x) (x)
#define	usecs_to_jiffies(x) ((x) / 1000)
#define	nsecs_to_jiffies(x) ((x) / 1000000)
#define	jiffies_to_msecs(x) (x)
#define	jiffies_to_usecs(x) ((x) * 1000LL)
#define	jiffies_to_nsecs(x) ((x) * 1000000LL)
#define	likely(...) __VA_ARGS__
#define	unlikely(...) __VA_ARGS__
#define	__round_mask(x, y) ((typeof(x))((y)-1))
#define	roundup(x, y) ((((x) % (y)) == 0) ? (x) : (x) - ((x) % (y)) + (y))
#define	rounddown(x, y) ((x) - ((x) % (y)))
#define	round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define	round_down(x, y) ((x) & ~__round_mask(x, y))
#define	DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define	min(a,b) (((a) < (b)) ? (a) : (b))
#define	max(a,b) (((a) > (b)) ? (a) : (b))
#define	max3(a,b,c) (((a) >= (b) && (a) >= (c)) ? (a) :	\
		     ((b) >= (a) && (b) >= (c)) ? (b) : (c))
#define	min3(a,b,c) (((a) <= (b) && (a) <= (c)) ? (a) : \
		     ((b) <= (a) && (b) <= (c)) ? (b) : (c))
#define	prefetch(x) (void)x
#define	barrier()	__asm__ __volatile__("": : :"memory")
#define	WRITE_ONCE(x, val) \
do { volatile typeof(x) __val = (val); (x) = __val; } while (0)
#define	READ_ONCE(x) ({			\
	typeof(x) __var = ({		\
		barrier();		\
		ACCESS_ONCE(x);		\
	});				\
	barrier();			\
	__var;				\
})
#define	KERN_INFO ""
#define	KERN_WARNING ""
#define	KERN_ERR ""
#define	KERN_DEBUG ""
#define	KERN_CONT ""
#define	KBUILD_MODNAME ""
#define	KERN_NOTICE ""
#define	BUG(...) __nop
#define	__WARN() __nop
#define	WARN(x,...) ({ (x); })
#define	WARN_ONCE(x,...) ({ (x); })
#define	BUG_ON(x) ({ (x); })
#define	WARN_ON(x) ({ (x); })
#define	WARN_ON_ONCE(x) ({ (x); })
#define	BUILD_BUG_ON(x) extern int dummy_array[(x) ? -1 : 1]
#define	lockdep_set_class_and_name(...) __nop
#define	lockdep_assert_held(...) __nop
#define	lockdep_assert_irqs_enabled(...) __nop
#define	lock_kernel(...) __nop
#define	unlock_kernel(...) __nop
#define	rwlock_init(l) __nop
#define	write_lock_irqsave(l, f) do { (f) = 1; atomic_lock(); } while (0)
#define	write_unlock_irqrestore(l, f) do { if (f) { (f) = 0; atomic_unlock(); } } while (0)
#define	read_lock_irqsave(l, f) do { (f) = 1; atomic_lock(); } while (0)
#define	read_unlock_irqrestore(l, f) do { if (f) { (f) = 0; atomic_unlock(); } } while (0)
#define	read_lock(l) atomic_lock()
#define	read_unlock(l) atomic_unlock()
#define	spin_lock_init(lock) __nop
#define	spin_lock_irqsave(l,f)  do { (f) = 1; atomic_lock(); } while (0)
#define	spin_unlock_irqrestore(l,f) do { if (f) { (f) = 0; atomic_unlock(); } } while (0)
#define	spin_lock(...)  atomic_lock()
#define	spin_unlock(...) atomic_unlock()
#define	spin_lock_irq(...)  atomic_lock()
#define	spin_unlock_irq(...) atomic_unlock()
#define	spin_lock_bh(...) atomic_lock()
#define	spin_unlock_bh(...) atomic_unlock()
#define	raw_spin_lock_init(lock) __nop
#define	raw_spin_lock(...)  atomic_lock()
#define	raw_spin_unlock(...) atomic_unlock()
#define	atomic_inc_return(...) atomic_inc(__VA_ARGS__)
#define	atomic_dec_return(...) atomic_dec(__VA_ARGS__)
#define	assert_spin_locked(...) __nop
#define	ASSERT_EXCLUSIVE_ACCESS(...) __nop
#define	IS_ERR_VALUE(x) ((unsigned long)(x) >= (unsigned long)-(1<<14))
#define	IS_ERR_OR_NULL(x) ((unsigned long)(x) == 0 || IS_ERR_VALUE(x))
#define	ERR_CAST(x) ((void *)(long)(const void *)(x))
#define	find_first_bit(addr, size) find_next_bit((addr), (size), 0)
#define	find_first_zero_bit(addr, size) find_next_zero_bit((addr), (size), 0)
#define	synchronize_sched() do { atomic_lock(); atomic_unlock(); } while (0)
#define	cond_resched() do { } while (0)
#define	signal_pending(...) check_signal()
#define	down_write(...) __nop
#define	down_read(...) __nop
#define	up_write(...) __nop
#define	up_read(...) __nop
#define	dump_stack(...) __nop
#define	get_user_pages(a,b,...) linux_get_user_pages(__VA_ARGS__)
#define	DECLARE_RWSEM(x) struct rw_semaphore x
#define	crc32(s, d, l) crc32_le(s, (unsigned char const *)d, l)
#define	CRCPOLY_LE 0xedb88320
#define	CRCPOLY_BE 0x04c11db7
#define	mb() __asm volatile("":::"memory")
#define	smp_wmb() mb()
#define	smp_mb() mb()
#define	smp_rmb() mb()
#define	smp_mb__after_clear_bit() mb()
#define	smp_mb__after_atomic() mb()
#define	smp_store_release(p, v) do {		\
	atomic_lock();				\
	*(p) = (v);				\
	atomic_unlock();			\
} while (0);
#define	smp_load_acquire(p) ({			\
	typeof(*(p)) __ret;			\
	atomic_lock();				\
	__ret = *(p);				\
	atomic_unlock();			\
	__ret;					\
})
#define	ACCESS_ONCE(x)		(*(volatile __typeof(x) *)&(x))
#define	devres_find(...) NULL
#define	fops_get(x) (x)
#define	fops_put(x) __nop
#define	replace_fops(f, fops) do {	\
	struct file *__file = (f);	\
	__file->f_op = (fops);		\
} while (0)
#define	__devinitconst
#define	__devinit
#define	__devexit
#define	__devexit_p
#define	dma_sync_single_for_cpu(...) __nop
#define	pgprot_noncached(x) (x)
#define	set_current_state(...) __nop
#define	__set_current_state(...) __nop
#define	task_pid_nr(...) (1)
#define	time_after(a,b) (((long)(b) - (long)(a)) < 0)
#define	time_after_eq(a,b) (((long)(b) - (long)(a)) <= 0)
#define	time_before(a,b) time_after(b,a)
#define	time_before_eq(a,b) time_after_eq(b,a)
#define	time_is_before_jiffies(a) time_after(jiffies,a)
#define	time_is_after_jiffies(a) time_before(jiffies,a)
#define	time_is_after_eq_jiffies(a) time_before_eq(jiffies,a)
#define	time_is_before_eq_jiffies(a) time_after_eq(jiffies,a)
#define	__attribute_const__
#undef __always_inline
#define	__always_inline inline
#define	noinline
#define	noinline_for_stack
#define	__cpu_to_be64(x) cpu_to_be64(x)
#define	__cpu_to_be32(x) cpu_to_be32(x)
#define	__cpu_to_be16(x) cpu_to_be16(x)
#define	__cpu_to_le64(x) cpu_to_le64(x)
#define	__cpu_to_le32(x) cpu_to_le32(x)
#define	__cpu_to_le16(x) cpu_to_le16(x)
#define	__le64_to_cpu(x) le64_to_cpu(x)
#define	__le32_to_cpu(x) le32_to_cpu(x)
#define	__le16_to_cpu(x) le16_to_cpu(x)
#define	__le64_to_cpus(p) le64_to_cpus(p)
#define	__le32_to_cpus(p) le32_to_cpus(p)
#define	__le16_to_cpus(p) le16_to_cpus(p)
#define	__be64_to_cpu(x) be64_to_cpu(x)
#define	__be32_to_cpu(x) be32_to_cpu(x)
#define	__be16_to_cpu(x) be16_to_cpu(x)
#define	__be64_to_cpus(p) be64_to_cpus(p)
#define	__be32_to_cpus(p) be32_to_cpus(p)
#define	__be16_to_cpus(p) be16_to_cpus(p)
#define	GP_DECONST(ptr) ((void *)(long)(ptr))
#define	get_unaligned(x) ({ __typeof(*(x)) __temp; memcpy(GP_DECONST(&__temp), (x), sizeof(__temp)); __temp; })
#define	put_unaligned(x, y) do { __typeof(*(y)) __temp = (x); memcpy((y), GP_DECONST(&__temp), sizeof(__temp)); } while (0)
#define	NSEC_PER_USEC	1000
#define	simple_strtoul strtoul
#define	strict_strtoul(a,b,c) ({char *_pp; *(c) = strtoul(a,&_pp,b); _pp;})
#define	simple_strtol strtol
#define	strict_strtol(a,b,c) ({char *_pp; *(c) = strtol(a,&_pp,b); _pp;})
#define	noop_llseek 0
#define	ETIME ECANCELED
#define	ENOSR ENOBUFS
#define	ENOTSUPP ENOTSUP
#define	EREMOTEIO EIO
#define	EBADRQC EBADMSG
#define	MAX_SCHEDULE_TIMEOUT LONG_MAX
#define	U8_MAX 255U
#define	U8_MIN (-(1 << 7))
#define	U16_MAX 65535U
#define	S16_MAX (65535U / 2U)
#define	S16_MIN (-(1 << 15))
#define	U32_MAX -1U
#define	S32_MAX ((-1U) / 2U)
#define	S32_MIN (-(1 << 31))
#define	I2C_NAME_SIZE 20
#define	__SPIN_LOCK_UNLOCKED(...) {}
#define	in_interrupt() 0
#define	wmb() __nop
#define	min_t(cast,x,y) ((((cast)(x)) < ((cast)(y))) ? (cast)(x) : (cast)(y))
#define	max_t(cast,x,y) ((((cast)(x)) > ((cast)(y))) ? (cast)(x) : (cast)(y))
#define	clamp_val(x,y,z) clamp_t(typeof(x),x,y,z)
#define	clamp(x,y,z) clamp_t(typeof(x),x,y,z)
#define	clamp_t(cast,x,y,z) \
((cast)((((cast)(x)) < ((cast)(y))) ? ((cast)(y)) : \
	(((cast)(x)) > ((cast)(z))) ? ((cast)(z)) : ((cast)(x))))
#define	try_then_request_module(x,...) (x)
#ifndef __cold
#define	__cold __attribute__((__cold__))
#endif
#ifndef __packed
#define	__packed __attribute__((__packed__))
#endif
#ifndef __pure
#define	__pure __attribute__((pure))
#endif
#ifndef __bitwise
#define	__bitwise
#endif
#ifndef __force
#define	__force
#endif
#define	numa_node_id(x) ((int)0)
#define	DEFINE_PER_CPU(type, name) \
    typeof(type) name

#define	put_user(val, ptr) ({						\
	int temp_err;							\
	__typeof(val) temp_var = (val);					\
	if (copy_to_user((ptr), &temp_var, sizeof(temp_var)) == 0)	\
		temp_err = 0;						\
	else								\
		temp_err = -EFAULT;					\
	temp_err;							\
})
#define	__put_user(temp_var, ptr) put_user(temp_var, ptr)
#define	get_user(temp_var, ptr) ({					\
	int temp_err;							\
	if (copy_from_user(&(temp_var), (ptr), sizeof(temp_var)) == 0)	\
		temp_err = 0;						\
	else								\
		temp_err = -EFAULT;					\
	temp_err;							\
})
#define	__get_user(temp_var, ptr) get_user(temp_var, ptr)
extern unsigned long copy_in_user(void *to, const void *from, unsigned long len);
#define	__copy_from_user(kptr, uptr, n) copy_from_user(kptr, uptr, n)
#define	__copy_to_user(uptr, kptr, n) copy_to_user(uptr, kptr, n)
extern unsigned long clear_user(void *uptr, unsigned long n);

#define	__clear_user(ptr, n) clear_user(ptr, n)
#undef errno
#define	errno errno_v4l

/* rcu - support */
#define	RCU_INIT_POINTER(p, v) do {	\
	(p) = (v);			\
} while (0)
#define	rcu_read_lock() atomic_lock()
#define	rcu_read_unlock() atomic_unlock()
#define	rcu_dereference(var)	\
  ({ __typeof(var) __temp;	\
     atomic_lock();		\
     __temp = (var);		\
     atomic_unlock();		\
     __temp; })
#define	rcu_dereference_protected(p,c) rcu_dereference(p)
#define	rcu_dereference_raw(p) rcu_dereference(p)
#define	rcu_assign_pointer(a,b) do { atomic_lock(); (a) = (b); atomic_unlock(); } while (0)
#define	synchronize_rcu() do {	\
	atomic_lock();		\
	atomic_unlock();	\
} while (0)

#define	add_input_randomness(...) __nop

#define	kill_fasync(...) __nop
#define	fasync_helper(...) (0)

#define	get_file(x) __nop

#define	ATOMIC_INIT(x) { (x) }
#define	ATOMIC64_INIT(x) { (x) }

#define	CLOCK_BOOTTIME CLOCK_UPTIME

#define	IRQ_NONE 0
#define	IRQ_HANDLED 1

#define	PLATFORM_DEVID_NONE (-1)
#define	PLATFORM_DEVID_AUTO (-2)

#define	DPM_ORDER_NONE 0

#define	RPM_ACTIVE 0
#define	RPM_SUSPENDED 0

#define	ilog2(n)				\
(						\
	__builtin_constant_p(n) ? (		\
		(n) < 1 ? -1 :			\
		(n) & (1ULL << 63) ? 63 :	\
		(n) & (1ULL << 62) ? 62 :	\
		(n) & (1ULL << 61) ? 61 :	\
		(n) & (1ULL << 60) ? 60 :	\
		(n) & (1ULL << 59) ? 59 :	\
		(n) & (1ULL << 58) ? 58 :	\
		(n) & (1ULL << 57) ? 57 :	\
		(n) & (1ULL << 56) ? 56 :	\
		(n) & (1ULL << 55) ? 55 :	\
		(n) & (1ULL << 54) ? 54 :	\
		(n) & (1ULL << 53) ? 53 :	\
		(n) & (1ULL << 52) ? 52 :	\
		(n) & (1ULL << 51) ? 51 :	\
		(n) & (1ULL << 50) ? 50 :	\
		(n) & (1ULL << 49) ? 49 :	\
		(n) & (1ULL << 48) ? 48 :	\
		(n) & (1ULL << 47) ? 47 :	\
		(n) & (1ULL << 46) ? 46 :	\
		(n) & (1ULL << 45) ? 45 :	\
		(n) & (1ULL << 44) ? 44 :	\
		(n) & (1ULL << 43) ? 43 :	\
		(n) & (1ULL << 42) ? 42 :	\
		(n) & (1ULL << 41) ? 41 :	\
		(n) & (1ULL << 40) ? 40 :	\
		(n) & (1ULL << 39) ? 39 :	\
		(n) & (1ULL << 38) ? 38 :	\
		(n) & (1ULL << 37) ? 37 :	\
		(n) & (1ULL << 36) ? 36 :	\
		(n) & (1ULL << 35) ? 35 :	\
		(n) & (1ULL << 34) ? 34 :	\
		(n) & (1ULL << 33) ? 33 :	\
		(n) & (1ULL << 32) ? 32 :	\
		(n) & (1ULL << 31) ? 31 :	\
		(n) & (1ULL << 30) ? 30 :	\
		(n) & (1ULL << 29) ? 29 :	\
		(n) & (1ULL << 28) ? 28 :	\
		(n) & (1ULL << 27) ? 27 :	\
		(n) & (1ULL << 26) ? 26 :	\
		(n) & (1ULL << 25) ? 25 :	\
		(n) & (1ULL << 24) ? 24 :	\
		(n) & (1ULL << 23) ? 23 :	\
		(n) & (1ULL << 22) ? 22 :	\
		(n) & (1ULL << 21) ? 21 :	\
		(n) & (1ULL << 20) ? 20 :	\
		(n) & (1ULL << 19) ? 19 :	\
		(n) & (1ULL << 18) ? 18 :	\
		(n) & (1ULL << 17) ? 17 :	\
		(n) & (1ULL << 16) ? 16 :	\
		(n) & (1ULL << 15) ? 15 :	\
		(n) & (1ULL << 14) ? 14 :	\
		(n) & (1ULL << 13) ? 13 :	\
		(n) & (1ULL << 12) ? 12 :	\
		(n) & (1ULL << 11) ? 11 :	\
		(n) & (1ULL << 10) ? 10 :	\
		(n) & (1ULL <<  9) ?  9 :	\
		(n) & (1ULL <<  8) ?  8 :	\
		(n) & (1ULL <<  7) ?  7 :	\
		(n) & (1ULL <<  6) ?  6 :	\
		(n) & (1ULL <<  5) ?  5 :	\
		(n) & (1ULL <<  4) ?  4 :	\
		(n) & (1ULL <<  3) ?  3 :	\
		(n) & (1ULL <<  2) ?  2 :	\
		(n) & (1ULL <<  1) ?  1 :	\
		(n) & (1ULL <<  0) ?  0 :	\
		-1) :				\
	(sizeof(n) <= 4) ?			\
	fls((u32)(n)) - 1 : flsll((u64)(n)) - 1	\
)

#if (defined(BYTE_ORDER) && defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN))
#if (BYTE_ORDER == LITTLE_ENDIAN)
#ifndef __LITTLE_ENDIAN
#define	__LITTLE_ENDIAN
#endif
#elif BYTE_ORDER == BIG_ENDIAN
#ifndef __BIG_ENDIAN
#define	__BIG_ENDIAN
#endif
#else
#error "Unknown byte order"
#endif
#endif

struct kernel_param;

typedef uint16_t umode_t;
typedef int8_t __s8;
typedef uint8_t __u8;
typedef int16_t __s16;
typedef uint16_t __u16;
typedef int32_t __s32;
typedef uint32_t __u32;
typedef int64_t __s64;
typedef uint64_t __u64;
typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
typedef uint32_t u32;
typedef int64_t s64;
typedef uint64_t u64;
typedef u64 dma_addr_t;
typedef u64 dma64_addr_t;
typedef u64 sector_t;
typedef uint16_t __le16;
typedef uint32_t __le32;
typedef uint64_t __le64;
typedef uint16_t __be16;
typedef uint32_t __be32;
typedef uint64_t __be64;
typedef uint32_t uint;
typedef int irqreturn_t;
typedef off_t __kernel_off_t;
typedef int __kernel_pid_t;
typedef unsigned long __kernel_ulong_t;
typedef unsigned long phys_addr_t;
typedef unsigned int __poll_t;
#define	__kernel_timespec timespec

#define	fwnode_graph_get_remote_port_parent(...) NULL
#define	fwnode_handle_get(x) x
#define	fwnode_handle_put(...) __nop
#define	dev_fwnode(...) NULL
#define	dmi_match(...) 0

#define	device_remove_groups(...) __nop
#define	device_add_groups(...) (int)0

#ifndef __GLIBC__
typedef long long loff_t;

#endif
typedef uint32_t gfp_t;
typedef struct timespec ktime_t;

#define	timespec64 timespec

static inline int
linux_abs(int a)
{
	return (a < 0 ? -a : a);
}

#undef abs
#define	abs(a) linux_abs(a)

typedef int (*cmp_func_t)(const void *a, const void *b);

#define	DEFINE_SHOW_ATTRIBUTE(...)

#endif					/* _LINUX_DEFS_H_ */
