/*-
 * Copyright (c) 2009 Hans Petter Selasky. All rights reserved.
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

#define	DIV_ROUND_CLOSEST(rem, div) \
((sizeof(typeof(div)) > 4) ?	    \
 ((((typeof(div))-1) <= 0) ?	    \
  div_round_closest_s64(rem,div) :  \
  div_round_closest_u64(rem,div)) : \
 ((((typeof(div))-1) <= 0) ?	    \
  div_round_closest_s32(rem,div) :  \
  div_round_closest_u32(rem,div)))

#define	mult_frac(x, numer, denom)			\
({							\
	typeof(x) quot = (x) / (denom);                 \
	typeof(x) rem  = (x) % (denom);                 \
        (quot * (numer)) + ((rem * (numer)) / (denom)); \
})

#define	is_power_of_2(x) (((-(x)) & (x)) == (x))
#define	__nop do {} while (0)
#define	__user
#define	__kernel
#define	__safe
#define	__deprecated
#define	__force
#define	__nocast
#define	__iomem
#define	__must_check
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
#define	__stringify(x)	#x
#define	ERESTARTSYS     512
#define	ENOIOCTLCMD     513
#define	EMEDIUMTYPE	514
#define	ENODATA		515
#define	symbol_request(x) (&(x))
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
#define	IS_MODULE(...) 0
#define	IS_BUILTIN(x,...) defined(x##__VA_ARGS__)
#define	IS_ENABLED(x,...) defined(x##__VA_ARGS__)
#define	THIS_MODULE (NULL)
#ifdef HAVE_DEBUG
#define	printk(...) printf(__VA_ARGS__)
#else
#define	printk(...) printk_nop()
#endif
#define	print_hex_dump_bytes(...) printk_nop()
#define	printk_ratelimit(...) printk_nop()
#define	printk_timed_ratelimit(...) printk_nop()
#define	pr_err_ratelimited(...) __nop
#define	pr_cont(...) __nop
#define	pr_err(...) __nop
#define	pr_info(...) __nop
#define	pr_dbg(...) __nop
#define	pr_debug(...) __nop
#define	pr_warn(...) __nop
#define	pr_warning(...) __nop
#define	pr_emerg(...) __nop
#define	dev_dbg(dev, fmt, ...) printk("DBG: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_debug(dev, fmt, ...) printk("DBG: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_dbg_ratelimited(dev, fmt, ...) printk("DBG: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_err(dev, fmt, ...) printk("ERR: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_err_ratelimited(dev, fmt, ...) printk("ERR: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_info(dev, fmt, ...) printk("INFO: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_info_ratelimited(dev, fmt, ...) printk("INFO: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_warn(dev, fmt, ...) printk("WARN: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_warn_ratelimited(dev, fmt, ...) printk("WARN: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_notice(dev, fmt, ...) printk("NOTICE: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	dev_notice_ratelimited(dev, fmt, ...) printk("NOTICE: %s: " fmt, dev_name(dev),## __VA_ARGS__)
#define	info(fmt, ...) printk("INFO: " fmt "\n",## __VA_ARGS__)
#define	warn(fmt, ...) printk("WARN: " fmt "\n",## __VA_ARGS__)
#define	dbg(fmt, ...) printk("DBG: " fmt "\n",## __VA_ARGS__)
#define	err(fmt, ...) printk("ERR: " fmt "\n",## __VA_ARGS__)
#define	notice(fmt, ...) printk("NOTICE: " fmt "\n",## __VA_ARGS__)
#define	kmem_cache_create(desc,size,align,arg,fn) ((struct kmem_cache *)(size))
#define	kmem_cache_destroy(...) __nop
#define	kmem_cache_free(ref,ptr) free(ptr)
#define	kmem_cache_alloc(ref,g) malloc((long)(ref))
#define	kmem_cache_zalloc(ref,g) calloc(1, (long)(ref))
#define	kmalloc(s,opt) malloc(s)
#define	kzalloc(s,opt) calloc(1, (s))
#define	krealloc(p,s,opt) realloc(p,(s))
#define	dma_alloc_coherent(d,s,h,g) calloc(1,(s))
#define	dma_free_coherent(d,s,v,h) free(v)
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
#define	sysfs_attr_init(x) __nop
#define	kobject_get_path(...) strdup("webcamd")
#define	kobject_put(...) __nop
#define	kobject_get(...) __nop
#define	kobject_uevent(...) __nop
#define	vfree(ptr) free_vm(ptr)
#define	kfree(ptr) free(ptr)
#define	kstrdup(a,b) strdup(a)
#define	might_sleep(x) __nop
#define	might_sleep_if(x) __nop
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
#define	no_llseek	NULL
#define	default_llseek	NULL
#define	GENMASK(lo, hi) (((1ULL << ((hi) - (lo) + 1)) - 1) << (lo))
#define	BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#define	BIT_WORD(nr) ((nr) / BITS_PER_LONG)
#define	BITS_PER_BYTE 8
#ifndef BITS_PER_LONG
#define	BITS_PER_LONG (sizeof(long) * BITS_PER_BYTE)
#endif
#define	for_each_set_bit_from(b, addr, size) \
    for ( ; (b) < (size); (b)++) \
	if ((addr)[(b) / BITS_PER_LONG] & BIT((b) % BITS_PER_LONG))
#define	BITS_TO_LONGS(n) (((n) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define	BIT(n) (1UL << (n))
#define	KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define	LINUX_VERSION_CODE KERNEL_VERSION(2, 6, 38)
#define	BUS_ID_SIZE 32
#define	DECLARE_BITMAP(n, max) unsigned long n[((max)+BITS_PER_LONG-1)/BITS_PER_LONG]
#define	MKDEV(maj,min) ((dev_t)((((maj) & 0xFFFFUL) << 16)|((min) & 0xFFFFUL)))
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
#define	DEFAULT_POLLMASK POLLNVAL
#define	POLL_ERR POLLERR
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
#define	ARRAY_SIZE(ptr) (sizeof(ptr) / sizeof((ptr)[0]))
#define	__KERNEL__
#define	capable(...) 1
#define	uninitialized_var(...) __VA_ARGS__
#define	HZ 1000
#define	NSEC_PER_SEC 1000000000LL
#define	jiffies get_jiffies_64()
#define	msecs_to_jiffies(x) (x)
#define	usecs_to_jiffies(x) ((x) / 1000)
#define	jiffies_to_msecs(x) (x)
#define	likely(...) __VA_ARGS__
#define	unlikely(...) __VA_ARGS__
#define	__round_mask(x, y) ((typeof(x))((y)-1))
#define	round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define	round_down(x, y) ((x) & ~__round_mask(x, y))
#define	DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define	min(a,b) (((a) < (b)) ? (a) : (b))
#define	max(a,b) (((a) > (b)) ? (a) : (b))
#define	prefetch(x) (void)x
#define	KERN_INFO ""
#define	KERN_WARNING ""
#define	KERN_ERR ""
#define	KERN_DEBUG ""
#define	KERN_CONT ""
#define	KBUILD_MODNAME ""
#define	KERN_NOTICE ""
#define	BUG(...) __nop
#define	WARN(x,...) ({ (x); })
#define	WARN_ONCE(x,...) ({ (x); })
#define	BUG_ON(x) ({ (x); })
#define	WARN_ON(x) ({ (x); })
#define	WARN_ON_ONCE(x) ({ (x); })
#define	BUILD_BUG_ON(x) extern int dummy_array[(x) ? -1 : 1]
#define	lockdep_set_class_and_name(...) __nop
#define	lockdep_assert_held(...) __nop
#define	lock_kernel(...) __nop
#define	unlock_kernel(...) __nop
#define	spin_lock_init(lock) __nop
#define	spin_lock_irqsave(l,f)  do { (f) = 1; atomic_lock(); } while (0)
#define	spin_unlock_irqrestore(l,f) do { if (f) { (f) = 0; atomic_unlock(); } } while (0)
#define	spin_lock(...)  atomic_lock()
#define	spin_unlock(...) atomic_unlock()
#define	spin_lock_irq(...)  atomic_lock()
#define	spin_unlock_irq(...) atomic_unlock()
#define	raw_spin_lock_init(lock) __nop
#define	raw_spin_lock(...)  atomic_lock()
#define	raw_spin_unlock(...) atomic_unlock()
#define	atomic_inc_return atomic_inc
#define	atomic_dec_return atomic_dec
#define	assert_spin_locked(...) __nop
#define	IS_ERR_VALUE(x) ((unsigned long)(x) >= (unsigned long)-(1<<14))
#define	IS_ERR_OR_NULL(x) ((unsigned long)(x) == 0 || IS_ERR_VALUE(x))
#define	ERR_CAST(x) ((void *)(long)(const void *)(x))
#define	find_first_bit(addr, size) find_next_bit((addr), (size), 0)
#define	find_first_zero_bit(addr, size) find_next_zero_bit((addr), (size), 0)
#define	synchronize_sched() do { atomic_lock(); atomic_unlock(); } while (0)
#define	signal_pending(...) check_signal()
#define	down_write(...) __nop
#define	down_read(...) __nop
#define	up_write(...) __nop
#define	up_read(...) __nop
#define	dump_stack(...) __nop
#define	get_user_pages(a,b,...) linux_get_user_pages(__VA_ARGS__)
#define	DECLARE_RWSEM(x) struct semaphore x
#define	crc32(s, d, l) crc32_le(s, (unsigned char const *)d, l)
#define	CRCPOLY_LE 0xedb88320
#define	CRCPOLY_BE 0x04c11db7
#define	mb() __asm volatile("":::"memory")
#define	smp_wmb() mb()
#define	smp_mb() mb()
#define	smp_rmb() mb()
#define	smp_mb__after_clear_bit() mb()
#define	smp_mb__after_atomic() mb()
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
#define	get_user(temp_var, ptr) ({					\
	int temp_err;							\
	if (copy_from_user(&(temp_var), (ptr), sizeof(temp_var)) == 0)	\
		temp_err = 0;						\
	else								\
		temp_err = -EFAULT;					\
	temp_err;							\
})
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
#define	list_for_each_entry_rcu(a,b,c) list_for_each_entry(a,b,c)
#define	list_add_rcu(a,b) list_add(a,b)
#define	list_add_tail_rcu(a,b) list_add_tail(a,b)
#define	list_del_rcu(a) list_del(a)
#define	synchronize_rcu() __nop

#define	add_input_randomness(...) __nop

#define	kill_fasync(...) __nop
#define	fasync_helper(...) (0)

#define	get_file(x) __nop

#define	ATOMIC_INIT(x) { (x) }

#define	IRQ_NONE 0
#define	IRQ_HANDLED 1

#define	PLATFORM_DEVID_NONE (-1)
#define	PLATFORM_DEVID_AUTO (-2)

#define	DPM_ORDER_NONE 0

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
typedef unsigned long phys_addr_t;

#ifndef __GLIBC__
typedef long long loff_t;

#endif
typedef uint32_t gfp_t;
typedef uint32_t dev_t;
typedef struct timespec ktime_t;

#endif					/* _LINUX_DEFS_H_ */
