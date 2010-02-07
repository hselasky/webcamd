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

int
linux_open(int f_v4b)
{
	struct cdev *cdev = cdev_get_device(f_v4b);
	struct cdev_sub *sub;
	uint8_t busy;

	if (cdev == NULL) {
		return (-1);
	}
	sub = &cdev->sub[f_v4b];

	atomic_lock();
	busy = sub->is_opened;
	sub->is_opened = 1;
	atomic_unlock();

	if (busy) {
		return (-1);
	}
	sub->fixed_file.f_flags = O_RDWR;	/* XXX */
	sub->fixed_file.f_op = cdev->ops;	/* reset */

	if (cdev->ops->open == NULL) {
		return (0);
	}
	if ((errno = -cdev->ops->open(&sub->fixed_inode, &sub->fixed_file))) {
		atomic_lock();
		sub->is_opened = 0;
		atomic_unlock();
		return (-1);
	}
	return (0);
}

int
linux_close(int f_v4b)
{
	struct cdev *cdev = cdev_get_device(f_v4b);
	struct cdev_sub *sub;
	uint8_t busy;
	uint32_t i;

	if (cdev == NULL)
		return (0);

	sub = &cdev->sub[f_v4b];

	atomic_lock();
	busy = sub->is_opened;
	sub->is_opened = 0;
	atomic_unlock();

	if (!busy)
		return (0);

	/* release all memory mapped regions */
	for (i = 0; i != LINUX_VMA_MAX; i++) {
		if (sub->fixed_vma[i].vm_buffer_address == NULL)
			continue;
		if (sub->fixed_vma[i].vm_buffer_address == MAP_FAILED)
			continue;

		if (sub->fixed_vma[i].vm_ops &&
		    sub->fixed_vma[i].vm_ops->close)
			sub->fixed_vma[i].vm_ops->close(&sub->fixed_vma[i]);

		sub->fixed_vma[i].vm_buffer_address = NULL;
		sub->fixed_vma[i].vm_ops = NULL;
	}

	if (sub->fixed_file.f_op->release == NULL)
		return (0);

	return ((sub->fixed_file.f_op->release) (&sub->fixed_inode, &sub->fixed_file));
}

int
linux_ioctl(int f_v4b, unsigned int cmd, void *arg)
{
	struct cdev *cdev = cdev_get_device(f_v4b);
	struct cdev_sub *sub;

	if (cdev == NULL)
		return (-EINVAL);

	sub = &cdev->sub[f_v4b];

	if (sub->is_opened == 0)
		return (-EINVAL);

	if (sub->fixed_file.f_op->unlocked_ioctl)
		return (sub->fixed_file.f_op->unlocked_ioctl(&sub->fixed_file,
		    cmd, (long)arg));
	else if (sub->fixed_file.f_op->ioctl)
		return (sub->fixed_file.f_op->ioctl(&sub->fixed_inode,
		    &sub->fixed_file, cmd, (long)arg));
	else
		return (-EINVAL);
}

ssize_t
linux_read(int f_v4b, char *ptr, size_t len)
{
	struct cdev *cdev = cdev_get_device(f_v4b);
	struct cdev_sub *sub;
	loff_t off = 0;
	int err;

	if (cdev == NULL)
		return (-EINVAL);

	sub = &cdev->sub[f_v4b];

	if (sub->is_opened == 0)
		return (-EINVAL);

	err = sub->fixed_file.f_op->read(&sub->fixed_file, ptr, len, &off);

	return (err);
}

ssize_t
linux_write(int f_v4b, char *ptr, size_t len)
{
	struct cdev *cdev = cdev_get_device(f_v4b);
	struct cdev_sub *sub;
	loff_t off = 0;
	int err;

	if (cdev == NULL)
		return (-EINVAL);

	sub = &cdev->sub[f_v4b];

	if (sub->is_opened == 0)
		return (-EINVAL);

	err = sub->fixed_file.f_op->write(&sub->fixed_file, ptr, len, &off);

	return (err);
}

void   *
linux_mmap(int f_v4b, uint8_t *addr, size_t len, off_t offset)
{
	struct cdev *cdev = cdev_get_device(f_v4b);
	struct cdev_sub *sub;
	int err;
	uint32_t i;

	if (cdev == NULL)
		return (MAP_FAILED);

	sub = &cdev->sub[f_v4b];

	if (sub->is_opened == 0)
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
		if ((sub->fixed_vma[i].vm_end -
		    sub->fixed_vma[i].vm_start) != len)
			continue;
		if (sub->fixed_vma[i].vm_pgoff != (offset >> PAGE_SHIFT))
			continue;
		if (sub->fixed_vma[i].vm_buffer_address == NULL)
			continue;
		if (sub->fixed_vma[i].vm_buffer_address == MAP_FAILED)
			continue;

		return (sub->fixed_vma[i].vm_buffer_address);
	}

	/* create new entry */
	for (i = 0; i != LINUX_VMA_MAX; i++) {
		if ((sub->fixed_vma[i].vm_buffer_address == NULL) ||
		    (sub->fixed_vma[i].vm_buffer_address == MAP_FAILED))
			break;
	}

	if (i == LINUX_VMA_MAX) {
		return (MAP_FAILED);
	}
	/* fill in information */
	sub->fixed_vma[i].vm_start = (unsigned long)addr;
	sub->fixed_vma[i].vm_end = (unsigned long)(addr + len);
	sub->fixed_vma[i].vm_pgoff = (offset >> PAGE_SHIFT);
	sub->fixed_vma[i].vm_buffer_address = MAP_FAILED;
	sub->fixed_vma[i].vm_flags = (VM_WRITE | VM_READ | VM_SHARED);

	err = sub->fixed_file.f_op->mmap(&sub->fixed_file, &sub->fixed_vma[i]);
	if (err) {
		sub->fixed_vma[i].vm_buffer_address = MAP_FAILED;
		return (MAP_FAILED);
	}
	return (sub->fixed_vma[i].vm_buffer_address);
}
