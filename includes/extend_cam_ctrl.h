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
 *  This is the sample code for Leopard USB3.0 camera use v4l2 and OpenCV for 
 *  camera streaming and display.                                             
 *                                                                            
 *  Common implementation of a v4l2 application                               
 *  1. Open a descriptor to the device.                                       
 *  2. Retrieve and analyse the device's capabilities.                        
 *  3. Set the capture format(YUV422 etc).                                    
 *  4. Prepare the device for buffering handling.                             
 *     When capturing a frame, you have to submit a buffer to the             
 *     device(queue) and retrieve it once it's been filled with               
 *     data(dequeue). Before you can do this, you must inform the device       
 *     about your buffer(buffer request)                                      
 *  5. For each buffer you wish to use, you must negotiate characteristics     
 *     with the device(buffer size, frame start offset in memory), and         
 *     create a new memory mapping for it                                     
 *  6. Put the device into streaming mode                                     
 *  7. Once your buffers are ready, all you have to do is keep queueing and   
 *     dequeueing your buffer repeatedly, and every call will bring you a new 
 *     frame. The delay you set between each frames by putting your program    
 *     to sleep is what determines your fps                                   
 *  8. Turn off streaming mode                                                
 *  9. Unmap the buffer                                                       
 * 10. Close your descriptor to the device                                    
 *                                                                            
 *  Author: Danyu L                                                           
 *  Last edit: 2020/01                                                        
 *****************************************************************************/
#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
/****************************************************************************
**                      	Global data 
*****************************************************************************/

struct buffer
{
   void *start;
   size_t length;
};

typedef enum
{
   RAW10_FLG = 10,
   RAW12_FLG = 12,
   YUYV_FLG = 16,
   RAW8_FLG = 8,
   MJPEG_FLG = 6
} bit_per_pixel_flag;

/** --- for LI_XU_SENSOR_UUID_HWFW_REV --- */
typedef enum
{
    LI_RAW_8_MODE          = 0x1000,
    LI_RAW_10_MODE         = 0x2000,
    LI_RAW_12_MODE         = 0x3000,
    LI_YUY2_MODE           = 0x4000,
    LI_RAW_8_DUAL_MODE     = 0x5000,
    LI_JPEG_MODE           = 0x6000
} li_datatype_fw_flag;

/****************************************************************************
**							 Function declaration
*****************************************************************************/
void set_restart_flag(int flag);
void video_change_res(int resolution_index);

void resize_window_enable(int enable);

void video_capture_save_raw();
inline void set_save_raw_flag(int flag);
int v4l2_core_save_data_to_file(
   const void *data, 
   int size);

void video_capture_save_bmp();
inline void set_save_bmp_flag(int flag);

inline int set_shift(int bpp);
void change_datatype(void *datatype);

inline int add_bayer_forcv(int bayer_flag);
void change_bayerpattern(void *bayer);

void add_gamma_val(float gamma_val_from_gui);
void add_blc(int blc_val_from_gui);

void awb_enable(int enable);
void clahe_enable(int enable);
void abc_enable(int enable);

int open_v4l2_device(
   char *device_name, 
   struct device *dev);

void *mmap_wrapper(int len);
void mmap_variables();
int set_bpp(int datatype);
void initialize_shared_memory_var();

template<class T>
void unmap_wrapper(T *data);

void unmap_variables();

void start_camera(struct device *dev);
void stop_Camera(struct device *dev);

void video_set_format(struct device *dev, int width,
                      int height, int pixelformat);
void video_set_format(struct device *dev);
void video_get_format(struct device *dev);

std::vector<std::string> 
get_resolutions(struct device *dev);

std::vector<int> 
get_frame_rates(struct device *dev);

void streaming_loop(struct device *dev, int socket);

void get_a_frame(struct device *dev);

void soft_ae_enable(int enable);
double calc_mean(struct device *dev, const void *p);
void apply_soft_ae(struct device *dev, const void *p);

void perform_shift(struct device *dev, const void *p, int shift);
void swap_two_bytes(struct device *dev, const void *p);

void rgb_ir_correction_enable(int enable);
void apply_color_correction_rgb_ir(
   struct device *dev, 
   const void *p);

int set_limit(int val, int max, int min);
void enable_rgb_gain_offset(
   int red_gain,   int green_gain,   int blue_gain,
   int red_offset, int green_offset, int blue_offset);
void disable_rgb_gain_offset();
void apply_rgb_gain_offset_pre_debayer(
   struct device *dev, const void *p);

void enable_rgb_matrix(
   int red_red,   int red_green,    int red_blue,
   int green_red, int green_green,  int green_blue,
   int blue_red,  int blue_green,   int blue_blue);

void disable_rgb_matrix();

void flip_enable(int enable);
void mirror_enable(int enable);
void canny_filter_enable(int enable);

void histogram_enable(int enable);
void motion_detector_enable(int enable);

void rgb_ir_correction_enable(int enable);
void apply_color_correction_rgb_ir(
   struct device *dev, 
   const void *p);
void rgb_ir_ir_display_enable(int enable);
void display_rgbir_ir_channel(
   struct device *dev, 
   const void *p);
void separate_dual_enable(int enable);
void display_info_enable(int enable);
void add_alpha_val(int alpha_val_from_gui);
void add_beta_val(int beta_val_from_gui);
void add_sharpness_val(int sharpness_val_from_gui);
void add_edge_thres_val(int edge_low_thres_val_from_gui);
void switch_on_keys();

void decode_process_a_frame(
   struct device *dev, 
   const void *p,
   double *cur_time);

int video_alloc_buffers(struct device *dev);
int video_free_buffers(struct device *dev);

void set_loop(int exit);
