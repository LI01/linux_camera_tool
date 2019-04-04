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

/* 
 * open the /dev/video* uvc camera device
 * 
 * args: 
 * 		device_name 
 * 		struct device *dev for adding fd
 * returns: 
 * 		file descriptor v4l2_dev
 */
int open_v4l2_device(char *device_name, struct device *dev);

/* 
 * retrive device's capabilities
 * 
 * args: 
 * 		struct device *dev - device infomation
 * returns: 
 * 		error value
 */
int check_dev_cap(struct device *dev);

/*
 * activate streaming 
 * 
 * args: 
 * 		struct device *dev - device infomation
 */
void start_camera(struct device *dev);

/*
 * deactivate streaming 
 * 
 * args: 
 * 		struct device *dev - device infomation
 */
void stop_Camera(struct device *dev);

/*
 * video get format - Leopard camera format is YUYV
 * run this to get the resolution
 * args: 
 * 		struct device *dev - device infomation
 * 	  	
 */
void video_get_format(struct device *dev);
/*
 * video set format - Leopard camera format is YUYV
 * need to do a v4l2-ctl --list-formats-ext to see the resolution
 * args: 
 * 		struct device *dev - device infomation
 * 	  	width - resoultion width
 * 		height - resolution height
 * 		pixelformat - V4L2_PIX_FMT_YUYV
 * 
 */
void video_set_format(struct device *dev, 
                      int width, int height, int pixelformat);

/* 
 * To get a frame in few steps
 * 1. prepare information about the buffer you are queueing
 * 	  (done in video_allocate_buffers)
 * 2. activate the device streaming capability
 * 3. queue the buffer,  handling your buffer over to the device,
 *    put it into the incoming queue, wait for it to write stuff in
 * 4. dequeue the buffer, the device is done, you may read the buffer
 * 
 * args: 
 * 		struct device *dev - put buffers in
*/
int streaming_loop(struct device *dev);

/* 
 * Typically start two loops:
 * 1. runs for as long as you want to
 *    capture frames (shoot the video).
 * 2. iterates over your buffers everytime. 
 *
 * args: 
 * 		struct device *dev - every infomation for camera
 */
void get_a_frame(struct device *dev);

int v4l2_core_save_data_to_file(const char *filename, const void *data, int size);

void set_save_raw_flag(int flag);
void set_save_bmp_flag(int flag);
void video_capture_save_raw();
void video_capture_save_bmp();

void change_datatype(void* datatype); 
void change_bayerpattern(void *bayer);
int set_shift(int *shift_flag); 
int add_bayer_forcv(int *bayer_flag);


/* 
 * opencv only support debayering 8 and 16 bits 
 * 
 * decode the frame, move each pixel by certain bits,
 * and mask it for 8 bits, render a frame using opencv
 * args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p - pointer for the buffer
 * 		int shift - values to shift(RAW10 - 2, RAW12 - 4) 
 * 
 */
void decode_a_frame(struct device *dev, const void *p, int shift);

/*
 * request, allocate and map buffers
 * 
 * args: 
 * 		struct device *dev - put buffers in
 * 		nbufs - number of buffer request, map
 * returns: 
 * 		errno 
 * 
 */ 
int video_alloc_buffers(struct device *dev, int nbufs);

/*
 * free, unmap buffers
 * 
 * args: 
 * 		struct device *dev - put buffers in
 * returns: 
 * 		errno 
 * 
 */ 
int video_free_buffers(struct device *dev);





