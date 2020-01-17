/*****************************************************************************
 * This file is part of the Linux Camera Tool 
 * Copyright (c) 2020 Leopard Imaging Inc.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *                                                                            
 *  This is the sample code for Leopard USB3.0 camera, mainly uses udev to    
 *  put Leopard camera on the right /dev/video# then open the camera later.   
 *  If couldn't find the Leopard USB Camera device, software will quit.       
 *                                                                            
 *  For OV580-STEREO camera, many of them doesn't have the capabilities of    
 *  changing exposure and not any firmware revision info available, will only 
 *  display the basic device information in the software.                     
 *                                                                            
 *  For how to use udev: http://www.signal11.us/oss/udev/                     
 *                                                                            
 *  Author: Danyu L                                                           
 *  Last edit: 2019/09                                                        
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/v4l2_devices.h"
/*****************************************************************************
**                      	Global data 
*****************************************************************************/   
static dev_info* info;
/******************************************************************************
**                           Function definition
*****************************************************************************/
/**
 * free the global variables on heap
 */
void free_device_vars()
{
    free(info);
}
/**
 * Getter for manufacurer name: "Leopard Imaging"
 * These USB descriptor are read-only 
 */
char *get_manufacturer_name()
{
    return info->manufacturer;
}

/**
 * Getter for product name 
 * usually sensor_name + serdes_name
 */
char *get_product()
{
    return info->product;
}

/**
 * Getter for camera serial number
 * Mostly camera driver don't support 
 * (only put a default serial number string in the driver) 
 * If you need to add sensor fuseId into driver, request for a firmware update
 */
// char *get_serial()
// {
//     return info->serial;
// }
/**
 * Getter for device number /dev/videoX
 */ 
char *get_dev_name()
{
    return info->dev_name;
}

/**
 * a wrapper function for get vendor id 
 */
int get_dev_vid(
    struct udev_device *dev)
{
    /// convert const char* to hex int
    int vid = strtol(
        udev_device_get_sysattr_value(dev, "idVendor"), 
        NULL, 
        16);
    return vid;

}
/**
 * a wrapper function for get product id 
 */
int get_dev_pid(
    struct udev_device *dev)
{
    /// convert const char* to hex int
    int pid = strtol(
        udev_device_get_sysattr_value(dev, "idProduct"), 
        NULL, 
        16);
    return pid;
}
/**
 * if it is OV580 stereo, don't read hw, fw rev etc
 */
int is_ov580_stereo()
{
    return info->is_ov580_st;
}
/**
 * 05a9:0581 is LI-USB Camera-OV9282-OV580, Camera-OV7251-OV580
 * 05a9:0583 is LI-USB Camera-OG01A1B-OV580
 * 2b03:0580 is LI-USB Camera-OV4689-OV580
 */
int is_ov580_stereo(struct udev_device *dev)
{
   
    info->is_ov580_st =
        (OV580_ST_VID == get_dev_vid(dev)) 
       || (OV580_OV4689_VID == get_dev_vid(dev) 
       && (OV580_OV4689_PID == get_dev_pid(dev)));
    return info->is_ov580_st;
}
/**
 * 2a0b is leopard manufacurer id
 */
int is_fx3_usb3(
    struct udev_device *dev)
{
    int is_fx3_usb3 = 
      (FX3_USB3_VID == get_dev_vid(dev))
    || (UNKNOWN_LI_VID1 == get_dev_vid(dev))
    || (UNKNOWN_LI_VID2 == get_dev_vid(dev))
    || (UNKNOWN_LI_VID3 == get_dev_vid(dev))        
    || (UNKNOWN_LI_VID4 == get_dev_vid(dev));
    
    return is_fx3_usb3;
}


/**
 * Fill the device information with given udev_device
 */
void fill_dev_info(
    struct udev_device *dev, 
    const char* dev_name)
{
    strcpy(info->dev_name, dev_name);
    /** 
     * From here, we can call get_sysattr_value() for each file
	 * in the device's /sys entry. The strings passed into these
	 * functions (idProduct, idVendor, serial, etc.) correspond
	 * directly to the files in the directory which represents
	 * the USB device. Note that USB strings are Unicode, UCS2
	 * encoded, but the strings returned from
	 * udev_device_get_sysattr_value() are UTF-8 encoded. 
     */
    printf("  VID/PID: %04x %04x\n",
           get_dev_vid(dev),
           get_dev_pid(dev));
    /// use strcpy will cause buffer overflow...
    memcpy(
        info->manufacturer, 
        udev_device_get_sysattr_value(dev, "manufacturer"), 
        20);
    memcpy(
        info->product, 
        udev_device_get_sysattr_value(dev, "product"),
        20);
    // memcpy(
    //     info->serial, 
    //     udev_device_get_sysattr_value(dev, "serial"), 
    //     64);
    printf("  %s\n  %s\n",
           info->manufacturer,
           info->product);
}
/**
 * Overwrite the device information if use leopard -d X option
 */
void update_dev_info(
    const char* dev_name)
{
    strcpy(info->dev_name, dev_name);
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices;
    struct udev_list_entry *dev_list_entry;
    struct udev_device *dev;
    int is_li_dev = 1;
    /** Create the udev object */
    udev = udev_new();
    if (!udev)
    {
        printf("Can't create udev\n");
        free_device_vars();
        exit(1);
    }
    /** Create a list of the devices in the 'v4l2' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "video4linux");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path;

        /** 
         * Get the filename of the /sys entry for the device
		 * and create a udev_device object (dev) representing it 
         */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        if (strcmp(dev_name, udev_device_get_devnode(dev)) == 0) {
            dev = udev_device_get_parent_with_subsystem_devtype(
                dev,
                "usb",
                "usb_device");
            if (!dev)
            {
                printf("Unable to find parent usb device.");                
                free_device_vars();
                exit(2);
            }
            int is_li_fx3_usb3 = is_fx3_usb3(dev);
            int is_li_ov580_stereo = is_ov580_stereo(dev);
            if (is_li_fx3_usb3 || is_li_ov580_stereo)
            { 
                is_li_dev = 1;
                fill_dev_info(dev, dev_name);
                udev_device_unref(dev);
                break;
            }
            else 
            {
                is_li_dev = 0;
                udev_device_unref(dev);
                continue;
            }
        }
    }
    /** Free the enumerator object */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    /** If it is not Leopard device, just exit */
    if (!is_li_dev) 
    {
        printf("Couldn't find Leopard USB3 Camera\r\n");
        free_device_vars();
        exit(0);
    }
}

/**
 * enumerate v4l2 device
 * input device set to default /dev/video0
 * will find any leopard image USB3 device and return the first found for fd
 * two video devices for one camera, later one is dummy...
 */
void enum_v4l2_device(char *dev_name)
{
    int is_li_dev = 1;
    /** put these variables on heap so it doesn't get buffer overflow */
    info = (dev_info*)malloc(sizeof(dev_info));
    //CLEAR(info);
    printf("********************Udev Device Start************************\n");
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices;
    struct udev_list_entry *dev_list_entry;
    struct udev_device *dev;
    /** Create the udev object */
    udev = udev_new();
    if (!udev)
    {
        printf("Can't create udev\n");
        free_device_vars();
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

        /** 
         * usb_device_get_devnode() returns the path to the device node
		 * itself in /dev. 
         */
        strcpy(dev_name, udev_device_get_devnode(dev));
        printf("Device Node Path: %s\n", dev_name);

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
            printf("Unable to find parent usb device.\r\n");
            free_device_vars();
            exit(2);
        }
        int is_li_fx3_usb3 = is_fx3_usb3(dev);
        int is_li_ov580_stereo = is_ov580_stereo(dev);
        
        /// just break out of for loop if find a leopard USB camera
        if (is_li_fx3_usb3 || is_li_ov580_stereo)
        { 
            is_li_dev = 1;
            if (is_li_fx3_usb3)
                printf("Find leopard USB3 camera at %s\n", dev_name);
            else if (is_li_ov580_stereo)
                printf("Find Omnivision Stereo USB3 camera at %s\n", dev_name);
            fill_dev_info(dev, dev_name);
            udev_device_unref(dev);
            break;
        }
        /// continue finding match for leopard USB camera in later for loop
        else 
        {
            is_li_dev = 0;
            udev_device_unref(dev);
            continue;
        }
    }
    /** Free the enumerator object */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    /** If it is not Leopard device, just exit */
    if (!is_li_dev) 
    {
        printf("Couldn't find Leopard USB3 Camera\r\n");
        free_device_vars();
        exit(0);
    }
}

