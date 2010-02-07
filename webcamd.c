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

#include <video4bsd.h>

struct f_videodev {
	pthread_t process;
	int	f;
	int	f_v4b;
	int	local_user;
};

struct vm_allocation {
	uint8_t *ptr;
	uint32_t size;
};

static const char *devnames[F_V4B_MAX] = {

	[F_V4B_VIDEO] = "/dev/video_daemon%d",

	[F_V4B_DVB_AUDIO] = "/dev/dvb/adapter%d/audio_daemon0",
	[F_V4B_DVB_CA] = "/dev/dvb/adapter%d/ca_daemon0",
	[F_V4B_DVB_DEMUX] = "/dev/dvb/adapter%d/demux_daemon0",
	[F_V4B_DVB_DVR] = "/dev/dvb/adapter%d/dvr_daemon0",

	[F_V4B_DVB_FRONTEND] = "/dev/dvb/adapter%d/frontend_daemon0",
	[F_V4B_DVB_OSD] = "/dev/dvb/adapter%d/osd_daemon0",
	[F_V4B_DVB_SEC] = "/dev/dvb/adapter%d/sec_daemon0",
	[F_V4B_DVB_VIDEO] = "/dev/dvb/adapter%d/video_daemon0",
};

static struct f_videodev f_videodev[F_V4B_MAX];
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

static struct vm_allocation vm_allocations[V4B_ALLOC_UNIT_MAX];

static void set_mmap(int f, void *);
static void *find_mmap_size(int f_v4b, uint32_t offset, uint32_t *psize, uint32_t *delta);
static void v4b_errx(int code, const char *str);

static int
find_video4bsd(void)
{
	pthread_t process = pthread_self();
	uint8_t n;

	for (n = 0; n != F_V4B_MAX; n++) {
		if (f_videodev[n].process == process) {
			if (f_videodev[n].local_user)
				return (-1);
			return (f_videodev[n].f);
		}
	}

	v4b_errx(1, "Cannot find Video4BSD process\n");

	return (0);
}


static void
set_localuser(int local_user)
{
	pthread_t process = pthread_self();
	uint8_t n;

	for (n = 0; n != F_V4B_MAX; n++) {
		if (f_videodev[n].process == process) {
			f_videodev[n].local_user = local_user;
			return;
		}
	}

	v4b_errx(1, "Cannot find Video4BSD process\n");
}

static void *
work_video4bsd(void *arg)
{
	struct f_videodev *dev = (struct f_videodev *)arg;
	static struct v4b_command cmd;
	void *mm_ptr;
	uint32_t size;
	uint32_t delta;
	int err;

	dev->process = pthread_self();

	pthread_set_kernel_prio();

	if (dev->f_v4b == F_V4B_VIDEO) {
		linux_init();

		f_usb = usb_linux_probe(u_unit, u_addr, u_index);
		if (f_usb < 0)
			v4b_errx(1, "Cannot find USB device");
	}
	while (1) {

		if (ioctl(dev->f, V4B_IOCTL_GET_COMMAND, &cmd) != 0)
			v4b_errx(1, "Cannot get next V4B command");

		linux_clear_signal();

#ifdef V4B_DEBUG
		printf("Received command %d 0x%08x\n", cmd.command, (uint32_t)cmd.arg);
#endif
		switch (cmd.command) {
		case V4B_CMD_OPEN:
			/* make sure we are closed before open */
			err = linux_close(dev->f_v4b);

			need_timer(1);

			/* try to open the device */
			err = linux_open(dev->f_v4b);

			if (err)
				need_timer(0);

			if (ioctl(dev->f, V4B_IOCTL_SYNC_COMMAND, &err) != 0)
				v4b_errx(1, "Cannot sync V4B command");
			break;

		case V4B_CMD_CLOSE:
			/* close device */
			err = linux_close(dev->f_v4b);

			if (err == 0)
				need_timer(0);

			if (ioctl(dev->f, V4B_IOCTL_SYNC_COMMAND, &err) != 0)
				v4b_errx(1, "Cannot sync V4B command");
			break;

		case V4B_CMD_READ:
			/* read from device */
			err = linux_read(dev->f_v4b, cmd.ptr, cmd.arg);
			if (err >= 0)
				err = 0;
			if (ioctl(dev->f, V4B_IOCTL_SYNC_COMMAND, &err) != 0)
				v4b_errx(1, "Cannot sync V4B command");
			break;

		case V4B_CMD_WRITE:
			/* write to device */
			err = linux_write(dev->f_v4b, cmd.ptr, cmd.arg);
			if (err >= 0)
				err = 0;
			if (ioctl(dev->f, V4B_IOCTL_SYNC_COMMAND, &err) != 0)
				v4b_errx(1, "Cannot sync V4B command");
			break;

		case V4B_CMD_IOCTL:
#ifdef V4B_DEBUG
			printf("IOCTL 0x%08x, %p\n", cmd.arg, cmd.ptr);
#endif
			err = linux_ioctl(dev->f_v4b, cmd.arg, cmd.ptr);
#ifdef V4B_DEBUG
			printf("IOCTL = %d\n", err);
#endif
			if (ioctl(dev->f, V4B_IOCTL_SYNC_COMMAND, &err) != 0)
				v4b_errx(1, "Cannot sync V4B command");
			break;

		case V4B_CMD_MMAP:

			/* XXX V4L hack */
			mm_ptr = find_mmap_size(dev->f_v4b, cmd.arg, &size, &delta);
			if (size != 0) {
				if (mm_ptr == MAP_FAILED) {
					err = EINVAL;
				} else {
					err = 0;
					set_mmap(dev->f, mm_ptr + delta);
				}
			} else {
				err = EINVAL;
			}
			if (ioctl(dev->f, V4B_IOCTL_SYNC_COMMAND, &err) != 0)
				v4b_errx(1, "Cannot sync V4B command");
			break;

		default:
			err = ENOTTY;
			if (ioctl(dev->f, V4B_IOCTL_SYNC_COMMAND, &err) != 0)
				v4b_errx(1, "Cannot sync V4B command");
			break;
		}
#ifdef V4B_DEBUG
		printf("Status = %d\n", err);
#endif

	}
	return (NULL);
}

static int
open_video4bsd(int unit)
{
	char temp[128];
	pthread_t dummy;
	uint8_t n;

	for (n = 0; n != F_V4B_MAX; n++) {

		snprintf(temp, sizeof(temp), devnames[n], unit);

		f_videodev[n].f = open(temp, O_RDWR);
		f_videodev[n].f_v4b = n;

		if (f_videodev[n].f < 0) {
			while (n--) {
				close(f_videodev[n].f);
				f_videodev[n].f = -1;
			}
			return (-1);
		}
	}

	for (n = 0; n != F_V4B_MAX; n++) {
		if (pthread_create(&dummy, NULL, work_video4bsd, f_videodev + n)) {
			v4b_errx(1, "Failed creating video4bsd process");
		}
	}

	return (0);
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

	if (u_videodev < 0) {
		for (u_videodev = 0;; u_videodev++) {
			if (u_videodev == V4B_DEVICES_MAX)
				v4b_errx(1, "Cannot open /dev/video_xxx or /dev/dvb/xxx. "
				    "Did you kldload video4bsd?");
			if (open_video4bsd(u_videodev) >= 0)
				break;
		}
	} else {
		if (open_video4bsd(u_videodev) < 0)
			v4b_errx(1, "Cannot open /dev/video_xxx or /dev/dvb/xxx. "
			    "Did you kldload video4bsd?");
	}

	if (rtprio(RTP_SET, getpid(), &prio_arg))
		printf("Cannot set realtime priority\n");

	while (1) {
		sleep(60);
	}

	return (0);
}

unsigned long
copy_to_user(void *to, const void *from, unsigned long n)
{
	struct v4b_data_chunk cmd = {
		.client_ptr = (uint8_t *)(from - (const void *)0),
		.peer_ptr = to,
		.length = n,
	};
	int f = find_video4bsd();

	if (f >= 0)
		return (ioctl(f, V4B_IOCTL_WRITE_DATA, &cmd) ? n : 0);

	memcpy(to, from, n);
	return (0);
}

unsigned long
copy_from_user(void *to, const void *from, unsigned long n)
{
	struct v4b_data_chunk cmd = {
		.client_ptr = to,
		.peer_ptr = (uint8_t *)(from - (const void *)0),
		.length = n,
	};
	int f = find_video4bsd();

	if (f >= 0)
		return (ioctl(f, V4B_IOCTL_READ_DATA, &cmd) ? n : 0);

	memcpy(to, from, n);
	return (0);
}

static void *
find_mmap_size(int f_v4b, uint32_t offset,
    uint32_t *psize, uint32_t *delta)
{
	struct v4l2_buffer buf = {0, 0, 0};
	void *ptr;
	int err;
	int i;

	set_localuser(1);

	for (i = 0;; i++) {

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		err = linux_ioctl(f_v4b, VIDIOC_QUERYBUF, &buf);
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
			ptr = linux_mmap(f_v4b, NULL,
			    buf.length, buf.m.offset);
			break;
		}
	}

	set_localuser(0);

	return (ptr);
}

static void
set_mmap(int f, void *ptr)
{
	struct v4b_mem_alloc cmd = {0, 0};
	unsigned int n;
	void *ptr_min;
	void *ptr_max;

	atomic_lock();
	for (n = 0; n != V4B_ALLOC_UNIT_MAX; n++) {
		if (vm_allocations[n].ptr == NULL)
			continue;

		ptr_min = vm_allocations[n].ptr;
		ptr_max = vm_allocations[n].ptr + vm_allocations[n].size - 1;

		if ((ptr >= ptr_min) && (ptr <= ptr_max)) {

			cmd.alloc_nr = n;
			cmd.page_count = (ptr - ptr_min) / PAGE_SIZE;

			atomic_unlock();
			if (ioctl(f, V4B_IOCTL_MAP_MEMORY, &cmd) != 0) {
#ifdef V4B_DEBUG
				printf("Mapping memory failed\n");
#endif
			}
			return;
		}
	}
	atomic_unlock();
}

void   *
malloc_vm(size_t size)
{
	struct v4b_mem_alloc cmd = {0, 0};
	void *ptr;
	uint32_t n;
	int error;
	int f = f_videodev[0].f;

	if (size == 0)
		size = 1;

	cmd.page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;

#ifdef V4B_DEBUG
	printf("size = %d\n", (int)size);
#endif
	atomic_lock();
	for (n = 0; n != V4B_ALLOC_UNIT_MAX; n++) {
		if (vm_allocations[n].ptr != NULL)
			continue;

		vm_allocations[n].ptr = (uint8_t *)1;	/* reserve */
		atomic_unlock();
#ifdef V4B_DEBUG
		printf("alloc_nr = %d\n", n);
#endif
		cmd.alloc_nr = n;

		error = ioctl(f, V4B_IOCTL_ALLOC_MEMORY, &cmd);
		if (error) {
			atomic_lock();
			vm_allocations[n].ptr = NULL;
#ifdef V4B_DEBUG
			printf("Alloc failed %d\n", __LINE__);
#endif
			if (errno == EBUSY)
				continue;

			break;
		}
		ptr = mmap(NULL, cmd.page_count * PAGE_SIZE,
		    PROT_READ | PROT_WRITE,
		    MAP_SHARED, f, V4B_ALLOC_PAGES_MAX *
		    PAGE_SIZE * n);

		if (ptr == MAP_FAILED) {
			ioctl(f, V4B_IOCTL_FREE_MEMORY, &cmd);
			atomic_lock();
			vm_allocations[n].ptr = NULL;
#ifdef V4B_DEBUG
			printf("Alloc failed %d\n", __LINE__);
#endif
			break;
		}
		vm_allocations[n].ptr = ptr;
		vm_allocations[n].size = size;
		return (ptr);
	}
	atomic_unlock();

	return (NULL);
}

void
free_vm(void *ptr)
{
	struct v4b_mem_alloc cmd = {0, 0};
	unsigned int n;
	int f = f_videodev[0].f;

	if (ptr == NULL)
		return;

	atomic_lock();
	for (n = 0; n != V4B_ALLOC_UNIT_MAX; n++) {
		if (vm_allocations[n].ptr != ptr)
			continue;

		atomic_unlock();

		cmd.alloc_nr = n;

		munmap(ptr, vm_allocations[n].size);

		if (ioctl(f, V4B_IOCTL_FREE_MEMORY, &cmd) != 0) {
#ifdef V4B_DEBUG
			printf("Free failed %d\n", __LINE__);
#endif
		}
		atomic_lock();

		vm_allocations[n].ptr = NULL;
		vm_allocations[n].size = 0;

		break;
	}
	atomic_unlock();
}

void
check_signal(void)
{
	int has_signal = 0;
	int f = f_videodev[0].f;

	if (f < 0)
		return;

	if (ioctl(f, V4B_IOCTL_GET_SIGNAL, &has_signal) != 0) {
#ifdef V4B_DEBUG
		printf("Get signal failed\n");
#endif
	}
	if (has_signal)
		linux_set_signal();
}
