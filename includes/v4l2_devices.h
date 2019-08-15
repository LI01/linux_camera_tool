/****************************************************************************
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc., 
  
  This is the sample code for Leopard USB3.0 camera, mainly uses udev to put 
  Leopard camera on the right /dev/video# then open the camera later
  For how to use udev: http://www.signal11.us/oss/udev/

  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/

#pragma once
#include <libudev.h>

void free_device_vars();
char* get_manufacturer_name(); 
char* get_product();
char *get_serial();
int is_leopard_usb3(struct udev_device *dev);
int is_ov580_stereo();
int is_ov580_stereo(struct udev_device *dev);

char *enum_v4l2_device(char *dev_name); 