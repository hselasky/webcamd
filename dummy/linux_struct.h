#ifndef _LINUX_STRUCT_H_
#define	_LINUX_STRUCT_H_

struct poll_table_struct;
struct vm_area_struct;
struct module;
struct file;
struct inode;
struct class;
struct device;
struct page;

typedef struct {
	volatile unsigned int counter;
} atomic_t;

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
};

struct cdev {
	const struct file_operations *ops;
	struct module *owner;
};

typedef void device_release_t (struct device *);

struct device {
	int	minor;
	device_release_t *release;
	struct device *parent;
	void   *driver_data;
	const struct file_operations *fops;
	struct cdev *cdev;
	struct class *class;
	dev_t	devt;
	char	name[64];
};

struct vm_area_struct {
	unsigned long vm_start;
	unsigned long vm_end;
};

typedef struct {
	unsigned long pgprot;
} pgprot_t;

typedef struct spinlock {
} spinlock_t;

struct file_operations {
	struct module *owner;
	loff_t  (*llseek) (struct file *, loff_t, int);
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
	unsigned int (*poll) (struct file *, struct poll_table_struct *);
	int     (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
	long    (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
	long    (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	int     (*mmap) (struct file *, struct vm_area_struct *);
	int     (*open) (struct inode *, struct file *);
	int     (*flush) (struct file *);
	int     (*release) (struct inode *, struct file *);
	int     (*check_flags) (int);
	int     (*dir_notify) (struct file *filp, unsigned long arg);
};

struct dentry {
	dev_t	d_inode;
};

struct file {
	void   *private_data;
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

#define	DEFINE_MUTEX(n) struct mutex n = { };
struct mutex {
	/* TODO */
};

typedef struct wait_queue_head {
	/* TODO */
} wait_queue_head_t;

typedef struct poll_table {
	/* TODO */
}	poll_table;

typedef struct timer_list {
	/* TODO */
} timer_list_t;

#endif					/* _LINUX_STRUCT_H_ */
