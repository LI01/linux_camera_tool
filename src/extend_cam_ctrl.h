/****************************************************************************
  This sample is released as public domain. It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

  This is the sample code for Leopard USB3.0 camera use v4l2 and opencv for 
  camera streaming and display.

  Common implementation of a v4l2 application
  1. Open a descriptor to the device.
  2. Retrieve and analyse the device's capabilities.
  3. Set the capture format(YUYV etc).
  4. Prepare the device for buffering handling. 
     when capturing a frame, you have to submit a buffer to the device(queue)
     and retrieve it once it's been filled with data(dequeue)
     Before you can do this, you must inform the cdevice about 
     your buffer(buffer request)
  5. For each buffer you wish to use, you must negotiate characteristics with 
     the device(buffer size, frame start offset in memory), and create a new
     memory mapping for it
  6. Put the device into streaming mode
  7. Once your buffers are ready, all you have to do is keep queueing and
     dequeueing your buffer repeatedly, and every call will bring you a new 
     frame. The delay you set between each frames by putting your program to
     sleep is what determines your fps
  8. Turn off streaming mode
  9. Unmap the buffer
 10. Close your descriptor to the device 
  
  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/
#pragma once

/****************************************************************************
**                      	Global data 
*****************************************************************************/

struct buffer
{
	void *start;
	size_t length;
};


/****************************************************************************
**							 Function declaration
*****************************************************************************/
int v4l2_core_save_data_to_file(const char *filename, const void *data, int size);
void set_save_raw_flag(int flag);
void video_capture_save_raw();

void set_save_bmp_flag(int flag);
void video_capture_save_bmp();

void change_datatype(void* datatype); 
int set_shift(int *shift_flag);

void change_bayerpattern(void *bayer); 
int add_bayer_forcv(int *bayer_flag);

void add_gamma_val(float gamma_val_from_gui);
void awb_enable(int enable);
void abc_enable(int enable);

int open_v4l2_device(char *device_name, struct device *dev);
int check_dev_cap(struct device *dev);

void mmap_variables();
void unmap_variables();

void start_camera(struct device *dev);
void stop_Camera(struct device *dev);

void video_set_format(struct device *dev, int width,
					  int height, int pixelformat);
void video_get_format(struct device *dev);

int streaming_loop(struct device *dev);

void get_a_frame(struct device *dev);
void decode_a_frame(struct device *dev, const void *p, int shift);
 
int video_alloc_buffers(struct device *dev, int nbufs);
int video_free_buffers(struct device *dev);





