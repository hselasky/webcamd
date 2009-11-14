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

static uint8_t init_done;

#include <sys/syscall.h>

int
linux_open(struct cdev *cdev)
{
	uint8_t busy;

	if (cdev == NULL) {
		errno = EINVAL;
		return (-1);
	}
	atomic_lock();
	busy = cdev->is_opened;
	cdev->is_opened = 1;
	atomic_unlock();

	if (busy) {
		errno = EBUSY;
		return (-1);
	}
	if ((errno = -cdev->ops->open(&cdev->fixed_inode, &cdev->fixed_file))) {
		atomic_lock();
		cdev->is_opened = 0;
		atomic_unlock();
		return (-1);
	}
	return (0);
}

int
linux_close(struct cdev *cdev)
{
	uint8_t busy;
	uint32_t i;

	if (cdev == NULL)
		return (0);

	atomic_lock();
	busy = cdev->is_opened;
	cdev->is_opened = 0;
	atomic_unlock();

	if (!busy)
		return (0);

	for (i = 0; i != LINUX_VMA_MAX; i++) {
		if ((cdev->fixed_vma[i].vm_buffer_address != NULL) &&
		    (cdev->fixed_vma[i].vm_buffer_address != (void *)-1)) {

			if (cdev->fixed_vma[i].vm_ops &&
			    cdev->fixed_vma[i].vm_ops->close)
				cdev->fixed_vma[i].vm_ops->close(&cdev->fixed_vma[i]);

			cdev->fixed_vma[i].vm_buffer_address = NULL;
			cdev->fixed_vma[i].vm_ops = NULL;
		}
	}

	return ((cdev->ops->release) (&cdev->fixed_inode, &cdev->fixed_file));
}

int
linux_ioctl(struct cdev *cdev, unsigned int cmd, void *arg)
{
	if (cdev == NULL)
		return (-EINVAL);

	if (cdev->is_opened == 0)
		return (-EINVAL);

	if (cdev->ops->unlocked_ioctl)
		return (cdev->ops->unlocked_ioctl(&cdev->fixed_file,
		    cmd, (long)arg));
	else if (cdev->ops->ioctl)
		return (cdev->ops->ioctl(&cdev->fixed_inode,
		    &cdev->fixed_file, cmd, (long)arg));
	else
		return (-EINVAL);
}

ssize_t
linux_read(struct cdev *cdev, char *ptr, size_t len)
{
	loff_t off = 0;
	int err;

	if (cdev == NULL)
		return (-EINVAL);

	if (cdev->is_opened == 0)
		return (-EINVAL);

	err = cdev->ops->read(&cdev->fixed_file, ptr, len, &off);

	return (err);
}

ssize_t
linux_write(struct cdev *cdev, char *ptr, size_t len)
{
	loff_t off = 0;
	int err;

	if (cdev == NULL)
		return (-EINVAL);

	if (cdev->is_opened == 0)
		return (-EINVAL);

	err = cdev->ops->write(&cdev->fixed_file, ptr, len, &off);

	return (err);
}

void   *
linux_mmap(struct cdev *cdev, uint8_t *addr, size_t len, off_t offset)
{
	int err;
	uint32_t i;

	if (cdev == NULL)
		return ((void *)-1);

	if (cdev->is_opened == 0)
		return ((void *)-1);

	for (i = 0; i != LINUX_VMA_MAX; i++) {
		if ((cdev->fixed_vma[i].vm_buffer_address == NULL) ||
		    (cdev->fixed_vma[i].vm_buffer_address == (void *)-1))
			break;
	}

	if (i == LINUX_VMA_MAX) {
		return ((void *)-1);
	}
	/* round length to nearest page size */
	len = -(-len & -PAGE_SIZE);

	/* fill in information */
	cdev->fixed_vma[i].vm_start = (unsigned long)addr;
	cdev->fixed_vma[i].vm_end = (unsigned long)(addr + len);
	cdev->fixed_vma[i].vm_pgoff = offset >> PAGE_SHIFT;
	cdev->fixed_vma[i].vm_buffer_address = (void *)-1;
	cdev->fixed_vma[i].vm_flags = (VM_WRITE | VM_READ | VM_SHARED);

	err = cdev->ops->mmap(&cdev->fixed_file, &cdev->fixed_vma[i]);
	if (err) {
		errno = -err;
		return ((void *)-1);
	}
	if (cdev->fixed_vma[i].vm_ops &&
	    cdev->fixed_vma[i].vm_ops->open)
		cdev->fixed_vma[i].vm_ops->open(&cdev->fixed_vma[i]);

	return (cdev->fixed_vma[i].vm_buffer_address);
}

/* wrappers for V4L */

PUBLIC_API int
v4lx_open_wrapper(const char *path, int oflag, int mode)
{
	int fd = -1;
	int bus = 0;
	int addr = 0;
	int index = 0;
	int err;

	if (init_done == 0) {
		init_done = 1;
		linux_init();
	}
	if ((strncmp(path, "/dev/", 5) == 0) && (
	    (sscanf(path, "/dev/video%d.%d.%d", &bus, &addr, &index) >= 1) ||
	    (sscanf(path, "/dev/v4l/video%d.%d.%d", &bus, &addr, &index) >= 1)
	    )) {

		fd = usb_linux_probe(bus, addr, index);
		if (fd < 0)
			goto done;

		err = linux_open(usb_linux2cdev(fd));
		if (err < 0) {
			usb_linux_detach(fd);
			errno = -err;
			fd = -1;
		}
	}
done:
	if (fd < 0) {
		fd = syscall(SYS_open, path, oflag, mode);
		if (fd < 0) {
			errno = EINVAL;
			fd = -1;
		}
	}
	return (fd);
}

PUBLIC_API int
v4lx_close_wrapper(int fd)
{
	struct cdev *pc;
	int err;

	pc = usb_linux2cdev(fd);
	if (pc != NULL) {
		linux_close(pc);
		usb_linux_detach(fd);
		err = 0;
		errno = 0;
	} else {
		err = syscall(SYS_close, fd);
		if (err) {
			errno = EINVAL;
			err = -1;
		}
	}
	return (err);
}

PUBLIC_API int
v4lx_ioctl_wrapper(int fd, unsigned long cmd, void *arg)
{
	struct cdev *pc;
	int err;

	pc = usb_linux2cdev(fd);
	if (pc != NULL) {
		err = linux_ioctl(pc, cmd, arg);
		if (err) {
			errno = -err;
			return (-1);
		}
	} else {
		err = syscall(SYS_ioctl, fd, cmd, arg);
		if (err) {
			errno = err;
			return (-1);
		}
	}
	return (err);
}

PUBLIC_API int
v4lx_read_wrapper(int fd, void *buf, size_t len)
{
	struct cdev *pc;
	int err;

	pc = usb_linux2cdev(fd);
	if (pc != NULL) {
		err = linux_read(pc, buf, len);
	} else {
		err = syscall(SYS_read, fd, buf, len);
	}
	if (err < 0) {
		errno = EINVAL;
		err = -1;
	}
	return (err);
}

PUBLIC_API int
v4lx_write_wrapper(int fd, void *buf, size_t len)
{
	struct cdev *pc;
	int err;

	pc = usb_linux2cdev(fd);
	if (pc != NULL) {
		err = linux_write(pc, buf, len);
	} else {
		err = syscall(SYS_write, fd, buf, len);
	}
	if (err < 0) {
		errno = EINVAL;
		err = -1;
	}
	return (err);
}

PUBLIC_API void *
v4lx_mmap_wrapper(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	struct cdev *pc;
	void *retval;

	pc = usb_linux2cdev(fd);
	errno = 0;
	if (pc != NULL) {
		retval = linux_mmap(pc, addr, len, offset);
	} else {
		retval = (void *)(long)syscall(SYS_mmap, addr, len, prot, flags, fd, offset);
	}
	return (retval);
}

PUBLIC_API int
v4lx_munmap_wrapper(void *addr, size_t len)
{
	errno = 0;
	return (0);
}
