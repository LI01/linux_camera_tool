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

#pragma once
#include <libudev.h>

typedef struct 
{
  char manufacturer[20];
  char product[20];
  char dev_name[20];
  // char serial[64];
  int is_ov580_st;
}dev_info;

void free_device_vars();

char* get_manufacturer_name(); 
char* get_product();
//char *get_serial();
char *get_dev_name();
int get_dev_vid(
    struct udev_device *dev);
int get_dev_pid(
    struct udev_device *dev);

int is_ov580_stereo();
int is_ov580_stereo(
  struct udev_device *dev);
int is_fx3_usb3(
  struct udev_device *dev);

void fill_dev_info(
    struct udev_device *dev, 
    const char* dev_name);
void update_dev_info(
  const char* dev_name);
void enum_v4l2_device(
  char *dev_name); 