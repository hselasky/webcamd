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

			if (cdev->fixed_vma[i].vm_ops)
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

	err = cdev->ops->mmap(&cdev->fixed_file, &cdev->fixed_vma[i]);
	if (err) {
		errno = -err;
		return ((void *)-1);
	}
	if (cdev->fixed_vma[i].vm_ops)
		cdev->fixed_vma[i].vm_ops->open(&cdev->fixed_vma[i]);

	return (cdev->fixed_vma[i].vm_buffer_address);
}

/* wrappers for V4L */

PUBLIC_API int
v4lx_open_wrapper(const char *path, int oflag, int mode)
{
	int fd = 0;
	int err;

	if (init_done == 0) {

		linux_init();

		init_done = 1;
	}
	if (usb_linux_probe(fd) != 0) {
		errno = ENODEV;
		fd = -1;
		goto done;
	}
	err = linux_open(usb_linux2cdev(fd));
	if (err < 0) {
		errno = -err;
		fd = -1;
	}
done:
	return (fd);
}

PUBLIC_API int
v4lx_close_wrapper(int fd)
{
	int err;

	err = linux_close(usb_linux2cdev(fd));

	if (err == 0)
		usb_linux_detach(fd);

	return (err);
}

PUBLIC_API int
v4lx_ioctl_wrapper(int fd, unsigned long cmd, void *arg)
{
	return (linux_ioctl(usb_linux2cdev(fd), cmd, arg));
}

PUBLIC_API int
v4lx_read_wrapper(int fd, void *buf, size_t len)
{
	return (linux_read(usb_linux2cdev(fd), buf, len));
}

PUBLIC_API int
v4lx_write_wrapper(int fd, void *buf, size_t len)
{
	return (linux_write(usb_linux2cdev(fd), buf, len));
}

PUBLIC_API void *
v4lx_mmap_wrapper(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
	return (linux_mmap(usb_linux2cdev(fd), addr, len, offset));
}

PUBLIC_API int
v4lx_munmap_wrapper(void *addr, size_t len)
{
	return (0);
}
