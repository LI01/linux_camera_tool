/****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly uses v4l2 system 
  call to obtain camera information on: exposure, gain, pan, tilt, zoom etc.
  Most bayer camera won't support PTZ control, some may have the ability of 
  enable auto exposure some may not. Please check with Leopard for detailed
  driver support.

  Author: Danyu L
  Last edit: 2019/07
*****************************************************************************/
#pragma once
/*****************************************************************************
**                      	Global data 
*****************************************************************************/
#define UNDEFINED_MAX_EXPOSURE_LINE ((1<<16)-1)
/****************************************************************************
**							 Function declaration
*****************************************************************************/
/** --- helper function ---*/
void error_handle_cam_ctrl();

void uvc_get_control(int fd, unsigned int id);
void uvc_set_control(int fd, unsigned int id, int value);

long uvc_query_ctrl_max(int fd, int id);
/**--------------------------------------------------------------------- */
void set_gain_auto(int fd, int auto_gain);
void get_gain_auto(int fd);

void set_gain(int fd, int analog_gain);
void get_gain(int fd);
int query_gain_max(int fd);

void set_exposure_absolute(int fd, int exposure_absolute);
void get_exposure_absolute(int fd);

void set_exposure_auto(int fd, int exposure_auto);
void get_exposure_auto(int fd);
int query_exposure_absolute_max(int fd);

void set_zoom_absolute(int fd, int zoom_absolute);
void get_zoom_absolute(int fd);

void set_pan_absolute(int fd, int pan_absolute);
void get_pan_absolute(int fd);

void set_tilt_absolute(int fd, int tilt_absolute);
void get_tilt_absolute(int fd);

void set_focus_absolute(int fd, int focus_absolute);
void get_focus_absolute(int fd);

/**--------------------------------------------------------------------- */
int get_current_height(int fd);
void set_frame_rate(int fd, int fps);
int get_frame_rate(int fd);

void usage( const char *argv0);