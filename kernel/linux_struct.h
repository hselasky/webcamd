#ifndef _LINUX_STRUCT_H_
#define	_LINUX_STRUCT_H_

struct bus_type;
struct cdev;
struct class;
struct cpumask;
struct device;
struct device_driver;
struct device_node;
struct dmi_system_id;
struct fb_videomode;
struct file;
struct inode;
struct input_device_id;
struct kmem_cache;
struct kobj_uevent_env;
struct lock_class_key;
struct module;
struct mutex;
struct page;
struct pci_dev;
struct poll_table_page;
struct pt_regs;
struct seq_file;
struct snd_pcm_runtime;
struct spi_board_info;
struct spi_device;
struct spi_master;
struct tasklet_struct;
struct usb_driver;
struct usb_interface;
struct vm_area_struct;
struct vm_operations_struct;
struct vfsmount;

#define	LINUX_VMA_MAX 128

#define	SET_SYSTEM_SLEEP_PM_OPS(...)
#define	SET_RUNTIME_PM_OPS(...)
#define	SIMPLE_DEV_PM_OPS(name, ...) \
const struct dev_pm_ops __maybe_unused name = { }

struct dev_pm_ops {
};

struct notifier_block {
	int     (*notifier_call) (struct notifier_block *, unsigned long, void *);
	struct notifier_block *next;
	int	priority;
};

enum dma_data_direction {
	DMA_DATA_DIRECTION_DUMMY
};

struct dma_buf {
	size_t size;
};

typedef struct pm_message {
	int	event;
} pm_message_t;

typedef struct poll_table_struct {
}	poll_table;

typedef struct {
	volatile unsigned int counter;
} atomic_t;

typedef struct {
	volatile uint64_t counter;
} atomic64_t;

struct module {
};

struct kobject {
	struct kobject *parent;
};

struct kobj_uevent_env {
	char	buf[512];
	int	buflen;
};

struct kref {
	atomic_t refcount;
};

typedef struct refcount_struct {
	atomic_t refs;
} refcount_t;

struct attribute {
	const char *name;
	struct module *owner;
	mode_t	mode;
};

struct kobj_attribute {
	struct attribute attr;
	ssize_t (*show)(struct kobject *, struct kobj_attribute *, char *);
	ssize_t (*store)(struct kobject *, struct kobj_attribute *, const char *, size_t);
};

struct bin_attribute {
	struct attribute attr;
	size_t	size;
	void   *private;
	ssize_t (*read) (struct file *, struct kobject *, struct bin_attribute *,
	    	char  *, loff_t, size_t);
	ssize_t (*write) (struct file *, struct kobject *, struct bin_attribute *,
	    	char  *, loff_t, size_t);
	int     (*mmap) (struct file *, struct kobject *, struct bin_attribute *,
	    	struct	vm_area_struct *);
};

struct attribute_group {
	const char *name;
	struct attribute **attrs;
	struct bin_attribute **bin_attrs;
	umode_t (*is_visible)(struct kobject *, struct attribute *, int);
};

#define	__ATTRIBUTE_GROUPS(name)				\
static const struct attribute_group *name##_groups[] = {	\
	&name##_group,						\
	NULL,							\
}

#define	ATTRIBUTE_GROUPS(name)					\
static const struct attribute_group name##_group = {		\
	.attrs = name##_attrs,					\
};								\
static const struct attribute_group *name##_groups[] = {	\
	&name##_group,						\
	NULL,							\
}

struct class_attribute {
	struct attribute attr;
	ssize_t (*show) (struct class *, char *buf);
	ssize_t (*store) (struct class *, const char *buf, size_t count);
};

struct device_attribute {
	struct attribute attr;
	ssize_t (*show) (struct device *, struct device_attribute *, char *);
	ssize_t (*store) (struct device *, struct device_attribute *, const char *, size_t);
};

struct bus_attribute {
	struct attribute attr;
	ssize_t (*show) (struct bus_type *, char *);
	ssize_t (*store) (struct bus_type *, const char *, size_t);
};

struct driver_attribute {
	struct attribute attr;
	ssize_t (*show) (struct device_driver *, char *);
	ssize_t (*store) (struct device_driver *, const char *, size_t);
};

struct device_type {
	const char *name;
	const struct attribute_group **groups;
	int     (*uevent) (struct device *dev, struct kobj_uevent_env *env);
	char   *(*devnode) (struct device *dev, mode_t *mode);
	void    (*release) (struct device *dev);

	const struct dev_pm_ops *pm;
};

#define	__ATTR(_name,_mode,_show,_store) {		\
        .attr = {.name = __stringify(_name),		\
	    .mode = _mode, .owner = THIS_MODULE },	\
        .show   = _show,				\
        .store  = _store,				\
}

#define	__ATTR_RO(_n) {					\
	.attr = { .name = __stringify(_n),		\
	    .mode = 0444, .owner = THIS_MODULE },	\
	.show = _n##_show,				\
}

#define	__ATTR_WO(_n) {					\
	.attr = { .name = __stringify(_n),		\
	    .mode = 0222, .owner = THIS_MODULE },	\
	.store = _n##_store,				\
}

#define	__ATTR_RW(_n)				\
	__ATTR(_n, 0666, _n##_show, _n##_store)

#define	__ATTR_NULL { }

#define	__BIN_ATTR(_name, _mode, _read, _write, _size)		\
{								\
	.attr = { .name = __stringify(_name), .mode = _mode },	\
	.read	= (_read),					\
	.write	= (_write),					\
	.size	= (_size),					\
}

#define	DEVICE_ATTR(_name,_mode,_show,_store)	\
struct device_attribute				\
	device_attr_##_name =			\
        __ATTR(_name,_mode,_show,_store)

#define	DEVICE_ATTR_RO(_name)				\
struct device_attribute					\
	device_attr_##_name = __ATTR_RO(_name)

#define	DEVICE_ATTR_RW(_name)				\
struct device_attribute					\
	device_attr_##_name = __ATTR_RW(_name)

#define	DRIVER_ATTR(_name,_mode,_show,_store)	\
struct driver_attribute				\
	driver_attr_##_name =			\
        __ATTR(_name,_mode,_show,_store)

#define	DRIVER_ATTR_RO(_name)				\
struct driver_attribute					\
	driver_attr_##_name = __ATTR_RO(_name)

#define	DRIVER_ATTR_WO(_name)				\
struct driver_attribute					\
	driver_attr_##_name = __ATTR_WO(_name)

#define	DRIVER_ATTR_RW(_name)				\
struct driver_attribute					\
	driver_attr_##_name = __ATTR_RW(_name)

#define	BIN_ATTR(_name, _mode, _read, _write, _size)	\
struct bin_attribute bin_attr_##_name =			\
    __BIN_ATTR(_name, _mode, _read, _write, _size)

struct class {
	const char *name;
	const char *nodename;
	const struct device_attribute *dev_attrs;
	const struct attribute_group **dev_groups;
	int     (*dev_uevent) (struct device *, struct kobj_uevent_env *);
	void    (*class_release) (struct class *);
	void    (*dev_release) (struct device *);
	char   *(*devnode) (struct device *, mode_t *);

	struct kref refcount;
};

struct device_driver {
	const char *name;
	struct module *owner;
	const struct dev_pm_ops *pm;
	struct bus_type *bus;
	const char *mod_name;
	void *of_match_table;
	uint8_t suppress_bind_attrs;

	TAILQ_ENTRY(device_driver) entry;
};

typedef void device_release_t (struct device *);

typedef void (*dr_release_t)(struct device *dev, void *res);
typedef int (*dr_match_t)(struct device *dev, void *res, void *match_data);

struct bus_type {
	const char *name;
	const char *dev_name;
	struct device *dev_root;
	struct bus_attribute *bus_attrs;
	struct device_attribute *dev_attrs;
	struct driver_attribute *drv_attrs;
	const struct attribute_group **bus_groups;
	const struct attribute_group **dev_groups;
	const struct attribute_group **drv_groups;

	int     (*match) (struct device *dev, struct device_driver *drv);
	int     (*uevent) (struct device *dev, struct kobj_uevent_env *env);
	int     (*probe) (struct device *dev);
	int     (*remove) (struct device *dev);
	void    (*shutdown) (struct device *dev);

	int     (*suspend) (struct device *dev, pm_message_t state);
	int     (*resume) (struct device *dev);

	const struct dev_pm_ops *pm;

	TAILQ_ENTRY(bus_type) entry;
};

struct device {
	int	minor;
	int	busnum;
	struct kobject kobj;
	struct kref refcount;
	device_release_t *release;
	struct device_driver *driver;
	struct device *parent;
	const struct device_type *type;
	void   *platform_data;
	void   *driver_data;
	void   *of_node;
	void   *fwnode;
	const struct file_operations *fops;
	const struct attribute_group **groups;
	struct bus_type *bus;
	struct cdev *cdev;
	struct class *class;
	struct device_driver driver_static;
	dev_t	devt;
	char	name[64];
	char	bus_name[32];
	char	bus_id[32];
};

struct pci_dev {
	struct device dev;
};

struct platform_device {
	struct device dev;
};

struct platform_driver {
	int (*probe)(struct platform_device *);
	int (*remove)(struct platform_device *);
	struct device_driver driver;
	const struct platform_device_id *id_table;
};

typedef unsigned long pgprot_t;
typedef unsigned long fl_owner_t;

struct vm_fault {
	void   *virtual_address;
	struct page *page;
};

struct vm_area_struct {
	unsigned long vm_start;
	unsigned long vm_end;
	unsigned long vm_pgoff;
	void   *vm_private_data;
	const struct vm_operations_struct *vm_ops;
	uint32_t vm_flags;
	pgprot_t vm_page_prot;
	void   *vm_buffer_address;
};

struct vm_operations_struct {
	void    (*open) (struct vm_area_struct *vma);
	void    (*close) (struct vm_area_struct *vma);
	int     (*fault) (struct vm_area_struct *vma, struct vm_fault *vmf);
};

typedef struct spinlock {
} spinlock_t;

typedef struct rw_semaphore {
} rw_semaphore_t;

typedef struct rwlock {
} rwlock_t;

typedef struct raw_spinlock {
} raw_spinlock_t;

struct file_operations {
	struct module *owner;
	loff_t  (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	unsigned int (*poll) (struct file *, poll_table *);
	int     (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
	long    (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long    (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	int     (*mmap) (struct file *, struct vm_area_struct *);
	int     (*open) (struct inode *, struct file *);
	int     (*flush) (struct file *, fl_owner_t);
	int     (*fasync) (int, struct file *, int);
	int     (*release) (struct inode *, struct file *);
	int     (*check_flags) (int);
	int     (*dir_notify) (struct file *filp, unsigned long arg);
	unsigned long (*get_unmapped_area) (struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
};

struct fasync_struct {
};

struct dentry {
	struct inode *d_inode;
};

struct file {
	void   *private_data;
	const struct file_operations *f_op;
	struct {
		struct dentry *dentry;
	}	f_path;
	int	f_flags;
};

#define	file_inode(_f) (_f)->f_dentry->d_inode
#define	f_dentry f_path.dentry
#define	iminor(i) ((i)->d_inode & 0xFFFFUL)

typedef struct inode {
	dev_t	d_inode;
	void   *i_private;
	char	i_rdev[0];		/* dummy, must be non-zero */
	struct cdev *i_cdev;		/* pointer to cdev */
} inode_t;

#define	F_V4B_SUBDEV_MAX 	8
#define	F_V4B_SUBSUBDEV_MAX 	4

enum {
	F_V4B_VIDEO,
	F_V4B_DVB_AUDIO,
	F_V4B_DVB_CA,
	F_V4B_DVB_DEMUX,
	F_V4B_DVB_DVR,
	F_V4B_DVB_FRONTEND,
	F_V4B_DVB_OSD,
	F_V4B_DVB_SEC,
	F_V4B_DVB_VIDEO,
	F_V4B_LIRC,
	F_V4B_EVDEV,
	F_V4B_JOYDEV,
	F_V4B_ROCCAT,
	F_V4B_MAX,
};

struct cdev_handle {
	struct dentry fixed_dentry;
	struct inode fixed_inode;
	struct file fixed_file;
	struct vm_area_struct fixed_vma[LINUX_VMA_MAX];
};

struct cdev {
	const struct file_operations *ops;
	struct module *owner;
	dev_t	mm_start;
	dev_t	mm_end;
	struct kobject kobj;
	uint8_t	is_alloced;
};

struct poll_wqueues {
	poll_table pt;
	struct poll_table_page *table;
	int	error;
};

struct usb_class_driver {
	char   *name;
	char   *(*devnode) (struct device *dev, mode_t *mode);
	const struct file_operations *fops;
	int	minor_base;
};

struct scatterlist {
};

struct va_format {
	const char *fmt;
	va_list *va;
};

struct old_timespec32 {
	int32_t tv_sec;
	int32_t tv_nsec;
};

struct old_timeval32 {
	int32_t tv_sec;
	int32_t tv_usec;
};

extern struct device_attribute dev_attr_abs;
extern struct device_attribute dev_attr_activate_slack;
extern struct device_attribute dev_attr_activation_height;
extern struct device_attribute dev_attr_activation_width;
extern struct device_attribute dev_attr_actual_cpi;
extern struct device_attribute dev_attr_actual_dpi;
extern struct device_attribute dev_attr_actual_profile;
extern struct device_attribute dev_attr_actual_sensitivity_x;
extern struct device_attribute dev_attr_actual_sensitivity_y;
extern struct device_attribute dev_attr_associate_remote;
extern struct device_attribute dev_attr_builtin_power_supply;
extern struct device_attribute dev_attr_bustype;
extern struct device_attribute dev_attr_button0_rawimg;
extern struct device_attribute dev_attr_button1_rawimg;
extern struct device_attribute dev_attr_button2_rawimg;
extern struct device_attribute dev_attr_button3_rawimg;
extern struct device_attribute dev_attr_button4_rawimg;
extern struct device_attribute dev_attr_button5_rawimg;
extern struct device_attribute dev_attr_button6_rawimg;
extern struct device_attribute dev_attr_button7_rawimg;
extern struct device_attribute dev_attr_buttons_luminance;
extern struct device_attribute dev_attr_color;
extern struct device_attribute dev_attr_coordinate_mode;
extern struct device_attribute dev_attr_deactivate_slack;
extern struct device_attribute dev_attr_debug;
extern struct device_attribute dev_attr_dev_debug;
extern struct device_attribute dev_attr_delay;
extern struct device_attribute dev_attr_diagnostic;
extern struct device_attribute dev_attr_ev;
extern struct device_attribute dev_attr_event_count;
extern struct device_attribute dev_attr_execute;
extern struct device_attribute dev_attr_ff;
extern struct device_attribute dev_attr_firmware_code;
extern struct device_attribute dev_attr_firmware_version;
extern struct device_attribute dev_attr_imon_clock;
extern struct device_attribute dev_attr_index;
extern struct device_attribute dev_attr_jitter;
extern struct device_attribute dev_attr_key;
extern struct device_attribute dev_attr_key_mask;
extern struct device_attribute dev_attr_led;
extern struct device_attribute dev_attr_max;
extern struct device_attribute dev_attr_max_power;
extern struct device_attribute dev_attr_min;
extern struct device_attribute dev_attr_min_height;
extern struct device_attribute dev_attr_min_width;
extern struct device_attribute dev_attr_modalias;
extern struct device_attribute dev_attr_mode;
extern struct device_attribute dev_attr_mode_key;
extern struct device_attribute dev_attr_model_code;
extern struct device_attribute dev_attr_mouse_left;
extern struct device_attribute dev_attr_mouse_middle;
extern struct device_attribute dev_attr_mouse_right;
extern struct device_attribute dev_attr_msc;
extern struct device_attribute dev_attr_name;
extern struct device_attribute dev_attr_odm_code;
extern struct device_attribute dev_attr_phys;
extern struct device_attribute dev_attr_pointer_mode;
extern struct device_attribute dev_attr_poll;
extern struct device_attribute dev_attr_power_mode;
extern struct device_attribute dev_attr_product;
extern struct device_attribute dev_attr_product_id;
extern struct device_attribute dev_attr_properties;
extern struct device_attribute dev_attr_protocols;
extern struct device_attribute dev_attr_quirks;
extern struct device_attribute dev_attr_rel;
extern struct device_attribute dev_attr_release_version;
extern struct device_attribute dev_attr_sensor_logical_height;
extern struct device_attribute dev_attr_sensor_logical_width;
extern struct device_attribute dev_attr_sensor_physical_height;
extern struct device_attribute dev_attr_sensor_physical_width;
extern struct device_attribute dev_attr_size;
extern struct device_attribute dev_attr_snd;
extern struct device_attribute dev_attr_startup_profile;
extern struct device_attribute dev_attr_status0_luminance;
extern struct device_attribute dev_attr_status1_luminance;
extern struct device_attribute dev_attr_status_led0_select;
extern struct device_attribute dev_attr_status_led1_select;
extern struct device_attribute dev_attr_stylus_lower;
extern struct device_attribute dev_attr_stylus_upper;
extern struct device_attribute dev_attr_sw;
extern struct device_attribute dev_attr_tcu;
extern struct device_attribute dev_attr_tool_mode;
extern struct device_attribute dev_attr_uniq;
extern struct device_attribute dev_attr_vendor;
extern struct device_attribute dev_attr_vendor_id;
extern struct device_attribute dev_attr_version;
extern struct device_attribute dev_attr_wakeup_protocols;
extern struct device_attribute dev_attr_weight;
extern struct device_attribute dev_attr_wheel;
extern struct device_attribute dev_attr_xtilt;
extern struct device_attribute dev_attr_ytilt;

#endif					/* _LINUX_STRUCT_H_ */
