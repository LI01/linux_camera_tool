/****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly uses udev to put 
  Leopard camera on the right /dev/video# then open the camera later
  For how to use udev: http://www.signal11.us/oss/udev/

  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/

#pragma once
#include <libudev.h>
char* get_manufacturer_name(); 
char* get_product();
char *get_serial();
char *enum_v4l2_device(char *dev_name); 