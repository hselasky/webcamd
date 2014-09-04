/* $FreeBSD: src/sys/dev/usb/usb_compat_linux.h,v 1.3 2009/05/21 01:05:21 thompsa Exp $ */
/*-
 * Copyright (c) 2007 Luigi Rizzo - Universita` di Pisa. All rights reserved.
 * Copyright (c) 2007-2009 Hans Petter Selasky. All rights reserved.
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

/*
 * Many definitions in this header file derive
 * from the Linux Kernel's usb.h and ch9.h
 */

#ifndef _LINUX_USB_H_
#define	_LINUX_USB_H_

#include <linux/pm.h>

#ifndef __packed
#define	__packed __attribute__((__packed__))
#endif

struct usb_device;
struct usb_interface;
struct usb_driver;
struct usb_linux_softc;
struct usb_device_id;
struct input_id;
struct urb;

typedef void (*usb_complete_t)(struct urb *);

#define	USB_LINUX_IFACE_MAX 32
#define	USB_MAXIADS	(USB_LINUX_IFACE_MAX / 2)

#define	USB_MAX_FULL_SPEED_ISOC_FRAMES (60 * 1)
#define	USB_MAX_HIGH_SPEED_ISOC_FRAMES (60 * 8)

#define	USB_SPEED_UNKNOWN 255		/* XXX */
#define	USB_SPEED_LOW LIBUSB20_SPEED_LOW
#define	USB_SPEED_FULL LIBUSB20_SPEED_FULL
#define	USB_SPEED_HIGH LIBUSB20_SPEED_HIGH
#define	USB_SPEED_VARIABLE LIBUSB20_SPEED_VARIABLE
#define	USB_SPEED_WIRELESS LIBUSB20_SPEED_VARIABLE
#define	USB_SPEED_SUPER LIBUSB20_SPEED_SUPER

#define	USB_CTRL_GET_TIMEOUT    5000	/* ms */
#define	USB_CTRL_SET_TIMEOUT    5000	/* ms */

#if 0
/*
 * Linux compatible USB device drivers put their device information
 * into the "usb_device_id" structure using the "USB_DEVICE()" macro.
 * The "MODULE_DEVICE_TABLE()" macro can be used to export this
 * information to userland.
 */
struct usb_device_id {
	/* which fields to match against */
	uint16_t match_flags;
#define	USB_DEVICE_ID_MATCH_VENDOR		0x0001
#define	USB_DEVICE_ID_MATCH_PRODUCT		0x0002
#define	USB_DEVICE_ID_MATCH_DEV_LO		0x0004
#define	USB_DEVICE_ID_MATCH_DEV_HI		0x0008
#define	USB_DEVICE_ID_MATCH_DEV_CLASS		0x0010
#define	USB_DEVICE_ID_MATCH_DEV_SUBCLASS	0x0020
#define	USB_DEVICE_ID_MATCH_DEV_PROTOCOL	0x0040
#define	USB_DEVICE_ID_MATCH_INT_CLASS		0x0080
#define	USB_DEVICE_ID_MATCH_INT_SUBCLASS	0x0100
#define	USB_DEVICE_ID_MATCH_INT_PROTOCOL	0x0200
#define	USB_DEVICE_ID_MATCH_INT_INFO \
	(USB_DEVICE_ID_MATCH_INT_CLASS | \
	 USB_DEVICE_ID_MATCH_INT_SUBCLASS | \
	 USB_DEVICE_ID_MATCH_INT_PROTOCOL)
#define	USB_DEVICE_ID_MATCH_DEV_RANGE \
	(USB_DEVICE_ID_MATCH_DEV_LO | \
	 USB_DEVICE_ID_MATCH_DEV_HI)
#define	USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION \
	(USB_DEVICE_ID_MATCH_DEVICE | \
	 USB_DEVICE_ID_MATCH_DEV_LO | \
	 USB_DEVICE_ID_MATCH_DEV_HI)

	/* Used for product specific matches; the BCD range is inclusive */
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice_lo;
	uint16_t bcdDevice_hi;

	/* Used for device class matches */
	uint8_t	bDeviceClass;
	uint8_t	bDeviceSubClass;
	uint8_t	bDeviceProtocol;

	/* Used for interface class matches */
	uint8_t	bInterfaceClass;
	uint8_t	bInterfaceSubClass;
	uint8_t	bInterfaceProtocol;

	/* Hook for driver specific information */
	unsigned long driver_info;
};

#else
#define	USB_DEVICE_ID_MATCH_INT_INFO \
	(USB_DEVICE_ID_MATCH_INT_CLASS | \
	 USB_DEVICE_ID_MATCH_INT_SUBCLASS | \
	 USB_DEVICE_ID_MATCH_INT_PROTOCOL)
#define	USB_DEVICE_ID_MATCH_DEV_RANGE \
	(USB_DEVICE_ID_MATCH_DEV_LO | \
	 USB_DEVICE_ID_MATCH_DEV_HI)
#define	USB_DEVICE_ID_MATCH_DEVICE_AND_VERSION \
	(USB_DEVICE_ID_MATCH_DEVICE | \
	 USB_DEVICE_ID_MATCH_DEV_LO | \
	 USB_DEVICE_ID_MATCH_DEV_HI)
#endif

#define	USB_DEVICE_ID_MATCH_DEVICE \
	(USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT)

#define	USB_DEVICE(vend,prod) \
	.match_flags = USB_DEVICE_ID_MATCH_VENDOR |	\
		USB_DEVICE_ID_MATCH_PRODUCT,		\
	.idVendor = (vend),				\
	.idProduct = (prod)

#define	USB_DEVICE_VER(vend,prod,lo_ver,hi_ver)		\
	.match_flags = USB_DEVICE_ID_MATCH_VENDOR |	\
		USB_DEVICE_ID_MATCH_PRODUCT |		\
		USB_DEVICE_ID_MATCH_DEV_LO |		\
		USB_DEVICE_ID_MATCH_DEV_HI,		\
	.idVendor = (vend),				\
	.idProduct = (prod),				\
	.bcdDevice_lo = (lo_ver),			\
	.bcdDevice_hi = (hi_ver)

#define	USB_INTERFACE_INFO(a,b,c)\
	.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS |	\
		USB_DEVICE_ID_MATCH_INT_SUBCLASS |	\
		USB_DEVICE_ID_MATCH_INT_PROTOCOL,	\
	.bInterfaceClass = (a),				\
	.bInterfaceSubClass = (b),			\
	.bInterfaceProtocol = (c)

#define	USB_DEVICE_AND_INTERFACE_INFO(vend, prod, a, b, c)\
	.match_flags = USB_DEVICE_ID_MATCH_VENDOR |	  \
		USB_DEVICE_ID_MATCH_PRODUCT |		  \
		USB_DEVICE_ID_MATCH_INT_CLASS |		  \
		USB_DEVICE_ID_MATCH_INT_SUBCLASS |	  \
		USB_DEVICE_ID_MATCH_INT_PROTOCOL,	  \
	.idVendor = (vend),				  \
	.idProduct = (prod),				  \
	.bInterfaceClass = (a),				  \
	.bInterfaceSubClass = (b),			  \
	.bInterfaceProtocol = (c)

/* The "usb_driver" structure holds the Linux USB device driver
 * callbacks, and a pointer to device ID's which this entry should
 * match against. Usually this entry is exposed to the USB emulation
 * layer using the "USB_DRIVER_EXPORT()" macro, which is defined
 * below.
 */
struct usb_driver {
	const char *name;

	int     (*probe) (struct usb_interface *intf,
	    	const	struct usb_device_id *id);

	void    (*disconnect) (struct usb_interface *intf);

	int     (*ioctl) (struct usb_interface *intf, unsigned int code,
	    	void  *buf);

	int     (*suspend) (struct usb_interface *intf, pm_message_t message);
	int     (*resume) (struct usb_interface *intf);

	int     (*reset_resume) (struct usb_interface *intf);

	const struct usb_device_id *id_table;

	void    (*shutdown) (struct usb_interface *intf);

	int     (*pre_reset) (struct usb_interface *);
	int     (*post_reset) (struct usb_interface *);

	LIST_ENTRY(usb_driver) linux_driver_list;

	uint8_t	supports_autosuspend;
	uint8_t	soft_unbind;
	uint8_t	no_dynamic_id;
};

/*
 * Generic USB descriptor header.
 */
struct usb_descriptor_header {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
} __packed;

/*
 * The following structure is the same as "usb_device_descriptor_t"
 * except that 16-bit values are "uint16_t" and not an array of "uint8_t".
 * It is used by Linux USB device drivers.
 */
struct usb_device_descriptor {
	uint8_t	bLength;
	uint8_t	bDescriptorType;

	uint16_t bcdUSB;
	uint8_t	bDeviceClass;
	uint8_t	bDeviceSubClass;
	uint8_t	bDeviceProtocol;
	uint8_t	bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t	iManufacturer;
	uint8_t	iProduct;
	uint8_t	iSerialNumber;
	uint8_t	bNumConfigurations;
} __packed;

/*
 * The following structure is the same as "usb_config_descriptor_t"
 * except that 16-bit values are "uint16_t" and not an array of "uint8_t".
 * It is used by Linux USB device drivers.
 */
struct usb_config_descriptor {
	uint8_t	bLength;
	uint8_t	bDescriptorType;

	uint16_t wTotalLength;
	uint8_t	bNumInterfaces;
	uint8_t	bConfigurationValue;
	uint8_t	iConfiguration;
	uint8_t	bmAttributes;
#define	USB_CONFIG_ATT_ONE		(1 << 7)	/* must be set */
#define	USB_CONFIG_ATT_SELFPOWER	(1 << 6)	/* self powered */
#define	USB_CONFIG_ATT_WAKEUP		(1 << 5)	/* can wakeup */
#define	USB_CONFIG_ATT_BATTERY		(1 << 4)	/* battery powered */
	uint8_t	bMaxPower;		/* max current in 2 mA units */
} __packed;

/*
 * The following structure is the same as
 * "usb_interface_descriptor_t". It is used by
 * Linux USB device drivers.
 */
struct usb_interface_descriptor {
	uint8_t	bLength;
	uint8_t	bDescriptorType;

	uint8_t	bInterfaceNumber;
	uint8_t	bAlternateSetting;
	uint8_t	bNumEndpoints;
	uint8_t	bInterfaceClass;
	uint8_t	bInterfaceSubClass;
	uint8_t	bInterfaceProtocol;
	uint8_t	iInterface;
} __packed;

struct usb_interface_assoc_descriptor {
	uint8_t	bLength;
	uint8_t	bDescriptorType;

	uint8_t	bFirstInterface;
	uint8_t	bInterfaceCount;
	uint8_t	bFunctionClass;
	uint8_t	bFunctionSubClass;
	uint8_t	bFunctionProtocol;
	uint8_t	iFunction;
} __packed;

/*
 * The following structure is the same as "usb_endpoint_descriptor_t"
 * except that 16-bit values are "uint16_t" and not an array of "uint8_t".
 * It is used by Linux USB device drivers.
 */
struct usb_endpoint_descriptor {
	uint8_t	bLength;
	uint8_t	bDescriptorType;

	uint8_t	bEndpointAddress;
	uint8_t	bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t	bInterval;

	/* extension for audio endpoints only: */
	uint8_t	bRefresh;
	uint8_t	bSynchAddress;
} __packed;

struct usb_ss_ep_comp_descriptor {
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bMaxBurst;
	uint8_t	bmAttributes;
	uint16_t wBytesPerInterval;
} __packed;

#define	USB_DT_SS_EP_COMP_SIZE		6
#define	USB_SS_MAX_STREAMS(x)		(1 << ((x) & 0x1f))
#define	USB_SS_MULT(x)			(1 + ((x) & 0x3))

#define	USB_DT_ENDPOINT_SIZE		7
#define	USB_DT_ENDPOINT_AUDIO_SIZE	9

/*
 * Endpoints
 */
#define	USB_ENDPOINT_NUMBER_MASK	0x0f	/* in bEndpointAddress */
#define	USB_ENDPOINT_DIR_MASK		0x80

#define	USB_ENDPOINT_XFERTYPE_MASK	0x03	/* in bmAttributes */
#define	USB_ENDPOINT_XFER_CONTROL	0
#define	USB_ENDPOINT_XFER_ISOC		1
#define	USB_ENDPOINT_XFER_BULK		2
#define	USB_ENDPOINT_XFER_INT		3
#define	USB_ENDPOINT_MAX_ADJUSTABLE	0x80

/* CONTROL REQUEST SUPPORT */

/*
 * Definition of direction mask for
 * "bEndpointAddress" and "bmRequestType":
 */
#define	USB_DIR_MASK			0x80
#define	USB_DIR_OUT			0x00	/* write to USB device */
#define	USB_DIR_IN			0x80	/* read from USB device */

/*
 * Definition of type mask for
 * "bmRequestType":
 */
#define	USB_TYPE_MASK			(0x03 << 5)
#define	USB_TYPE_STANDARD		(0x00 << 5)
#define	USB_TYPE_CLASS			(0x01 << 5)
#define	USB_TYPE_VENDOR			(0x02 << 5)
#define	USB_TYPE_RESERVED		(0x03 << 5)

/*
 * Definition of receiver mask for
 * "bmRequestType":
 */
#define	USB_RECIP_MASK			0x1f
#define	USB_RECIP_DEVICE		0x00
#define	USB_RECIP_INTERFACE		0x01
#define	USB_RECIP_ENDPOINT		0x02
#define	USB_RECIP_OTHER			0x03

/*
 * Definition of standard request values for
 * "bRequest":
 */
#define	USB_REQ_GET_STATUS		0x00
#define	USB_REQ_CLEAR_FEATURE		0x01
#define	USB_REQ_SET_FEATURE		0x03
#define	USB_REQ_SET_ADDRESS		0x05
#define	USB_REQ_GET_DESCRIPTOR		0x06
#define	USB_REQ_SET_DESCRIPTOR		0x07
#define	USB_REQ_GET_CONFIGURATION	0x08
#define	USB_REQ_SET_CONFIGURATION	0x09
#define	USB_REQ_GET_INTERFACE		0x0A
#define	USB_REQ_SET_INTERFACE		0x0B
#define	USB_REQ_SYNCH_FRAME		0x0C

#define	USB_REQ_SET_ENCRYPTION		0x0D	/* Wireless USB */
#define	USB_REQ_GET_ENCRYPTION		0x0E
#define	USB_REQ_SET_HANDSHAKE		0x0F
#define	USB_REQ_GET_HANDSHAKE		0x10
#define	USB_REQ_SET_CONNECTION		0x11
#define	USB_REQ_SET_SECURITY_DATA	0x12
#define	USB_REQ_GET_SECURITY_DATA	0x13
#define	USB_REQ_SET_WUSB_DATA		0x14
#define	USB_REQ_LOOPBACK_DATA_WRITE	0x15
#define	USB_REQ_LOOPBACK_DATA_READ	0x16
#define	USB_REQ_SET_INTERFACE_DS	0x17

/*
 * USB feature flags are written using USB_REQ_{CLEAR,SET}_FEATURE, and
 * are read as a bit array returned by USB_REQ_GET_STATUS.  (So there
 * are at most sixteen features of each type.)
 */
#define	USB_DEVICE_SELF_POWERED		0	/* (read only) */
#define	USB_DEVICE_REMOTE_WAKEUP	1	/* dev may initiate wakeup */
#define	USB_DEVICE_TEST_MODE		2	/* (wired high speed only) */
#define	USB_DEVICE_BATTERY		2	/* (wireless) */
#define	USB_DEVICE_B_HNP_ENABLE		3	/* (otg) dev may initiate HNP */
#define	USB_DEVICE_WUSB_DEVICE		3	/* (wireless) */
#define	USB_DEVICE_A_HNP_SUPPORT	4	/* (otg) RH port supports HNP */
#define	USB_DEVICE_A_ALT_HNP_SUPPORT	5	/* (otg) other RH port does */
#define	USB_DEVICE_DEBUG_MODE		6	/* (special devices only) */

#define	USB_ENDPOINT_HALT		0	/* IN/OUT will STALL */

#define	PIPE_ISOCHRONOUS		0x01	/* UE_ISOCHRONOUS */
#define	PIPE_INTERRUPT			0x03	/* UE_INTERRUPT */
#define	PIPE_CONTROL			0x00	/* UE_CONTROL */
#define	PIPE_BULK			0x02	/* UE_BULK */

/* Whenever Linux references an USB endpoint:
 * a) to initialize "urb->pipe"
 * b) second argument passed to "usb_control_msg()"
 *
 * Then it uses one of the following macros. The "endpoint" argument
 * is the physical endpoint value masked by 0xF. The "dev" argument
 * is a pointer to "struct usb_device".
 */
#define	usb_sndctrlpipe(dev,endpoint) \
  usb_create_host_endpoint(dev, PIPE_CONTROL, (endpoint) & ~USB_DIR_IN)

#define	usb_rcvctrlpipe(dev,endpoint) \
  usb_create_host_endpoint(dev, PIPE_CONTROL, (endpoint) | USB_DIR_IN)

#define	usb_sndisocpipe(dev,endpoint) \
  usb_create_host_endpoint(dev, PIPE_ISOCHRONOUS, (endpoint) & ~USB_DIR_IN)

#define	usb_rcvisocpipe(dev,endpoint) \
  usb_create_host_endpoint(dev, PIPE_ISOCHRONOUS, (endpoint) | USB_DIR_IN)

#define	usb_sndbulkpipe(dev,endpoint) \
  usb_create_host_endpoint(dev, PIPE_BULK, (endpoint) & ~USB_DIR_IN)

#define	usb_rcvbulkpipe(dev,endpoint) \
  usb_create_host_endpoint(dev, PIPE_BULK, (endpoint) | USB_DIR_IN)

#define	usb_sndintpipe(dev,endpoint) \
  usb_create_host_endpoint(dev, PIPE_INTERRUPT, (endpoint) & ~USB_DIR_IN)

#define	usb_rcvintpipe(dev,endpoint) \
  usb_create_host_endpoint(dev, PIPE_INTERRUPT, (endpoint) | USB_DIR_IN)

#define	usb_pipein(endpoint) ((endpoint) & USB_DIR_IN)
#define	usb_pipeout(endpoint) ((~(endpoint)) & USB_DIR_IN)

/* The following four structures makes up a tree, where we have the
 * leaf structure, "usb_host_endpoint", first, and the root structure,
 * "usb_device", last. The four structures below mirror the structure
 * of the USB descriptors belonging to an USB configuration. Please
 * refer to the USB specification for a definition of "endpoints" and
 * "interfaces".
 */
struct usb_host_endpoint {
	struct usb_endpoint_descriptor desc;
	struct usb_ss_ep_comp_descriptor ss_ep_comp;

	TAILQ_HEAD(, urb) bsd_urb_list;

	struct libusb20_transfer *bsd_xfer[2];

	uint8_t *extra;			/* Extra descriptors */

	uint16_t extralen;

	uint8_t	bsd_iface_index;

	void   *align[0];
};

struct usb_host_interface {
	struct usb_interface_descriptor desc;

	/* the following array has size "desc.bNumEndpoint" */
	struct usb_host_endpoint *endpoint;

	const char *string;		/* iInterface string, if present */
	uint8_t *extra;			/* Extra descriptors */

	uint16_t extralen;

	uint8_t	bsd_iface_index;

	void   *align[0];
};

struct usb_interface {
	struct device dev;

	/* array of alternate settings for this interface */
	struct usb_host_interface *altsetting;
	struct usb_host_interface *cur_altsetting;
	struct usb_device *linux_udev;
	void   *bsd_priv_sc;		/* device specific information */

	uint8_t	num_altsetting;		/* number of alternate settings */
	uint8_t	bsd_iface_index;
	uint8_t	needs_remote_wakeup;

	void   *align[0];
};

#define	usb_host_config usb_config

struct usb_config {
	struct usb_config_descriptor desc;
	struct usb_interface *interface[USB_LINUX_IFACE_MAX];
	struct usb_interface_assoc_descriptor *intf_assoc[USB_MAXIADS];
};

struct usb_device {
	struct device dev;
	struct device *bus;
	void   *parent;
	struct usb_config *config;
	struct usb_config *actconfig;
	struct usb_host_endpoint *ep_in[16];
	struct usb_host_endpoint *ep_out[16];

	struct usb_device_descriptor descriptor;
	struct usb_config bsd_config;
	struct usb_host_endpoint ep0;

	struct libusb20_device *bsd_udev;
	struct usb_interface *bsd_iface_start;
	struct usb_interface *bsd_iface_end;
	struct usb_host_endpoint *bsd_endpoint_start;
	struct usb_host_endpoint *bsd_endpoint_end;

	/* static strings from the device */
	char   *product;		/* iProduct string, if present */
	char   *manufacturer;		/* iManufacturer string, if present */
	char   *serial;			/* iSerialNumber string, if present */

	uint16_t devnum;
	uint16_t bsd_last_ms;		/* completion time of last ISOC
					 * transfer */

	uint8_t	speed;			/* LIBUSB20_SPEED_XXX */

	char	devpath[1];

	void   *align[0];
};

/*
 * The following structure is used to extend "struct urb" when we are
 * dealing with an isochronous endpoint. It contains information about
 * the data offset and data length of an isochronous packet.
 * The "actual_length" field is updated before the "complete"
 * callback in the "urb" structure is called.
 */
struct usb_iso_packet_descriptor {
	uint32_t offset;		/* depreciated buffer offset (the
					 * packets are usually back to back) */
	uint16_t length;		/* expected length */
	uint16_t actual_length;
	int16_t	status;			/* status */
};

/*
 * The following structure holds various information about an USB
 * transfer. This structure is used for all kinds of USB transfers.
 *
 * URB is short for USB Request Block.
 */
struct urb {
	TAILQ_ENTRY(urb) bsd_urb_list;

	struct usb_device *dev;		/* (in) pointer to associated device */
	unsigned int pipe;		/* (in) pipe */
	uint8_t *setup_packet;		/* (in) setup packet (control only) */
	void   *transfer_buffer;	/* (in) associated data buffer */
	void   *context;		/* (in) context for completion */
	usb_complete_t complete;	/* (in) completion routine */

	uint32_t transfer_buffer_length;/* (in) data buffer length */
	uint32_t actual_length;		/* (return) actual transfer length */
	uint32_t timeout;		/* (in) FreeBSD specific */
	uint32_t reject;		/* (internal) reject URB */

	uint16_t transfer_flags;	/* (in) */
#define	URB_SHORT_NOT_OK	0x0001	/* report short transfers like errors */
#define	URB_ISO_ASAP		0x0002	/* ignore "start_frame" field */
#define	URB_ZERO_PACKET		0x0004	/* the USB transfer ends with a short
					 * packet */
#define	URB_NO_TRANSFER_DMA_MAP 0x0008	/* "transfer_dma" is valid on submit */
#define	URB_WAIT_WAKEUP		0x0010	/* custom flags */
#define	URB_IS_SLEEPING		0x0020	/* custom flags */
#define	URB_FREE_BUFFER		0x0040	/* free transfer buffer with the URB */

	uint16_t start_frame;		/* (modify) start frame (ISO) */
	uint16_t number_of_packets;	/* (in) number of ISO packets */
	uint16_t interval;		/* (modify) transfer interval
					 * (INT/ISO) */
	uint16_t error_count;		/* (return) number of ISO errors */
	int16_t	status;			/* (return) status */

	uint8_t	setup_dma;		/* (in) not used on FreeBSD */
	dma_addr_t transfer_dma;	/* (in) not used on FreeBSD */
	uint8_t	bsd_no_resubmit;	/* (internal) FreeBSD specific */

	struct usb_iso_packet_descriptor iso_frame_desc[];	/* (in) ISO ONLY */
};

/* various prototypes */

uint16_t usb_get_current_frame_number(struct usb_device *dev);

int	usb_submit_urb(struct urb *urb, uint16_t mem_flags);
int	usb_unlink_urb(struct urb *urb);
int	usb_clear_halt(struct usb_device *dev, unsigned int);
int	usb_control_msg(struct usb_device *dev, unsigned int, uint8_t request, uint8_t requesttype, uint16_t value, uint16_t index, void *data, uint16_t size, uint32_t timeout);
int	usb_set_interface(struct usb_device *dev, uint8_t ifnum, uint8_t alternate);

unsigned int usb_create_host_endpoint(struct usb_device *dev, uint8_t type, uint8_t ep);
struct urb *usb_alloc_urb(uint16_t iso_packets, uint16_t mem_flags);
struct usb_host_interface *usb_altnum_to_altsetting(const struct usb_interface *intf, uint8_t alt_index);
struct usb_interface *usb_ifnum_to_if(struct usb_device *dev, uint8_t iface_no);

void   *usb_buffer_alloc(struct usb_device *dev, uint32_t size, uint16_t mem_flags, dma_addr_t *dma_addr);
void   *usb_get_intfdata(struct usb_interface *intf);

void	usb_buffer_free(struct usb_device *dev, uint32_t size, void *addr, dma_addr_t dma_addr);
void	usb_free_urb(struct urb *urb);
void	usb_init_urb(struct urb *urb);
void	usb_kill_urb(struct urb *urb);
void	usb_set_intfdata(struct usb_interface *intf, void *data);
int	usb_register(struct usb_driver *drv);
int	usb_deregister(struct usb_driver *drv);

struct usb_linux_softc *usb_linux2usb(int fd);
int	usb_linux_probe_p(int *p_bus, int *p_addr, int *p_index, const char **ppdesc);
int	usb_linux_detach(int fd);
int	usb_linux_suspend(int fd);
int	usb_linux_resume(int fd);

#define	interface_to_usbdev(intf) (intf)->linux_udev
#define	interface_to_bsddev(intf) (intf)->linux_udev->bsd_udev

/* chapter 9 stuff, taken from "include/linux/usb/ch9.h" */

#define	USB_DT_DEVICE                   0x01
#define	USB_DT_CONFIG                   0x02
#define	USB_DT_STRING                   0x03
#define	USB_DT_INTERFACE                0x04
#define	USB_DT_ENDPOINT                 0x05
#define	USB_DT_DEVICE_QUALIFIER         0x06
#define	USB_DT_OTHER_SPEED_CONFIG       0x07
#define	USB_DT_INTERFACE_POWER          0x08
#define	USB_DT_OTG                      0x09
#define	USB_DT_DEBUG                    0x0a
#define	USB_DT_INTERFACE_ASSOCIATION    0x0b
#define	USB_DT_SECURITY                 0x0c
#define	USB_DT_KEY                      0x0d
#define	USB_DT_ENCRYPTION_TYPE          0x0e
#define	USB_DT_BOS                      0x0f
#define	USB_DT_DEVICE_CAPABILITY        0x10
#define	USB_DT_WIRELESS_ENDPOINT_COMP   0x11
#define	USB_DT_WIRE_ADAPTER             0x21
#define	USB_DT_RPIPE                    0x22
#define	USB_DT_CS_RADIO_CONTROL         0x23

#define	USB_DT_CS_DEVICE                (USB_TYPE_CLASS | USB_DT_DEVICE)
#define	USB_DT_CS_CONFIG                (USB_TYPE_CLASS | USB_DT_CONFIG)
#define	USB_DT_CS_STRING                (USB_TYPE_CLASS | USB_DT_STRING)
#define	USB_DT_CS_INTERFACE             (USB_TYPE_CLASS | USB_DT_INTERFACE)
#define	USB_DT_CS_ENDPOINT              (USB_TYPE_CLASS | USB_DT_ENDPOINT)

#define	USB_CLASS_PER_INTERFACE         0x00
#define	USB_CLASS_AUDIO                 0x01
#define	USB_CLASS_COMM                  0x02
#define	USB_CLASS_HID                   0x03
#define	USB_CLASS_PHYSICAL              0x05
#define	USB_CLASS_STILL_IMAGE           0x06
#define	USB_CLASS_PRINTER               0x07
#define	USB_CLASS_MASS_STORAGE          0x08
#define	USB_CLASS_HUB                   0x09
#define	USB_CLASS_CDC_DATA              0x0a
#define	USB_CLASS_CSCID                 0x0b
#define	USB_CLASS_CONTENT_SEC           0x0d
#define	USB_CLASS_VIDEO                 0x0e
#define	USB_CLASS_WIRELESS_CONTROLLER   0xe0
#define	USB_CLASS_MISC                  0xef
#define	USB_CLASS_APP_SPEC              0xfe
#define	USB_CLASS_VENDOR_SPEC           0xff

struct usb_ctrlrequest {
	__u8	bRequestType;
	__u8	bRequest;
	__le16	wValue;
	__le16	wIndex;
	__le16	wLength;
} __packed;

int	usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd);
int	usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd);
int	usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd);
int	usb_endpoint_xfer_control(const struct usb_endpoint_descriptor *epd);
int	usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd);
int	usb_endpoint_xfer_isoc(const struct usb_endpoint_descriptor *epd);
void	usb_fill_control_urb(struct urb *, struct usb_device *, unsigned int, unsigned char *setup_packet, void *transfer_buffer, int buffer_length, usb_complete_t complete_fn, void *context);
void	usb_fill_bulk_urb(struct urb *, struct usb_device *, unsigned int, void *transfer_buffer, int buffer_length, usb_complete_t complete_fn, void *context);
void	usb_fill_int_urb(struct urb *, struct usb_device *, unsigned int, void *transfer_buffer, int buffer_length, usb_complete_t complete_fn, void *context, int interval);
struct usb_interface *usb_get_intf(struct usb_interface *intf);
void	usb_put_intf(struct usb_interface *intf);
struct usb_device *usb_get_dev(struct usb_device *intf);
void	usb_put_dev(struct usb_device *intf);
int	usb_string(struct usb_device *dev, int index, char *buf, size_t size);
int	usb_make_path(struct usb_device *dev, char *buf, size_t size);
int	usb_autopm_get_interface(struct usb_interface *);
int	usb_autopm_set_interface(struct usb_interface *);
int	usb_driver_claim_interface(struct usb_driver *, struct usb_interface *, void *);

#define	usb_autopm_put_interface(...) __nop
#define	usb_autopm_enable(...) __nop
#define	usb_autopm_disable(...) __nop
#define	usb_mark_last_busy(...) __nop
#define	usb_driver_release_interface(...) __nop

#define	usb_endpoint_type(epd) \
	((epd)->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
#define	usb_endpoint_num(epd) \
	((epd)->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)
#define	usb_endpoint_is_bulk_in(epd) \
	(usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_in(epd))
#define	usb_endpoint_is_bulk_out(epd) \
	(usb_endpoint_xfer_bulk(epd) && usb_endpoint_dir_out(epd))
#define	usb_endpoint_is_int_in(epd) \
	(usb_endpoint_xfer_int(epd) && usb_endpoint_dir_in(epd))
#define	usb_endpoint_is_int_out(epd) \
	(usb_endpoint_xfer_int(epd) && usb_endpoint_dir_out(epd))
#define	usb_endpoint_is_isoc_in(epd) \
	(usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_in(epd))
#define	usb_endpoint_is_isoc_out(epd) \
	(usb_endpoint_xfer_isoc(epd) && usb_endpoint_dir_out(epd))

int	usb_bulk_msg(struct usb_device *, unsigned int, void *, int, int *, int);
int	usb_match_device(struct usb_device *, const struct usb_device_id *);
int	usb_match_one_id(struct usb_interface *, const struct usb_device_id *);
const struct usb_device_id *usb_match_id(struct usb_interface *, const struct usb_device_id *);
int	usb_reset_configuration(struct usb_device *dev);
int	usb_lock_device_for_reset(struct usb_device *udev, const struct usb_interface *iface);
void	usb_unlock_device(struct usb_device *udev);
int	usb_reset_device(struct usb_device *dev);
uint8_t	usb_pipetype(unsigned int);
uint16_t usb_maxpacket(struct usb_device *dev, int endpoint, int is_out);
void	usb_enable_autosuspend(struct usb_device *udev);
int	__usb_get_extra_descriptor(char *, unsigned, unsigned char, void **);
int	usb_translate_errors(int);

#define	usb_pipeisoc(pipe) (usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define	usb_pipeint(pipe) (usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define	usb_pipecontrol(pipe) (usb_pipetype((pipe)) == PIPE_CONTROL)
#define	usb_pipebulk(pipe) (usb_pipetype((pipe)) == PIPE_BULK)

#define	usb_interrupt_msg(dev, pipe, data, len, actlen, timo) \
    usb_bulk_msg(dev, pipe, data, len, actlen, timo)

#define	usb_get_extra_descriptor(desc, type, ptr) \
	__usb_get_extra_descriptor((desc)->extra, \
        (desc)->extralen, type, (void *)(ptr))

#define	usb_hub_for_each_child(a,b,c) \
	for ((b) = 0, (c) = NULL; 0;)

int	usb_endpoint_maxp(const struct usb_endpoint_descriptor *);

#define	usb_alloc_coherent(...) usb_buffer_alloc(__VA_ARGS__)
#define	usb_free_coherent(...) usb_buffer_free(__VA_ARGS__)
#define	usb_debug_root NULL

#define	to_usb_interface(d) container_of(d, struct usb_interface, dev)

#define	USB_STATE_URB_BUF 0x01

int	usb_autopm_get_interface_async(struct usb_interface *);
int	usb_autopm_get_interface_no_resume(struct usb_interface *);
int	usb_autopm_get_interface_no_suspend(struct usb_interface *);

void	usb_autopm_put_interface_async(struct usb_interface *);
void	usb_autopm_put_interface_no_suspend(struct usb_interface *);
void	usb_autopm_put_interface_no_resume(struct usb_interface *);

void	usb_block_urb(struct urb *);
void	usb_unblock_urb(struct urb *);

#endif					/* _LINUX_USB_H_ */
