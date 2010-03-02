/*-
 * Copyright (c) 2009-2010 Hans Petter Selasky. All rights reserved.
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

#include <sys/mman.h>
#include <sys/syscall.h>

struct cdev_handle *
linux_open(int f_v4b, int fflags)
{
	struct cdev *cdev = cdev_get_device(f_v4b);
	struct cdev_handle *handle;
	int error;

	if (cdev == NULL)
		return (NULL);

	handle = malloc(sizeof(*handle));
	if (handle == NULL)
		return (NULL);

	memset(handle, 0, sizeof(*handle));

	handle->fixed_file.f_flags = fflags;
	handle->fixed_file.f_op = cdev->ops;
	handle->fixed_dentry.d_inode = &handle->fixed_inode;
	handle->fixed_file.f_path.dentry = &handle->fixed_dentry;
	handle->fixed_inode.d_inode = cdev_get_mm(f_v4b);

	if (cdev->ops->open == NULL) {
		return (handle);
	}
	if ((error = -cdev->ops->open(&handle->fixed_inode, &handle->fixed_file))) {
		free(handle);
		return (NULL);
	}
	return (handle);
}

int
linux_close(struct cdev_handle *handle)
{
	int i;
	int error;

	if (handle == NULL)
		return (0);

	/* release all memory mapped regions */
	for (i = 0; i != LINUX_VMA_MAX; i++) {
		if (handle->fixed_vma[i].vm_buffer_address == NULL)
			continue;
		if (handle->fixed_vma[i].vm_buffer_address == MAP_FAILED)
			continue;

		if (handle->fixed_vma[i].vm_ops &&
		    handle->fixed_vma[i].vm_ops->close)
			handle->fixed_vma[i].vm_ops->close(&handle->fixed_vma[i]);

		handle->fixed_vma[i].vm_buffer_address = NULL;
		handle->fixed_vma[i].vm_ops = NULL;
	}

	if (handle->fixed_file.f_op->release != NULL)
		error = (handle->fixed_file.f_op->release)
		    (&handle->fixed_inode, &handle->fixed_file);
	else
		error = 0;

	free(handle);

	return (error);
}

static void
linux_fix_f_flags(struct file *fp, int fflags)
{
	if (fflags & O_NONBLOCK) {
		if (!(fp->f_flags & O_NONBLOCK))
			fp->f_flags |= O_NONBLOCK;
	} else {
		if (fp->f_flags & O_NONBLOCK)
			fp->f_flags &= ~O_NONBLOCK;
	}
}

int
linux_ioctl(struct cdev_handle *handle, int fflags,
    unsigned int cmd, void *arg)
{
	if (handle == NULL)
		return (-EINVAL);

	linux_fix_f_flags(&handle->fixed_file, fflags);

	if (handle->fixed_file.f_op->unlocked_ioctl != NULL)
		return (handle->fixed_file.f_op->unlocked_ioctl(&handle->fixed_file,
		    cmd, (long)arg));
	else if (handle->fixed_file.f_op->ioctl != NULL)
		return (handle->fixed_file.f_op->ioctl(&handle->fixed_inode,
		    &handle->fixed_file, cmd, (long)arg));
	else
		return (-EINVAL);
}

int
linux_poll(struct cdev_handle *handle)
{
	int error;

	if (handle == NULL)
		return (POLLNVAL);

	if (handle->fixed_file.f_op->poll == NULL)
		return (POLLNVAL);

	error = handle->fixed_file.f_op->poll(&handle->fixed_file, NULL);

	return (error);
}

ssize_t
linux_read(struct cdev_handle *handle, int fflags, char *ptr, size_t len)
{
	loff_t off = 0;
	int error;

	if (handle == NULL)
		return (-EINVAL);

	if (handle->fixed_file.f_op->read == NULL)
		return (-EINVAL);

	linux_fix_f_flags(&handle->fixed_file, fflags);

	error = handle->fixed_file.f_op->read(&handle->fixed_file, ptr, len, &off);

	return (error);
}

ssize_t
linux_write(struct cdev_handle *handle, int fflags, char *ptr, size_t len)
{
	loff_t off = 0;
	int error;

	if (handle == NULL)
		return (-EINVAL);

	if (handle->fixed_file.f_op->write == NULL)
		return (-EINVAL);

	linux_fix_f_flags(&handle->fixed_file, fflags);

	error = handle->fixed_file.f_op->write(&handle->fixed_file, ptr, len, &off);

	return (error);
}

void   *
linux_mmap(struct cdev_handle *handle, int fflags,
    uint8_t *addr, size_t len, off_t offset)
{
	int err;
	uint32_t i;

	if (handle == NULL)
		return (MAP_FAILED);

	/* sanity checks */
	if (len == 0)
		return (MAP_FAILED);

	if (offset & (PAGE_SIZE - 1))
		return (MAP_FAILED);

	/* round up length */
	if (len & (PAGE_SIZE - 1)) {
		len += PAGE_SIZE;
		len &= ~(PAGE_SIZE - 1);
	}
	/* check if the entry is already mapped */
	for (i = 0; i != LINUX_VMA_MAX; i++) {
		if ((handle->fixed_vma[i].vm_end -
		    handle->fixed_vma[i].vm_start) != len)
			continue;
		if (handle->fixed_vma[i].vm_pgoff != (offset >> PAGE_SHIFT))
			continue;
		if (handle->fixed_vma[i].vm_buffer_address == NULL)
			continue;
		if (handle->fixed_vma[i].vm_buffer_address == MAP_FAILED)
			continue;

		return (handle->fixed_vma[i].vm_buffer_address);
	}

	/* create new entry */
	for (i = 0; i != LINUX_VMA_MAX; i++) {
		if ((handle->fixed_vma[i].vm_buffer_address == NULL) ||
		    (handle->fixed_vma[i].vm_buffer_address == MAP_FAILED))
			break;
	}

	if (i == LINUX_VMA_MAX) {
		return (MAP_FAILED);
	}
	/* fill in information */
	handle->fixed_vma[i].vm_start = (unsigned long)addr;
	handle->fixed_vma[i].vm_end = (unsigned long)(addr + len);
	handle->fixed_vma[i].vm_pgoff = (offset >> PAGE_SHIFT);
	handle->fixed_vma[i].vm_buffer_address = MAP_FAILED;
	handle->fixed_vma[i].vm_flags = (VM_WRITE | VM_READ | VM_SHARED);

	err = handle->fixed_file.f_op->mmap(&handle->fixed_file, &handle->fixed_vma[i]);
	if (err) {
		handle->fixed_vma[i].vm_buffer_address = MAP_FAILED;
		return (MAP_FAILED);
	}
	return (handle->fixed_vma[i].vm_buffer_address);
}
