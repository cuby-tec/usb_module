/*
 * USB Skeleton driver - 2.0
 *
 * Copyright (C) 2001-2004 Greg Kroah-Hartman (greg@kroah.com)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 * This driver is based on the 2.6.3 version of drivers/usb/usb-skeleton.c
 * but has been rewritten to be easy to read and use, as no locks are now
 * needed anymore.
 * 
 * sudo cp 180-cuby-permissions.rules /etc/udev/rules.d/
 * insmod test1.ko
 *
 */
/* Project: ctl
 * Company: CUBY,Ltd
 * Date: 24-07-2017
 * Description:
 * Driver for control and dataexchange with TM4C123GXL,
 * controller TM4C123GH6PMI.
 * 80MHz 32-bit ARM Cortex-M4 with floating point.
 * Dual 12-bit ADC, 5x64-bit 6x32-bit timers, MC PWMs.
 * make file ---  $make
 *
 */



//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
//#include <linux/smp_lock.h>
#include <linux/usb.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include <linux/export.h>
//#include <linux/errno.h>
//#include <linux/err.h>

#include <linux/mutex.h>
#include <generated/uapi/linux/version.h>

MODULE_LICENSE("Dual BSD/GPL");


#define err 	printk
#define dbg		printk
#define info	printk


#define READ_TIIMEOUT	3000


#define CTL_DEBUG

/* Define these values to match your devices */
#define USB_VENDOR_ID	0x1cbe
#define USB_PRODUCT_ID	0x0003
#ifdef CTL_DEBUG
/* table of devices that work with this driver */
static struct usb_device_id ctl_table [] = {
	{ USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
	{ }					/* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, ctl_table);

/* Get a minor range for your devices from the usb maintainer */
/* comment: I believe that I picked that value arbitrarily in case the USB subsystem
 * uses dynamic minors when plugging in a device. */
#define USB_CTL_MINOR_BASE	96 //192

/* Structure to hold all of our device specific stuff */
struct usb_ctl {
	struct mutex	sem;
	/* Structure to hold all of our device specific stuff */
	struct usb_device *	udev;			/* the usb device for this device */
	struct usb_interface *	interface;		/* the interface for this device */
	unsigned char *		bulk_in_buffer;		/* the buffer to receive data */
	size_t			bulk_in_size;		/* the size of the receive buffer */
	__u8			bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
	__u8			bulk_out_endpointAddr;	/* the address of the bulk out endpoint */
	struct kref		kref;
};
#define to_ctl_dev(d) container_of(d, struct usb_ctl, kref)

static struct usb_driver ctl_driver;

// Line 57
static void ctl_delete(struct kref *kref)
{
	struct usb_ctl *dev = to_ctl_dev(kref);

	usb_put_dev(dev->udev);
	kfree (dev->bulk_in_buffer);
	kfree (dev);
}

// Line 66
/* Open method */
static int ctl_open(struct inode *inode, struct file *file)
{
	struct usb_ctl *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);

	interface = usb_find_interface(&ctl_driver, subminor);
	if (!interface) {//err
		err ("%s - error, can't find device for minor %d",
		     __FUNCTION__, subminor);
		retval = -ENODEV;
		goto exit;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		retval = -ENODEV;
		goto exit;
	}

	/* increment our usage count for the device */
	kref_get(&dev->kref);

	/* save our object in the file's private structure */
	file->private_data = dev;

exit:
	return retval;
}

// Line 99
static int ctl_release(struct inode *inode, struct file *file)
{
	struct usb_ctl *dev;

	dev = (struct usb_ctl *)file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* decrement the count on our device */
	kref_put(&dev->kref, ctl_delete);
	return 0;
}


// Line 112
static ssize_t ctl_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
	struct usb_ctl *dev;
	int retval = 0, bytes_read;

	dev = (struct usb_ctl *)file->private_data;

	/* do a blocking bulk read to get data from the device */
	retval = usb_bulk_msg(dev->udev,
			      usb_rcvbulkpipe(dev->udev, dev->bulk_in_endpointAddr),
			      dev->bulk_in_buffer,
			      min(dev->bulk_in_size, count),
			      &bytes_read, READ_TIIMEOUT); // HZ*10

	/* if the read was successful, copy the data to userspace */
	if (!retval) {
		if (copy_to_user(buffer, dev->bulk_in_buffer, count))
			retval = -EFAULT;
		else
			retval = bytes_read;
	}

	return retval;
}


// Line 137
//static void ctl_write_bulk_callback(struct urb *urb, struct pt_regs *regs)
static void ctl_write_bulk_callback (struct urb *urb)
{
	/* sync/async unlink faults aren't errors */
	if (urb->status &&
	    !(urb->status == -ENOENT ||
	      urb->status == -ECONNRESET ||
	      urb->status == -ESHUTDOWN)) {
		dbg("%s - nonzero write bulk status received: %d",
		    __FUNCTION__, urb->status);
	}

	/* Free the transfer buffer. usb_buffer_free() is the
	release-counterpart of usb_buffer_alloc() called
	from tele_write() */
	/* free up our allocated buffer
	 * extern void usb_free_urb(struct urb *urb);
	 * */
//	usb_buffer_free(urb->dev, urb->transfer_buffer_length,
//			urb->transfer_buffer, urb->transfer_dma);
	usb_free_coherent(urb->dev, urb->transfer_buffer_length,
			urb->transfer_buffer, urb->transfer_dma);
}

// Line 153
static ssize_t ctl_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos)
{
	struct usb_ctl *dev;
	int retval = 0;
	struct urb *urb = NULL;
	char *buf = NULL;

	dev = (struct usb_ctl *)file->private_data;

	/* verify that we actually have some data to write */
	if (count == 0)
		goto exit;

	/* create a urb, and a buffer for it, and copy the data to the urb */
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		retval = -ENOMEM;
		goto error;
	}

//	buf = usb_buffer_alloc(dev->udev, count, GFP_KERNEL, &urb->transfer_dma);
	buf = usb_alloc_coherent(dev->udev, count, GFP_KERNEL, &urb->transfer_dma);
	if (!buf) {
		retval = -ENOMEM;
		goto error;
	}
	if (copy_from_user(buf, user_buffer, count)) {
		retval = -EFAULT;
		goto error;
	}

	/* initialize the urb properly */
	usb_fill_bulk_urb(urb, dev->udev,
			  usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),
			  buf, count, ctl_write_bulk_callback, dev);
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	/* send the data out the bulk port */
	retval = usb_submit_urb(urb, GFP_KERNEL);
	if (retval) {
		err("%s - failed submitting write urb, error %d", __FUNCTION__, retval);
		goto error;
	}

	/* release our reference to this urb, the USB core will eventually free it entirely */
	usb_free_urb(urb);

exit:
	return count;

error:
//	usb_buffer_free(dev->udev, count, buf, urb->transfer_dma);
	usb_free_coherent(dev->udev, count, buf, urb->transfer_dma);
	usb_free_urb(urb);
	kfree(buf);
	return retval;
}


// Line 209
/* Conventional char driver entry points.
See Chapter 5, "Character Drivers." */
static struct file_operations ctl_fops = {
	.owner =	THIS_MODULE,
	.read =		ctl_read,
	.write =	ctl_write,
	.open =		ctl_open,
	.release =	ctl_release,
};


// Line 221
/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with devfs and the driver core
 * struct usb_class_driver {
	char *name;
	char *(*devnode)(struct device *dev, umode_t *mode);
	const struct file_operations *fops;
	int minor_base;
};
 */
/*
static struct usb_class_driver ctl_class = {
	.name = "usbctl%d",
	.fops = &ctl_fops,
	.mode = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH,
	.minor_base = USB_CTL_MINOR_BASE,
};
*/
static struct usb_class_driver ctl_class = {
		.name = "usbctl%d",
		.fops = &ctl_fops,
		.minor_base = USB_CTL_MINOR_BASE,
};


// Line 228
static int ctl_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_ctl *dev = NULL;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	size_t buffer_size;
	int i;
	int retval = -ENOMEM;

	/* allocate memory for our device state and initialize it */
	dev = kmalloc(sizeof(struct usb_ctl), GFP_KERNEL);
	if (dev == NULL) {
		err("Out of memory");
		goto error;
	}
	memset(dev, 0x00, sizeof (*dev));
	kref_init(&dev->kref);
// added 25.07.2017
	mutex_init(&dev->sem);

	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	/* set up the endpoint information */
	/* use only the first bulk-in and bulk-out endpoints */
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (!dev->bulk_in_endpointAddr &&
		    (endpoint->bEndpointAddress & USB_DIR_IN) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) {
			/* we found a bulk in endpoint */
			buffer_size = endpoint->wMaxPacketSize;
			dev->bulk_in_size = buffer_size;
			dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
			dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
			if (!dev->bulk_in_buffer) {
				err("Could not allocate bulk_in_buffer");
				goto error;
			}
		}

		if (!dev->bulk_out_endpointAddr &&
		    !(endpoint->bEndpointAddress & USB_DIR_IN) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) {
			/* we found a bulk out endpoint */
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
		}
	}
	if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) {
		err("Could not find both bulk-in and bulk-out endpoints");
		goto error;
	}

	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);

	/* we can register the device now, as it is ready */
	retval = usb_register_dev(interface, &ctl_class);
	if (retval) {
		/* something prevented us from registering this driver */
		err("Not able to get a minor for this device.");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	/* let the user know what node this device is now attached to */
	info("USB CNC Control device now attached to usbctl%d", interface->minor);
	return 0;

error:
	if (dev)
		kref_put(&dev->kref, ctl_delete);
	return retval;
}

// Line 305
static void ctl_disconnect(struct usb_interface *interface)
{
	struct usb_ctl *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	/* prevent skel_open() from racing skel_disconnect() */
//	lock_kernel();
	mutex_lock(&dev->sem);

	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &ctl_class);

//	unlock_kernel();
	mutex_unlock(&dev->sem);

	/* decrement our usage count */
	kref_put(&dev->kref, ctl_delete);

	info("USB CTL #%d now disconnected", minor);
}

//Line 327
static struct usb_driver ctl_driver = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
		/* Подключение файлов для ядра 2.6.9 */
		.owner = THIS_MODULE,
#endif
	.name = "CNCcontrol",
	.id_table = ctl_table,
	.probe = ctl_probe,
	.disconnect = ctl_disconnect,
};


// Line 335
static int __init usb_ctl_init(void)
{
	int result;

	/* register this driver with the USB subsystem */
	result = usb_register(&ctl_driver);
	if (result)
		err("usb_register failed. Error number %d", result);

	return result;
}

// Line 347
static void __exit usb_ctl_exit(void)
{
	/* deregister this driver with the USB subsystem */
	usb_deregister(&ctl_driver);
}

module_init (usb_ctl_init);
module_exit (usb_ctl_exit);


#else
//=====================
static int hello_init(void)
{
printk(KERN_ALERT "Hello, world\n");
return 0;
}
static void hello_exit(void)
{
printk(KERN_ALERT "Goodbye, cruel world\n");
}
module_init(hello_init);
module_exit(hello_exit);
#endif




