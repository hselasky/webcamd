/*-
 * Copyright (c) 2009-2021 Hans Petter Selasky. All rights reserved.
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

#include <signal.h>

#include <sys/filio.h>

#include <linux/input.h>

#define to_urb(d) container_of(d, struct urb, kref)

static int min_bufsize;

module_param(min_bufsize, int, 0644);
MODULE_PARM_DESC(min_bufsize, "Set minimum USB buffer size");

static int min_frames;

module_param(min_frames, int, 0644);
MODULE_PARM_DESC(min_frames, "Set minimum ISOC buffering in milliseconds");

struct usb_linux_softc {
	struct libusb20_config *pcfg;
	struct libusb20_device *pdev;
	struct usb_driver *udrv;
	struct usb_device *p_dev;
	struct usb_interface *ui;
	pthread_t thread;
	volatile int thread_started;
	volatile int thread_stopping;
};

static struct usb_linux_softc uls[16];
static struct device usb_dummy_bus;

/* prototypes */

static libusb20_tr_callback_t usb_linux_isoc_callback;
static libusb20_tr_callback_t usb_linux_bulk_intr_callback;
static libusb20_tr_callback_t usb_linux_ctrl_callback;

static const struct usb_device_id *usb_linux_lookup_id(struct LIBUSB20_DEVICE_DESC_DECODED *pdd, struct LIBUSB20_INTERFACE_DESC_DECODED *pid, const struct usb_device_id *id);
static void usb_linux_free_device(struct usb_device *dev);

static struct usb_device *usb_linux_create_usb_device(struct usb_linux_softc *sc, struct libusb20_device *udev, struct libusb20_config *pcfg, uint16_t addr);
static void usb_linux_cleanup_interface(struct usb_device *, struct usb_interface *);
static void usb_linux_complete(struct libusb20_transfer *);
static int usb_unlink_urb_sub(struct urb *, uint8_t);
static int usb_setup_endpoint(struct usb_device *dev, struct usb_host_endpoint *uhe, int bufsize);
static struct usb_host_endpoint *usb_find_host_endpoint(struct usb_device *dev, unsigned int pipe);

/*------------------------------------------------------------------------*
 * FreeBSD USB interface
 *------------------------------------------------------------------------*/

static struct {
	struct usb_driver *lh_first;
}	usb_linux_driver_list;

/*------------------------------------------------------------------------*
 *	usb_linux_lookup_id
 *
 * This functions takes an array of "struct usb_device_id" and tries
 * to match the entries with the information in "struct usb2_attach_arg".
 * If it finds a match the matching entry will be returned.
 * Else "NULL" will be returned.
 *------------------------------------------------------------------------*/
static const struct usb_device_id *
usb_linux_lookup_id(
    struct LIBUSB20_DEVICE_DESC_DECODED *pdd,
    struct LIBUSB20_INTERFACE_DESC_DECODED *pid,
    const struct usb_device_id *id)
{
	if (id == NULL) {
		goto done;
	}
	/*
	 * Keep on matching array entries until we find one with
	 * "match_flags" equal to zero, which indicates the end of the
	 * array:
	 */
	for (; id->match_flags; id++) {

		if ((id->match_flags & USB_DEVICE_ID_MATCH_VENDOR) &&
		    (id->idVendor != pdd->idVendor)) {
			continue;
		}
		if ((id->match_flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
		    (id->idProduct != pdd->idProduct)) {
			continue;
		}
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
		    (id->bcdDevice_lo > pdd->bcdDevice)) {
			continue;
		}
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
		    (id->bcdDevice_hi < pdd->bcdDevice)) {
			continue;
		}
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
		    (id->bDeviceClass != pdd->bDeviceClass)) {
			continue;
		}
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
		    (id->bDeviceSubClass != pdd->bDeviceSubClass)) {
			continue;
		}
		if ((id->match_flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
		    (id->bDeviceProtocol != pdd->bDeviceProtocol)) {
			continue;
		}
		if ((pdd->bDeviceClass == 0xFF) &&
		    (!(id->match_flags & USB_DEVICE_ID_MATCH_VENDOR)) &&
		    (id->match_flags & USB_DEVICE_ID_MATCH_INT_INFO)) {
			continue;
		}
		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS) &&
		    (id->bInterfaceClass != pid->bInterfaceClass)) {
			continue;
		}
		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS) &&
		    (id->bInterfaceSubClass != pid->bInterfaceSubClass)) {
			continue;
		}
		if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL) &&
		    (id->bInterfaceProtocol != pid->bInterfaceProtocol)) {
			continue;
		}
		/* we found a match! */
		return (id);
	}

done:
	return (NULL);
}

static void
thread_io(int dummy)
{
}

static void *
usb_exec(void *arg)
{
	struct usb_linux_softc *sc = arg;
	struct libusb20_device *dev = sc->p_dev->bsd_udev;
	int err;

	signal(SIGIO, &thread_io);

	sc->thread_started = 1;

	while (1) {

		atomic_lock();
		err = libusb20_dev_process(dev);
		atomic_unlock();

		/* check for USB events */
		if (err != 0) {
			/* check if device is detached */
			err = 1;
			err = ioctl(libusb20_dev_get_fd(dev), FIONBIO, &err);
			if (err != 0)
				exit(0);
			if (sc->thread_stopping)
				break;
			else
				usleep(100000);
		}
		/* wait for USB event from kernel */
		libusb20_dev_wait_process(dev, -1);

		if (sc->thread_stopping)
			break;
	}

	sc->thread_started = 0;

	pthread_exit(NULL);

	return (NULL);
}

static void
usb_linux_create_event_thread(struct usb_device *dev)
{
	struct usb_linux_softc *sc = dev->parent;

	if (sc->thread_started)
		return;

	sc->thread_started = 0;
	sc->thread_stopping = 0;

	/* start USB event thread again */
	if (pthread_create(&sc->thread, NULL,
	    usb_exec, sc)) {
		printf("Failed creating USB process\n");
	} else {
		uint32_t drops;

		atomic_lock();
		drops = atomic_drop();
		atomic_unlock();

		while (sc->thread_started == 0)
			schedule();

		atomic_lock();
		atomic_pickup(drops);
		atomic_unlock();
	}
}

struct usb_linux_softc *
usb_linux2usb(int fd)
{
	return (uls + fd);
}

void
usb_linux_detach_sub(struct usb_linux_softc *sc)
{
	struct usb_device *p_dev = sc->p_dev;
	struct usb_interface *ui = sc->ui;

	/*
	 * Make sure that we free all FreeBSD USB transfers belonging to
	 * this Linux "usb_interface", hence they will most likely not be
	 * needed any more.
	 */
	usb_linux_cleanup_interface(p_dev, ui);

	libusb20_dev_close(sc->pdev);

	usb_linux_free_device(p_dev);

	free(sc->pcfg);

	memset(sc, 0, sizeof(*sc));
}

static void
usb_linux_fill_ep_info(struct usb_device *udev,
    struct usb_host_interface *uhi)
{
	struct usb_host_endpoint *uhe_end;
	struct usb_host_endpoint *uhe;
	uint8_t ea;

	uhe_end = uhi->endpoint + uhi->desc.bNumEndpoints;

	for (uhe = uhi->endpoint; uhe != uhe_end; uhe++) {

		ea = uhe->desc.bEndpointAddress;

		/* skip any bogus control endpoints */
		if ((ea & USB_ENDPOINT_NUMBER_MASK) == 0)
			continue;

		if (ea & USB_ENDPOINT_DIR_MASK)
			udev->ep_in[ea & 15] = uhe;
		else
			udev->ep_out[ea & 15] = uhe;
	}
}

/*------------------------------------------------------------------------*
 *	usb_linux_probe
 *
 * This function is the FreeBSD probe and attach callback.
 *------------------------------------------------------------------------*/
int
usb_linux_probe_p(int *p_bus, int *p_addr, int *p_index, const char **pp_desc)
{
	const struct usb_device_id *id;
	struct usb_linux_softc *sc;
	struct libusb20_backend *pbe;
	struct libusb20_device *pdev;
	struct libusb20_config *pcfg = NULL;
	struct libusb20_interface *pifc;
	struct usb_driver *udrv;
	struct usb_device *p_dev;
	struct usb_interface *ui;
	uint8_t i;
	uint8_t match_bus_addr;
	uint8_t index_copy;
	uint8_t device_index;
	uint8_t bus = *p_bus;
	uint8_t addr = *p_addr;
	uint8_t index = *p_index;

	for (i = 0;; i++) {
		if (i == ARRAY_SIZE(uls))
			return (-ENOMEM);
		if (uls[i].udrv == NULL)
			break;
	}

	sc = uls + i;

	device_index = i;

	match_bus_addr = (addr != 0);

	if (!match_bus_addr)
		index = bus;

	index_copy = index;

	pbe = libusb20_be_alloc_default();
	if (pbe == NULL)
		return (-ENXIO);

	pdev = NULL;
	while ((pdev = libusb20_be_device_foreach(pbe, pdev))) {

		if (libusb20_dev_get_mode(pdev) != LIBUSB20_MODE_HOST)
			continue;

		if (match_bus_addr) {
			if (libusb20_dev_get_bus_number(pdev) != bus)
				continue;
			if (libusb20_dev_get_address(pdev) != addr)
				continue;
		} else {
			addr = libusb20_dev_get_address(pdev);
		}
		if (libusb20_dev_open(pdev, 4 * 16))
			continue;

		pcfg = libusb20_dev_alloc_config(pdev,
		    libusb20_dev_get_config_index(pdev));
		if (pcfg == NULL)
			continue;

		for (i = 0; i != pcfg->num_interface; i++) {
			pifc = pcfg->interface + i;

			/*
			 * Skip the FreeBSD USB root HUBs due to false
			 * detection by some V4L drivers.
			 */
			if (pifc->desc.bInterfaceClass == USB_CLASS_HUB)
				continue;

			LIST_FOREACH(udrv, &usb_linux_driver_list, linux_driver_list) {
				id = usb_linux_lookup_id(libusb20_dev_get_device_desc(pdev),
				    &pifc->desc, udrv->id_table);
				if (id != NULL) {
					if (!index--)
						goto found;

					/*
					 * Allow for multiple webcams on the
					 * same device
					 */

					if ((!match_bus_addr) &&
					    (!(id->match_flags & USB_DEVICE_ID_MATCH_INT_INFO))) {
						goto next_dev;
					}
					/* Proceed with next interface */
					break;
				}
			}
		}
next_dev:
		free(pcfg);
		libusb20_dev_close(pdev);
	}
	libusb20_be_free(pbe);
	return (-ENXIO);

found:
	switch (libusb20_dev_get_speed(pdev)) {
	case LIBUSB20_SPEED_LOW:
		webcamd_speed = 1;
		break;
	case LIBUSB20_SPEED_FULL:
		webcamd_speed = 12;
		break;
	case LIBUSB20_SPEED_SUPER:
		webcamd_speed = 5000;
		break;
	default:
		webcamd_speed = 480;
		break;
	}
	webcamd_vendor = libusb20_dev_get_device_desc(pdev)->idVendor;
	webcamd_product = libusb20_dev_get_device_desc(pdev)->idProduct;

	*p_bus = libusb20_dev_get_bus_number(pdev);
	*p_addr = libusb20_dev_get_address(pdev);
	*p_index = index_copy;
	*pp_desc = udrv->name;

	if (pidfile_create(*p_bus, *p_addr, index_copy)) {
		fprintf(stderr, "Webcamd is already running for "
		    "ugen%d.%d.%d\n",
		    libusb20_dev_get_bus_number(pdev),
		    libusb20_dev_get_address(pdev),
		    index_copy);
		exit(1);
	}
	p_dev = usb_linux_create_usb_device(sc, pdev, pcfg, addr);
	if (p_dev == NULL) {
		free(pcfg);
		libusb20_be_free(pbe);
		return (-ENOMEM);
	}
	ui = p_dev->bsd_iface_start + i;
	sc->ui = ui;

	sc->udrv = udrv;
	sc->p_dev = p_dev;
	sc->pcfg = pcfg;
	sc->pdev = pdev;

	usb_linux_create_event_thread(p_dev);

	while (udrv->probe(ui, id) != 0) {
		i++;
try_interface:
		/* try next interface, if any */
		if (i != pcfg->num_interface) {
			pifc = pcfg->interface + i;
			ui = p_dev->bsd_iface_start + i;
			sc->ui = ui;

			if (pifc->desc.bInterfaceClass == USB_CLASS_HUB) {
				i++;
				goto try_interface;
			}
			id = usb_linux_lookup_id(
			    libusb20_dev_get_device_desc(pdev),
			    &pifc->desc, udrv->id_table);
			if (id == NULL) {
				i++;
				goto try_interface;
			}
			continue;
		}

		/* try next driver, if any */
		if ((udrv = LIST_NEXT(udrv, linux_driver_list)) != NULL) {
			*pp_desc = udrv->name;
			sc->udrv = udrv;
			i = 0;
			goto try_interface;
		}
		usb_linux_detach_sub(sc);
		libusb20_be_free(pbe);
		return (-ENXIO);
	}
	libusb20_be_dequeue_device(pbe, pdev);
	libusb20_be_free(pbe);

	/* detach kernel driver, if any */
	libusb20_dev_detach_kernel_driver(pdev, i);

	return (device_index);
}

/*------------------------------------------------------------------------*
 *	usb_linux_detach
 *
 * This function is the FreeBSD detach callback. It is called from the
 * FreeBSD USB stack through the "device_detach()" function.
 *------------------------------------------------------------------------*/
int
usb_linux_detach(int fd)
{
	struct usb_linux_softc *sc = usb_linux2usb(fd);
	struct usb_driver *udrv = sc->udrv;
	struct usb_interface *ui = sc->ui;
	struct libusb20_device *pdev = sc->pdev;

	(udrv->disconnect) (ui);

	usb_linux_detach_sub(sc);

	libusb20_dev_free(pdev);

	return (0);
}

/*------------------------------------------------------------------------*
 *	usb_linux_suspend
 *
 * This function is the FreeBSD suspend callback. Usually it does nothing.
 *------------------------------------------------------------------------*/
int
usb_linux_suspend(int fd)
{
	struct usb_linux_softc *sc = usb_linux2usb(fd);
	struct usb_driver *udrv = sc->udrv;
	struct usb_interface *ui = sc->ui;
	struct pm_message pm = {0};

	if (udrv->suspend) {
		(udrv->suspend) (ui, pm);
	}
	return (0);
}

/*------------------------------------------------------------------------*
 *	usb_linux_resume
 *
 * This function is the FreeBSD resume callback. Usually it does nothing.
 *------------------------------------------------------------------------*/
int
usb_linux_resume(int fd)
{
	struct usb_linux_softc *sc = usb_linux2usb(fd);
	struct usb_driver *udrv = sc->udrv;
	struct usb_interface *ui = sc->ui;

	if (udrv->resume) {
		(udrv->resume) (ui);
	}
	return (0);
}

/*------------------------------------------------------------------------*
 * Linux emulation layer
 *------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*
 *	usb_max_isoc_frames
 *
 * The following function returns the maximum number of isochronous
 * frames that we support per URB. It is not part of the Linux USB API.
 *------------------------------------------------------------------------*/
static uint16_t
usb_max_isoc_frames(struct usb_device *dev, struct usb_host_endpoint *uhe)
{
	uint32_t frames;
	uint8_t fps_shift;

	frames = 8 * min_frames;
	if (frames == 0)
		frames = 8 * 30;	/* use 30ms default buffering time */

	switch (libusb20_dev_get_speed(dev->bsd_udev)) {
	case LIBUSB20_SPEED_LOW:
	case LIBUSB20_SPEED_FULL:
		return (frames / 8);
	default:
		fps_shift = uhe->desc.bInterval;
		if (fps_shift > 0)
			fps_shift--;
		if (fps_shift > 3)
			fps_shift = 3;
		return (frames >> fps_shift);
	}
}

/*------------------------------------------------------------------------*
 *	usb_get_current_frame_number
 *
 * The following function returns the last received frame number
 * from the USB controller.
 *------------------------------------------------------------------------*/
uint16_t
usb_get_current_frame_number(struct usb_device *dev)
{
	uint16_t temp;

	atomic_lock();
	temp = dev->bsd_last_ms;
	atomic_unlock();

	switch (libusb20_dev_get_speed(dev->bsd_udev)) {
	case LIBUSB20_SPEED_LOW:
	case LIBUSB20_SPEED_FULL:
		return (temp);
	default:
		return (temp * 8);
	}
}

/*------------------------------------------------------------------------*
 *	usb_submit_urb
 *
 * This function is used to queue an URB after that it has been
 * initialized. If it returns non-zero, it means that the URB was not
 * queued.
 *------------------------------------------------------------------------*/
static void
usb_submit_urb_sub(struct libusb20_transfer *xfer)
{
	if (xfer == NULL)
		return;
	libusb20_tr_start(xfer);
}

int
usb_submit_urb(struct urb *urb, uint16_t mem_flags)
{
	struct usb_host_endpoint *uhe;
	int err;

	if (urb == NULL)
		return (-EINVAL);

	atomic_lock();
	if (urb->reject != 0) {
		atomic_unlock();
		return (-ENXIO);
	}
	uhe = usb_find_host_endpoint(urb->dev, urb->pipe);
	if (uhe == NULL) {
		atomic_unlock();
		return (-EINVAL);
	}
	err = usb_setup_endpoint(urb->dev, uhe,
	    urb->transfer_buffer_length);
	if (err) {
		atomic_unlock();
		return (-EPIPE);
	}
	/*
	 * Check that we have got a FreeBSD USB transfer that will dequeue
	 * the URB structure and do the real transfer. If there are no USB
	 * transfers, then we return an error.
	 */
	if (uhe->bsd_xfer[0] ||
	    uhe->bsd_xfer[1]) {
		/* we are ready! */

		/* indicate that we are queued */
		urb->hcpriv = USB_HCPRIV_QUEUED;

		/* check if URB is not already submitted */
		if (urb->bsd_urb_list.tqe_prev == NULL) {
			TAILQ_INSERT_TAIL(&uhe->bsd_urb_list, urb, bsd_urb_list);
			urb->status = -EINPROGRESS;
		}
		/*
		 * Check if this is a re-submit in context of a USB
		 * callback. If that is the case, we should not start
		 * any more USB transfers!
		 */
		if (urb->bsd_no_resubmit == 0) {
			usb_submit_urb_sub(uhe->bsd_xfer[0]);
			usb_submit_urb_sub(uhe->bsd_xfer[1]);
		}
		err = 0;
	} else {
		/* no pipes have been setup yet! */
		urb->status = -EINVAL;
		err = -EINVAL;
	}
	atomic_unlock();
	return (err);
}

/*------------------------------------------------------------------------*
 *	usb_unlink_urb
 *
 * This function is used to stop an URB after that it is been
 * submitted, but before the "complete" callback has been called. On
 *------------------------------------------------------------------------*/
int
usb_unlink_urb(struct urb *urb)
{
	int err;

	atomic_lock();
	err = usb_unlink_urb_sub(urb, 0);
	atomic_unlock();
	return (err);
}

static void
usb_unlink_bsd(struct libusb20_transfer *xfer,
    struct urb *urb, uint8_t drain)
{
	uint32_t drops;

	if ((xfer != NULL) && (urb != NULL)) {
		while (libusb20_tr_get_priv_sc1(xfer) == (void *)urb) {

			/* restart transfer */
			atomic_lock();
			libusb20_tr_stop(xfer);
			libusb20_tr_start(xfer);
			atomic_unlock();

			/* check if we should drain */
			if (drain == 0)
				break;

			atomic_lock();
			drops = atomic_drop();
			atomic_unlock();

			usleep(10000);

			atomic_lock();
			atomic_pickup(drops);
			atomic_unlock();
		}
	}
}

static int
usb_unlink_urb_sub(struct urb *urb, uint8_t drain)
{
	struct usb_host_endpoint *uhe;
	uint16_t x;

	if (urb == NULL) {
		return (-EINVAL);
	}
	uhe = usb_find_host_endpoint(urb->dev, urb->pipe);
	if (uhe == NULL) {
		return (-EINVAL);
	}
	if (urb->bsd_urb_list.tqe_prev) {

		/* not started yet, just remove it from the queue */
		TAILQ_REMOVE(&uhe->bsd_urb_list, urb, bsd_urb_list);
		urb->bsd_urb_list.tqe_prev = NULL;
		urb->status = -ECONNRESET;
		urb->actual_length = 0;

		for (x = 0; x < urb->number_of_packets; x++) {
			urb->iso_frame_desc[x].actual_length = 0;
		}

		if (urb->complete) {
			urb->hcpriv = NULL;
			(urb->complete) (urb);
		}
	} else {

		/*
		 * If the URB is not on the URB list, then check if one of
		 * the FreeBSD USB transfer are processing the current URB.
		 * If so, re-start that transfer, which will lead to the
		 * termination of that URB:
		 */
		usb_unlink_bsd(uhe->bsd_xfer[0], urb, drain);
		usb_unlink_bsd(uhe->bsd_xfer[1], urb, drain);
	}
	return (0);
}

/*------------------------------------------------------------------------*
 *	usb_clear_halt
 *
 * This function must always be used to clear the stall. Stall is when
 * an USB endpoint returns a stall message to the USB host controller.
 * Until the stall is cleared, no data can be transferred.
 *------------------------------------------------------------------------*/
int
usb_clear_halt(struct usb_device *dev, unsigned int pipe)
{
	struct usb_host_endpoint *uhe;
	int err;

	atomic_lock();
	uhe = usb_find_host_endpoint(dev, pipe);
	if (uhe == NULL) {
		atomic_unlock();
		return (-EINVAL);
	}
	err = usb_setup_endpoint(dev, uhe, 0);
	atomic_unlock();
	if (err)
		return (-EPIPE);

	libusb20_tr_clear_stall_sync(uhe->bsd_xfer[0]);

	return (0);			/* success */
}

/*------------------------------------------------------------------------*
 *	usb_control_msg
 *
 * The following function performs a control transfer sequence one any
 * control, bulk or interrupt endpoint, specified by "uhe". A control
 * transfer means that you transfer an 8-byte header first followed by
 * a data-phase as indicated by the 8-byte header. The "timeout" is
 * given in milliseconds.
 *
 * Return values:
 *   0: Success
 * < 0: Failure
 * > 0: Acutal length
 *------------------------------------------------------------------------*/
int
usb_control_msg(struct usb_device *dev, unsigned int pipe,
    uint8_t request, uint8_t requesttype,
    uint16_t value, uint16_t wIndex, void *data,
    uint16_t size, uint32_t timeout)
{
	struct LIBUSB20_CONTROL_SETUP_DECODED req;
	struct usb_host_endpoint *uhe;
	int err;
	uint16_t actlen;
	uint8_t type;
	uint8_t addr;

	LIBUSB20_INIT(LIBUSB20_CONTROL_SETUP, &req);

	req.bmRequestType = requesttype;
	req.bRequest = request;
	req.wValue = value;
	req.wIndex = wIndex;
	req.wLength = size;

	atomic_lock();
	uhe = usb_find_host_endpoint(dev, pipe);
	atomic_unlock();

	if (uhe == NULL) {
		return (-EINVAL);
	}
	type = (uhe->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK);
	addr = (uhe->desc.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);

	if (type != USB_ENDPOINT_XFER_CONTROL) {
		return (-EINVAL);
	}
	if (addr == 0) {
		/*
		 * The FreeBSD USB stack supports standard control
		 * transfers on control endpoint zero:
		 */
		err = libusb20_dev_request_sync(dev->bsd_udev, &req,
		    data, &actlen, timeout, 0);
		if (err)
			return (-EPIPE);

		return (actlen);
	}
	/* TODO: implement support for non-endpoint zero */
	return (-EINVAL);
}

/*------------------------------------------------------------------------*
 *	usb_set_interface
 *
 * The following function will select which alternate setting of an
 * USB interface you plan to use. By default alternate setting with
 * index zero is selected. Note that "iface_no" is not the interface
 * index, but rather the value of "bInterfaceNumber".
 *------------------------------------------------------------------------*/
int
usb_set_interface(struct usb_device *dev, uint8_t iface_no, uint8_t alt_index)
{
	struct usb_interface *p_ui = usb_ifnum_to_if(dev, iface_no);
	struct usb_interface *ui;
	uint32_t drops;
	int err;

	if (p_ui == NULL)
		return (-EINVAL);
	if (alt_index >= p_ui->num_altsetting)
		return (-EINVAL);

	/* XXX hack to get rid of any locks */

	atomic_lock();
	drops = atomic_drop();
	atomic_unlock();

	/*
	 * Due to the LibUSB v2.0 design, doing an alternate setting
	 * on one endpoint means we need to kill the URB's on the
	 * other endpoints aswell!
	 */
	for (ui = dev->bsd_iface_start;
	    ui != dev->bsd_iface_end; ui++)
		usb_linux_cleanup_interface(dev, ui);

	err = libusb20_dev_set_alt_index(dev->bsd_udev,
	    p_ui->bsd_iface_index, alt_index);

	/* XXX */

	atomic_lock();
	atomic_pickup(drops);
	atomic_unlock();

	if (err) {
		err = -EPIPE;
	} else {
		p_ui->cur_altsetting = p_ui->altsetting + alt_index;

		usb_linux_fill_ep_info(dev, p_ui->cur_altsetting);
	}

	usb_linux_create_event_thread(dev);

	return (err);
}

static void
usb_unsetup_endpoint(struct usb_device *dev,
    struct usb_host_endpoint *uhe)
{
	if (uhe->bsd_xfer[0]) {
		libusb20_tr_close(uhe->bsd_xfer[0]);
		uhe->bsd_xfer[0] = NULL;
	}
	if (uhe->bsd_xfer[1]) {
		libusb20_tr_close(uhe->bsd_xfer[1]);
		uhe->bsd_xfer[1] = NULL;
	}
}

/*------------------------------------------------------------------------*
 *	usb_setup_endpoint
 *
 * The following function is an extension to the Linux USB API that
 * allows you to set a maximum buffer size for a given USB endpoint.
 * The maximum buffer size is per URB. If you don't call this function
 * to set a maximum buffer size, the endpoint will not be functional.
 * Note that for isochronous endpoints the maximum buffer size must be
 * a non-zero dummy, hence this function will base the maximum buffer
 * size on "wMaxPacketSize".
 *------------------------------------------------------------------------*/
static int
usb_setup_endpoint(struct usb_device *dev,
    struct usb_host_endpoint *uhe, int bufsize)
{
	uint8_t type = uhe->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	uint8_t addr = uhe->desc.bEndpointAddress;
	uint8_t ep_index = (((addr & 0x80) / 0x40) | (addr * 4)) % (16 * 4);
	uint8_t speed;

	if (uhe->bsd_xfer[0] ||
	    uhe->bsd_xfer[1]) {
		/* transfer already setup */
		return (0);
	}
	uhe->bsd_xfer[0] = libusb20_tr_get_pointer(dev->bsd_udev, ep_index + 0);
	uhe->bsd_xfer[1] = libusb20_tr_get_pointer(dev->bsd_udev, ep_index + 1);

	if (type == USB_ENDPOINT_XFER_CONTROL) {
		unsigned bufsize = 65536 + 8;

		/* one transfer and one frame */
		if (libusb20_tr_open(uhe->bsd_xfer[0], bufsize,
		    2, addr))
			goto failure;

		if (libusb20_tr_open(uhe->bsd_xfer[1], bufsize,
		    2, addr))
			goto failure;

		libusb20_tr_set_callback(uhe->bsd_xfer[0],
		    &usb_linux_ctrl_callback);
		libusb20_tr_set_callback(uhe->bsd_xfer[1],
		    &usb_linux_ctrl_callback);

		libusb20_tr_set_priv_sc0(uhe->bsd_xfer[0], uhe);
		libusb20_tr_set_priv_sc0(uhe->bsd_xfer[1], uhe);

	} else if (type == USB_ENDPOINT_XFER_ISOC) {
		/*
		 * Isochronous transfers are special in that they don't fit
		 * into the BULK/INTR/CONTROL transfer model.
		 */

		/* need double buffering */

		if (libusb20_tr_open(uhe->bsd_xfer[0], 0,
		    usb_max_isoc_frames(dev, uhe), addr))
			goto failure;

		if (libusb20_tr_open(uhe->bsd_xfer[1], 0,
		    usb_max_isoc_frames(dev, uhe), addr))
			goto failure;

		libusb20_tr_set_callback(uhe->bsd_xfer[0],
		    &usb_linux_isoc_callback);
		libusb20_tr_set_callback(uhe->bsd_xfer[1],
		    &usb_linux_isoc_callback);

		libusb20_tr_set_priv_sc0(uhe->bsd_xfer[0], uhe);
		libusb20_tr_set_priv_sc0(uhe->bsd_xfer[1], uhe);

	} else {

		speed = libusb20_dev_get_speed(dev->bsd_udev);

		/* figure out a sensible buffer size */
		if (bufsize < 0)
			bufsize = 0;
		if (bufsize < min_bufsize)
			bufsize = min_bufsize;
		if (speed == LIBUSB20_SPEED_LOW) {
			if (bufsize < 256)
				bufsize = 256;
		} else if (type == USB_ENDPOINT_XFER_INT ||
		    speed == LIBUSB20_SPEED_FULL) {
			if (bufsize < 4096)
				bufsize = 4096;
		} else {
			if (bufsize < 131072)
				bufsize = 131072;
		}

		/* one transfer and one frame */
		if (libusb20_tr_open(uhe->bsd_xfer[0], bufsize,
		    1, addr))
			goto failure;

		if (libusb20_tr_open(uhe->bsd_xfer[1], bufsize,
		    1, addr))
			goto failure;

		libusb20_tr_set_callback(uhe->bsd_xfer[0],
		    &usb_linux_bulk_intr_callback);
		libusb20_tr_set_callback(uhe->bsd_xfer[1],
		    &usb_linux_bulk_intr_callback);

		libusb20_tr_set_priv_sc0(uhe->bsd_xfer[0], uhe);
		libusb20_tr_set_priv_sc0(uhe->bsd_xfer[1], uhe);
	}
	return (0);

failure:
	if (uhe->bsd_xfer[0]) {
		libusb20_tr_close(uhe->bsd_xfer[0]);
		uhe->bsd_xfer[0] = NULL;
	}
	if (uhe->bsd_xfer[1]) {
		libusb20_tr_close(uhe->bsd_xfer[1]);
		uhe->bsd_xfer[1] = NULL;
	}
	return (-EINVAL);
}

static char *
usb_string_dup(struct usb_device *udev, uint8_t string_id)
{
	char buf[80];
	int size;

	if (string_id == 0)
		return (strdup(""));

	if ((size = usb_string(udev, string_id,
	    buf, sizeof(buf) - 1)) > 0) {
		buf[size] = '\0';
		return (strdup(buf));
	}
	return (strdup(""));
}

static struct usb_interface_assoc_descriptor *
usb_find_iad(struct usb_device *p_ud, int max, u8 inum)
{
	struct usb_interface_assoc_descriptor *intf_assoc;
	int first_intf;
	int last_intf;
	int i;

	for (i = 0; i != max; i++) {
		intf_assoc = p_ud->bsd_config.intf_assoc[i];
		if (intf_assoc->bInterfaceCount == 0)
			continue;

		first_intf = intf_assoc->bFirstInterface;
		last_intf = first_intf + intf_assoc->bInterfaceCount - 1;

		if (inum >= first_intf && inum <= last_intf)
			return (intf_assoc);
	}
	return (NULL);
}

/*------------------------------------------------------------------------*
 *	usb_linux_create_usb_device
 *
 * The following function is used to build up a per USB device
 * structure tree, that mimics the Linux one. The root structure
 * is returned by this function.
 *------------------------------------------------------------------------*/
static struct usb_device *
usb_linux_create_usb_device(struct usb_linux_softc *sc,
    struct libusb20_device *udev, struct libusb20_config *pcfg,
    uint16_t addr)
{
	struct libusb20_interface *id;
	struct libusb20_interface *aid;
	struct libusb20_endpoint *ed;
	struct usb_device *p_ud = NULL;
	struct usb_interface *p_ui = NULL;
	struct usb_host_interface *p_uhi = NULL;
	struct usb_host_endpoint *p_uhe = NULL;
	const uint8_t *pdesc;
	uint32_t size;
	uint16_t niface_total;
	uint16_t nedesc;
	uint16_t iface_index;
	uint8_t i;
	uint8_t j;
	uint8_t k;
	uint8_t num_config_iface;
	uint8_t num_iad;

	/*
	 * We do two passes. One pass for computing necessary memory size
	 * and one pass to initialize all the allocated memory structures.
	 */
	nedesc = 0;
	iface_index = 0;
	niface_total = 0;
	num_config_iface = pcfg->num_interface;
	num_iad = 0;
	if (num_config_iface > USB_LINUX_IFACE_MAX)
		num_config_iface = USB_LINUX_IFACE_MAX;

	for (i = 0; i != num_config_iface; i++) {
		id = pcfg->interface + i;
		iface_index++;
		niface_total++;
		nedesc += id->num_endpoints;
		for (j = 0; j != id->num_altsetting; j++) {
			aid = id->altsetting + j;
			nedesc += aid->num_endpoints;
			niface_total++;
		}
	}

	size = ((sizeof(*p_ud) * 1) +
	    (sizeof(*p_uhe) * nedesc) +
	    (sizeof(*p_ui) * iface_index) +
	    (sizeof(*p_uhi) * niface_total));

	p_ud = malloc(size);

	if (p_ud == NULL)
		goto done;

	memset(p_ud, 0, size);

	p_uhe = (void *)(p_ud + 1);
	p_ui = (void *)(p_uhe + nedesc);
	p_uhi = (void *)(p_ui + iface_index);

	p_ud->bus = &usb_dummy_bus;

	switch (libusb20_dev_get_speed(udev)) {
	case LIBUSB20_SPEED_LOW:
		p_ud->speed = USB_SPEED_LOW;
		p_ud->ep0.desc.wMaxPacketSize = cpu_to_le16(8);
		break;
	case LIBUSB20_SPEED_FULL:
		p_ud->speed = USB_SPEED_FULL;
		switch (libusb20_dev_get_device_desc(udev)->bMaxPacketSize0) {
		case 8:
			p_ud->ep0.desc.wMaxPacketSize = cpu_to_le16(8);
			break;
		case 16:
			p_ud->ep0.desc.wMaxPacketSize = cpu_to_le16(16);
			break;
		case 32:
			p_ud->ep0.desc.wMaxPacketSize = cpu_to_le16(32);
			break;
		default:
			p_ud->ep0.desc.wMaxPacketSize = cpu_to_le16(64);
			break;
		}
		break;
	case LIBUSB20_SPEED_HIGH:
		p_ud->speed = USB_SPEED_HIGH;
		p_ud->ep0.desc.wMaxPacketSize = cpu_to_le16(64);
		break;
	case LIBUSB20_SPEED_SUPER:
		p_ud->speed = USB_SPEED_SUPER;
		p_ud->ep0.desc.wMaxPacketSize = cpu_to_le16(512);
		break;
	case LIBUSB20_SPEED_VARIABLE:
		p_ud->speed = USB_SPEED_VARIABLE;
		p_ud->ep0.desc.wMaxPacketSize = cpu_to_le16(512);

		break;
	default:
		p_ud->speed = USB_SPEED_HIGH;
		break;
	}

	p_ud->bsd_udev = udev;
	p_ud->bsd_iface_start = p_ui;
	p_ud->bsd_iface_end = p_ui + iface_index;
	p_ud->bsd_endpoint_start = p_uhe;
	p_ud->bsd_endpoint_end = p_uhe + nedesc;
	p_ud->devnum = addr;
	p_ud->config = &p_ud->bsd_config;
	p_ud->actconfig = &p_ud->bsd_config;

	p_ud->ep0.desc.bLength = 7;
	p_ud->ep0.desc.bDescriptorType = 5;	/* endpoint descriptor */
	p_ud->ep0.desc.bEndpointAddress = 0;
	p_ud->ep0.desc.bmAttributes = USB_ENDPOINT_XFER_CONTROL;
	TAILQ_INIT(&p_ud->ep0.bsd_urb_list);

	p_ud->ep_in[0] = &p_ud->ep0;
	p_ud->ep_out[0] = &p_ud->ep0;

	libusb20_me_encode(&p_ud->descriptor, sizeof(p_ud->descriptor),
	    libusb20_dev_get_device_desc(udev));

	libusb20_me_encode(&p_ud->bsd_config.desc,
	    sizeof(p_ud->bsd_config.desc), &pcfg->desc);

	/* make sure number of interfaces value is correct */
	p_ud->bsd_config.desc.bNumInterfaces = num_config_iface;

	/*
	 * Look for interface association
	 * descriptors:
	 */
	pdesc = NULL;
	while ((pdesc = libusb20_desc_foreach(
	    &pcfg->extra, pdesc)) != NULL) {
		if (pdesc[0] >= (uint8_t)sizeof(p_ud->bsd_config.intf_assoc[0]) &&
		    pdesc[1] == USB_DT_INTERFACE_ASSOCIATION &&
		    num_iad != USB_MAXIADS) {
			p_ud->bsd_config.intf_assoc[num_iad++] = (void *)pdesc;
		}
	}

	for (i = 0; i != num_config_iface; i++) {
		id = pcfg->interface + i;

		p_ud->bsd_config.interface[i] = p_ui;
		p_ui->dev.parent = &p_ud->dev;
		p_ui->dev.driver_static.name = "webcamd-usb-interface";
		p_ui->dev.driver = &p_ui->dev.driver_static;
		p_ui->altsetting = p_uhi;
		p_ui->cur_altsetting = p_uhi;
		p_ui->num_altsetting = id->num_altsetting + 1;
		p_ui->bsd_iface_index = i;
		p_ui->usb_dev = p_ud;

		for (j = 0; j != p_ui->num_altsetting; j++) {
			libusb20_me_encode(&p_uhi->desc, sizeof(p_uhi->desc),
			    &id->desc);

			p_uhi->desc.bNumEndpoints = id->num_endpoints;
			p_uhi->endpoint = p_uhe;
			p_uhi->string = "";
			p_uhi->bsd_iface_index = iface_index;
			p_uhi->extra = id->extra.ptr;
			p_uhi->extralen = id->extra.len;
			p_uhi++;

			/*
			 * Look for interface association
			 * descriptors:
			 */
			pdesc = NULL;
			while ((pdesc = libusb20_desc_foreach(
			    &id->extra, pdesc)) != NULL) {
				if (pdesc[0] >= (uint8_t)sizeof(p_ud->bsd_config.intf_assoc[0]) &&
				    pdesc[1] == USB_DT_INTERFACE_ASSOCIATION &&
				    num_iad != USB_MAXIADS) {
					p_ud->bsd_config.intf_assoc[num_iad++] = (void *)pdesc;
				}
			}

			for (k = 0; k != id->num_endpoints; k++) {
				ed = id->endpoints + k;
				libusb20_me_encode(&p_uhe->desc,
				    sizeof(p_uhe->desc), &ed->desc);

				/*
				 * Look for the SuperSpeed endpoint
				 * companion descriptor:
				 */
				pdesc = NULL;
				while ((pdesc = libusb20_desc_foreach(
				    &ed->extra, pdesc)) != NULL) {
					if (pdesc[0] >= (uint8_t)sizeof(p_uhe->ss_ep_comp) &&
					    pdesc[1] == USB_DT_SS_ENDPOINT_COMP) {
						memcpy(&p_uhe->ss_ep_comp, pdesc, sizeof(p_uhe->ss_ep_comp));
						break;
					}
				}

				/*
				 * Look for interface association
				 * descriptors:
				 */
				pdesc = NULL;
				while ((pdesc = libusb20_desc_foreach(
				    &ed->extra, pdesc)) != NULL) {
					if (pdesc[0] >= (uint8_t)sizeof(p_ud->bsd_config.intf_assoc[0]) &&
					    pdesc[1] == USB_DT_INTERFACE_ASSOCIATION &&
					    num_iad != USB_MAXIADS) {
						p_ud->bsd_config.intf_assoc[num_iad++] = (void *)pdesc;
					}
				}

				TAILQ_INIT(&p_uhe->bsd_urb_list);
				p_uhe->bsd_iface_index = i;
				p_uhe->extra = ed->extra.ptr;
				p_uhe->extralen = ed->extra.len;
				p_uhe++;
			}

			if (j == 0)
				id = id->altsetting;
			else
				id++;
		}

		usb_linux_fill_ep_info(p_ud, p_ui->cur_altsetting);

		p_ui++;
	}

	/* resolve all IADs */
	for (i = 0; i != num_config_iface; i++) {
		id = pcfg->interface + i;
		p_ui = p_ud->bsd_config.interface[i];
		p_ui->intf_assoc = usb_find_iad(p_ud, num_iad,
		    id->altsetting[0].desc.bInterfaceNumber);
	}

	p_ud->parent = sc;
	p_ud->dev.driver_static.name = "webcamd";
	p_ud->dev.driver = &p_ud->dev.driver_static;

	get_device(&p_ud->dev);		/* make sure we don't get freed */

	p_ud->product = usb_string_dup(p_ud,
	    p_ud->descriptor.iProduct);
	p_ud->manufacturer = usb_string_dup(p_ud,
	    p_ud->descriptor.iManufacturer);
	p_ud->serial = usb_string_dup(p_ud,
	    p_ud->descriptor.iSerialNumber);
done:
	return (p_ud);
}

/*------------------------------------------------------------------------*
 *	usb_alloc_urb
 *
 * This function should always be used when you allocate an URB for
 * use with the USB Linux stack. In case of an isochronous transfer
 * you must specifiy the maximum number of "iso_packets" which you
 * plan to transfer per URB. This function is always blocking, and
 * "mem_flags" are not regarded like on Linux.
 *------------------------------------------------------------------------*/
struct urb *
usb_alloc_urb(uint16_t iso_packets, uint16_t mem_flags)
{
	struct urb *urb;
	uint32_t size;

	size = sizeof(*urb) + (iso_packets * sizeof(urb->iso_frame_desc[0]));

	urb = malloc(size);
	if (urb) {
		usb_init_urb(urb);
		urb->number_of_packets = iso_packets;
	}
	return (urb);
}

unsigned int
usb_create_host_endpoint(struct usb_device *dev, uint8_t type, uint8_t ep)
{
	return ((type << 30) | ((ep & 0xFF) << 15) |
	    ((dev->devnum & 0x7F) << 8) | (ep & USB_DIR_IN));
}

/*------------------------------------------------------------------------*
 *	usb_find_host_endpoint
 *
 * The following function will return the Linux USB host endpoint
 * structure that matches the given endpoint type and endpoint
 * value. If no match is found, NULL is returned. This function is not
 * part of the Linux USB API and is only used internally.
 *------------------------------------------------------------------------*/
static struct usb_host_endpoint *
usb_find_host_endpoint(struct usb_device *dev, unsigned int pipe)
{
	struct usb_host_endpoint *uhe;
	struct usb_host_endpoint *uhe_end;
	struct usb_host_interface *uhi;
	struct usb_interface *ui;
	uint8_t ea;
	uint8_t at;
	uint8_t mask;
	uint8_t type = (pipe >> 30) & 3;
	uint8_t ep = (pipe >> 15) & 255;

	if (dev == NULL)
		return (NULL);

	if (type == USB_ENDPOINT_XFER_CONTROL)
		mask = USB_ENDPOINT_NUMBER_MASK;
	else
		mask = (USB_ENDPOINT_DIR_MASK | USB_ENDPOINT_NUMBER_MASK);

	ep &= mask;

	/*
	 * Iterate over all the interfaces searching the selected alternate
	 * setting only, and all belonging endpoints.
	 */
	for (ui = dev->bsd_iface_start;
	    ui != dev->bsd_iface_end;
	    ui++) {
		uhi = ui->cur_altsetting;
		if (uhi) {
			uhe_end = uhi->endpoint + uhi->desc.bNumEndpoints;
			for (uhe = uhi->endpoint;
			    uhe != uhe_end;
			    uhe++) {
				ea = uhe->desc.bEndpointAddress;
				at = uhe->desc.bmAttributes;

				if (((ea & mask) == ep) &&
				    ((at & USB_ENDPOINT_XFERTYPE_MASK) == type)) {
					return (uhe);
				}
			}
		}
	}

	if ((type == USB_ENDPOINT_XFER_CONTROL) &&
	    ((ep & USB_ENDPOINT_NUMBER_MASK) == 0)) {
		return (&dev->ep0);
	}
	return (NULL);
}

/*------------------------------------------------------------------------*
 *	usb_altnum_to_altsetting
 *
 * The following function returns a pointer to an alternate setting by
 * index given a "usb_interface" pointer. If the alternate setting by
 * index does not exist, NULL is returned. And alternate setting is a
 * variant of an interface, but usually with slightly different
 * characteristics.
 *------------------------------------------------------------------------*/
struct usb_host_interface *
usb_altnum_to_altsetting(const struct usb_interface *intf, uint8_t alt_index)
{
	if (alt_index >= intf->num_altsetting) {
		return (NULL);
	}
	return (intf->altsetting + alt_index);
}

/*------------------------------------------------------------------------*
 *	usb_ifnum_to_if
 *
 * The following function searches up an USB interface by
 * "bInterfaceNumber". If no match is found, NULL is returned.
 *------------------------------------------------------------------------*/
struct usb_interface *
usb_ifnum_to_if(struct usb_device *dev, uint8_t iface_no)
{
	struct usb_interface *p_ui;

	for (p_ui = dev->bsd_iface_start;
	    p_ui != dev->bsd_iface_end;
	    p_ui++) {
		if ((p_ui->num_altsetting > 0) &&
		    (p_ui->altsetting->desc.bInterfaceNumber == iface_no)) {
			return (p_ui);
		}
	}
	return (NULL);
}

/*------------------------------------------------------------------------*
 *	usb_buffer_alloc
 *------------------------------------------------------------------------*/
void   *
usb_buffer_alloc(struct usb_device *dev, uint32_t size,
    uint16_t mem_flags, dma_addr_t *dma_addr)
{
	void *ptr;

	if (dma_addr)
		*dma_addr = 0;
	ptr = malloc(size);
	if (ptr)
		memset(ptr, 0, size);
	return (ptr);
}

/*------------------------------------------------------------------------*
 *	usb_get_intfdata
 *------------------------------------------------------------------------*/
void   *
usb_get_intfdata(struct usb_interface *intf)
{
	return (intf->bsd_priv_sc);
}

/*------------------------------------------------------------------------*
 *	usb_register
 *
 * The following function is used by the "USB_DRIVER_EXPORT()" macro,
 * and is used to register a Linux USB driver, so that its
 * "usb_device_id" structures gets searched a probe time. This
 * function is not part of the Linux USB API, and is for internal use
 * only.
 *------------------------------------------------------------------------*/
int
usb_register(struct usb_driver *drv)
{
	atomic_lock();
	LIST_INSERT_HEAD(&usb_linux_driver_list, drv, linux_driver_list);
	atomic_unlock();

	return (0);
}

/*------------------------------------------------------------------------*
 *	usb_deregister
 *
 * The following function is used by the "USB_DRIVER_EXPORT()" macro,
 * and is used to deregister a Linux USB driver. This function will
 * ensure that all driver instances belonging to the Linux USB device
 * driver in question, gets detached before the driver is
 * unloaded. This function is not part of the Linux USB API, and is
 * for internal use only.
 *------------------------------------------------------------------------*/
int
usb_deregister(struct usb_driver *drv)
{
	atomic_lock();
	LIST_REMOVE(drv, linux_driver_list);
	atomic_unlock();

	return (0);
}

/*------------------------------------------------------------------------*
 *	usb_linux_free_device
 *
 * The following function is only used by the FreeBSD USB stack, to
 * cleanup and free memory after that a Linux USB device was attached.
 *------------------------------------------------------------------------*/
static void
usb_linux_free_device(struct usb_device *dev)
{
	struct usb_host_endpoint *uhe;
	struct usb_host_endpoint *uhe_end;

	atomic_lock();
	uhe = dev->bsd_endpoint_start;
	uhe_end = dev->bsd_endpoint_end;
	while (uhe != uhe_end) {
		usb_unsetup_endpoint(dev, uhe);
		uhe++;
	}
	usb_unsetup_endpoint(dev, &dev->ep0);
	atomic_unlock();

	free(dev->product);
	free(dev->manufacturer);
	free(dev->serial);
	free(dev);
}

/*------------------------------------------------------------------------*
 *	usb_buffer_free
 *------------------------------------------------------------------------*/
void
usb_buffer_free(struct usb_device *dev, uint32_t size,
    void *addr, dma_addr_t dma_addr)
{
	free(addr);
}


/*------------------------------------------------------------------------*
 *	urb_destroy
 *------------------------------------------------------------------------*/
static void
urb_destroy(struct kref *kref)
{
	struct urb *urb = to_urb(kref);

	/* make sure that the current URB is not active */
	usb_kill_urb(urb);

	/* free transfer buffer, if free buffer flag is set */
	if (urb->transfer_flags & URB_FREE_BUFFER)
		free(urb->transfer_buffer);

	/* just free it */
	free(urb);
}

/*------------------------------------------------------------------------*
 *	usb_free_urb
 *------------------------------------------------------------------------*/
void
usb_free_urb(struct urb *urb)
{
	if (urb == NULL)
		return;
	kref_put(&urb->kref, urb_destroy);
}

/*------------------------------------------------------------------------*
 *	usb_init_urb
 *
 * The following function can be used to initialize a custom URB. It
 * is not recommended to use this function. Use "usb_alloc_urb()"
 * instead.
 *------------------------------------------------------------------------*/
void
usb_init_urb(struct urb *urb)
{
	if (urb == NULL)
		return;

	memset(urb, 0, sizeof(*urb));
	kref_init(&urb->kref);
	INIT_LIST_HEAD(&urb->anchor_list);
}

/*------------------------------------------------------------------------*
 *	usb_kill_urb
 *------------------------------------------------------------------------*/
void
usb_kill_urb(struct urb *urb)
{
	atomic_lock();
	usb_unlink_urb_sub(urb, 1);
	atomic_unlock();
}

/*------------------------------------------------------------------------*
 *	usb_set_intfdata
 *
 * The following function sets the per Linux USB interface private
 * data pointer. It is used by most Linux USB device drivers.
 *------------------------------------------------------------------------*/
void
usb_set_intfdata(struct usb_interface *intf, void *data)
{
	intf->bsd_priv_sc = data;
}

/*------------------------------------------------------------------------*
 *	usb_linux_cleanup_interface
 *
 * The following function will release all FreeBSD USB transfers
 * associated with a Linux USB interface. It is for internal use only.
 *------------------------------------------------------------------------*/
static void
usb_linux_cleanup_interface(struct usb_device *dev,
    struct usb_interface *iface)
{
	struct usb_host_interface *uhi;
	struct usb_host_interface *uhi_end;
	struct usb_host_endpoint *uhe;
	struct usb_host_endpoint *uhe_end;
	struct usb_linux_softc *sc = dev->parent;
	uint32_t drops;

	atomic_lock();
	uhi = iface->altsetting;
	uhi_end = iface->altsetting + iface->num_altsetting;
	while (uhi != uhi_end) {
		uhe = uhi->endpoint;
		uhe_end = uhi->endpoint + uhi->desc.bNumEndpoints;
		while (uhe != uhe_end) {
			usb_unsetup_endpoint(dev, uhe);
			uhe++;
		}
		uhi++;
	}

	drops = atomic_drop();
	atomic_unlock();

	while (sc->thread_started != 0) {
		sc->thread_stopping = 1;
		pthread_kill(sc->thread, SIGIO);
		schedule();
	}

	atomic_lock();
	atomic_pickup(drops);
	atomic_unlock();
}

/*------------------------------------------------------------------------*
 *	usb_linux_complete
 *------------------------------------------------------------------------*/
static void
usb_linux_complete(struct libusb20_transfer *xfer)
{
	struct urb *urb;

	urb = libusb20_tr_get_priv_sc1(xfer);
	libusb20_tr_set_priv_sc1(xfer, NULL);

	if (urb->complete) {
		urb->hcpriv = NULL;
		(urb->complete) (urb);
	}
}

/*------------------------------------------------------------------------*
 *	usb_linux_isoc_callback
 *
 * The following is the FreeBSD isochronous USB callback. Isochronous
 * frames are USB packets transferred 1000 or 8000 times per second,
 * depending on whether a full- or high- speed USB transfer is
 * used.
 *------------------------------------------------------------------------*/
static void
usb_linux_isoc_callback(struct libusb20_transfer *xfer)
{
	uint16_t x;
	uint32_t actlen;
	struct urb *urb = libusb20_tr_get_priv_sc1(xfer);
	struct usb_host_endpoint *uhe = libusb20_tr_get_priv_sc0(xfer);
	struct usb_iso_packet_descriptor *uipd;
	uint8_t status = libusb20_tr_get_status(xfer);
	uint8_t is_short = 0;

	switch (status) {
	case LIBUSB20_TRANSFER_COMPLETED:

		atomic_lock();
		urb->dev->bsd_last_ms = libusb20_tr_get_time_complete(xfer);
		atomic_unlock();

		for (x = 0; x < urb->number_of_packets; x++) {
			uipd = urb->iso_frame_desc + x;
			actlen = libusb20_tr_get_length(xfer, x);
			if (uipd->length > actlen) {
				is_short = 1;
				if (urb->transfer_flags & URB_SHORT_NOT_OK) {
					/* XXX should be EREMOTEIO */
					uipd->status = -EPIPE;
				} else {
					uipd->status = 0;
				}
			} else {
				uipd->status = 0;
			}
			uipd->actual_length = actlen;
		}

		actlen = libusb20_tr_get_actual_length(xfer);
		urb->actual_length = actlen;

		/* check for short transfer */
		if (is_short) {
			/* short transfer */
			if (urb->transfer_flags & URB_SHORT_NOT_OK) {
				/* XXX should be EREMOTEIO */
				urb->status = -EPIPE;
			} else {
				urb->status = 0;
			}
		} else {
			/* success */
			urb->status = 0;
		}

		/* call callback */

		urb->bsd_no_resubmit = 1;
		usb_linux_complete(xfer);
		urb->bsd_no_resubmit = 0;

	case LIBUSB20_TRANSFER_START:
tr_setup:
		/* get next transfer */
		urb = TAILQ_FIRST(&uhe->bsd_urb_list);
		if (urb == NULL) {
			/* nothing to do */
			break;
		}
		TAILQ_REMOVE(&uhe->bsd_urb_list, urb, bsd_urb_list);
		urb->bsd_urb_list.tqe_prev = NULL;

		x = libusb20_tr_get_max_frames(xfer);
		if (urb->number_of_packets > x) {
			/* XXX simply truncate the transfer */
			urb->number_of_packets = x;
		}
		/* setup transfer */
		for (x = 0; x < urb->number_of_packets; x++) {
			uipd = urb->iso_frame_desc + x;
			libusb20_tr_setup_isoc(xfer,
			    ((uint8_t *)urb->transfer_buffer) +
			    uipd->offset,
			    uipd->length, x);
		}

		libusb20_tr_set_priv_sc1(xfer, urb);
		if (urb->timeout == 0) {
			/*
			 * Sometimes we are late putting stuff into the
			 * schedule and the transfer never completes leading
			 * to a hang situation. Always have a timeout for
			 * isochronous transfers so that we can recover.
			 */
			libusb20_tr_set_timeout(xfer, 250);
		} else {
			libusb20_tr_set_timeout(xfer, urb->timeout);
		}
		libusb20_tr_set_total_frames(xfer, urb->number_of_packets);
		libusb20_tr_submit(xfer);

		/* get other transfer */
		if (xfer == uhe->bsd_xfer[0])
			xfer = uhe->bsd_xfer[1];
		else
			xfer = uhe->bsd_xfer[0];

		/* start the other transfer, if not already started */
		if (xfer != NULL)
			libusb20_tr_start(xfer);

		break;

	default:			/* Error */
		if (status == LIBUSB20_TRANSFER_CANCELLED) {
			urb->status = -ECONNRESET;
		} else if (status == LIBUSB20_TRANSFER_TIMED_OUT) {
			urb->status = 0;/* pretend we are successful */
		} else {
			urb->status = -EPIPE;	/* stalled */
		}

		/* Set zero for "actual_length" */
		urb->actual_length = 0;

		/* Set zero for "actual_length" */
		for (x = 0; x < urb->number_of_packets; x++) {
			urb->iso_frame_desc[x].actual_length = 0;
			urb->iso_frame_desc[x].status = urb->status;
		}

		/* call callback */
		urb->bsd_no_resubmit = 1;
		usb_linux_complete(xfer);
		urb->bsd_no_resubmit = 0;

		if (status == LIBUSB20_TRANSFER_CANCELLED) {
			/* we need to return in this case */
			break;
		}
		goto tr_setup;
	}
}

/*------------------------------------------------------------------------*
 *	usb_linux_bulk_intr_callback
 *
 * The following is the FreeBSD BULK/INTERRUPT callback. It dequeues
 * Linux USB stack compatible URB's, transforms the URB fields into a
 * FreeBSD USB transfer, and defragments the USB transfer as
 * required. When the transfer is complete the "complete" callback is
 * called.
 *------------------------------------------------------------------------*/
static void
usb_linux_bulk_intr_callback(struct libusb20_transfer *xfer)
{
	struct urb *urb = libusb20_tr_get_priv_sc1(xfer);
	struct usb_host_endpoint *uhe = libusb20_tr_get_priv_sc0(xfer);
	uint32_t max_bulk = libusb20_tr_get_max_total_length(xfer);
	uint32_t actlen;
	uint8_t status = libusb20_tr_get_status(xfer);

	switch (status) {
	case LIBUSB20_TRANSFER_COMPLETED:

		actlen = libusb20_tr_get_actual_length(xfer);

		urb->actual_length = actlen;

		/* check for short transfer */
		if (actlen < urb->transfer_buffer_length) {
			/* short transfer */
			if (urb->transfer_flags & URB_SHORT_NOT_OK) {
				urb->status = -EPIPE;
			} else {
				urb->status = 0;
			}
		} else {
			/* success */
			urb->status = 0;
		}

		/* call callback */
		urb->bsd_no_resubmit = 1;
		usb_linux_complete(xfer);
		urb->bsd_no_resubmit = 0;

	case LIBUSB20_TRANSFER_START:
tr_setup:
		/* get next transfer */
		urb = TAILQ_FIRST(&uhe->bsd_urb_list);
		if (urb == NULL) {
			/* nothing to do */
			break;
		}
		TAILQ_REMOVE(&uhe->bsd_urb_list, urb, bsd_urb_list);
		urb->bsd_urb_list.tqe_prev = NULL;

		libusb20_tr_set_priv_sc1(xfer, urb);

		/* assert valid transfer size */
		if (urb->transfer_buffer_length > max_bulk) {
			urb->status = -EFBIG;

			/* call callback */
			urb->bsd_no_resubmit = 1;
			usb_linux_complete(xfer);
			urb->bsd_no_resubmit = 0;
			goto tr_setup;
		}
		/* check if we need to force a short transfer */

		if (urb->transfer_flags & URB_ZERO_PACKET) {
			libusb20_tr_set_flags(xfer, LIBUSB20_TRANSFER_FORCE_SHORT);
		} else {
			libusb20_tr_set_flags(xfer, 0);
		}

		/* setup data transfer */

		libusb20_tr_setup_bulk(xfer, urb->transfer_buffer,
		    urb->transfer_buffer_length, urb->timeout);
		libusb20_tr_submit(xfer);

		/* get other transfer */
		if (xfer == uhe->bsd_xfer[0])
			xfer = uhe->bsd_xfer[1];
		else
			xfer = uhe->bsd_xfer[0];

		/* start the other transfer, if not already started */
		if (xfer != NULL)
			libusb20_tr_start(xfer);
		break;

	default:
		if (status == LIBUSB20_TRANSFER_CANCELLED) {
			urb->status = -ECONNRESET;
		} else {
			urb->status = -EPIPE;
		}

		/* Set zero for "actual_length" */
		urb->actual_length = 0;

		/* call callback */
		urb->bsd_no_resubmit = 1;
		usb_linux_complete(xfer);
		urb->bsd_no_resubmit = 0;

		if (status == LIBUSB20_TRANSFER_CANCELLED) {
			/* we need to return in this case */
			break;
		}
		goto tr_setup;
	}
}

/*------------------------------------------------------------------------*
 *	usb_linux_ctrl_callback
 *
 * The following is the FreeBSD CONTROL callback. It dequeues Linux
 * USB stack compatible URB's, transforms the URB fields into a
 * FreeBSD USB transfer. When the transfer is complete the "complete"
 * callback is called.
 *------------------------------------------------------------------------*/
static void
usb_linux_ctrl_callback(struct libusb20_transfer *xfer)
{
	struct urb *urb = libusb20_tr_get_priv_sc1(xfer);
	struct usb_host_endpoint *uhe = libusb20_tr_get_priv_sc0(xfer);
	uint32_t max_ctrl = libusb20_tr_get_max_total_length(xfer);
	uint32_t actlen;
	uint8_t status = libusb20_tr_get_status(xfer);

	switch (status) {
	case LIBUSB20_TRANSFER_COMPLETED:

		actlen = libusb20_tr_get_actual_length(xfer);

		/* subtract size of setup packet */
		if (actlen >= 8)
			actlen -= 8;
		else
			actlen = 0;

		urb->actual_length = actlen;

		/* check for short transfer */
		if (actlen < urb->transfer_buffer_length) {
			/* short transfer */
			if (urb->transfer_flags & URB_SHORT_NOT_OK) {
				urb->status = -EPIPE;
			} else {
				urb->status = 0;
			}
		} else {
			/* success */
			urb->status = 0;
		}

		/* call callback */
		urb->bsd_no_resubmit = 1;
		usb_linux_complete(xfer);
		urb->bsd_no_resubmit = 0;

	case LIBUSB20_TRANSFER_START:
tr_setup:
		/* get next transfer */
		urb = TAILQ_FIRST(&uhe->bsd_urb_list);
		if (urb == NULL) {
			/* nothing to do */
			break;
		}
		TAILQ_REMOVE(&uhe->bsd_urb_list, urb, bsd_urb_list);
		urb->bsd_urb_list.tqe_prev = NULL;

		libusb20_tr_set_priv_sc1(xfer, urb);

		/* assert valid transfer size */
		if (max_ctrl < 8 ||
		    urb->transfer_buffer_length > (max_ctrl - 8)) {
			urb->status = -EFBIG;

			/* call callback */
			urb->bsd_no_resubmit = 1;
			usb_linux_complete(xfer);
			urb->bsd_no_resubmit = 0;
			goto tr_setup;
		}
		libusb20_tr_set_flags(xfer, 0);

		/* setup control transfer */
		libusb20_tr_setup_control(xfer, urb->setup_packet,
		    urb->transfer_buffer, urb->timeout);

		libusb20_tr_submit(xfer);

		/* get other transfer */
		if (xfer == uhe->bsd_xfer[0])
			xfer = uhe->bsd_xfer[1];
		else
			xfer = uhe->bsd_xfer[0];

		/* start the other transfer, if not already started */
		if (xfer != NULL)
			libusb20_tr_start(xfer);
		break;

	default:
		if (status == LIBUSB20_TRANSFER_CANCELLED) {
			urb->status = -ECONNRESET;
		} else {
			urb->status = -EPIPE;
		}

		/* Set zero for "actual_length" */
		urb->actual_length = 0;

		/* call callback */
		urb->bsd_no_resubmit = 1;
		usb_linux_complete(xfer);
		urb->bsd_no_resubmit = 0;

		if (status == LIBUSB20_TRANSFER_CANCELLED) {
			/* we need to return in this case */
			break;
		}
		goto tr_setup;
	}
}

/* The following functions directly derive from Linux: */

int
usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

int
usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

int
usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK);
}

int
usb_endpoint_xfer_control(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_CONTROL);
}

int
usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT);
}

int
usb_endpoint_xfer_isoc(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_ISOC);
}

void
usb_fill_control_urb(struct urb *urb,
    struct usb_device *dev,
    unsigned int pipe,
    unsigned char *setup_packet,
    void *transfer_buffer,
    int buffer_length,
    usb_complete_t complete_fn,
    void *context)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->setup_packet = setup_packet;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
}

void
usb_fill_bulk_urb(struct urb *urb,
    struct usb_device *dev,
    unsigned int pipe,
    void *transfer_buffer,
    int buffer_length,
    usb_complete_t complete_fn,
    void *context)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
}

void
usb_fill_int_urb(struct urb *urb,
    struct usb_device *dev,
    unsigned int pipe,
    void *transfer_buffer,
    int buffer_length,
    usb_complete_t complete_fn,
    void *context,
    int interval)
{
	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = transfer_buffer;
	urb->transfer_buffer_length = buffer_length;
	urb->complete = complete_fn;
	urb->context = context;
	if (dev->speed == USB_SPEED_HIGH ||
	    dev->speed == USB_SPEED_SUPER ||
	    dev->speed == USB_SPEED_VARIABLE)
		urb->interval = 1 << (interval - 1);
	else
		urb->interval = interval;
	urb->start_frame = -1;
}

struct usb_interface *
usb_get_intf(struct usb_interface *intf)
{
	if (intf)
		get_device(&intf->dev);
	return intf;
}

void
usb_put_intf(struct usb_interface *intf)
{
	if (intf)
		put_device(&intf->dev);
}

struct usb_device *
usb_get_dev(struct usb_device *dev)
{
	if (dev)
		get_device(&dev->dev);

	return dev;
}

void
usb_put_dev(struct usb_device *dev)
{
	if (dev)
		put_device(&dev->dev);
}

int
usb_string(struct usb_device *dev, int index, char *buf, size_t size)
{
	if (size == 0)
		return (0);

	if (libusb20_dev_req_string_simple_sync(
	    dev->bsd_udev, index, buf, size)) {
		*buf = 0;
		return (-EINVAL);
	}
	return (strlen(buf));
}

int
usb_make_path(struct usb_device *dev, char *buf, size_t size)
{
	int actual;

	actual = snprintf(buf, size, "usb-/dev/usb-/dev/usb");
	return (actual >= (int)size) ? -1 : actual;
}

struct api_context {
	struct completion done;
	int	status;
};

static void
usb_api_blocking_completion(struct urb *urb)
{
	struct api_context *ctx = urb->context;

	ctx->status = urb->status;
	complete(&ctx->done);
}

static int
usb_start_wait_urb(struct urb *urb, int timeout, int *actual_length)
{
	struct api_context ctx;
	int retval;

	init_completion(&ctx.done);
	ctx.status = -ECONNRESET;

	urb->context = &ctx;
	urb->actual_length = 0;
	urb->timeout = timeout;
	retval = usb_submit_urb(urb, GFP_NOIO);
	if (retval == 0) {
		wait_for_completion(&ctx.done);
		retval = ctx.status;
	}
	if (actual_length)
		*actual_length = urb->actual_length;

	usb_free_urb(urb);
	uninit_completion(&ctx.done);
	return (retval);
}

int
usb_bulk_msg(struct usb_device *usb_dev, unsigned int pipe,
    void *data, int len, int *actual_length, int timeout)
{
	struct urb *urb;

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
		return (-ENOMEM);

	if (usb_pipetype(pipe) == PIPE_INTERRUPT) {
		usb_fill_int_urb(urb, usb_dev, pipe, data, len,
		    usb_api_blocking_completion, NULL, 0);
	} else {
		usb_fill_bulk_urb(urb, usb_dev, pipe, data, len,
		    usb_api_blocking_completion, NULL);
	}

	return (usb_start_wait_urb(urb, timeout, actual_length));
}

/* Calling the usb_match_xxx() functions directly is deferred. */

int
usb_match_device(struct usb_device *dev, const struct usb_device_id *id)
{
	int flags = id->match_flags;

	if ((flags & USB_DEVICE_ID_MATCH_VENDOR) &&
	    (id->idVendor != le16_to_cpu(dev->descriptor.idVendor)))
		goto no_match;

	if ((flags & USB_DEVICE_ID_MATCH_PRODUCT) &&
	    (id->idProduct != le16_to_cpu(dev->descriptor.idProduct)))
		goto no_match;

	if ((flags & USB_DEVICE_ID_MATCH_DEV_LO) &&
	    (id->bcdDevice_lo > le16_to_cpu(dev->descriptor.bcdDevice)))
		goto no_match;

	if ((flags & USB_DEVICE_ID_MATCH_DEV_HI) &&
	    (id->bcdDevice_hi < le16_to_cpu(dev->descriptor.bcdDevice)))
		goto no_match;

	if ((flags & USB_DEVICE_ID_MATCH_DEV_CLASS) &&
	    (id->bDeviceClass != dev->descriptor.bDeviceClass))
		goto no_match;

	if ((flags & USB_DEVICE_ID_MATCH_DEV_SUBCLASS) &&
	    (id->bDeviceSubClass != dev->descriptor.bDeviceSubClass))
		goto no_match;

	if ((flags & USB_DEVICE_ID_MATCH_DEV_PROTOCOL) &&
	    (id->bDeviceProtocol != dev->descriptor.bDeviceProtocol))
		goto no_match;

	return (1);

no_match:
	return (0);
}

int
usb_match_one_id(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_host_interface *intf;
	struct usb_device *dev;

	if (id == NULL)
		goto no_match;

	intf = interface->cur_altsetting;
	dev = interface_to_usbdev(interface);

	if (!usb_match_device(dev, id))
		goto no_match;

	if ((dev->descriptor.bDeviceClass == USB_CLASS_VENDOR_SPEC) &&
	    (!(id->match_flags & USB_DEVICE_ID_MATCH_VENDOR)) &&
	    (id->match_flags & USB_DEVICE_ID_MATCH_INT_INFO))
		goto no_match;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_CLASS) &&
	    (id->bInterfaceClass != intf->desc.bInterfaceClass))
		goto no_match;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_SUBCLASS) &&
	    (id->bInterfaceSubClass != intf->desc.bInterfaceSubClass))
		goto no_match;

	if ((id->match_flags & USB_DEVICE_ID_MATCH_INT_PROTOCOL) &&
	    (id->bInterfaceProtocol != intf->desc.bInterfaceProtocol))
		goto no_match;

	return (1);

no_match:
	return (0);
}

const struct usb_device_id *
usb_match_id(struct usb_interface *interface, const struct usb_device_id *id)
{
	if (id == NULL)
		goto no_match;

	for (; id->match_flags; id++) {
		if (usb_match_one_id(interface, id))
			return (id);
	}

	/* check for special case */
	if (id->driver_info != 0)
		return (id);

no_match:
	return (NULL);
}

int
usb_reset_configuration(struct usb_device *dev)
{
	return (-EINVAL);		/* not implemented */
}

int
usb_lock_device_for_reset(struct usb_device *udev,
    const struct usb_interface *iface)
{
	return (0);
}

void
usb_unlock_device(struct usb_device *udev)
{

}

int
usb_reset_device(struct usb_device *dev)
{
	return (0);
}

void
usb_reset_endpoint(struct usb_device *dev, unsigned int epaddr)
{
	usb_clear_halt(dev, epaddr);
}

uint8_t
usb_pipetype(unsigned int pipe)
{
	return ((pipe >> 30) & 3);
}

int
usb_autopm_get_interface(struct usb_interface *intf)
{
	return (0);
}

int
usb_autopm_set_interface(struct usb_interface *intf)
{
	return (0);
}

int
usb_driver_claim_interface(struct usb_driver *drv,
    struct usb_interface *intf, void *priv)
{
	return (0);
}

uint16_t
usb_maxpacket(struct usb_device *dev, int endpoint, int is_out)
{
	struct usb_host_endpoint *ep;
	uint8_t index = (endpoint >> 15) & 15;

	if (is_out) {
		ep = dev->ep_out[index];
	} else {
		ep = dev->ep_in[index];
	}
	if (ep == NULL)
		return (0);
	return (le16_to_cpu(ep->desc.wMaxPacketSize));
}

void
usb_enable_autosuspend(struct usb_device *udev)
{
}

int
__usb_get_extra_descriptor(char *buffer, unsigned size,
    unsigned char type, void **ptr)
{
	struct usb_descriptor_header *header;

	while (size >= sizeof(struct usb_descriptor_header)) {
		header = (struct usb_descriptor_header *)buffer;

		if (header->bLength < 2)
			return (-1);

		if (header->bLength < size)
			return (-1);

		if (header->bDescriptorType == type) {
			*ptr = header;
			return (0);
		}
		buffer += header->bLength;
		size -= header->bLength;
	}
	return (-1);
}

int
usb_endpoint_maxp(const struct usb_endpoint_descriptor *epd)
{
	return (le16_to_cpu(epd->wMaxPacketSize) & 0x7ff);
}

int
usb_endpoint_maxp_mult(const struct usb_endpoint_descriptor *epd)
{
	return ((le16_to_cpu(epd->wMaxPacketSize) >> 11) & 3) + 1;
}

int
usb_translate_errors(int error_code)
{
	switch (error_code) {
	case 0:
	case -ENOMEM:
	case -ENODEV:
	case -EOPNOTSUPP:
		return (error_code);
	default:
		return (-EIO);
	}
}

int
usb_autopm_get_interface_async(struct usb_interface *intf)
{
	return (0);
}

int
usb_autopm_get_interface_no_suspend(struct usb_interface *intf)
{
	return (0);
}

int
usb_autopm_get_interface_no_resume(struct usb_interface *intf)
{
	return (0);
}

void
usb_autopm_put_interface_async(struct usb_interface *intf)
{

}

void
usb_autopm_put_interface_no_suspend(struct usb_interface *intf)
{

}

void
usb_autopm_put_interface_no_resume(struct usb_interface *intf)
{

}

void
usb_block_urb(struct urb *urb)
{
	if (urb == NULL)
		return;
	atomic_lock();
	urb->reject++;
	atomic_unlock();
}

void
usb_unblock_urb(struct urb *urb)
{
	if (urb == NULL)
		return;
	atomic_lock();
	urb->reject--;
	atomic_unlock();
}

void
usb_poison_urb(struct urb *urb)
{
	usb_block_urb(urb);
}

void
usb_unpoison_urb(struct urb *urb)
{
	usb_unblock_urb(urb);
}

void
usb_queue_reset_device(struct usb_interface *intf)
{

}

struct usb_host_endpoint *
usb_pipe_endpoint(struct usb_device *dev, unsigned int pipe)
{
	struct usb_host_endpoint **eps;

	eps = usb_pipein(pipe) ? dev->ep_in : dev->ep_out;
	return (eps[usb_pipeendpoint(pipe)]);
}

static const int usb_pipetypes[4] = {
	PIPE_CONTROL, PIPE_ISOCHRONOUS, PIPE_BULK, PIPE_INTERRUPT
};

int
usb_urb_ep_type_check(const struct urb *urb)
{
	const struct usb_host_endpoint *ep;

	ep = usb_pipe_endpoint(urb->dev, urb->pipe);
	if (ep == NULL)
		return (-EINVAL);

	if (usb_pipetype(urb->pipe) !=
	    usb_pipetypes[usb_endpoint_type(&ep->desc)])
		return (-EINVAL);

	return (0);
}

struct urb *
usb_get_urb(struct urb *urb)
{
	if (urb)
    		kref_get(&urb->kref);
	return (urb);
}

static int
usb_anchor_check_wakeup(struct usb_anchor *anchor)
{
	return (atomic_read(&anchor->suspend_wakeups) == 0 &&
		list_empty(&anchor->urb_list));
}

void
init_usb_anchor(struct usb_anchor *anchor)
{
	memset(anchor, 0, sizeof(*anchor));
	INIT_LIST_HEAD(&anchor->urb_list);
	init_waitqueue_head(&anchor->wait);
	spin_lock_init(&anchor->lock);
}

int
usb_wait_anchor_empty_timeout(struct usb_anchor *anchor,
				  unsigned int timeout)
{
	return (wait_event_timeout(anchor->wait,
				   usb_anchor_check_wakeup(anchor),
				   msecs_to_jiffies(timeout)));
}

static void
usb_unanchor_urb_locked(struct urb *urb, struct usb_anchor *anchor)
{
	urb->anchor = NULL;
	list_del(&urb->anchor_list);
	usb_put_urb(urb);
	if (usb_anchor_check_wakeup(anchor))
		wake_up(&anchor->wait);
}

void
usb_unanchor_urb(struct urb *urb)
{
	struct usb_anchor *anchor;
	unsigned long flags;

	if (!urb)
		return;

	anchor = urb->anchor;
	if (!anchor)
		return;

	spin_lock_irqsave(&anchor->lock, flags);
	if (likely(anchor == urb->anchor))
		usb_unanchor_urb_locked(urb, anchor);
	spin_unlock_irqrestore(&anchor->lock, flags);
}

void
usb_kill_anchored_urbs(struct usb_anchor *anchor)
{
	struct urb *urb;

	spin_lock_irq(&anchor->lock);
	while (!list_empty(&anchor->urb_list)) {
		urb = list_entry(anchor->urb_list.prev, struct urb, anchor_list);
		usb_get_urb(urb);
		spin_unlock_irq(&anchor->lock);

		usb_kill_urb(urb);
		usb_put_urb(urb);

		spin_lock_irq(&anchor->lock);
	}
	spin_unlock_irq(&anchor->lock);
}

void
usb_anchor_urb(struct urb *urb, struct usb_anchor *anchor)
{
	unsigned long flags;

	spin_lock_irqsave(&anchor->lock, flags);
	usb_get_urb(urb);
	list_add_tail(&urb->anchor_list, &anchor->urb_list);
	urb->anchor = anchor;
	spin_unlock_irqrestore(&anchor->lock, flags);
}

/*------------------------------------------------------------------------*
 *	USB scatter gather list support
 *------------------------------------------------------------------------*/

static void
sg_clean(struct usb_sg_request *io)
{
	if (io->urbs) {
		while (io->entries--)
			usb_free_urb(io->urbs[io->entries]);
		kfree(io->urbs);
		io->urbs = NULL;
	}
	io->dev = NULL;
}

static void
sg_complete(struct urb *urb)
{
	unsigned long flags;
	struct usb_sg_request *io = urb->context;
	int status = urb->status;

	spin_lock_irqsave(&io->lock, flags);

	if (io->status == 0 && status && status != -ECONNRESET) {
		int i, found;

		io->status = status;

		spin_unlock_irqrestore(&io->lock, flags);
		for (i = 0, found = 0; i < io->entries; i++) {
			if (!io->urbs[i])
				continue;
			if (found) {
				usb_block_urb(io->urbs[i]);
				usb_unlink_urb(io->urbs[i]);
			} else if (urb == io->urbs[i]) {
				found = 1;
			}
		}
		spin_lock_irqsave(&io->lock, flags);
	}

	io->bytes += urb->actual_length;
	io->count--;
	if (!io->count)
		complete(&io->complete);

	spin_unlock_irqrestore(&io->lock, flags);
}

int
usb_sg_init(struct usb_sg_request *io, struct usb_device *dev,
    unsigned pipe, unsigned period, struct scatterlist *sg,
    int nents, size_t length, gfp_t mem_flags)
{
	int i;
	int urb_flags;

	if (!io || !dev || !sg || usb_pipecontrol(pipe) || usb_pipeisoc(pipe) || nents != 1)
		return (-EINVAL);

	spin_lock_init(&io->lock);
	io->dev = dev;
	io->pipe = pipe;
	io->entries = nents;

	/* initialize all the urbs we'll use */
	io->urbs = kmalloc_array(io->entries, sizeof(*io->urbs), mem_flags);
	if (!io->urbs)
		goto nomem;

	urb_flags = 0;
	if (usb_pipein(pipe))
		urb_flags |= URB_SHORT_NOT_OK;

	for (i = 0; i != 1; i++) {
		struct urb *urb;
		unsigned len;

		urb = usb_alloc_urb(0, mem_flags);
		if (!urb) {
			io->entries = i;
			goto nomem;
		}
		io->urbs[i] = urb;

		urb->dev = NULL;
		urb->pipe = pipe;
		urb->interval = period;
		urb->transfer_flags = urb_flags;
		urb->complete = sg_complete;
		urb->context = io;
		urb->transfer_buffer = (void *)sg->dma_address;

		len = sg->length;
		if (length) {
			len = min_t(size_t, len, length);
			length -= len;
			if (length == 0)
				io->entries = i + 1;
		}
		urb->transfer_buffer_length = len;
	}

	io->count = io->entries;
	io->status = 0;
	io->bytes = 0;
	init_completion(&io->complete);
	return (0);

nomem:
	sg_clean(io);
	return (-ENOMEM);
}

void
usb_sg_wait(struct usb_sg_request *io)
{
	int i;
	int entries = io->entries;

	spin_lock_irq(&io->lock);
	for (i = 0; i < entries && !io->status; ) {
		int retval;

		io->urbs[i]->dev = io->dev;
		spin_unlock_irq(&io->lock);

		retval = usb_submit_urb(io->urbs[i], GFP_NOIO);

		switch (retval) {
		case -ENXIO:
		case -EAGAIN:
		case -ENOMEM:
			retval = 0;
			break;
		case 0:
			++i;
			break;
		default:
			io->urbs[i]->status = retval;
			usb_sg_cancel(io);
		}
		spin_lock_irq(&io->lock);
		if (retval && (io->status == 0 || io->status == -ECONNRESET))
			io->status = retval;
	}
	io->count -= entries - i;
	if (io->count == 0)
		complete(&io->complete);
	spin_unlock_irq(&io->lock);

	wait_for_completion(&io->complete);

	sg_clean(io);
}

void
usb_sg_cancel(struct usb_sg_request *io)
{
	unsigned long flags;
	int i;

	spin_lock_irqsave(&io->lock, flags);
	if (io->status || io->count == 0) {
		spin_unlock_irqrestore(&io->lock, flags);
		return;
	}

	io->status = -ECONNRESET;
	io->count++;
	spin_unlock_irqrestore(&io->lock, flags);

	for (i = io->entries - 1; i >= 0; --i) {
		usb_block_urb(io->urbs[i]);

		usb_unlink_urb(io->urbs[i]);
	}

	spin_lock_irqsave(&io->lock, flags);
	io->count--;
	if (!io->count)
		complete(&io->complete);
	spin_unlock_irqrestore(&io->lock, flags);
}
