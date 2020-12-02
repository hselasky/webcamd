/*-
 * Copyright (c) 2010-2020 Hans Petter Selasky. All rights reserved.
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
#include <sys/param.h>
#include <sys/rtprio.h>
#include <sys/time.h>
#include <sys/filio.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <grp.h>
#include <pwd.h>
#include <sysexits.h>
#include <ctype.h>

#include <libutil.h>

#include <cuse.h>

#include <kernel/linux_hal.h>

#include <linux/idr.h>

static cuse_open_t v4b_open;
static cuse_close_t v4b_close;
static cuse_read_t v4b_read;
static cuse_write_t v4b_write;
static cuse_ioctl_t v4b_ioctl;
static cuse_poll_t v4b_poll;

static struct cuse_methods v4b_methods = {
	.cm_open = v4b_open,
	.cm_close = v4b_close,
	.cm_read = v4b_read,
	.cm_write = v4b_write,
	.cm_ioctl = v4b_ioctl,
	.cm_poll = v4b_poll,
};

/*
 * The first letter in the devicename gives the unit number allocation
 * group from "A" to "Z". This letter is removed when creating the
 * character device.
 */
#define	UNIT_MAX ('G' - 'A')

const char *webcamd_devnames[F_V4B_MAX] = {

	[F_V4B_VIDEO] = "Avideo%d",

	[F_V4B_DVB_AUDIO] = "Bdvb/adapter%d/audio%d",
	[F_V4B_DVB_CA] = "Bdvb/adapter%d/ca%d",
	[F_V4B_DVB_DEMUX] = "Bdvb/adapter%d/demux%d",
	[F_V4B_DVB_DVR] = "Bdvb/adapter%d/dvr%d",

	[F_V4B_DVB_FRONTEND] = "Bdvb/adapter%d/frontend%d",
	[F_V4B_DVB_OSD] = "Bdvb/adapter%d/osd%d",
	[F_V4B_DVB_SEC] = "Bdvb/adapter%d/sec%d",
	[F_V4B_DVB_VIDEO] = "Bdvb/adapter%d/video%d",

	[F_V4B_LIRC] = "Clirc%d",

	[F_V4B_EVDEV] = "Dinput/event%d",

	[F_V4B_JOYDEV] = "Einput/js%d",

	[F_V4B_ROCCAT] = "Froccat%d",
};

static int u_unit;
static int u_addr;
static int u_index;
static int u_videodev = -1;
static const char *u_devicename;
static const char *u_serialname;
static int u_match_index;
static int do_list;
static int do_fork;
static int do_realtime = 1;
static struct pidfh *local_pid = NULL;
static const char *d_desc;
static gid_t gid;
static uid_t uid;
static int gid_found;
static int uid_found;
static int vtuner_client;
static int vtuner_server;

#define	CHR_MODE 0660

char	global_fw_prefix[128] = {"/boot/modules"};
int	webcamd_hal_register;

#define	v4b_errx(code, fmt, ...) do {			\
    syslog(LOG_ERR, "webcamd: " fmt "\n",##		\
	__VA_ARGS__); 					\
    exit(code);						\
} while (0)

static void
v4b_work_exec_hup(int dummy)
{

}

static void *
v4b_work(void *arg)
{
	signal(SIGHUP, v4b_work_exec_hup);

	while (1) {
		if (cuse_wait_and_process() != 0)
			break;
	}

	exit(0);			/* we are done */

	return (NULL);
}

static int
v4b_convert_error(int error)
{
	;				/* indent fix */
	if (error < 0) {
		switch (error) {
		case -EBUSY:
			error = CUSE_ERR_BUSY;
			break;
		case -EWOULDBLOCK:
			error = CUSE_ERR_WOULDBLOCK;
			break;
		case -EINVAL:
		case -ENOENT:
		case -ENOTTY:
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

	switch (fflags & (CUSE_FFLAG_WRITE | CUSE_FFLAG_READ)) {
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

	cuse_dev_set_per_file_handle(cdev, NULL);

	/* close device */
	error = linux_close(handle);

	if (error == 0)
		need_timer(0);

	return (v4b_convert_error(error));
}

#ifndef CUSE_FFLAG_COMPAT32
#define	CUSE_FFLAG_COMPAT32 0
#endif

static int
v4b_read(struct cuse_dev *cdev, int fflags,
    void *peer_ptr, int len)
{
	struct cdev_handle *handle;
	int error;

	handle = cuse_dev_get_per_file_handle(cdev);

	/* read from device */
	error = linux_read(handle,
	    fflags & (CUSE_FFLAG_NONBLOCK | CUSE_FFLAG_COMPAT32), peer_ptr, len);

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
	error = linux_write(handle,
	    fflags & (CUSE_FFLAG_NONBLOCK | CUSE_FFLAG_COMPAT32),
	    (uint8_t *)((const uint8_t *)peer_ptr - (const uint8_t *)0), len);

	return (v4b_convert_error(error));
}

unsigned short webcamd_vendor;
unsigned short webcamd_product;
unsigned int webcamd_speed;

static int
v4b_ioctl(struct cuse_dev *cdev, int fflags,
    unsigned long cmd, void *peer_data)
{
	struct v4l2_buffer buf;
	struct v4l2_buffer_compat32 buf32;
	struct cdev_handle *handle;
	void *ptr;
	int error;

	handle = cuse_dev_get_per_file_handle(cdev);

	/* we support blocking/non-blocking I/O */
	if (cmd == FIONBIO || cmd == FIOASYNC)
		return (0);

	/* execute ioctl */
	error = linux_ioctl(handle,
	    fflags & (CUSE_FFLAG_NONBLOCK | CUSE_FFLAG_COMPAT32),
	    cmd, peer_data);

	if ((cmd == VIDIOC_QUERYBUF) && (error >= 0)) {
		if (copy_from_user(&buf, peer_data, sizeof(buf)) != 0) {
			error = -EFAULT;
			goto done;
		}
		ptr = linux_mmap(handle, fflags,
		    (void *)(long)PAGE_SIZE,
		    buf.length, buf.m.offset);

		if (ptr != MAP_FAILED) {
			buf.m.offset = cuse_vmoffset(ptr);
		} else {
			buf.m.offset = 0x80000000UL;
		}

		if (copy_to_user(peer_data, &buf, sizeof(buf)) != 0) {
			error = -EFAULT;
			goto done;
		}
	} else if ((cmd == _VIDIOC_QUERYBUF32) && (error >= 0)) {
		if (copy_from_user(&buf32, peer_data, sizeof(buf32)) != 0) {
			error = -EFAULT;
			goto done;
		}
		ptr = linux_mmap(handle, fflags,
		    (void *)(long)PAGE_SIZE,
		    buf32.length, buf32.m.offset);

		if (ptr != MAP_FAILED) {
			buf32.m.offset = cuse_vmoffset(ptr);
		} else {
			buf32.m.offset = 0x80000000UL;
		}

		if (copy_to_user(peer_data, &buf32, sizeof(buf32)) != 0) {
			error = -EFAULT;
			goto done;
		}
	} else if ((cmd == WEBCAMD_IOCTL_GET_USB_VENDOR_ID) && (error < 0)) {
		if (copy_to_user(peer_data, &webcamd_vendor,
		    sizeof(webcamd_vendor)) != 0) {
			error = -EFAULT;
			goto done;
		}
		error = 0;
	} else if ((cmd == WEBCAMD_IOCTL_GET_USB_PRODUCT_ID) && (error < 0)) {
		if (copy_to_user(peer_data, &webcamd_product,
		    sizeof(webcamd_product)) != 0) {
			error = -EFAULT;
			goto done;
		}
		error = 0;
	} else if ((cmd == WEBCAMD_IOCTL_GET_USB_SPEED) && (error < 0)) {
		if (copy_to_user(peer_data, &webcamd_speed,
		    sizeof(webcamd_speed)) != 0) {
			error = -EFAULT;
			goto done;
		}
		error = 0;
	}
done:
	return (v4b_convert_error(error));
}

static int
v4b_poll(struct cuse_dev *cdev, int fflags, int events)
{
	struct cdev_handle *handle;
	int error;
	int revents;

	handle = cuse_dev_get_per_file_handle(cdev);

	/* poll device */
	error = linux_poll(handle);

	revents = 0;

	if (error & (POLLPRI | POLLIN | POLLRDNORM))
		revents |= events & CUSE_POLL_READ;

	if (error & (POLLOUT | POLLWRNORM))
		revents |= events & CUSE_POLL_WRITE;
#if 0
	/* currently we mask away any poll errors */
	if (error & (POLLHUP | POLLNVAL | POLLERR))
		revents |= events & CUSE_POLL_ERROR;
#endif
	return (revents);
}

static void
v4b_create(int unit)
{
	struct cdev_handle *handle;
	pthread_t dummy;
	unsigned int n;
	unsigned int p;
	unsigned int q;
	int id;
	char buf[128];
	int unit_num[UNIT_MAX][F_V4B_SUBDEV_MAX];
	const char *dname;
	struct cuse_dev *pdev;

	/* The DVB V2 API is ASYNC and we need to wait for it: */
	flush_scheduled_work();

	for (n = 0; n != UNIT_MAX; n++) {
		for (p = 0; p != F_V4B_SUBDEV_MAX; p++) {
			unit_num[n][p] = (unit < 0) ? -1 : (unit + p);
		}
	}

	for (n = 0; n != (F_V4B_MAX * F_V4B_SUBDEV_MAX *
	    F_V4B_SUBSUBDEV_MAX); n++) {

		handle = linux_open(n, O_RDONLY);

		if (handle != NULL) {

			linux_close(handle);

			dname = webcamd_devnames[n / (F_V4B_SUBSUBDEV_MAX *
			    F_V4B_SUBDEV_MAX)];
			id = dname[0] - 'A';
			p = (n % F_V4B_SUBDEV_MAX);
			q = ((n / F_V4B_SUBDEV_MAX) % F_V4B_SUBSUBDEV_MAX);

			if (unit_num[id][p] < 0) {
				if (cuse_alloc_unit_number_by_id(
				    &unit_num[id][p],
				    CUSE_ID_WEBCAMD(id)) != 0) {
					v4b_errx(1, "Cannot allocate "
					    "uniq unit number");
				}
			}
again:
			pdev = cuse_dev_create(&v4b_methods, (void *)(long)n,
			    0, uid, gid, CHR_MODE, dname + 1,
			    unit_num[id][p], q);

			/*
			 * Resolve device naming conflict with new
			 * kernel evdev module:
			 */
			if (pdev == NULL && unit < 0 &&
			    (n / (F_V4B_SUBDEV_MAX * F_V4B_SUBSUBDEV_MAX)) == F_V4B_EVDEV &&
			    cuse_alloc_unit_number_by_id(&unit_num[id][p], CUSE_ID_WEBCAMD(id)) == 0)
				goto again;

			snprintf(buf, sizeof(buf), dname + 1,
			    unit_num[id][p], q);

			syslog(LOG_INFO, "Creating /dev/%s\n", buf);

			for (p = 0; p != 4; p++) {
				if (pthread_create(&dummy, NULL, v4b_work, NULL)) {
					v4b_errx(1, "Failed creating Cuse4BSD process");
				}
			}

			if (webcamd_hal_register)
				hal_add_device(buf);
		}
	}
}

uid_t
v4b_get_uid(void)
{
	return (uid);
}

gid_t
v4b_get_gid(void)
{
	return (gid);
}

int
v4b_get_perm(void)
{
	return (CHR_MODE);
}

static void
usage(void)
{
	fprintf(stderr,
	    "usage: webcamd -d [ugen]<unit>.<addr> -i 0 -v -1 -B\n"
	    "	-d <USB device>\n"
	    "	-i <interface or client number>\n"
	    "	-m <parameter>=<value>\n"
	    "	-s Show available parameters\n"
	    "	-l Show available USB devices\n"
	    "	-S <SerialNumberString> as output by -l option\n"
	    "	-N <DeviceNameString> as output by -l option\n"
	    "	-M <match index> for use with -S and -N options\n"
	    "	-v <video device number>\n"
	    "	-B Run in background\n"
	    "	-f <firmware path> [%s]\n"
	    "	-r Do not set realtime priority\n"
	    "	-U <user> Set user for character devices\n"
	    "	-G <group> Set group for character devices\n"
	    "	-H Register device by HAL daemon\n"
	    "	-D <host:port:ndev> Connect to remote host instead of USB\n"
	    "	-L <host:port:ndev> Make DVB device available from TCP/IP\n"
	    "	-h Print help\n"
	    "NOTE: The minimum options needed is one of -d, -S, -s, -l, -N or -D\n",
	    global_fw_prefix
	);
	exit(EX_USAGE);
}

static const char string_unknown[] = {"unknown"};

static void
string_filter(char *ptr)
{
	char *old = ptr;
	char *tmp = ptr;
	char ch;

	while ((ch = *ptr) != '\0') {
		if (isalnum(ch) == 0)
			*ptr = '-';
		ptr++;
	}
	/* trim minus from tail */
	while (ptr-- != old) {
		ch = *ptr;
		if (ch != '-')
			break;
		*ptr = '\0';
	}

	/* trim minus from head */
	ptr = old;
	while ((ch = *ptr) == '-')
		ptr++;
	while ((ch = *ptr) != '\0')
		*old++ = *ptr++;
	*old = '\0';

	/* check if string is empty */
	if (*tmp == '\0')
		strcpy(tmp, string_unknown);
}

struct find_match {
	struct find_match *next;
	char   *ser;
	char   *txt;
	uint32_t match_num;
};

static struct find_match *
new_match(struct find_match **ppfirst, const char *ser, const char *txt)
{
	struct find_match *ptr;

	for (ptr = *ppfirst; ptr != 0; ptr = ptr->next) {
		if (strcmp(ptr->ser, ser) == 0 &&
		    strcmp(ptr->txt, txt) == 0) {
			ptr->match_num++;
			return (ptr);
		}
	}
	ptr = malloc(sizeof(*ptr) + strlen(ser) + strlen(txt) + 2);
	if (ptr == NULL)
		v4b_errx(EX_SOFTWARE, "Out of memory");
	ptr->txt = (char *)(ptr + 1);
	strcpy(ptr->txt, txt);
	ptr->ser = ptr->txt + strlen(txt) + 1;
	strcpy(ptr->ser, ser);
	ptr->match_num = 0;
	ptr->next = *ppfirst;
	*ppfirst = ptr;
	return (ptr);
}

static void
free_match(struct find_match **ppfirst)
{
	struct find_match *ptr;

	while (1) {
		ptr = *ppfirst;
		if (ptr == NULL)
			break;
		*ppfirst = ptr->next;
		free(ptr);
	}
}

static void
find_devices(void)
{
	struct LIBUSB20_DEVICE_DESC_DECODED *pddesc;
	struct libusb20_backend *pbe;
	struct libusb20_device *pdev;
	const char *ptr;
	char ser[128];
	char txt[128];
	char *sub;
	int found = 0;
	int match_number = 0;
	struct find_match *first_match = NULL;
	struct find_match *curr_match = NULL;

	pbe = libusb20_be_alloc_default();
	if (pbe == NULL)
		v4b_errx(EX_SOFTWARE, "Cannot allocate USB backend");

	if (do_list != 0)
		printf("Available device(s):\n");

	pdev = NULL;
	while ((pdev = libusb20_be_device_foreach(pbe, pdev))) {
		if (libusb20_dev_get_mode(pdev) != LIBUSB20_MODE_HOST)
			continue;

		ptr = libusb20_dev_get_desc(pdev);
		if (ptr != NULL) {
			sub = strchr(ptr, '<');
			if (sub == NULL)
				strcpy(txt, string_unknown);
			else
				strlcpy(txt, sub + 1, sizeof(txt));
			sub = strchr(txt, '>');
			if (sub != NULL)
				*sub = 0;
		} else {
			strcpy(txt, string_unknown);
		}

		pddesc = libusb20_dev_get_device_desc(pdev);
		if (pddesc == NULL || pddesc->iSerialNumber == 0 ||
		    libusb20_dev_open(pdev, 0) != 0 ||
		    libusb20_dev_req_string_simple_sync(pdev,
		    pddesc->iSerialNumber, ser, sizeof(ser)) != 0) {
			strcpy(ser, string_unknown);
		}
		libusb20_dev_close(pdev);

		string_filter(ser);
		string_filter(txt);

		if (do_list) {
			curr_match = new_match(&first_match, ser, txt);

			printf("webcamd [-d ugen%u.%u] -N %s -S %s -M %d\n",
			    libusb20_dev_get_bus_number(pdev),
			    libusb20_dev_get_address(pdev),
			    txt, ser, curr_match->match_num);

		} else if ((u_devicename == NULL || strcmp(txt, u_devicename) == 0) &&
			    (u_serialname == NULL || strcmp(ser, u_serialname) == 0) &&
			    (u_addr == 0 || (libusb20_dev_get_address(pdev) == u_addr &&
		    libusb20_dev_get_bus_number(pdev) == u_unit))) {

			if (found++ == u_match_index) {
				u_unit = libusb20_dev_get_bus_number(pdev);
				u_addr = libusb20_dev_get_address(pdev);
				libusb20_be_free(pbe);
				free_match(&first_match);
				return;
			}
		}
	}
	libusb20_be_free(pbe);
	free_match(&first_match);

	if (do_list != 0) {
		printf("Show webcamd usage:\n"
		    "webcamd -h\n");
		if (curr_match != NULL)
			exit(0);
	}
	v4b_errx(EX_SOFTWARE, "No USB device match found");
}

static void
v4b_exit(void)
{
	if (local_pid != NULL) {
		pidfile_remove(local_pid);
		local_pid = NULL;
	}
	closelog();
}

int
pidfile_create(int bus, int addr, int index)
{
	char buf[64];

	if (local_pid != NULL)
		return (0);

	snprintf(buf, sizeof(buf), "/var/run/webcamd."
	    "%d.%d.%d.pid", bus, addr, index);

	local_pid = pidfile_open(buf, 0600, NULL);
	if (local_pid == NULL) {
		return (EEXIST);
	} else {
		pidfile_write(local_pid);
	}

	syslog(LOG_INFO, "Attached to ugen%d.%d[%d]\n", bus, addr, index);

	return (0);
}

/*
 * The following three functions were copied from FreeBSD's chown
 * utility.
 */
static uid_t
id(const char *name, const char *type)
{
	uid_t val;
	char *ep;

	val = strtoul(name, &ep, 10);
	if (*ep != '\0')
		v4b_errx(EX_USAGE, "%s: illegal %s name", name, type);
	return (val);
}

static void
a_gid(const char *s)
{
	struct group *gr;

	gid = ((gr = getgrnam(s)) != NULL) ? gr->gr_gid : id(s, "group");
	gid_found = 1;
}

static void
a_uid(const char *s)
{
	struct passwd *pw;

	uid = ((pw = getpwnam(s)) != NULL) ? pw->pw_uid : id(s, "user");
	uid_found = 1;
}

int
main(int argc, char **argv)
{
	const char *params = "N:Bd:f:i:M:m:S:sv:hHrU:G:D:lL:";
	char *ptr;
	int opt;
	int opt_valid = 0;

	openlog("webcamd", LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_DAEMON);
	while ((opt = getopt(argc, argv, params)) != -1) {
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

		case 'M':
			u_match_index = atoi(optarg);
			break;

		case 'N':
			u_devicename = optarg;
			break;

		case 'S':
			u_serialname = optarg;
			break;

		case 'l':
			do_list = 1;
			break;

		case 'i':
			u_index = atoi(optarg);
			break;

		case 'D':
		case 'L':
		case 's':
			opt_valid = 1;
			break;
		case 'm':
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

		case 'r':
			do_realtime = 0;
			break;

		case 'H':
			webcamd_hal_register = 1;
			break;

		case 'U':
			a_uid(optarg);
			break;

		case 'G':
			a_gid(optarg);
			break;

		default:
			usage();
			break;
		}
	}

	if (!uid_found)
		a_uid("webcamd");

	if (!gid_found)
		a_gid("webcamd");

	if (u_devicename != NULL || u_serialname != NULL || do_list != 0) {
		find_devices();
	} else if (u_addr == 0 && opt_valid == 0) {
		/* list devices by default if no option was specified */
		do_list = 1;
		find_devices();
	}
	if (do_fork) {
		/* need to daemonise before creating any threads */
		if (pidfile_create(u_unit, u_addr, u_index)) {
			syslog(LOG_ERR, "Webcamd is already running for "
			    "ugen%d.%d.%d\n", u_unit, u_addr, u_index);
			exit(EX_USAGE);
		}
		if (daemon(0, 0) != 0)
			v4b_errx(EX_USAGE, "Cannot daemonize");
	}
	atexit(&v4b_exit);

	if (cuse_init() != 0) {
		v4b_errx(EX_USAGE, "Could not open /dev/cuse. "
		    "Did you kldload cuse4bsd?");
	}
	if (do_realtime != 0) {
		struct rtprio rtp;

		memset(&rtp, 0, sizeof(rtp));

		rtp.type = RTP_PRIO_REALTIME;
		rtp.prio = 8;

		if (rtprio(RTP_SET, getpid(), &rtp) != 0)
			syslog(LOG_WARNING, "Cannot set realtime priority\n");
	}
	/* get all module parameters registered */

	linux_parm();

	/* process all module parameters */

	optreset = 1;
	optind = 1;

	while ((opt = getopt(argc, argv, params)) != -1) {
		switch (opt) {
			char *ndev;
			char *type;
			char *host;
			char *port;

		case 'm':
			ptr = strstr(optarg, "=");
			if (ptr == NULL) {
				v4b_errx(EX_USAGE, "invalid parameter for "
				    "-m option: '%s'", optarg);
			}
			*ptr = 0;
			if (mod_set_param(optarg, ptr + 1) < 0) {
				syslog(LOG_WARNING, "cannot set module "
				    "parameter '%s'='%s'\n", optarg, ptr + 1);
			}
			break;

		case 'D':
			host = optarg;
			port = strstr(host, ":");
			if (port == NULL) {
				v4b_errx(EX_USAGE, "invalid syntax for "
				    "-D option: '%s'", optarg);
			}
			*port++ = 0;
			ndev = strstr(port, ":");
			if (ndev == NULL) {
				v4b_errx(EX_USAGE, "invalid syntax for "
				    "-D option: '%s'", port);
			}
			*ndev++ = 0;
			ptr = strstr(ndev, ":");
			if (ptr != NULL)
				*ptr = 0;

			if (mod_set_param("vtuner_client.devices", ndev) < 0 ||
			    mod_set_param("vtuner_client.host", host) < 0 ||
			    mod_set_param("vtuner_client.port", port) < 0) {
				v4b_errx(EX_USAGE, "Cannot set all module "
				    "parameters for vTuner client.");
			}
			break;

		case 'L':
			host = optarg;
			port = strstr(host, ":");
			if (port == NULL) {
				v4b_errx(EX_USAGE, "invalid syntax for "
				    "-L option: '%s'", optarg);
			}
			*port++ = 0;
			ndev = strstr(port, ":");
			if (ndev == NULL) {
				v4b_errx(EX_USAGE, "invalid syntax for "
				    "-L option: '%s'", port);
			}
			*ndev++ = 0;
			ptr = strstr(ndev, ":");
			if (ptr != NULL)
				*ptr = 0;

			if (mod_set_param("vtuner_server.devices", ndev) < 0 ||
			    mod_set_param("vtuner_server.host", host) < 0 ||
			    mod_set_param("vtuner_server.port", port) < 0) {
				v4b_errx(EX_USAGE, "Cannot set all module "
				    "parameters for vTuner server.");
			}
			break;

		case 's':
			printf("List of available parameters:\n");
			mod_show_params();
			exit(0);

		default:
			break;
		}
	}
	if (mod_get_int_param("vtuner_client.devices") != 0)
		vtuner_client = 1;

	if (mod_get_int_param("vtuner_server.devices") != 0)
		vtuner_server = 1;

	if (vtuner_client && vtuner_server)
		v4b_errx(EX_USAGE, "Cannot specify both vTuner server and client");

	/* system init */

	thread_init();
	idr_init_cache();

	/* run rest of Linux init code */

	linux_init();
	linux_late();

	if (vtuner_client == 0) {
		if (usb_linux_probe_p(&u_unit, &u_addr, &u_index, &d_desc) < 0)
			v4b_errx(EX_USAGE, "Cannot find USB device");
	}
	if (vtuner_server == 0) {
		if (webcamd_hal_register)
			hal_init(u_unit, u_addr, u_index, d_desc);

		v4b_create(u_videodev);

		v4b_work(NULL);
	} else {
		while (1)
			pause();
	}
	return (0);
}

unsigned long
copy_to_user(void *to, const void *from, unsigned long n)
{
	int error;

	if (vtuner_server != 0) {
		memcpy(to, from, n);
		return (0);
	}
#ifdef CONFIG_COMPAT
	error = compat_copy_to_user(to, from, n);
	if (error != 0)
#endif
		error = cuse_copy_out(from, to, (int)n);

	return ((error != 0) ? n : 0);
}

unsigned long
copy_from_user(void *to, const void *from, unsigned long n)
{
	int error;

	if (vtuner_server != 0) {
		memcpy(to, from, n);
		return (0);
	}
#ifdef CONFIG_COMPAT
	error = compat_copy_from_user(to, from, n);
	if (error != 0)
#endif
		error = cuse_copy_in(from, to, (int)n);

	return ((error != 0) ? n : 0);
}

unsigned long
copy_in_user(void *to, const void *from, unsigned long len)
{
	uint8_t buffer[PAGE_SIZE] __aligned(sizeof(void *));
	unsigned long oldlen = len;

	while (len > 0) {
		unsigned long delta = len;
		if (delta > sizeof(buffer))
			delta = sizeof(buffer);

		if (copy_from_user(buffer, from, delta) != 0)
			return (oldlen);
		if (copy_to_user(to, buffer, delta) != 0)
			return (oldlen);

		to = (char *)to + delta;
		from = (const char *)from + delta;
		len -= delta;
	}
	return (0);	/* success */
}

static uint32_t zero_alloc[1];

int
is_vmalloc_addr(void *addr)
{
	return (cuse_is_vmalloc_addr(addr));
}

void   *
malloc_vm(size_t size)
{
	if (size == 0)
		return (zero_alloc);

	return (cuse_vmalloc(size));
}

void
free_vm(void *ptr)
{
	if (ptr == zero_alloc)
		return;

	cuse_vmfree(ptr);
}

int
check_signal(void)
{
	return (cuse_got_peer_signal() == 0 ||
	    thread_got_stopping() == 0);
}

void
poll_wakeup_internal(void)
{
	cuse_poll_wakeup();
}

struct cdev_handle *
get_current_cdev_handle(void)
{
	struct cuse_dev *cdev;

	cdev = cuse_dev_get_current(NULL);

	if (cdev == NULL)
		return (NULL);

	return (cuse_dev_get_per_file_handle(cdev));
}
