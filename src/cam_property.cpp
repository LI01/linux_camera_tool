/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly uses v4l2 system 
  call to obtain camera information on: exposure, gain, pan, tilt, zoom etc.
  Most bayer camera won't support PTZ control, some may have the ability of 
  enable auto exposure some may not. Please check with Leopard for detailed
  driver support.

  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/cam_property.h"
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
    case ERANGE:
        err = "Struct v4l2_control value is out of bounds";
        break;
    case EINVAL:
        err = "Invalid request code";
        break;
    case EBUSY:
        err = "Busy, another application took over the control";
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
void uvc_get_control(int fd, unsigned int id)
{
    struct v4l2_control ctrl;
    ctrl.id = id;

    if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0)
        error_handle_cam_ctrl();

    printf("Control 0x%08x value %u\n", id, ctrl.value);
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
/**--------------------------------------------------------------------------- */
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
    printf("Get Current Frame Rate = %d\n", param.parm.capture.timeperframe.denominator);
    //printf("Get frame rate num= %d\n", param.parm.capture.timeperframe.numerator);
    return param.parm.capture.timeperframe.denominator;
}

/**--------------------------------------------------------------------------- */
void set_gain_auto (int fd, int auto_gain)
{
    uvc_set_control(fd, V4L2_CID_AUTOGAIN, auto_gain);
}

void get_gain_auto (int fd)
{
    uvc_get_control(fd, V4L2_CID_AUTOGAIN);
}

void set_gain(int fd, int analog_gain)
{
    uvc_set_control(fd, V4L2_CID_GAIN, analog_gain);
}

void get_gain(int fd)
{
    uvc_get_control(fd, V4L2_CID_GAIN);
}

void set_exposure_absolute(int fd, int exposure_absolute)
{
    if(exposure_absolute < MIN_EXPOSURE || exposure_absolute > MAX_EXPOSURE)
        return;

    uvc_set_control(fd, V4L2_CID_EXPOSURE_ABSOLUTE, exposure_absolute);
}
void get_exposure_absolute(int fd)
{
    uvc_get_control(fd, V4L2_CID_EXPOSURE_ABSOLUTE);
}

// from below, it might not support by every camera

/**  
 *  Enables automatic adjustments of the exposure time and/or iris aperture. 
 *  The effect of manual changes of the exposure time or iris aperture while 
 *  these features are enabled is undefined, drivers should ignore such requests. 
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

void usage( const char *argv0)
{
	printf("Usage: %s [options]\n", argv0);
	printf("Supported options:\n");
	printf("-n, --nbufs n		Set the number of video buffers\n");
	printf("-s, --size WxH		Set the frame size\n");
	printf("-t, --time-per-frame	Set the time per frame (eg. 25 = 25 fps)\n");
}