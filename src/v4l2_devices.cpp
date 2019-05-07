/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly uses udev to put 
  Leopard camera on the right /dev/video# then open the camera later
  For how to use udev: http://www.signal11.us/oss/udev/

  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/

#include "../includes/shortcuts.h"
#include "../includes/v4l2_devices.h"

char manufacturer[20];
char product[20];
char serial[64];

char *get_manufacturer_name()
{
    return manufacturer;
}
char *get_product()
{
    return product;
}
char *get_serial()
{
    return serial;
}
char *enum_v4l2_device(char *dev_name)
{

    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices;
    struct udev_list_entry *dev_list_entry;
    struct udev_device *dev;

    printf("********************Udev Device Start************************\n");
    /** Create the udev object */
    udev = udev_new();
    if (!udev)
    {
        printf("Can't create udev\n");
        exit(1);
    }

    /** Create a list of the devices in the 'v4l2' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    /** For each item enumerated, print out its information.
	 * udev_list_entry_foreach is a macro which expands to
	 * a loop. The loop will be executed for each member in
	 * devices, setting dev_list_entry to a list entry
	 * which contains the device's path in /sys. */
    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;

        /** Get the filename of the /sys entry for the device
		 * and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        char dev_name_tmp[64];

        /** usb_device_get_devnode() returns the path to the device node
		 * itself in /dev. */
        printf("Device Node Path: %s\n", udev_device_get_devnode(dev));
        strcpy(dev_name_tmp, udev_device_get_devnode(dev));
        printf("%s\n", dev_name_tmp);

        /** The device pointed to by dev contains information about
		 * the hidraw device. In order to get information about the
		 * USB device, get the parent device with the
		 * subsystem/devtype pair of "usb"/"usb_device". This will
		 * be several levels up the tree, but the function will find
		 * it.*/
        dev = udev_device_get_parent_with_subsystem_devtype(
            dev,
            "usb",
            "usb_device");
        if (!dev)
        {
            printf("Unable to find parent usb device.");
            exit(1);
        }

        /**
         * 2a0b is leopard manufacurer id, use udev to update video device id
         */
        if ((strcmp("2a0b", udev_device_get_sysattr_value(dev, "idVendor"))) == 0)
        {
            strcpy(dev_name, dev_name_tmp);
            /** From here, we can call get_sysattr_value() for each file
		     * in the device's /sys entry. The strings passed into these
		     * functions (idProduct, idVendor, serial, etc.) correspond
		     * directly to the files in the directory which represents
		     * the USB device. Note that USB strings are Unicode, UCS2
		     * encoded, but the strings returned from
		     * udev_device_get_sysattr_value() are UTF-8 encoded. */
            printf("  VID/PID: %s %s\n",
                   udev_device_get_sysattr_value(dev, "idVendor"),
                   udev_device_get_sysattr_value(dev, "idProduct"));

            strcpy(manufacturer, udev_device_get_sysattr_value(dev, "manufacturer"));
            strcpy(product, udev_device_get_sysattr_value(dev, "product"));
            strcpy(serial, udev_device_get_sysattr_value(dev, "serial"));
            printf("  %s\n  %s\n",
                   manufacturer,
                   product);
            /** Add serial number in usb descriptor will need to request
               a firmware updates, values will get from FX3 and sensor fuse id */
            // printf("  serial: %s\n",
            //        serial);

            //TODO:delete this for other computer?
            return dev_name;
        }

        udev_device_unref(dev);
    }
    /** Free the enumerator object */
    udev_enumerate_unref(enumerate);

    udev_unref(udev);
    return dev_name;
}