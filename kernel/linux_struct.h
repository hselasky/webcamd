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
struct page;
struct cdev;
struct mutex;
struct device_type;
struct dmi_system_id;
struct input_device_id;
struct pci_dev;
struct tasklet_struct;

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
	const struct device_attribute *dev_attrs;
	int     (*dev_uevent) (struct device *, struct kobj_uevent_env *);
	void    (*class_release) (struct class *);
	void    (*dev_release) (struct device *);

	struct kref refcount;
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
	void   *driver_data;
	const struct file_operations *fops;
	struct cdev *cdev;
	struct class *class;
	dev_t	devt;
	char	name[64];
	char	bus_name[32];
	char	bus_id[32];
};

struct device_driver {
	char	name[32];
	struct module *owner;
};

typedef unsigned long pgprot_t;

struct vm_fault {

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
	int     (*flush) (struct file *);
	int     (*release) (struct inode *, struct file *);
	int     (*check_flags) (int);
	int     (*dir_notify) (struct file *filp, unsigned long arg);
	unsigned long (*get_unmapped_area) (struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
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

#define	iminor(i) ((i)->d_inode & 0xFFFFUL)

typedef struct inode {
	dev_t	d_inode;
} inode_t;

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
	F_V4B_MAX,
};

struct cdev_sub {
	struct dentry fixed_dentry;
	struct inode fixed_inode;
	struct file fixed_file;
	struct vm_area_struct fixed_vma[LINUX_VMA_MAX];
	uint8_t	is_opened;
};

struct cdev {
	const struct file_operations *ops;
	struct module *owner;
	struct cdev_sub sub[F_V4B_MAX];
	dev_t	mm_start;
	dev_t	mm_end;
	uint8_t	is_alloced;
};

struct poll_wqueues {
	poll_table pt;
	struct poll_table_page *table;
	int	error;
};

#endif					/* _LINUX_STRUCT_H_ */
