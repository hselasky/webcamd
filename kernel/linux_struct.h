#ifndef _LINUX_STRUCT_H_
#define	_LINUX_STRUCT_H_

struct vm_area_struct;
struct vm_operations_struct;
struct poll_table_page;
struct kobj_uevent_env;
struct module;
struct file;
struct inode;
struct class;
struct device;
struct device_driver;
struct dev_pm_ops;
struct page;
struct cdev;
struct mutex;
struct dmi_system_id;
struct input_device_id;
struct pci_dev;
struct tasklet_struct;
struct usb_driver;
struct usb_interface;
struct pt_regs;

#define	LINUX_VMA_MAX 16

typedef struct poll_table_struct {
}	poll_table;

typedef struct {
	volatile unsigned int counter;
} atomic_t;

struct rcu_head {

};

struct module {

};

struct kobject {
};

struct kobj_uevent_env {
	char	buf[512];
	int	buflen;
};

struct kref {
	atomic_t refcount;
};

struct attribute {
	const char *name;
	struct module *owner;
	mode_t	mode;
};

struct attribute_group {
	const char *name;
	struct attribute **attrs;
};

struct class_attribute {
	struct attribute attr;
	ssize_t (*show) (struct class *, char *buf);
	ssize_t (*store) (struct class *, const char *buf, size_t count);
};

struct device_attribute {
	struct attribute attr;
	ssize_t (*show) (struct device *, struct device_attribute *, char *buf);
	ssize_t (*store) (struct device *, struct device_attribute *, const char *buf, size_t count);
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

#define	__ATTR_NULL { }

#define	DEVICE_ATTR(_name,_mode,_show,_store)	\
struct device_attribute				\
	device_attr_##_name =			\
        __ATTR(_name,_mode,_show,_store)

struct class {
	const char *name;
	const char *nodename;
	const struct device_attribute *dev_attrs;
	int     (*dev_uevent) (struct device *, struct kobj_uevent_env *);
	void    (*class_release) (struct class *);
	void    (*dev_release) (struct device *);
	char   *(*devnode) (struct device *, mode_t *);

	struct kref refcount;
};

struct device_driver {
	const char *name;
	struct module *owner;
	struct dev_pm_ops *pm;
};

typedef void device_release_t (struct device *);

struct device {
	int	minor;
	int	busnum;
	struct kobject kobj;
	struct kref refcount;
	device_release_t *release;
	struct device_driver *driver;
	struct device *parent;
	struct device_type *type;
	void   *platform_data;
	void   *driver_data;
	const struct file_operations *fops;
	struct cdev *cdev;
	struct class *class;
	struct device_driver driver_static;
	dev_t	devt;
	char	name[64];
	char	bus_name[32];
	char	bus_id[32];
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

#define	f_dentry f_path.dentry
#define	iminor(i) ((i)->d_inode & 0xFFFFUL)

typedef struct inode {
	dev_t	d_inode;
	void   *i_private;
	char	i_rdev[0];		/* dummy, must be non-zero */
} inode_t;

#define	F_V4B_SUBDEV_MAX 	8

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

extern struct device_attribute dev_attr_abs;
extern struct device_attribute dev_attr_associate_remote;
extern struct device_attribute dev_attr_bustype;
extern struct device_attribute dev_attr_coordinate_mode;
extern struct device_attribute dev_attr_delay;
extern struct device_attribute dev_attr_diagnostic;
extern struct device_attribute dev_attr_ev;
extern struct device_attribute dev_attr_event_count;
extern struct device_attribute dev_attr_execute;
extern struct device_attribute dev_attr_ff;
extern struct device_attribute dev_attr_firmware_code;
extern struct device_attribute dev_attr_imon_clock;
extern struct device_attribute dev_attr_jitter;
extern struct device_attribute dev_attr_key;
extern struct device_attribute dev_attr_led;
extern struct device_attribute dev_attr_modalias;
extern struct device_attribute dev_attr_model_code;
extern struct device_attribute dev_attr_mouse_left;
extern struct device_attribute dev_attr_mouse_middle;
extern struct device_attribute dev_attr_mouse_right;
extern struct device_attribute dev_attr_msc;
extern struct device_attribute dev_attr_name;
extern struct device_attribute dev_attr_odm_code;
extern struct device_attribute dev_attr_phys;
extern struct device_attribute dev_attr_pointer_mode;
extern struct device_attribute dev_attr_product;
extern struct device_attribute dev_attr_properties;
extern struct device_attribute dev_attr_protocols;
extern struct device_attribute dev_attr_rel;
extern struct device_attribute dev_attr_size;
extern struct device_attribute dev_attr_snd;
extern struct device_attribute dev_attr_stylus_lower;
extern struct device_attribute dev_attr_stylus_upper;
extern struct device_attribute dev_attr_sw;
extern struct device_attribute dev_attr_tool_mode;
extern struct device_attribute dev_attr_uniq;
extern struct device_attribute dev_attr_vendor;
extern struct device_attribute dev_attr_version;
extern struct device_attribute dev_attr_wheel;
extern struct device_attribute dev_attr_xtilt;
extern struct device_attribute dev_attr_ytilt;
extern struct device_attribute dev_attr_status_led0_select;
extern struct device_attribute dev_attr_status_led1_select;
extern struct device_attribute dev_attr_status0_luminance;
extern struct device_attribute dev_attr_status1_luminance;
extern struct device_attribute dev_attr_buttons_luminance;
extern struct device_attribute dev_attr_button0_rawimg;
extern struct device_attribute dev_attr_button1_rawimg;
extern struct device_attribute dev_attr_button2_rawimg;
extern struct device_attribute dev_attr_button3_rawimg;
extern struct device_attribute dev_attr_button4_rawimg;
extern struct device_attribute dev_attr_button5_rawimg;
extern struct device_attribute dev_attr_button6_rawimg;
extern struct device_attribute dev_attr_button7_rawimg;

#endif					/* _LINUX_STRUCT_H_ */
