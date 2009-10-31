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

#define	LINUX_VMA_MAX 16

typedef struct poll_table_struct {
}	poll_table;

typedef struct {
	volatile unsigned int counter;
} atomic_t;

struct module {

};

struct kref {
	atomic_t refcount;
};

struct attribute {
	const char *name;
	struct module *owner;
	mode_t	mode;
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
	int     (*dev_uevent) (struct device *dev, struct kobj_uevent_env *env);
	struct kref refcount;
};

typedef void device_release_t (struct device *);

struct device {
	int	minor;
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
	char	name[1];
};

typedef struct {
	unsigned long pgprot;
} pgprot_t;

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

struct tasklet_struct {
	struct tasklet_struct *next;
	unsigned long state;
	atomic_t count;
	void    (*func) (unsigned long);
	unsigned long data;
};

#define	iminor(i) (i)->d_inode & 0xFF

typedef struct inode {
	dev_t	d_inode;
} inode_t;

struct cdev {
	const struct file_operations *ops;
	struct module *owner;

	struct dentry fixed_dentry;
	struct inode fixed_inode;
	struct file fixed_file;
	struct vm_area_struct fixed_vma[LINUX_VMA_MAX];
	uint8_t	is_opened;
};

struct poll_wqueues {
	poll_table pt;
	struct poll_table_page *table;
	int	error;
};

#endif					/* _LINUX_STRUCT_H_ */
