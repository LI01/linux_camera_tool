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
 *  This is the sample code for Leopard USB3.0 camera, mainly uses v4l2        
 *  system call to obtain camera information on: exposure, gain, pan, tilt,   
 *  zoom etc. Most bayer camera won't support PTZ control, some may have       
 *  the ability of enable auto exposure some may not. Please check with       
 *  Leopard for detailed driver support.                                                           *
 *                                                                            
 *  Author: Danyu L                                                           
 *  Last edit: 2019/08                                                        
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/cam_property.h"

extern int *cur_exposure; /// use shared memory for multiprocess for soft ae
extern int *cur_gain;   /// use shared memory for multiprocess for soft ae


/**
 * handle the error for camera control
 * args:
 * 		none
 * returns:
 * 		none
 */
void error_handle_cam_ctrl()
{
    int res = errno;

    const char *err;
    switch (res)
    {
    case EAGAIN:
        err = "The device is in ther state where it can't perform";    
        break;
    case EBADF:
        err = "File descriptor is not a valid";
        break;
    case ERANGE:
        err = "Struct v4l2_control value is out of bounds";
        break;
    case EINVAL:
        err = "Invalid request code";
        break;
    case EBUSY:
        err = "Busy, another application took over the control";
        break;
    case EFAULT:
        err = "Failed on copying data from/to userspace";
        break;
    case ENODEV:
        err = "Device not found or was removed";
        break;
    case ENOMEM:
        err = "Not enough memory to handle the desired operation";
        break;
    case ENOTTY:
        err = "Required functionality is not available";
        break;
    case ENOSPC:
        err = "Request overcommit USB bandwidth reserved for periodic transfers";
        break;
    case EPERM:
        err = "Permission denied";
        break;  
    case EIO:
        err = "I/O Error";
        break;
    case ENXIO:
        err = "No device corresponding to this device special file exists";
        break;
    case EACCES:
        err = "Set a read-only control/get a write-only control";
        break;
    default:
        err = strerror(res);
        break;
    }

    printf("failed %s. (System code: %d) \n", err, res);

    return;
}

/**
 * helper function for camera control getter
 * args: 
 * 		int fd - put buffers in
 * 		control id
 */
long uvc_get_control(int fd, unsigned int id)
{
    struct v4l2_control ctrl;
    ctrl.id = id;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0)
        error_handle_cam_ctrl();

    printf("Control 0x%08x value %u\n", id, ctrl.value);
    return ctrl.value;
}

/**
 * helper function for camera control setter
 * args: 
 * 		int fd - put buffers in
 * 		control id
 * 		value - value you want to set into
 */
void uvc_set_control(int fd, unsigned int id, int value)
{
    struct v4l2_control ctrl;
    CLEAR(ctrl);
    ctrl.id = id;
    ctrl.value = value;

    if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0)
        error_handle_cam_ctrl();

    printf("Control 0x%08x set to %u, is %u\n", id, value,
           ctrl.value);
}
/**
 * To query the attributes of a control applications
 * __s32 minimum
 * __s32 maximum
 * __s32 step
 * __s32 default_value
 * args:
 *      int fd - put buffers in
 * 		control id
 */
long uvc_query_ctrl_max(int fd, int id)
{
    struct v4l2_queryctrl control;
    CLEAR(control);
    control.id = id;
    if (ioctl(fd, VIDIOC_QUERYCTRL, &control) < 0)
        error_handle_cam_ctrl();
    long value;
    value = control.maximum;
    return value;
}

/**--------------------------------------------------------------------- */
void set_gain_auto (int fd, int auto_gain)
{
    uvc_set_control(fd, V4L2_CID_AUTOGAIN, auto_gain);
}

void get_gain_auto (int fd)
{
    uvc_get_control(fd, V4L2_CID_AUTOGAIN);
}
/**
 * add this check to minimize the occurrence of split screen
 */
void set_gain(int fd, int analog_gain)
{
    if (analog_gain != *cur_gain) 
    {   
        *cur_gain = analog_gain;
        uvc_set_control(fd, V4L2_CID_GAIN, analog_gain);
    }
}

int get_gain(int fd)
{
    *cur_gain = (int)uvc_get_control(fd, V4L2_CID_GAIN);
    return *cur_gain;
}

int query_gain_max(int fd)
{
    return (int)uvc_query_ctrl_max(fd, V4L2_CID_GAIN);
}
/**
 * add this check to minimize the occurrence of split screen
 */
void set_exposure_absolute(int fd, int exposure_absolute)
{
    if (exposure_absolute != *cur_exposure) 
    {   *cur_exposure = exposure_absolute;
        uvc_set_control(fd, V4L2_CID_EXPOSURE_ABSOLUTE, exposure_absolute);
    }
}
int get_exposure_absolute(int fd)
{
    *cur_exposure = (int)uvc_get_control(fd, V4L2_CID_EXPOSURE_ABSOLUTE);
    return *cur_exposure;
}

int query_exposure_absolute_max(int fd)
{
    return (int)uvc_query_ctrl_max(fd, V4L2_CID_EXPOSURE_ABSOLUTE);
}

// from below, it might not support by every camera

/**  
 *  Enables automatic adjustments of the exposure time and/or iris aperture. 
 *  The effect of manual changes of the exposure time or iris aperture while 
 *  these features are enabled is undefined, drivers should ignore such 
 *  requests. 
 *  
 *  Possible values are:
 *   V4L2_EXPOSURE_AUTO 	Automatic exposure time, automatic iris aperture.
 *   V4L2_EXPOSURE_MANUAL 	Manual exposure time, manual iris.
 *   V4L2_EXPOSURE_SHUTTER_PRIORITY 	Manual exposure time, auto iris.
 *   V4L2_EXPOSURE_APERTURE_PRIORITY 	Auto exposure time, manual iris.
 *
 */
void set_exposure_auto(int fd, int exposure_auto)
{
    uvc_set_control(fd, V4L2_CID_EXPOSURE_AUTO, exposure_auto);
}
void get_exposure_auto(int fd)
{
    uvc_get_control(fd, V4L2_CID_EXPOSURE_AUTO);
}

void set_zoom_absolute(int fd, int zoom_absolute)
{
    uvc_set_control(fd, V4L2_CID_ZOOM_ABSOLUTE, zoom_absolute);
}
void get_zoom_absolute(int fd)
{
    uvc_get_control(fd, V4L2_CID_ZOOM_ABSOLUTE);
}

void set_pan_absolute(int fd, int pan_absolute)
{
    uvc_set_control(fd, V4L2_CID_PAN_ABSOLUTE, pan_absolute);
}
void get_pan_absolute(int fd)
{
    uvc_get_control(fd, V4L2_CID_PAN_ABSOLUTE);
}

void set_tilt_absolute(int fd, int tilt_absolute)
{
    uvc_set_control(fd, V4L2_CID_TILT_ABSOLUTE, tilt_absolute);
}
void get_tilt_absolute(int fd)
{
    uvc_get_control(fd, V4L2_CID_TILT_ABSOLUTE);
}

void set_focus_absolute(int fd, int focus_absolute)
{
    uvc_set_control(fd, V4L2_CID_FOCUS_ABSOLUTE, focus_absolute);
}
void get_focus_absolute(int fd)
{
    uvc_get_control(fd, V4L2_CID_FOCUS_ABSOLUTE);
}
/**--------------------------------------------------------------------- */
void set_frame_rate(int fd, int fps)
{
    struct v4l2_streamparm param;
    CLEAR(param);
    param.type = (v4l2_buf_type) V4L2_CAP_VIDEO_CAPTURE;
    param.parm.capture.timeperframe.numerator = 1;
    param.parm.capture.timeperframe.denominator = fps;
    if(ioctl(fd, VIDIOC_S_PARM, &param) < 0)
        error_handle_cam_ctrl();
    printf("Set Frame Rate = %d\n", fps);
}

int get_frame_rate(int fd)
{
    struct v4l2_streamparm param;
    CLEAR(param);
    param.type =  (v4l2_buf_type) V4L2_CAP_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_G_PARM, &param) < 0)
        error_handle_cam_ctrl();
    printf("Get Current Frame Rate = %d\n", 
        param.parm.capture.timeperframe.denominator);
    //printf("Get frame rate num= %d\n", 
    //    param.parm.capture.timeperframe.numerator);
    return param.parm.capture.timeperframe.denominator;
}
/**------------------------------------------------------------------------ */
/** Get current resolution height
 *  args:
 *      int fd - file descriptor for camera device
 * 		control id
 */
int get_current_height(int fd)
{
    struct v4l2_format fmt;
    int ret;
    CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(fd, VIDIOC_G_FMT, &fmt);
	if (ret < 0)
	{
		printf("Unable to get format: %s (%d).\n", strerror(errno),
			   errno);
		return 0;
	}
    return fmt.fmt.pix.height;
}

/** 
 * retrive device's capabilities
 * 
 * args: 
 * 		int fd - file descriptor for camera device
 * returns: 
 * 		error value
 */
int check_dev_cap(int fd)
{
	printf("********************Device Infomation************************\n");
    struct v4l2_capability cap;
	CLEAR(cap);
	int ret;
	ret = (ioctl(fd, VIDIOC_QUERYCAP, &cap));
	if (ret < 0)
	{
		printf("VIDIOC_QUERYCAP error:%s\n", strerror(errno));
		return -1;
	}

	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0)
	{
		printf("Error opening device: video capture not supported.\n");
		return -1;
	}
	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		printf("Does not support streaming\n");

		return -1;
	}
	if (!(cap.capabilities & V4L2_CAP_READWRITE))
	{
		printf("Does not support read, try with mmap\n");
		return -1;
	}
	return 0;
}

void usage( const char *argv0)
{
	printf("Usage: %s [options]\n", argv0);
	printf("Supported options:\n");
	printf("-n, --nbufs n		Set the number of video buffers\n");
	printf("-s, --size WxH		Set the frame size\n");
	printf("-t, --time-per-frame	Set the time per frame (eg. 25 = 25 fps)\n");
    printf("-d, --device n          Set device number to /dev/videon\n");
    printf("-h, --help			    Show this help screen\n");
}