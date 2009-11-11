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

struct usb_linux_softc {
	struct libusb20_config *pcfg;
	struct libusb20_device *pdev;
	struct usb_driver *udrv;
	struct usb_device *p_dev;
	struct usb_interface *ui;
	struct cdev *c_dev;
	pthread_t thread;
	int	fds [2];
	volatile int thread_started;
};

static struct usb_linux_softc uls[16];
static struct usb_linux_softc *puls = NULL;

/* prototypes */

static libusb20_tr_callback_t usb_linux_isoc_callback;
static libusb20_tr_callback_t usb_linux_non_isoc_callback;

static uint16_t usb_max_isoc_frames(struct usb_device *);
static const struct usb_device_id *usb_linux_lookup_id(struct LIBUSB20_DEVICE_DESC_DECODED *pdd, struct LIBUSB20_INTERFACE_DESC_DECODED *pid, const struct usb_device_id *id);
static void usb_linux_free_device(struct usb_device *dev);

static struct usb_device *usb_linux_create_usb_device(struct usb_linux_softc *sc, struct libusb20_device *udev, struct libusb20_config *pcfg, uint16_t addr);
static void usb_linux_cleanup_interface(struct usb_device *, struct usb_interface *);
static void usb_linux_complete(struct libusb20_transfer *);
static int usb_unlink_urb_sub(struct urb *, uint8_t);
static int usb_setup_endpoint(struct usb_device *dev, struct usb_host_endpoint *uhe, uint8_t do_setup);

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

static void *
usb_exec(void *arg)
{
	struct usb_linux_softc *sc = arg;
	struct libusb20_device *dev = sc->p_dev->bsd_udev;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	sc->thread_started = 1;

	while (1) {

		/* check for cancel */
		pthread_testcancel();

		/* check for USB events */
		if (libusb20_dev_process(dev) != 0) {
			/* device detached */
			break;
		}
		/* wait for USB event from kernel */
		libusb20_dev_wait_process(dev, -1);
	}

	pthread_exit(NULL);

	return (NULL);
}

static void
usb_linux_create_event_thread(struct usb_device *dev)
{
	struct usb_linux_softc *sc = dev->parent;

	sc->thread_started = 0;

	/* start USB event thread again */
	if (pthread_create(&sc->thread, NULL,
	    usb_exec, sc)) {
		printf("Failed creating USB process\n");
	} else {
		while (sc->thread_started == 0)
			pthread_yield();
	}
}

struct usb_linux_softc *
usb_linux2usb(int fd)
{
	uint8_t i;

	for (i = 0;; i++) {
		if (i == ARRAY_SIZE(uls))
			return (NULL);
		if (uls[i].fds[0] == fd)
			break;
	}
	return (uls + i);
}

struct cdev *
usb_linux2cdev(int fd)
{
	struct usb_linux_softc *sc;

	sc = usb_linux2usb(fd);
	if (sc == NULL)
		return (NULL);

	return (sc->c_dev);
}

void
usb_linux_set_cdev(struct cdev *cdev)
{
	puls->c_dev = cdev;
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

	if (sc->fds[0] > -1)
		close(sc->fds[0]);
	if (sc->fds[1] > -1)
		close(sc->fds[1]);

	free(sc->pcfg);

	memset(sc, 0, sizeof(*sc));
}

/*------------------------------------------------------------------------*
 *	usb_linux_probe
 *
 * This function is the FreeBSD probe and attach callback.
 *------------------------------------------------------------------------*/
int
usb_linux_probe(uint8_t bus, uint8_t addr, uint8_t index)
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

	for (i = 0;; i++) {
		if (i == ARRAY_SIZE(uls))
			return (-ENOMEM);
		if (uls[i].udrv == NULL)
			break;
	}

	sc = uls + i;

	puls = sc;			/* XXX */

	match_bus_addr = (addr != 0);

	if (!match_bus_addr)
		index = bus;

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
	p_dev = usb_linux_create_usb_device(sc, pdev, pcfg, addr);
	if (p_dev == NULL) {
		free(pcfg);
		libusb20_be_free(pbe);
		return (-ENOMEM);
	}
	ui = p_dev->bsd_iface_start + i;

	sc->udrv = udrv;
	sc->p_dev = p_dev;
	sc->ui = ui;
	sc->pcfg = pcfg;
	sc->pdev = pdev;

	sc->fds[0] = -1;
	sc->fds[1] = -1;

	usb_linux_create_event_thread(p_dev);

	if (pipe(sc->fds) != 0) {
		usb_linux_detach_sub(sc);
		libusb20_be_free(pbe);
		return (-ENOMEM);
	}
	if (udrv->probe(ui, id) != 0) {
		usb_linux_detach_sub(sc);
		libusb20_be_free(pbe);
		return (-ENXIO);
	}
	libusb20_be_dequeue_device(pbe, pdev);
	libusb20_be_free(pbe);

	return (sc->fds[0]);
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

	if (udrv->suspend) {
		(udrv->suspend) (ui, 0);
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
usb_max_isoc_frames(struct usb_device *dev)
{
	;				/* indent fix */
	switch (libusb20_dev_get_speed(dev->bsd_udev)) {
	case LIBUSB20_SPEED_LOW:
	case LIBUSB20_SPEED_FULL:
		return (USB_MAX_FULL_SPEED_ISOC_FRAMES);
	default:
		return (USB_MAX_HIGH_SPEED_ISOC_FRAMES);
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
	if (libusb20_tr_pending(xfer))
		return;
	libusb20_tr_start(xfer);
}

int
usb_submit_urb(struct urb *urb, uint16_t mem_flags)
{
	struct usb_host_endpoint *uhe;

	if (urb == NULL) {
		return (-EINVAL);
	}
	if (urb->pipe == NULL) {
		return (-EINVAL);
	}
	uhe = urb->pipe;

	if (usb_setup_endpoint(urb->dev, uhe, 1)) {
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

		TAILQ_INSERT_HEAD(&uhe->bsd_urb_list, urb, bsd_urb_list);

		urb->status = -EINPROGRESS;

		usb_submit_urb_sub(uhe->bsd_xfer[0]);
		usb_submit_urb_sub(uhe->bsd_xfer[1]);
		return (0);
	} else {
		/* no pipes have been setup yet! */
		urb->status = -EINVAL;
		return (-EINVAL);
	}
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
	return (usb_unlink_urb_sub(urb, 0));
}

static void
usb_unlink_bsd(struct libusb20_transfer *xfer,
    struct urb *urb, uint8_t drain)
{
	if ((xfer != NULL) &&
	    (libusb20_tr_pending(xfer) != 0) &&
	    (libusb20_tr_get_priv_sc1(xfer) == (void *)urb)) {
		/* restart transfer */
		libusb20_tr_stop(xfer);
		libusb20_tr_start(xfer);
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
	if (urb->pipe == NULL) {
		return (-EINVAL);
	}
	uhe = urb->pipe;

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
usb_clear_halt(struct usb_device *dev, struct usb_host_endpoint *uhe)
{
	if (uhe == NULL)
		return (-EINVAL);

	if (usb_setup_endpoint(dev, uhe, 1))
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
usb_control_msg(struct usb_device *dev, struct usb_host_endpoint *uhe,
    uint8_t request, uint8_t requesttype,
    uint16_t value, uint16_t wIndex, void *data,
    uint16_t size, uint32_t timeout)
{
	struct LIBUSB20_CONTROL_SETUP_DECODED req;
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

	usb_linux_cleanup_interface(dev, p_ui);
	err = libusb20_dev_set_alt_index(dev->bsd_udev,
	    p_ui->bsd_iface_index, alt_index);

	/* XXX */

	atomic_lock();
	atomic_pickup(drops);
	atomic_unlock();

	if (err)
		err = -EPIPE;
	else
		p_ui->cur_altsetting = p_ui->altsetting + alt_index;

	usb_linux_create_event_thread(dev);

	return (err);
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
int
usb_setup_endpoint(struct usb_device *dev,
    struct usb_host_endpoint *uhe, uint8_t do_setup)
{
	uint16_t bufsize;
	uint8_t type = uhe->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	uint8_t addr = uhe->desc.bEndpointAddress;
	uint8_t ep_index = ((addr / 0x40) | (addr * 4)) % (16 * 4);
	uint8_t speed;

	if (do_setup == 0) {
		if (uhe->bsd_xfer[0]) {
			libusb20_tr_close(uhe->bsd_xfer[0]);
			uhe->bsd_xfer[0] = NULL;
		}
		if (uhe->bsd_xfer[1]) {
			libusb20_tr_close(uhe->bsd_xfer[1]);
			uhe->bsd_xfer[1] = NULL;
		}
		return (0);
	}
	if (uhe->bsd_xfer[0] ||
	    uhe->bsd_xfer[1]) {
		/* transfer already setup */
		return (0);
	}
	uhe->bsd_xfer[0] = libusb20_tr_get_pointer(dev->bsd_udev, ep_index + 0);
	uhe->bsd_xfer[1] = libusb20_tr_get_pointer(dev->bsd_udev, ep_index + 1);

	if (type == USB_ENDPOINT_XFER_CONTROL)
		return (-EINVAL);

	if (type == USB_ENDPOINT_XFER_ISOC) {

		/*
		 * Isochronous transfers are special in that they don't fit
		 * into the BULK/INTR/CONTROL transfer model.
		 */

		/* need double buffering */

		if (libusb20_tr_open(uhe->bsd_xfer[0], 0,
		    usb_max_isoc_frames(dev), addr))
			goto failure;

		if (libusb20_tr_open(uhe->bsd_xfer[1], 0,
		    usb_max_isoc_frames(dev), addr))
			goto failure;

		libusb20_tr_set_callback(uhe->bsd_xfer[0],
		    &usb_linux_isoc_callback);
		libusb20_tr_set_callback(uhe->bsd_xfer[1],
		    &usb_linux_isoc_callback);

		libusb20_tr_set_priv_sc0(uhe->bsd_xfer[0], uhe);
		libusb20_tr_set_priv_sc0(uhe->bsd_xfer[1], uhe);

	} else {

		speed = libusb20_dev_get_speed(dev->bsd_udev);

		/* select a sensible buffer size */
		if (speed == LIBUSB20_SPEED_LOW) {
			bufsize = 256;
		} else if (speed == LIBUSB20_SPEED_FULL) {
			bufsize = 4096;
		} else {
			bufsize = 16384;
		}

		/* one transfer and one frame */
		if (libusb20_tr_open(uhe->bsd_xfer[0], bufsize,
		    1, addr))
			goto failure;

		libusb20_tr_set_callback(uhe->bsd_xfer[0],
		    &usb_linux_non_isoc_callback);

		libusb20_tr_set_priv_sc0(uhe->bsd_xfer[0], uhe);
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
	uint32_t size;
	uint16_t niface_total;
	uint16_t nedesc;
	uint16_t iface_index;
	uint8_t i;
	uint8_t j;
	uint8_t k;

	/*
	 * We do two passes. One pass for computing necessary memory size
	 * and one pass to initialize all the allocated memory structures.
	 */
	nedesc = 0;
	iface_index = 0;
	niface_total = 0;

	for (i = 0; i != pcfg->num_interface; i++) {
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

	p_ud->product = "";
	p_ud->manufacturer = "";
	p_ud->serial = "";
	p_ud->speed = libusb20_dev_get_speed(udev);
	p_ud->bsd_udev = udev;
	p_ud->bsd_iface_start = p_ui;
	p_ud->bsd_iface_end = p_ui + iface_index;
	p_ud->bsd_endpoint_start = p_uhe;
	p_ud->bsd_endpoint_end = p_uhe + nedesc;
	p_ud->devnum = addr;

	libusb20_me_encode(&p_ud->descriptor, sizeof(p_ud->descriptor),
	    libusb20_dev_get_device_desc(udev));

	for (i = 0; i != pcfg->num_interface; i++) {
		id = pcfg->interface + i;

		p_ui->altsetting = p_uhi;
		p_ui->cur_altsetting = p_uhi;
		p_ui->num_altsetting = id->num_altsetting + 1;
		p_ui->bsd_iface_index = i;
		p_ui->linux_udev = p_ud;

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

			for (k = 0; k != id->num_endpoints; k++) {
				ed = id->endpoints + k;
				libusb20_me_encode(&p_uhe->desc,
				    sizeof(p_uhe->desc), &ed->desc);
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
		p_ui++;
	}

	p_ud->parent = sc;
	get_device(&p_ud->dev);		/* make sure we don't get freed */

done:
	return p_ud;
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
		urb->number_of_packets = iso_packets;
	}
	return (urb);
}

/*------------------------------------------------------------------------*
 *	usb_find_host_endpoint
 *
 * The following function will return the Linux USB host endpoint
 * structure that matches the given endpoint type and endpoint
 * value. If no match is found, NULL is returned. This function is not
 * part of the Linux USB API and is only used internally.
 *------------------------------------------------------------------------*/
struct usb_host_endpoint *
usb_find_host_endpoint(struct usb_device *dev, uint8_t type, uint8_t ep)
{
	struct usb_host_endpoint *uhe;
	struct usb_host_endpoint *uhe_end;
	struct usb_host_interface *uhi;
	struct usb_interface *ui;
	uint8_t ea;
	uint8_t at;
	uint8_t mask;

	if (dev == NULL) {
		return (NULL);
	}
	if (type == USB_ENDPOINT_XFER_CONTROL) {
		mask = USB_ENDPOINT_NUMBER_MASK;
	} else {
		mask = (USB_ENDPOINT_DIR_MASK | USB_ENDPOINT_NUMBER_MASK);
	}

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

	if ((type == USB_ENDPOINT_XFER_CONTROL) && ((ep & USB_ENDPOINT_NUMBER_MASK) == 0)) {
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
usb_buffer_alloc(struct usb_device *dev, uint32_t size, uint16_t mem_flags, dma_addr_t *dma_addr)
{
	if (dma_addr)
		*dma_addr = 0;
	return (malloc(size));
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
	int err;

	uhe = dev->bsd_endpoint_start;
	uhe_end = dev->bsd_endpoint_end;
	while (uhe != uhe_end) {
		err = usb_setup_endpoint(dev, uhe, 0);
		uhe++;
	}
	err = usb_setup_endpoint(dev, &dev->ep0, 0);
	free(dev);
}

/*------------------------------------------------------------------------*
 *	usb_buffer_free
 *------------------------------------------------------------------------*/
void
usb_buffer_free(struct usb_device *dev, uint32_t size,
    void *addr, uint8_t dma_addr)
{
	free(addr);
}

/*------------------------------------------------------------------------*
 *	usb_free_urb
 *------------------------------------------------------------------------*/
void
usb_free_urb(struct urb *urb)
{
	if (urb == NULL) {
		return;
	}
	/* make sure that the current URB is not active */
	usb_kill_urb(urb);

	/* just free it */
	free(urb);
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
	if (urb == NULL) {
		return;
	}
	memset(urb, 0, sizeof(*urb));
}

/*------------------------------------------------------------------------*
 *	usb_kill_urb
 *------------------------------------------------------------------------*/
void
usb_kill_urb(struct urb *urb)
{
	if (usb_unlink_urb_sub(urb, 1)) {
		/* ignore */
	}
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
usb_linux_cleanup_interface(struct usb_device *dev, struct usb_interface *iface)
{
	struct usb_host_interface *uhi;
	struct usb_host_interface *uhi_end;
	struct usb_host_endpoint *uhe;
	struct usb_host_endpoint *uhe_end;
	struct usb_linux_softc *sc = dev->parent;
	int err;

	uhi = iface->altsetting;
	uhi_end = iface->altsetting + iface->num_altsetting;
	while (uhi != uhi_end) {
		uhe = uhi->endpoint;
		uhe_end = uhi->endpoint + uhi->desc.bNumEndpoints;
		while (uhe != uhe_end) {
			err = usb_setup_endpoint(dev, uhe, 0);
			uhe++;
		}
		uhi++;
	}

	pthread_cancel(sc->thread);
	pthread_join(sc->thread, NULL);
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
		usb_linux_complete(xfer);

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
		libusb20_tr_set_timeout(xfer, urb->timeout);
		libusb20_tr_set_total_frames(xfer, urb->number_of_packets);
		libusb20_tr_submit(xfer);
		break;

	default:			/* Error */
		if (status == LIBUSB20_TRANSFER_CANCELLED) {
			urb->status = -ECONNRESET;
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
		usb_linux_complete(xfer);

		if (status == LIBUSB20_TRANSFER_CANCELLED) {
			/* we need to return in this case */
			break;
		}
		goto tr_setup;
	}
}

/*------------------------------------------------------------------------*
 *	usb_linux_non_isoc_callback
 *
 * The following is the FreeBSD BULK/INTERRUPT and CONTROL USB
 * callback. It dequeues Linux USB stack compatible URB's, transforms
 * the URB fields into a FreeBSD USB transfer, and defragments the USB
 * transfer as required. When the transfer is complete the "complete"
 * callback is called.
 *------------------------------------------------------------------------*/
static void
usb_linux_non_isoc_callback(struct libusb20_transfer *xfer)
{
	struct urb *urb = libusb20_tr_get_priv_sc1(xfer);
	struct usb_host_endpoint *uhe = libusb20_tr_get_priv_sc0(xfer);
	uint32_t max_bulk = libusb20_tr_get_max_total_length(xfer);
	uint32_t actlen;
	uint8_t status = libusb20_tr_get_status(xfer);

	switch (status) {
	case LIBUSB20_TRANSFER_COMPLETED:

		actlen = libusb20_tr_get_actual_length(xfer);

		urb->bsd_length_rem -= actlen;
		urb->bsd_data_ptr += actlen;
		urb->actual_length += actlen;

		/* check for short transfer */
		if ((urb->bsd_length_rem != 0) &&
		    (actlen < max_bulk)) {
			urb->bsd_length_rem = 0;

			/* short transfer */
			if (urb->transfer_flags & URB_SHORT_NOT_OK) {
				urb->status = -EPIPE;
			} else {
				urb->status = 0;
			}
		} else {
			/* check remainder */
			if (urb->bsd_length_rem > 0) {
				goto setup_bulk;
			}
			/* success */
			urb->status = 0;
		}

		/* call callback */
		usb_linux_complete(xfer);

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

		/* setup data transfer */

		urb->bsd_length_rem = urb->transfer_buffer_length;
		urb->bsd_data_ptr = urb->transfer_buffer;
		urb->actual_length = 0;

setup_bulk:
		if (max_bulk > urb->bsd_length_rem) {
			max_bulk = urb->bsd_length_rem;
		}
		/* check if we need to force a short transfer */

		if ((max_bulk == urb->bsd_length_rem) &&
		    (urb->transfer_flags & URB_ZERO_PACKET)) {
			libusb20_tr_set_flags(xfer, LIBUSB20_TRANSFER_FORCE_SHORT);
		} else {
			libusb20_tr_set_flags(xfer, 0);
		}

		libusb20_tr_setup_bulk(xfer, urb->bsd_data_ptr, max_bulk, urb->timeout);
		libusb20_tr_submit(xfer);
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
		usb_linux_complete(xfer);

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
    struct usb_host_endpoint *pipe,
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
    struct usb_host_endpoint *pipe,
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
    struct usb_host_endpoint *pipe,
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
	if (dev->speed == USB_SPEED_HIGH)
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
	uint64_t expire;
	int retval;

	init_completion(&ctx.done);
	urb->context = &ctx;
	urb->actual_length = 0;
	retval = usb_submit_urb(urb, GFP_NOIO);
	if (retval)
		goto out;

	expire = timeout ? msecs_to_jiffies(timeout) : (120 * HZ);
	if (!wait_for_completion_timeout(&ctx.done, expire)) {
		usb_kill_urb(urb);
		retval = (ctx.status == -ENOENT ? -ETIMEDOUT : ctx.status);

	} else
		retval = ctx.status;
out:
	if (actual_length)
		*actual_length = urb->actual_length;

	usb_free_urb(urb);
	uninit_completion(&ctx.done);
	return retval;
}

int
usb_bulk_msg(struct usb_device *usb_dev, struct usb_host_endpoint *ep,
    void *data, int len, int *actual_length, int timeout)
{
	struct urb *urb;

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
		return -ENOMEM;
	if ((ep->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
	    USB_ENDPOINT_XFER_INT) {

		usb_fill_int_urb(urb, usb_dev, ep, data, len,
		    usb_api_blocking_completion, NULL,
		    ep->desc.bInterval);
	} else
		usb_fill_bulk_urb(urb, usb_dev, ep, data, len,
		    usb_api_blocking_completion, NULL);

	return usb_start_wait_urb(urb, timeout, actual_length);
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
