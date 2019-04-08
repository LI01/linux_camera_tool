#pragma once
#define MIN_EXPOSURE 1
#define MAX_EXPOSURE 65535

/*
 * handle the error for camera control
 * args:
 * 		none
 * returns:
 * 		none
 */
void error_handle_cam_ctrl();

/*
 * helper function for camera control getter
 * args: 
 * 		int fd - put buffers in
 * 		control id
 */
void uvc_get_control(int fd, unsigned int id);

/*
 * helper function for camera control setter
 * args: 
 * 		int fd - put buffers in
 * 		control id
 * 		value - value you want to set into
 */
void uvc_set_control(int fd, unsigned int id, int value);

void set_frame_rate(int fd, int fps);
int get_frame_rate(int fd);


void set_auto_gain (int fd, int auto_gain);
void get_auto_gain (int fd);
void set_gain(int fd, int analog_gain);
void get_gain(int fd);
void set_exposure_absolute(int fd, int exposure_absolute);
void get_exposure_absolute(int fd);
void set_exposure_auto(int fd, int exposure_auto);
void get_exposure_auto(int fd);
void set_zoom_absolute(int fd, int zoom_absolute);
void get_zoom_absolute(int fd);
void set_pan_absolute(int fd, int pan_absolute);
void get_pan_absolute(int fd);
void set_tilt_absolute(int fd, int tilt_absolute);
void get_tilt_absolute(int fd);
void set_focus_absolute(int fd, int focus_absolute);
void get_focus_absolute(int fd);
