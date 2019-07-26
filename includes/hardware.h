/*****************************************************************************
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
  
  This is the sample code for Leopard USB3.0 camera, hardware.h stores macros
  for spefic camera tool build. Please refer to the explanation below for 
  customize build. 

  Author: Danyu L
  Last edit: 2019/07
*****************************************************************************/
#pragma once
/**
 * ----------------------macros for your build--------------------------------
 * 1. uncomment HAVE_OPENCV_CUDA_SUPPORT if you have OpenCV CUDA support and 
 * want to accelerate RAW image processing speed
 * 2. uncomment DUAL_CAM if you are using a stereo camera. It will display 
 * left and right image in two different windows, e.g. AR0144_DUAL, AR0231_DUAL
 * 3. uncomment USING_CLAHE if you want to use CLAHE for adjusting brightness
 * and contrast instead of basic historgram ax+b
 * 4. uncomment IMG_FLIP_VERTICALLY if you want to show image flip vertically, 
 * noticing captured raw image wouldn't be flipped
 * 5. uncomment IMG_FLIP_HORIZONTALLY if you want to show image flip 
 * horizontally, noticing captured raw image wouldn't be flipped
 * 6. uncomment DISPLAY_FRAME_RATE_RES_INFO if you want display info in "cam"
 * 7. uncomment RESIZE_OPENCV_WINDOW if you want to resize window to 720p
 */
 //#define HAVE_OPENCV_CUDA_SUPPORT
 //#define DUAL_CAM
#define USING_CLAHE
//#define IMG_FLIP_VERTICALLY
//#define IMG_FLIP_HORIZONTALLY
#define DISPLAY_FRAME_RATE_RES_INFO
#define RESIZE_OPENCV_WINDOW 

/**
 * rgb gain and offset limits
 */
#define MAX_RGB_GAIN 2000
#define MIN_RGB_GAIN 1
#define MAX_RGB_OFFSET 255
#define MIN_RGB_OFFSET -255

/**
 * --------------------------camera specific test functions-------------------
 * The following macros are for specific cameras testing function. 
 * Uncomment the specific one if you are using that one for demo
 */
// #define AP0202_WRITE_REG_ON_THE_FLY
// #define AP0202_WRITE_REG_IN_FLASH
// #define OS05A20_PTS_QUERY
// #define AR0231_MIPI_TESTING
// #define IMX334_MONO_MIPI_TESTING


/** 
 * -------------------------- 8-bit I2C slave address list -------------------
 * Put a list here so you don't ask me what is the slave address.
 * Note that for different hardware and revsion, we might have different slave
 * address for same sensors, so try with the address in comment if possible.
 * If you still have problem to access the sensor registers, you can contact
 * our support for a firmware update since most likely that driver wasn't 
 * enabling generic i2c slave register access.
 * Disclaimer: this list doesn't include all our USB3 slave address,
 * just all the cameras I got in touch before.
 */

/**  
 * On-semi Sensor 
 * 16-bit addr, 16-bit value
 * */
#define AP020X_I2C_ADDR         (0xBA)
#define AR0231_I2C_ADDR         (0x20)// 0x30
#define AR0144_I2C_ADDR         (0x20)// 0x30
#define AR0234_I2C_ADDR         (0x20)

/**   
 * Sony Sensor   
 * 16-bit addr, 8-bit value
 */
#define IMX334_I2C_ADDR         (0x34)
#define IMX390_I2C_ADDR         (0x34)
#define IMX324_I2C_ADDR         (0x34)
#define IMX477_I2C_ADDR         (0x20)
/** 
 * Omnivision Sensor
 * 16-bit addr, 8-bit value
 */
#define OV2311_I2C_ADDR         (0xC0)
#define OS05A20_I2C_ADDR        (0x6C)
#define OV5640_I2C_ADDR         (0x78)
#define OV7251_I2C_ADDR         (0xE7)
/** 
 * Toshiba Bridge 
 * 16-bit addr, 16-bit value
 */
#define TC_MIPI_BRIDGE_I2C_ADDR (0x1C)

/**  
 * Maxim Serdes 
 * GMSL1: 8-bit addr, 8-bit value
 * GMSL2: 16-bit addr, 8-bit value
*/
#define MAX96705_SER_I2C_ADDR   (0x80)
#define MAX9295_SER_I2C_ADDR    (0x80) //0x88, 0xC4
#define MAX9272_SER_I2C_ADDR    (0x90)
#define MAX9296_DESER_I2C_ADDR  (0x90) //0xD4

/** 
 * TI Serdes
 * 16-bit addr, 8-bit value 
 * */
#define TI913_SER_I2C_ADDR      (0xB4)//0xB2
#define TI953_SER_I2C_ADDR      (0x60) 
#define TI914_DESER_I2C_ADDR    (0xC0)
#define TI954_DESER_I2C_ADDR    (0x30)

/**
 * --------------------------for nbuf numbers---------------------------------
 * Currently, default nbufs = 2 so the frame rate is higher.
 * Increasing nbufs will results in longer delay for ctrls like 
 * exposure, gain since mmap will allocate buffers to cover more 
 * frames.
 */
#define V4L_BUFFERS_DEFAULT	    (2) 
#define V4L_BUFFERS_MAX	        (32)


/**
 * FYI: default VID for Leopard Imaging USB3 camera in case you 
 * want to add your own udev rules for USB video device
 */ 
#define USB_VENDOR_ID           (0x2A0B) 

/** --- for LI_XU_GENERIC_I2C_RW --- */
#define GENERIC_REG_WRITE_FLG (0x80)
#define GENERIC_REG_READ_FLG  (0x00)
