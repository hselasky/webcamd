/*-
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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
#include <sys/types.h>
#include <sys/rtprio.h>
#include <sys/param.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <libutil.h>

#include <cuse4bsd.h>

static cuse_open_t v4b_open;
static cuse_close_t v4b_close;
static cuse_read_t v4b_read;
static cuse_write_t v4b_write;
static cuse_ioctl_t v4b_ioctl;
static cuse_poll_t v4b_poll;
static cuse_mmap_t v4b_mmap;

static struct cuse_methods v4b_methods = {
	.cm_open = v4b_open,
	.cm_close = v4b_close,
	.cm_read = v4b_read,
	.cm_write = v4b_write,
	.cm_ioctl = v4b_ioctl,
	.cm_poll = v4b_poll,
	.cm_mmap = v4b_mmap,
};

static const char *devnames[F_V4B_MAX] = {

	[F_V4B_VIDEO] = "/dev/video%d",

	[F_V4B_DVB_AUDIO] = "/dev/dvb/adapter%d/audio0",
	[F_V4B_DVB_CA] = "/dev/dvb/adapter%d/ca0",
	[F_V4B_DVB_DEMUX] = "/dev/dvb/adapter%d/demux0",
	[F_V4B_DVB_DVR] = "/dev/dvb/adapter%d/dvr0",

	[F_V4B_DVB_FRONTEND] = "/dev/dvb/adapter%d/frontend0",
	[F_V4B_DVB_OSD] = "/dev/dvb/adapter%d/osd0",
	[F_V4B_DVB_SEC] = "/dev/dvb/adapter%d/sec0",
	[F_V4B_DVB_VIDEO] = "/dev/dvb/adapter%d/video0",
};

static int u_unit = 0;
static int u_addr = 0;
static int u_index = 0;
static int u_videodev = -1;
static int f_usb = -1;
static int do_fork = 0;
static struct pidfh *local_pid = NULL;

char	global_fw_prefix[128];

#if 0
#define	V4B_DEBUG
#endif

static void *v4b_find_mmap_size(struct cdev_handle *handle, int fflags, uint32_t offset, uint32_t *psize, uint32_t *delta);
static void v4b_errx(int code, const char *str);

static void
v4b_work_exec_hup(int dummy)
{

}

static void *
v4b_work(void *arg)
{
	pthread_set_kernel_prio();

	signal(SIGHUP, v4b_work_exec_hup);

	while (1) {
		if (cuse_wait_and_process() != 0)
			break;
	}

	return (NULL);
}

static int
v4b_convert_error(int error)
{
	if (error < 0) {
		switch (error) {
			case -EBUSY:error = CUSE_ERR_BUSY;
			break;
		case -EWOULDBLOCK:
			error = CUSE_ERR_WOULDBLOCK;
			break;
		case -EINVAL:
			error = CUSE_ERR_INVALID;
			break;
		case -ENOMEM:
			error = CUSE_ERR_NO_MEMORY;
			break;
		case -EFAULT:
			error = CUSE_ERR_FAULT;
			break;
		case -EINTR:
			error = CUSE_ERR_SIGNAL;
			break;
		default:
			error = CUSE_ERR_OTHER;
			break;
		}
	}
	return (error);
}

static int
v4b_open(struct cuse_dev *cdev, int fflags)
{
	struct cdev_handle *handle;
	int f_v4b;
	int fflags_linux;

	f_v4b = (int)(long)cuse_dev_get_priv0(cdev);

	switch (fflags & (CUSE_FFLAG_READ | CUSE_FFLAG_READ)) {
	case (CUSE_FFLAG_WRITE | CUSE_FFLAG_READ):
		fflags_linux = O_RDWR;
		break;
	case CUSE_FFLAG_READ:
		fflags_linux = O_RDONLY;
		break;
	case CUSE_FFLAG_WRITE:
		fflags_linux = O_WRONLY;
		break;
	default:
		return (CUSE_ERR_INVALID);
	}

	need_timer(1);

	/* try to open the device */
	handle = linux_open(f_v4b, fflags_linux);

	if (handle == NULL) {
		need_timer(0);
		return (CUSE_ERR_INVALID);
	}
	cuse_dev_set_per_file_handle(cdev, handle);
	return (0);
}

static int
v4b_close(struct cuse_dev *cdev, int fflags)
{
	struct cdev_handle *handle;
	int error;

	handle = cuse_dev_get_per_file_handle(cdev);

	/* close device */
	error = linux_close(handle);

	if (error == 0)
		need_timer(0);

	return (v4b_convert_error(error));
}

static int
v4b_read(struct cuse_dev *cdev, int fflags,
    void *peer_ptr, int len)
{
	struct cdev_handle *handle;
	int error;

	handle = cuse_dev_get_per_file_handle(cdev);

	/* read from device */
	error = linux_read(handle, fflags & CUSE_FFLAG_NONBLOCK, peer_ptr, len);

	return (v4b_convert_error(error));
}

static int
v4b_write(struct cuse_dev *cdev, int fflags,
    const void *peer_ptr, int len)
{
	struct cdev_handle *handle;
	int error;

	handle = cuse_dev_get_per_file_handle(cdev);

	/* write to device */
	error = linux_write(handle, fflags & CUSE_FFLAG_NONBLOCK,
	    (uint8_t *)((const uint8_t *)peer_ptr - (const uint8_t *)0), len);

	return (v4b_convert_error(error));
}

int
v4b_ioctl(struct cuse_dev *cdev, int fflags,
    unsigned long cmd, void *peer_data)
{
	struct cdev_handle *handle;
	int error;

	handle = cuse_dev_get_per_file_handle(cdev);

	/* execute ioctl */
	error = linux_ioctl(handle, fflags & CUSE_FFLAG_NONBLOCK, cmd, peer_data);

	return (v4b_convert_error(error));
}

int
v4b_poll(struct cuse_dev *cdev, int fflags, int events)
{
	return (events & (CUSE_POLL_READ | CUSE_POLL_WRITE | CUSE_POLL_ERROR));
}

int
v4b_mmap(struct cuse_dev *cdev, int fflags,
    unsigned long offset, unsigned long *vaddr)
{
	struct cdev_handle *handle;
	void *mm_ptr;
	uint32_t size;
	uint32_t delta;
	int error;

	handle = cuse_dev_get_per_file_handle(cdev);

	/* XXX V4L hack */
	mm_ptr = v4b_find_mmap_size(handle, fflags & CUSE_FFLAG_NONBLOCK, offset, &size, &delta);
	if (size != 0) {
		if (mm_ptr == MAP_FAILED) {
			error = CUSE_ERR_INVALID;
		} else {
			error = 0;

			*vaddr = (unsigned long)(mm_ptr + delta);
		}
	} else {
		error = CUSE_ERR_INVALID;
	}

	return (error);
}

static void
v4b_create(int unit)
{
	struct cdev_handle *handle;
	pthread_t dummy;
	uint8_t n;

	for (n = 0; n != F_V4B_MAX; n++) {

		handle = linux_open(n, O_RDONLY);

		if (handle != NULL) {
			linux_close(handle);

			cuse_dev_create(&v4b_methods, (void *)(long)n,
			    0, 0 /* UID_ROOT */ , 5 /* GID_OPERATOR */ , 0600, devnames[n], n);

			printf("Creating /dev/");
			printf(devnames[n], n);
			printf("\n");
		}
	}

	for (n = 0; n != 4; n++) {
		if (pthread_create(&dummy, NULL, v4b_work, NULL)) {
			v4b_errx(1, "Failed creating Video4BSD process");
		}
	}
}

static void
usage(void)
{
	fprintf(stderr,
	    "usage: webcamd -d [ugen]<unit>.<addr> -i 0 -v -1 -B\n"
	    "	-d <USB device>\n"
	    "	-i <interface number>\n"
	    "	-v <video device number>\n"
	    "	-B Run in background\n"
	    "	-f <firmware path>\n"
	    "	-h Print help\n"
	);
	exit(1);
}

static void
v4b_errx(int code, const char *str)
{
	fprintf(stderr, "%s\n", str);
	exit(code);
}

static void
v4b_exit(void)
{
	if (local_pid != NULL) {
		pidfile_remove(local_pid);
		local_pid = NULL;
	}
}

int
pidfile_create(int bus, int addr, int index)
{
	char buf[64];

	snprintf(buf, sizeof(buf), "/var/run/webcamd."
	    "%d.%d.%d.pid", bus, addr, index);

	local_pid = pidfile_open(buf, 0600, NULL);
	if (local_pid == NULL) {
		return (EEXIST);
	} else {
		if (do_fork) {
			if (daemon(0, 0) != 0)
				v4b_errx(1, "Cannot daemonize");
		}
		pidfile_write(local_pid);
	}

	printf("Attached ugen%d.%d[%d] to Video4BSD unit %d\n",
	    bus, addr, index, u_videodev);

	return (0);
}

int
main(int argc, char **argv)
{
	struct rtprio prio_arg = {RTP_PRIO_REALTIME, 16};
	const char *ptr;
	int opt;

	atexit(&v4b_exit);

	while ((opt = getopt(argc, argv, "Bd:f:i:v:h")) != -1) {
		switch (opt) {
		case 'd':
			ptr = optarg;

			if ((ptr[0] == 'u') &&
			    (ptr[1] == 'g') &&
			    (ptr[2] == 'e') &&
			    (ptr[3] == 'n'))
				ptr += 4;

			if (sscanf(ptr, "%d.%d", &u_unit, &u_addr) != 2)
				usage();
			break;

		case 'i':
			u_index = atoi(optarg);
			break;

		case 'v':
			u_videodev = atoi(optarg);
			break;

		case 'B':
			do_fork = 1;
			break;

		case 'f':
			strlcpy(global_fw_prefix, optarg,
			    sizeof(global_fw_prefix));
			break;

		default:
			usage();
			break;
		}
	}

	if (cuse_init() != 0) {
		v4b_errx(1, "Could not open /dev/cuse. "
		    "Did you kldload cuse4bsd?");
	}
	if (u_videodev < 0) {
		if (cuse_alloc_unit_number(&u_videodev) != 0)
			v4b_errx(1, "Cannot allocate unique unit number");
	}
	if (rtprio(RTP_SET, getpid(), &prio_arg))
		printf("Cannot set realtime priority\n");

	linux_init();

	f_usb = usb_linux_probe(u_unit, u_addr, u_index);
	if (f_usb < 0)
		v4b_errx(1, "Cannot find USB device");

	v4b_create(u_videodev);

	v4b_work(NULL);

	return (0);
}

unsigned long
copy_to_user(void *to, const void *from, unsigned long n)
{
	int error;

	error = cuse_copy_out(from, to, (int)n);

	return ((error != 0) ? n : 0);
}

unsigned long
copy_from_user(void *to, const void *from, unsigned long n)
{
	int error;

	error = cuse_copy_in(from, to, (int)n);

	return ((error != 0) ? n : 0);
}

static void *
v4b_find_mmap_size(struct cdev_handle *handle, int fflags,
    uint32_t offset, uint32_t *psize, uint32_t *delta)
{
	struct v4l2_buffer buf = {0, 0, 0};
	void *ptr;
	int err;
	int i;

	cuse_set_local(1);

	for (i = 0;; i++) {

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		err = linux_ioctl(handle, fflags, VIDIOC_QUERYBUF, &buf);
		if (err) {
			*psize = 0;
			*delta = 0;
			ptr = MAP_FAILED;
			break;
		}
		if ((offset >= buf.m.offset) &&
		    (offset <= (buf.m.offset + buf.length - 1))) {
			*psize = buf.length;
			*delta = offset - buf.m.offset;
			ptr = linux_mmap(handle, fflags, NULL,
			    buf.length, buf.m.offset);
			break;
		}
	}

	cuse_set_local(0);

	return (ptr);
}

void   *
malloc_vm(size_t size)
{
	return (cuse_vmalloc(size));
}

void
free_vm(void *ptr)
{
	cuse_vmfree(ptr);
}

int
check_signal(void)
{
	return (cuse_got_peer_signal() == 0);
}
