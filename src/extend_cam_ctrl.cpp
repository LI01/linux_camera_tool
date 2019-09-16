/*****************************************************************************
*  This program is free software; you can redistribute it and/or modify      *
*  it under the terms of the GNU General Public License as published by      *
*  the Free Software Foundation; either version 2 of the License, or         *
*  (at your option) any later version.                                       *
*                                                                            *
*  This program is distributed in the hope that it will be useful,           *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*  GNU General Public License for more details.                              *
*                                                                            *
*  You should have received a copy of the GNU General Public License along   *
*  with this program; if not, write to the Free Software Foundation, Inc.,   *
*                                                                            *
*  This is the sample code for Leopard USB3.0 camera use v4l2 and OpenCV for *
*  camera streaming and display.                                             *
*                                                                            *
*  Common implementation of a v4l2 application                               *
*  1. Open a descriptor to the device.                                       *
*  2. Retrieve and analyse the device's capabilities.                        *
*  3. Set the capture format(YUV422 etc).                                    *
*  4. Prepare the device for buffering handling.                             *
*     When capturing a frame, you have to submit a buffer to the             *
*     device(queue) and retrieve it once it's been filled with               *
*     data(dequeue). Before you can do this, you must inform the device      * 
*     about your buffer(buffer request)                                      *
*  5. For each buffer you wish to use, you must negotiate characteristics    * 
*     with the device(buffer size, frame start offset in memory), and        * 
*     create a new memory mapping for it                                     *
*  6. Put the device into streaming mode                                     *
*  7. Once your buffers are ready, all you have to do is keep queueing and   *
*     dequeueing your buffer repeatedly, and every call will bring you a new *
*     frame. The delay you set between each frames by putting your program   * 
*     to sleep is what determines your fps                                   *
*  8. Turn off streaming mode                                                *
*  9. Unmap the buffer                                                       *
* 10. Close your descriptor to the device                                    *
*                                                                            *
*  Author: Danyu L                                                           *
*  Last edit: 2019/08                                                        *
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/extend_cam_ctrl.h"
#include "../includes/isp_lib.h"
#include "../includes/utility.h"

#include <omp.h> /**for openmp */

/*****************************************************************************
**                      	Global variables 
*****************************************************************************/
/** 
 * Since we use fork, variables btw two processes are not shared
 * use mmap for all variables you need to share between gui, videostreaming
 */
static int *save_bmp;			  /// flag for saving bmp
static int *save_raw;			  /// flag for saving raw
static int *bayer_flag;			  /// flag for choosing bayer pattern
static int *bpp;				  /// flag for datatype bits per pixel
static int *awb_flag;			  /// flag for enable awb
static int *clahe_flag;			  /// flag for enable CLAHE
static int *abc_flag;			  /// flag for enable auto brightness&contrast
static float *gamma_val;		  /// gamma correction value from gui
static int *blc; 				  /// black level correction value from gui
static int *loop;				  /// while (*loop)
static int *rgb_gainoffset_flg;   /// flag for rgb gain and offset enable
static int *r_gain;				  /// values for rgb gain, offset correction
static int *b_gain;
static int *g_gain;
static int *r_offset;
static int *b_offset;
static int *g_offset;
static int *rgb_matrix_flg; 	  /// flag for rgb2rgb matrix enable
static int *rr, *rg, *rb;		  /// values for rgb2rgb matrix
static int *gr, *gg, *gb;
static int *br, *bg, *bb;
static int *soft_ae_flag; 		  /// flag for software AE
static int *flip_flag, *mirror_flag;
static int *show_edge_flag;
static int *rgb_ir_color, *rgb_ir_ir;
static int *separate_dual;
static int *display_info_ena;
static int *resize_window_ena;

static int *alpha, *beta, *sharpness;
static int *edge_low_thres;

int *cur_exposure; 				  /// update current exposure for AE
int *cur_gain;	 				  /// update current gain for AE

static int image_count;			  /// image count number add to capture name
		  
struct v4l2_buffer queuebuffer;   /// queuebuffer for enqueue, dequeue buffer
static constexpr const char* window_name = "Camera View"; 
/*****************************************************************************
**                      	External Callbacks
*****************************************************************************/
extern char *get_product(); 		/// put product name in captured image name
extern int get_li_datatype();
/// for software ae
extern void set_gain(int fd, int analog_gain);
extern void set_exposure_absolute(int fd, int exposure_absolute);
extern int get_current_height(int fd);
extern void free_device_vars();

/******************************************************************************
**                           Function definition
*****************************************************************************/
/*
 * flip the flag of one enable/disable switch
 * use *flag for these global shared memory flag
 */
void enable_wrapper(int *flag, int enable)
{
	switch (enable)
	{
	case 1: 
		*flag = TRUE; 
		break;
	case 0: 
		*flag = FALSE; 
		break;
	default: 
		*flag = FALSE; 
		break;
	}
}
/**
 * resize opencv image
 * args:
 * 		enable - set/reset the flag when get check button toggle
 */
void resize_window_enable(int enable)
{
	enable_wrapper(resize_window_ena, enable);
}


/**
 * callback for save raw image from gui
 */
void video_capture_save_raw()
{
	set_save_raw_flag(1);
}

/**
 * set save raw image flag 
 * args:
 * 		flag - set the flag when get capture button clicked,
 * 			   reset flag to zero once image is saved
 */
inline void set_save_raw_flag(int flag)
{
	*save_raw = flag;
}

/**
 * save data to file
 * args:
 *   data - pointer to data
 *   size - data size in bytes = sizeof(uint8_t)
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int v4l2_core_save_data_to_file(
	const void *data, 
	int size)
{
	FILE *fp;
	int ret = 0;
	char buf_name[40];
	snprintf(buf_name, sizeof(buf_name), "captures_%s_%d.raw",
			 get_product(), image_count);

	if ((fp = fopen(buf_name, "wb")) != NULL)
	{
		ret = fwrite((uint8_t *)data, size, 1, fp);

		if (ret < 1)
			ret = 1; /**write error*/
		else
			ret = 0;

		fflush(fp); /**flush data stream to file system*/
		if (fsync(fileno(fp)) || fclose(fp))
			fprintf(stderr, "V4L2_CORE: (save_data_to_file) error \
				- couldn't write buffer to file: %s\n",
					strerror(errno));
		else
			printf("V4L2_CORE: saved data to %s\n", buf_name);
	}
	else
		ret = 1;

	image_count++;
	set_save_raw_flag(0);

	return (ret);
}

/**
 * callback for save bmp image from gui
 */
void video_capture_save_bmp()
{
	set_save_bmp_flag(1);
}

/**
 * set save raw image flag 
 * args:
 * 		flag - set the flag when get capture button clicked,
 * 			   reset flag to zero once image is saved
 */
inline void set_save_bmp_flag(int flag)
{
	*save_bmp = flag;
}

/**
 * save data to bitmap using OpenCV
 * args:
 *	 opencv mat image to be captured
 *
 * asserts:
 *   none
 */
static int save_frame_image_bmp(
	cv::InputOutputArray& opencvImage)
{

	printf("save one capture bmp\n");
	cv::imwrite(cv::format(
		"captures_%s_%d.bmp",
		get_product(), 
		image_count),
		opencvImage);
	image_count++;
	set_save_bmp_flag(0);
	return 0;
}

/**
 * return the shift value for choosing sensor datatype
 * RAW10 - shift 2 bits
 * RAW12 - shift 4 bits
 * YUV422 - shift 0 bit
 * RAW8 - shift 0 bit - tightly packed 8-bit in fpga
 * Crosslink doesn't support decode RAW14, RAW16 so far,
 * these two datatypes weren't used in USB3 camera
 */
inline int set_shift(int bpp)
{
	switch (bpp)
	{
	case RAW10_FLG:
		return 2;
	case RAW12_FLG:
		return 4;
	case YUYV_FLG:
		return 0;
	case RAW8_FLG:
		return -1;
	default:
		return 2;
	}
}

/**
 * callback for change sensor datatype bit per pixel
 * args:
 * 		datatype - RAW10  -> set *bpp accordingly
 *  			   RAW12  
 * 				   YUV422 
 * 				   RAW8   
 */
void change_datatype(void *datatype)
{
	if (strcmp((char *)datatype, "1") == 0)
		*bpp = RAW10_FLG;
	if (strcmp((char *)datatype, "2") == 0)
		*bpp = RAW12_FLG;
	if (strcmp((char *)datatype, "3") == 0)
		*bpp = YUYV_FLG;
	if (strcmp((char *)datatype, "4") == 0)
		*bpp = RAW8_FLG;
}


/**
 * callback for change sensor bayer pattern for debayering
 * determine the sensor bayer pattern to correctly debayer the image
 *   CV_BayerBG2BGR =46   -> bayer_flag_increment = 0
 *   CV_BayerGB2BGR =47   -> bayer_flag_increment = 1
 *   CV_BayerRG2BGR =48   -> bayer_flag_increment = 2
 *   CV_BayerGR2BGR =49	  -> bayer_flag_increment = 3
 * args:
 * 		bayer 		- for updating *bayer_flag
 * 		*bayer_flag - flag for determining bayer pattern 
 *  			   
 */
void change_bayerpattern(void *bayer)
{
	if (strcmp((char *)bayer, "1") == 0)
		*bayer_flag = CV_BayerBG2BGR_FLG;
	if (strcmp((char *)bayer, "2") == 0)
		*bayer_flag = CV_BayerGB2BGR_FLG;
	if (strcmp((char *)bayer, "3") == 0)
		*bayer_flag = CV_BayerRG2BGR_FLG;
	if (strcmp((char *)bayer, "4") == 0)
		*bayer_flag = CV_BayerGR2BGR_FLG;
	if (strcmp((char *)bayer, "5") == 0)
		*bayer_flag = CV_MONO_FLG;
}

/** callback for set blc from gui */
void add_blc(int blc_val_from_gui)
{
	*blc = blc_val_from_gui;
}

/** callback for set gamma_val for gamma correction from gui */
void add_gamma_val(float gamma_val_from_gui)
{
	*gamma_val = gamma_val_from_gui;
}


/**
 * set awb flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void awb_enable(int enable)
{
	enable_wrapper(awb_flag, enable);
}


/**
 * set abc flag 
 * args:
 * 		enable - set/reset the flag when get check button toggle
 */
void abc_enable(int enable)
{
	enable_wrapper(abc_flag, enable);
}

/**
 * set CLAHE flag 
 * args:
 * 		enable - set/reset the flag when get check button toggle
 */
void clahe_enable(int enable)
{
	enable_wrapper(clahe_flag, enable);
}

/** 
 * open the /dev/video* uvc camera device
 * 
 * V4L2 devices can be opened more than once. When this is supported by 
 * the driver, users can for example start a “panel” application to change
 * controls like brightness or audio volume, while another application 
 * captures video and audio. In other words, panel applications are comparable
 * to an ALSA audio mixer application. Just opening a V4L2 device should not 
 * change the state of the device.
 * 
 * Once an application has allocated the memory buffers needed for streaming 
 * data (by calling the ioctl VIDIOC_REQBUFS or ioctl VIDIOC_CREATE_BUFS ioctls,
 * or implicitly by calling the read() or write() functions) that application 
 * (filehandle) becomes the owner of the device. It is no longer allowed to make
 * changes that would affect the buffer sizes (e.g. by calling the VIDIOC_S_FMT 
 * ioctl) and other applications are no longer allowed to allocate buffers or 
 * start or stop streaming. The EBUSY error code will be returned instead.
 * 
 * args: 
 * 		device_name 
 * 		struct device *dev for adding fd
 * returns: 
 * 		file descriptor v4l2_dev
 */
int open_v4l2_device(
	char *device_name, 
	struct device *dev)
{
	int v4l2_dev;

	if (device_name == NULL)
		return -5;

	v4l2_dev = open(device_name, O_RDWR);
	if (-1 == v4l2_dev)
	{
		perror("open video device fail");
		return -1;
	}
	dev->fd = v4l2_dev;

	return v4l2_dev;
}
/** a mmap wrapper */
void *mmap_wrapper(int len)
{
	void *p = mmap(NULL, len, PROT_READ | PROT_WRITE, 
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (p == (void *) -1)
		perror("mmap shared memory failed\n");
	return p;
}
/** mmap the variables for processes share */
void mmap_variables()
{
	gamma_val 			= (float *)mmap_wrapper(sizeof(float));
	save_bmp 			 = (int *)mmap_wrapper(sizeof(int));
	save_raw 			 = (int *)mmap_wrapper(sizeof(int));
	bpp 				 = (int *)mmap_wrapper(sizeof(int));
	bayer_flag 			 = (int *)mmap_wrapper(sizeof(int));
	awb_flag 			 = (int *)mmap_wrapper(sizeof(int));
	clahe_flag 			 = (int *)mmap_wrapper(sizeof(int));
	abc_flag 			 = (int *)mmap_wrapper(sizeof(int));
	blc 				 = (int *)mmap_wrapper(sizeof(int));
	loop 				 = (int *)mmap_wrapper(sizeof(int));
	rgb_gainoffset_flg   = (int *)mmap_wrapper(sizeof(int));
	rgb_matrix_flg 	     = (int *)mmap_wrapper(sizeof(int));
	r_gain 				 = (int *)mmap_wrapper(sizeof(int));
	g_gain 				 = (int *)mmap_wrapper(sizeof(int));
	b_gain 				 = (int *)mmap_wrapper(sizeof(int));
	r_offset 			 = (int *)mmap_wrapper(sizeof(int));
	g_offset 			 = (int *)mmap_wrapper(sizeof(int));
	b_offset 			 = (int *)mmap_wrapper(sizeof(int));
	rr 					 = (int *)mmap_wrapper(sizeof(int));
	rg 					 = (int *)mmap_wrapper(sizeof(int));
	rb 					 = (int *)mmap_wrapper(sizeof(int));
	gr 					 = (int *)mmap_wrapper(sizeof(int));
	gg 					 = (int *)mmap_wrapper(sizeof(int));
	gb 					 = (int *)mmap_wrapper(sizeof(int));
	br 					 = (int *)mmap_wrapper(sizeof(int));
	bg 					 = (int *)mmap_wrapper(sizeof(int));
	bb 					 = (int *)mmap_wrapper(sizeof(int));
	cur_exposure 		 = (int *)mmap_wrapper(sizeof(int));
	cur_gain 			 = (int *)mmap_wrapper(sizeof(int));
	soft_ae_flag 		 = (int *)mmap_wrapper(sizeof(int));
	flip_flag 			 = (int *)mmap_wrapper(sizeof(int));
	mirror_flag 		 = (int *)mmap_wrapper(sizeof(int));
	show_edge_flag 	     = (int *)mmap_wrapper(sizeof(int));
	rgb_ir_color 		 = (int *)mmap_wrapper(sizeof(int));
	rgb_ir_ir 			 = (int *)mmap_wrapper(sizeof(int));
	separate_dual 		 = (int *)mmap_wrapper(sizeof(int));
	display_info_ena 	 = (int *)mmap_wrapper(sizeof(int));
	alpha 				 = (int *)mmap_wrapper(sizeof(int));
	beta 				 = (int *)mmap_wrapper(sizeof(int));
	sharpness 			 = (int *)mmap_wrapper(sizeof(int));
	edge_low_thres 		 = (int *)mmap_wrapper(sizeof(int));
	resize_window_ena 	 = (int *)mmap_wrapper(sizeof(int));
}
/** 
 * set bit per pixel of the camera read from uvc extension unit
 * datatype flag that is written in the camera driver
 */
int set_bpp(int datatype)
{
	switch(datatype)
	{
		case LI_RAW_10_MODE:
			return RAW10_FLG;
		case LI_RAW_12_MODE:
			return RAW12_FLG;
		case LI_RAW_8_MODE:
		case LI_RAW_8_DUAL_MODE:
			return RAW8_FLG;
		case LI_YUY2_MODE:
			return YUYV_FLG;
		case LI_JPEG_MODE:
			return YUYV_FLG;//haven't support yet	
		default:
			return RAW10_FLG;
	}
}

/** initialize share memory variables after declaration */
void initialize_shared_memory_var()
{
	*gamma_val = 1.0;
	*blc = 0;
	if (strcmp(get_product(), "USB Camera-OV580") == 0)
		*bpp = RAW8_FLG;
	else if (strcmp(get_product(), "OV580 STEREO") == 0)
		*bpp = YUYV_FLG;
	else
		*bpp = set_bpp(get_li_datatype());
	*bayer_flag = CV_BayerBG2BGR_FLG;
	*display_info_ena = TRUE;
	*alpha = 1;
	*beta = 0;
	*sharpness = 1;
}

/** wrapper for unmap shared memory */
template<class T>
void unmap_wrapper(T *data)
{
	int ret = munmap(data, sizeof(T));
	if (ret < 0)
		printf("Unable to unmap shared memory (%d)\n", errno);	
}

/** unmap all the variables after stream ends */
void unmap_variables()
{
	unmap_wrapper(save_bmp);
	unmap_wrapper(save_raw);
	unmap_wrapper(bpp);
	unmap_wrapper(bayer_flag);
	unmap_wrapper(awb_flag);
	unmap_wrapper(clahe_flag);
	unmap_wrapper(abc_flag);
	unmap_wrapper(gamma_val);
	unmap_wrapper(blc);
	unmap_wrapper(loop);
	unmap_wrapper(rgb_gainoffset_flg);
	unmap_wrapper(r_gain);
	unmap_wrapper(g_gain);
	unmap_wrapper(b_gain);
	unmap_wrapper(r_offset);
	unmap_wrapper(g_offset);
	unmap_wrapper(b_offset);
	unmap_wrapper(rgb_matrix_flg);
	unmap_wrapper(rr);
	unmap_wrapper(rg);
	unmap_wrapper(rb);
	unmap_wrapper(gr);
	unmap_wrapper(gg);
	unmap_wrapper(gb);
	unmap_wrapper(br);
	unmap_wrapper(bg);
	unmap_wrapper(bb);
	unmap_wrapper(cur_exposure);
	unmap_wrapper(cur_gain);
	unmap_wrapper(soft_ae_flag);
	unmap_wrapper(flip_flag);
	unmap_wrapper(mirror_flag);
	unmap_wrapper(show_edge_flag);
	unmap_wrapper(rgb_ir_color);
	unmap_wrapper(rgb_ir_ir);
	unmap_wrapper(separate_dual);
	unmap_wrapper(display_info_ena);
	unmap_wrapper(alpha);
	unmap_wrapper(beta);
	unmap_wrapper(sharpness);
	unmap_wrapper(edge_low_thres);
	unmap_wrapper(resize_window_ena);
	free_device_vars();
}

/**
 * activate streaming 
 * 
 * args: 
 * 		struct device *dev - device infomation
 */
void start_camera(struct device *dev)
{
	int ret;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = (ioctl(dev->fd, VIDIOC_STREAMON, &type));
	if (ret < 0)
	{
		printf("Couldn't start camera streaming\n");
		return;
	}
	mmap_variables();
	initialize_shared_memory_var();
	return;
}

/**
 * deactivate streaming 
 * 
 * args: 
 * 		struct device *dev - device infomation
 */
void stop_Camera(struct device *dev)
{
	int ret;
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(dev->fd, VIDIOC_STREAMOFF, &type);
	if (ret < 0)
	{
		perror("Couldn't stop camera streaming\n");
		return;
	}
	return;
}

/**
 * video set format - Leopard camera format is YUYV
 * need to do a v4l2-ctl --list-formats-ext to see the resolution
 * args: 
 * 		struct device *dev 	- device infomation
 * 	  	width 				- resolution width
 * 		height 				- resolution height
 * 		pixelformat 		- V4L2_PIX_FMT_YUYV
 * 
 */
void video_set_format(
	struct device *dev, 
	int width,
	int height, 
	int pixelformat)
{
	struct v4l2_format fmt;
	int ret;

	fmt.fmt.pix.width = width;
	dev->width = width;
	fmt.fmt.pix.height = height;
	dev->height = height;
	fmt.fmt.pix.pixelformat = pixelformat;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev->fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0)
	{
		printf("Unable to set format: %s (%d).\n", strerror(errno),
			   errno);
		return;
	}
	printf("Get Video Format: %c%c%c%c (%08x) %ux%u\n"
		   "byte per line:%d\nsize image:%u\n",
		   (fmt.fmt.pix.pixelformat >> 0) & 0xff,
		   (fmt.fmt.pix.pixelformat >> 8) & 0xff,
		   (fmt.fmt.pix.pixelformat >> 16) & 0xff,
		   (fmt.fmt.pix.pixelformat >> 24) & 0xff,
		   fmt.fmt.pix.pixelformat,
		   fmt.fmt.pix.width,
		   fmt.fmt.pix.height,
		   fmt.fmt.pix.bytesperline,
		   fmt.fmt.pix.sizeimage);
	return;
}

/**
 * video get format - Leopard camera format is YUYV
 * run this to get the resolution
 * args: 
 * 		struct device *dev - device infomation
 * 	  	
 */
void video_get_format(struct device *dev)
{
	struct v4l2_format fmt;
	int ret;

	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(dev->fd, VIDIOC_G_FMT, &fmt);
	if (ret < 0)
	{
		printf("Unable to get format: %s (%d).\n", strerror(errno),
			   errno);
		return;
	}
	dev->width = fmt.fmt.pix.width;
	dev->height = fmt.fmt.pix.height;
	dev->bytesperline = fmt.fmt.pix.bytesperline;
	dev->imagesize = fmt.fmt.pix.bytesperline ? fmt.fmt.pix.sizeimage : 0;

	printf("\nGet Current Video Format:%c%c%c%c (%08x)%ux%u\n"
		   "byte per line:%d\nsize image:%u\n",
		   (fmt.fmt.pix.pixelformat >> 0) & 0xff,
		   (fmt.fmt.pix.pixelformat >> 8) & 0xff,
		   (fmt.fmt.pix.pixelformat >> 16) & 0xff,
		   (fmt.fmt.pix.pixelformat >> 24) & 0xff,
		   fmt.fmt.pix.pixelformat,
		   fmt.fmt.pix.width,
		   fmt.fmt.pix.height,
		   fmt.fmt.pix.bytesperline,
		   fmt.fmt.pix.sizeimage);
	return;
}

/** 
 * To get frames in few steps
 * 1. prepare information about the buffer you are queueing
 * 	  (done in video_allocate_buffers)
 * 2. activate the device streaming capability
 * 3. queue the buffer,  handling your buffer over to the device,
 *    put it into the incoming queue, wait for it to write stuff in
 * 4. decode the frame
 * 5. dequeue the buffer, the device is done, you may read the buffer
 * 6. put 3-5 in a for loop
 * 
 * args: 
 * 		struct device *dev - every infomation for camera
*/
void streaming_loop(struct device *dev)
{
	cv::namedWindow(window_name, cv::WINDOW_NORMAL);

	image_count = 0;
	*loop = 1;
	while (*loop)
		get_a_frame(dev);
	unmap_variables();
}

/** 
 * Typically start two loops:
 * 1. runs for as long as you want to
 *    capture frames (shoot the video).
 * 2. iterates over your buffers everytime. 
 *
 * args: 
 * 		struct device *dev - every infomation for camera
 */
void get_a_frame(struct device *dev)
{
	
	for (size_t i = 0; i < dev->nbufs; i++)
	{
		/// time measured in OpenCV for fps
		double cur;
		tic(cur);
		double *cur_time = &cur;
		queuebuffer.index = i;

		/// The buffer's waiting in the outgoing queue
		if (ioctl(dev->fd, VIDIOC_DQBUF, &queuebuffer) < 0)
		{
			perror("VIDIOC_DQBUF");
			return;
		}

		decode_process_a_frame(dev, dev->buffers[i].start, cur_time);

		if (ioctl(dev->fd, VIDIOC_QBUF, &queuebuffer) < 0)
		{
			perror("VIDIOC_QBUF");
			return;
		}
	}

	return;
}


/**
 * set software ae 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void soft_ae_enable(int enable)
{
	enable_wrapper(soft_ae_flag, enable);
}
/**
 * calculate the mean value of a given unprocessed image for a defined ROI
 * return the value used for software AE
 * args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 */
double calc_mean(
	struct device *dev, 
	const void *p)
{
	/// define ROI: 256x256 square pixels in the middle of image
	int size = ROI_SIZE;
	int width = dev->width;
	int height = dev->height;
	int start_x = (width - size) / 2;
	int start_y = (height - size) / 2;
	int bytes_per_pixel = BYTES_PER_BPP(*bpp);
	int start_pos = PIX(start_x, start_y, width) * bytes_per_pixel;
	double total = 0, mean = 0;
	unsigned char *src_char = (unsigned char *)p;

	if (bytes_per_pixel == 2)
	{
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				/// upper and lower byte
				total += (double)(src_char[start_pos + PIX(j, i, width) * bytes_per_pixel] |
								  src_char[start_pos + PIX(j, i, width) * bytes_per_pixel + 1] << 8);
			}
		}
	}
	else
	{
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				total += (double)src_char[start_pos + PIX(j, i, width)];
			}
		}
	}
	mean = total / size / size;
	return mean;
}
/**
 * simple and brute software generate auto exposure
 * set a target mean value and the range, if the current image illuminance 
 * is smaller than the target mean, increase exposure until the max_exp_time,
 * if it still not enough, starting to increase gain. Do the similar thing for 
 * decrease gain, exposure 
 * args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 */
void apply_soft_ae(
	struct device *dev, 
	const void *p)
{
	int local_gain = *cur_gain;
	int local_exposure = *cur_exposure;
	long cur_gain_x_exp = local_gain * local_exposure;
	int max_exp_time = get_current_height(dev->fd) * MAX_EXP_TIME_FACTOR;
	int min_exp_time = MIN_EXP_TIME;
	int min_gain = MIN_GAIN;
	int max_gain = MAX_GAIN;
	float target_mean = (float)SETBIT(0x01, (*bpp) - 2) * TARGET_MEAN_FACTOR;
	float image_mean = calc_mean(dev, p);
	int ae_done = FALSE;

	if (((image_mean > target_mean * 0.8) 
	&& (image_mean < target_mean * 1.2)) 
	|| ((cur_gain_x_exp >= max_exp_time * max_gain) 
	&& (image_mean < target_mean * 0.8)) 
	|| ((cur_gain_x_exp <= max_exp_time * min_gain) 
	&& (image_mean > target_mean * 1.2)))
	{
		ae_done = TRUE;
		return;
	}

	ae_done = FALSE;
	int cal_exp_x_gain = (int)(cur_gain_x_exp * (target_mean / image_mean));
	cur_gain_x_exp = (cur_gain_x_exp + cal_exp_x_gain) / 2;

	if (cur_gain_x_exp >= max_exp_time * min_gain)
	{
		local_exposure = max_exp_time;
		local_gain = local_gain * (target_mean / image_mean);
		local_gain = set_limit(local_gain, max_gain, min_gain);
	}
	else if (cur_gain_x_exp < min_exp_time * min_gain)
	{
		local_exposure = min_exp_time;
		local_gain = min_gain;
	}
	else
	{
		local_gain = min_gain;
		local_exposure = (int)local_exposure * (target_mean / image_mean);
	}
	cur_gain_x_exp = local_exposure * local_gain;
	set_exposure_absolute(dev->fd, local_exposure);
	set_gain(dev->fd, local_gain);
}

/** 
 * Shift bits for 16-bit stream and get lower 8-bit for OpenCV debayering.
 * Info of Leopard USB3 RAW data stream is explained in /pic/datatypeExp.jpg 
 * args: 
 * 		struct device *dev  - every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 * 		int shift 			- shift bits
 * */
void perform_shift(
	struct device *dev, 
	const void *p, 
	int shift)
{

	//Timer timer;
	unsigned char tmp;
	unsigned short *src_short = (unsigned short *)p;
	unsigned char *dst = (unsigned char *)p;
	unsigned short ts;

	/** 
	 * use OpenMP loop parallelism to accelerate shifting 
	 * If there is no speed up, that is because this operation is heavily 
	 * memory bound. All the cores share one memory bus, so using more threads 
	 * does not give you more bandwidth and speedup.
	 * This will change if the resolution is smaller so buffer size is smaller. 
	 */
#pragma omp for nowait
	for (size_t i = 0; i < dev->height; i++)
	{
		for (size_t j = 0; j < dev->width; j++)
		{
			ts = *(src_short++);
			if (ts > *blc)
				tmp = (ts - *blc) >> shift;
			else
				tmp = 0;

			*(dst++) = (unsigned char)tmp;
		}
	}
	
}

/** 
 * Easier way to debug if it is FPGA byte swap problem
 * ---------------------------INTERNAL USE ONLY------------------------
 * Swap higher and lower 8-bit to see if there is still artifact exists
 * args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 */
void swap_two_bytes(struct device *dev, const void *p)
{
	uint16_t *dst = (uint16_t *)p;
	uint16_t *src = (uint16_t *)p;
	uint16_t tmp;

	for (size_t i = 0; i < dev->height; i++)
	{
		for (size_t j = 0; j < dev->width; j++)
		{
			tmp = src[PIX(j, i, dev->width)];
			dst[PIX(j, i, dev->width)] = LOWBYTE(tmp) << 8 | HIGHBYTE(tmp);
		}
	}
}

/** 
 * Easier way to debug if it is FPGA byte swap problem
 * ---------------------------INTERNAL USE ONLY------------------------
 * Swap higher and lower 8-bit to see if there is still artifact exists
 * args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 */
void swap_four_bytes(struct device *dev, const void *p)
{
	uint32_t *dst = (uint32_t *)p;
	uint32_t *src = (uint32_t *)p;
	uint32_t tmp;

	for (size_t i = 0; i < dev->height/2; i++)
	{
		for (size_t j = 0; j < dev->width/2; j++)
		{
			tmp = src[PIX(j, i, dev->width)];
			dst[PIX(j, i, dev->width)] = ((uint16_t)tmp << 16) |
			 ((uint16_t) (tmp >> 16));
		}
	}
}

/**
 * set the limit for rgb color correction input value
 */
int set_limit(int val, int max, int min)
{
	return val = val < min ? min : val > max ? max : val;
}

/** pass the variable from GUI to camera streaming thread
 * and print out these values
 * set the flag on enable rgb_gain_offset
 */
void enable_rgb_gain_offset(
	int red_gain, 	int green_gain,   int blue_gain,
	int red_offset, int green_offset, int blue_offset)
{
	*r_gain 	= red_gain;
	*g_gain 	= green_gain;
	*b_gain 	= blue_gain;
	*r_offset 	= red_offset;
	*g_offset 	= green_offset;
	*b_offset 	= blue_offset;
	*rgb_gainoffset_flg = 1;
	printf("----------rgb gain & offset enabled----------\r\n");
	printf("r gain = %-4d\tg gain = %-4d\tb gain = %-4d\r\n"
		   "r offset = %-3d\tg offset = %-3d\tb offset = %-3d\r\n ",
		   *r_gain, *g_gain, *b_gain, *r_offset, *g_offset, *b_offset);
}

/**
 * disable the flag on rgb_gain_offset
 */
void disable_rgb_gain_offset()
{
	printf("---------rgb gain & offset disabled---------\r\n");
	*rgb_gainoffset_flg = 0;
}

/**
 * apply rgb gain and offset color correction before debayering
 * only support four bayer patterns now: GRBG, GBRG, RGGB, BGGR
 * args: 
 * 		struct device *dev 	- every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 */
void apply_rgb_gain_offset_pre_debayer(
	struct device *dev, const void *p)
{
	if ( *r_gain == 256 && *g_gain == 256 && *b_gain == 256 &&
		 *r_offset == 0 && *g_offset == 0 && *b_offset == 0)
		return;
	int pixel_max_val = BIT(*bpp) - 1;
	int height = dev->height;
	int width = dev->width;
	int itmp;
	uint16_t tmp;
	uint16_t *src_short = (uint16_t *)p;
	uint16_t *dst = (uint16_t *)p;

	switch (*bayer_flag)
	{
	case CV_BayerGR2BGR_FLG: //GRBG
		for (int i = 0; i < height; i += 2)
		{
			for (int j = 0; j < width; j += 2)
			{
				// G
				tmp = src_short[PIX(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *g_offset;
				itmp = (itmp * (*g_gain)) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[PIX(j, i, width)] = itmp;
				// R
				tmp = src_short[RIGHT(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *r_offset;
				itmp = (itmp * (*r_gain)) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[RIGHT(j, i, width)] = itmp;
				// B
				tmp = src_short[BOTTOM(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *b_offset;
				itmp = (itmp * (*b_gain)) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[BOTTOM(j, i, width)] = itmp;
				// G
				tmp = src_short[BOTTOM_RIGHT(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *g_offset;
				itmp = (itmp * (*g_gain)) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[BOTTOM_RIGHT(j, i, width)] = itmp;
			}
		}

		break;
	case CV_BayerGB2BGR_FLG: //GBRG
		for (int i = 0; i < height; i += 2)
		{
			for (int j = 0; j < width; j += 2)
			{
				// G
				tmp = src_short[PIX(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *g_offset;
				itmp = (itmp * (*g_gain)) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[PIX(j, i, width)] = itmp;
				// B
				tmp = src_short[RIGHT(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *b_offset;
				itmp = (itmp * (*b_gain)) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[RIGHT(j, i, width)] = itmp;
				// R
				tmp = src_short[BOTTOM(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *r_offset;
				itmp = (itmp * (*r_gain)) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[BOTTOM(j, i, width)] = itmp;
				// G
				tmp = src_short[BOTTOM_RIGHT(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *g_offset;
				itmp = (itmp * (*g_gain)) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[BOTTOM_RIGHT(j, i, width)] = itmp;
			}
		}
		break;
	case CV_BayerRG2BGR_FLG: //RGGB
		for (int i = 0; i < height; i += 2)
		{
			for (int j = 0; j < width; j += 2)
			{
				// R
				tmp = src_short[PIX(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *r_offset;
				itmp = itmp * (*r_gain) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[PIX(j, i, width)] = itmp;
				// G
				tmp = src_short[RIGHT(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *g_offset;
				itmp = itmp * (*g_gain) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[RIGHT(j, i, width)] = itmp;
				// G
				tmp = src_short[BOTTOM(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *g_offset;
				itmp = itmp * (*g_gain) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[BOTTOM(j, i, width)] = itmp;
				// B
				tmp = src_short[BOTTOM_RIGHT(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *b_offset;
				itmp = itmp * (*b_gain) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[BOTTOM_RIGHT(j, i, width)] = itmp;
			}
		}
		break;
	case CV_BayerBG2BGR_FLG: //BGGR
		for (int i = 0; i < height; i += 2)
		{
			for (int j = 0; j < width; j += 2)
			{
				// B
				tmp = src_short[PIX(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *b_offset;
				itmp = itmp * (*b_gain) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[PIX(j, i, width)] = itmp;
				// G
				tmp = src_short[RIGHT(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *g_offset;
				itmp = itmp * (*g_gain) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[RIGHT(j, i, width)] = itmp;
				// G
				tmp = src_short[BOTTOM(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *g_offset;
				itmp = itmp * (*g_gain) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[BOTTOM(j, i, width)] = itmp;
				// R
				tmp = src_short[BOTTOM_RIGHT(j, i, width)];
				tmp = set_limit(tmp, pixel_max_val, 0);
				itmp = tmp + *r_offset;
				itmp = itmp * (*r_gain) / GAIN_FACTOR;
				itmp = set_limit(itmp, pixel_max_val, 0);
				dst[BOTTOM_RIGHT(j, i, width)] = itmp;
			}
		}
		break;
	case CV_MONO_FLG:
		break;
	default:
		break;
	}
}

/** 
 * pass the variable from GUI to camera streaming thread
 * and print out these values
 * set the flag on enable rgb to rgb matrix
  * args: 
 * 		red_red, red_green etc - rgb2rgb matrix parameters
 */
void enable_rgb_matrix(
	int red_red, 	int red_green, 		int red_blue,
	int green_red, 	int green_green, 	int green_blue,
	int blue_red, 	int blue_green, 	int blue_blue)
{
	*rr = red_red;
	*rg = red_green;
	*rb = red_blue;
	*gr = green_red;
	*gg = green_green;
	*gb = green_blue;
	*br = blue_red;
	*bg = blue_green;
	*bb = blue_blue;
	*rgb_matrix_flg = 1;
	
	printf("-----------rgb matrix enabled-----------\r\n");
	printf("rr = %-4d\trg = %-4d\trb = %-4d\r\n"
		   "gr = %-4d\tgg = %-4d\tgb = %-4d\r\n"
		   "br = %-4d\tbg = %-4d\tbb = %-4d\r\n",
		   *rr, *rg, *rb, *gr, *gg, *gb, *br, *bg, *bb);
}

/**
 * disable the flag on rgb to rgb matrix
 */
void disable_rgb_matrix()
{
	printf("-----------rgb matrix disabled----------\r\n");
	*rgb_matrix_flg = 0;
}


/**
 * set flip flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void flip_enable(int enable)
{
	enable_wrapper(flip_flag, enable);
}

/**
 * set mirror flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void mirror_enable(int enable)
{
	enable_wrapper(mirror_flag, enable);
}

/**
 * set canny filter flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void canny_filter_enable(int enable)
{
	enable_wrapper(show_edge_flag, enable);
}

/**
 * set separate dual display enable flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void separate_dual_enable(int enable)
{
	enable_wrapper(separate_dual, enable);
}

/**
 * set display mat info flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void display_info_enable(int enable)
{
	enable_wrapper(display_info_ena, enable);
}

/** callback for set alpha for contrast correction from gui */
void add_alpha_val(int alpha_val_from_gui)
{
	*alpha = alpha_val_from_gui;
}

/** callback for set beta for brightness correction from gui */
void add_beta_val(int beta_val_from_gui)
{
	*beta = beta_val_from_gui;
}

/** callback for set sharpness for sharpness correction from gui */
void add_sharpness_val(int sharpness_val_from_gui)
{
	*sharpness = sharpness_val_from_gui;
}

/** callback for add edge detection for threshold value from gui */
void add_edge_thres_val(int edge_low_thres_val_from_gui)
{
	*edge_low_thres = edge_low_thres_val_from_gui;
}

/** add switch keep to exit program */
void switch_on_keys()
{
	char key = (char)cv::waitKey(_1MS);
	switch (key) {
	case 'q':
	case 'Q':
	case _ESC_KEY_ASCII:
		cv::destroyWindow(window_name);
		set_loop(0);
		break;
	default:
		break;
	}
}
/** 
 * like the name said, group all the isp functions that can 
 * perform on both OpenCV non-CUDA and OpenCV CUDA
 * args:
 *     cv::InputOutputArray opencvImage - camera stream buffer array
 * 			that can be modified inside the functions
 */
static void group_gpu_image_proc(
	cv::InputOutputArray opencvImage)
{

	if (*(gamma_val) != (float)1)
		apply_gamma_correction(opencvImage,*gamma_val);
	
	if (*(alpha) != (float)1 || (*beta) != (float)0)
		apply_brightness_and_contrast(opencvImage, *alpha, *beta);
	
	if (*abc_flag)
		apply_auto_brightness_and_contrast(opencvImage, 1);

	if (*sharpness != (float)1)
		sharpness_control(opencvImage,*sharpness);

	if (*clahe_flag)
		apply_clahe(opencvImage);
		
	if (*show_edge_flag) 
		canny_filter_control(opencvImage,*edge_low_thres);

}
/** 
 * OpenCV only support debayering 8 and 16 bits 
 * 
 * decode the frame, move each pixel by certain bits,
 * and mask it for 8 bits, render a frame using OpenCV
 * perform all flagged image processing tasks here 
 * args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p - pointer for the buffer

 * 
 */
void decode_process_a_frame(
	struct device *dev, 
	const void *p,
	double *cur_time)
{
	int height = dev->height;
	int width = dev->width;
	int shift = set_shift(*bpp);
	cv::Mat share_img;
#ifdef HAVE_OPENCV_CUDA_SUPPORT
	cv::cuda::GpuMat gpu_img;
#endif

	if (*soft_ae_flag)
		apply_soft_ae(dev, p);
	if (*save_raw)
		v4l2_core_save_data_to_file(p, dev->imagesize);
	
	/** --- for raw8, raw10, raw12 bayer camera ---*/
	if (shift != 0)
	{
		if (*rgb_gainoffset_flg)
			apply_rgb_gain_offset_pre_debayer(dev, p);
		if (*rgb_ir_color)
			apply_color_correction_rgb_ir(dev, p);
		if (*rgb_ir_ir)
			display_rgbir_ir_channel(dev, p);
		/** 
		 * --- for raw10, raw12 camera ---
		 * tried with CV_16UC1 and then cast back, it doesn't really improve fps
		 * I guess use openmp is already efficient enough 
		 */
		if (shift > 0) 
			perform_shift(dev, p, shift);
			
		/**
		 *  --- for raw8 camera ---
		 * each pixel is 8-bit instead of 16-bit now
		 * need to adjust read width
		 */
		else 
			width = width * 2;
		
		//swap_two_bytes(dev, p);
		cv::Mat img(height, width, CV_8UC1, (void *)p);
#ifdef HAVE_OPENCV_CUDA_SUPPORT
		gpu_img.upload(img);
		debayer_awb_a_frame(gpu_img, *bayer_flag, *awb_flag);
		gpu_img.download(img);
#else
		debayer_awb_a_frame(img, *bayer_flag, *awb_flag);	
#endif

		if (*rgb_matrix_flg)
		{	
			//Timer timer;
			int ccm[3][3] = {
				{*rr, *rg, *rb},
				{*gr, *gg, *gb},
				{*br, *bg, *bb}
			};
			if (*rr==256 && *rg==0 && *rb==0 && \
				*gr==0 && *gg==256 && *gb==0 && \
				*br==0 && *bg==0 && *bb==256)
					apply_rgb_matrix_post_debayer(img, 
					(int *)ccm);
			
		}
		share_img = img;
	}

	/** --- for yuv camera ---*/
	else if (shift == 0)
	{	
		cv::Mat img(height, width, CV_8UC2, (void *)p);
		cv::cvtColor(img, img, cv::COLOR_YUV2BGR_YUY2);
		if (*bayer_flag == CV_MONO_FLG && img.type() != CV_8UC1)
			cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
		share_img = img;
	}

#ifdef HAVE_OPENCV_CUDA_SUPPORT
	gpu_img.upload(share_img);
	group_gpu_image_proc(gpu_img);
	gpu_img.download(share_img);
#else
	group_gpu_image_proc(share_img);
#endif 

	if (*flip_flag)
		cv::flip(share_img, share_img, 0);
	if (*mirror_flag)
		cv::flip(share_img, share_img, 1);
	if (*save_bmp)
		save_frame_image_bmp(share_img);

	if (*separate_dual) 
		display_dual_stereo_separately(share_img);
	else 
	{
		cv::destroyWindow("cam_left");
		cv::destroyWindow("cam_right");
	}
	/** if image larger than 720p by any dimension, resize the window */
	//if (*resize_window_ena) 
		if (width >= CROPPED_WIDTH || height >= CROPPED_HEIGHT)
			cv::resizeWindow(window_name, CROPPED_WIDTH, CROPPED_HEIGHT);
	if (*display_info_ena)
		display_current_mat_stream_info(share_img, cur_time);

	cv::imshow(window_name, share_img);
	switch_on_keys();

}


/**
 * set rgb ir color correction flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void rgb_ir_correction_enable(int enable)
{
	enable_wrapper(rgb_ir_color, enable);
}

/**
 *  Bayer color correction for RGB-IR sensor
 *
 * This handle the conversion for bayer pattern that have infrared instead
 * of a second green pixel. 
 * The original pattern look like this:
 * R  IR or IR R  or B  G  or G  B
 * G  B     B  G     IR R     R  IR
 * 
 * change into this:
 * R' G' or G' R' or B' G' or G' B'
 * G' B'    B' G'    G' R'    R' G' 
 *
 * Procedure:
 * 1. get individual bayer mosaic color channel
 * 2. substract IR from red, green, blue color channel
 * 		R' = R - IR * ir_r
 * 		G' = G - IR * ir_g
 * 		B' = B - IR * ir_b
 * , where ir_r, ir_g, ir_b are pre-calibrated IR removal coefficient,
 * ask sensor vendor for these coefficients
 * 3. Need to apply AWB, gamma correction, black level correction beforeheads
 * 4. Put the processed image into OpenCV for normal bayer pattern debayer
 * args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 */
void apply_color_correction_rgb_ir(
	struct device *dev, 
	const void *p)
{
	int pixel_max_val = BIT(*bpp) - 1;
	int height = dev->height;
	int width = dev->width;
	uint16_t b_orig, b_proc;
	uint16_t g_orig, g_proc;
	uint16_t r_orig, r_proc;
	uint16_t ir_orig;

	float ir_r = 1.3;
	float ir_g = 0.35;
	float ir_b = 0.3;

	uint16_t *src_short = (uint16_t *)p;
	uint16_t *dst = (uint16_t *)p;

	for (int i = 0; i < height; i += 2)
	{
		for (int j = 0; j < width; j += 2)
		{
			// IR
			ir_orig = src_short[BOTTOM(j, i, width)];
			ir_orig = set_limit(ir_orig, pixel_max_val, 0);

			// B
			b_orig = src_short[PIX(j, i, width)];
			b_orig = set_limit(b_orig, pixel_max_val, 0);
			b_proc = b_orig - ir_orig * ir_b;
			b_proc = set_limit(b_proc, pixel_max_val, 0);
			dst[PIX(j, i, width)] = b_proc;

			// G
			g_orig = src_short[RIGHT(j, i, width)];
			g_orig = set_limit(g_orig, pixel_max_val, 0);
			g_proc = g_orig - ir_orig * ir_g;
			g_proc = set_limit(g_proc, pixel_max_val, 0);
			dst[RIGHT(j, i, width)] = g_proc;
			dst[BOTTOM(j, i, width)] = g_proc;

			// R
			r_orig = src_short[BOTTOM_RIGHT(j, i, width)];
			r_orig = set_limit(r_orig, pixel_max_val, 0);
			r_proc = r_orig - ir_orig * ir_r;
			r_proc = set_limit(r_proc, pixel_max_val, 0);
			dst[BOTTOM_RIGHT(j, i, width)] = r_proc;
		}
	}
	
}

/**
 * set rgb ir ir channel flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void rgb_ir_ir_display_enable(int enable)
{
	enable_wrapper(rgb_ir_ir, enable);
}
/** display the IR channel in a RGB-IR raw camera,
 *  the size of the image will be usually 1/4 of the full resolution
 *  (depend on the RGB-IR sensor's CCM)
 *  args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 */
void display_rgbir_ir_channel(
	struct device *dev, 
	const void *p)
{
	int pixel_max_val = BIT(*bpp) - 1;
	int height = dev->height;
	int width = dev->width;

	uint16_t ir_orig1, ir_orig2, ir_orig3, ir_orig4;
	uint16_t *src_short = (uint16_t *)p;
	uint16_t *dst = (uint16_t *)p;
	for (int i=0; i < height; i+=4) {
		for (int j=0; j < width; j+=4) {
			// IR1
			ir_orig1 = src_short[BOTTOM_RIGHT(j,i,width)];
			ir_orig1 = set_limit(ir_orig1, pixel_max_val, 0);
			//dst[PIX(j/2, i/2, width/2)] = ir_orig1;
			dst[PIX(j, i, width)] = ir_orig1;
			dst[RIGHT(j, i, width)] = ir_orig1;
			dst[BOTTOM(j, i, width)] = ir_orig1;
			// IR2
			ir_orig2 = src_short[BOTTOM_RIGHT(j,i+2,width)];
			ir_orig2 = set_limit(ir_orig2, pixel_max_val, 0);
			dst[PIX(j, i+2, width)] = ir_orig2;
			dst[RIGHT(j, i+2, width)] = ir_orig2;
			dst[BOTTOM(j, i+2, width)] = ir_orig2;
			//dst[PIX(j/2, i/2+1, width/2)] = ir_orig2;
			// IR3
			ir_orig3 = src_short[BOTTOM_RIGHT(j+2,i,width)];
			ir_orig3 = set_limit(ir_orig3, pixel_max_val, 0);
			//dst[PIX(j/2+1, i/2, width/2)] = ir_orig3;
			dst[PIX(j+2, i, width)] = ir_orig3;
			dst[RIGHT(j+2, i, width)] = ir_orig3;
			dst[BOTTOM(j+2, i, width)] = ir_orig3;
			// IR4
			ir_orig4 = src_short[BOTTOM_RIGHT(j+2,i+2,width)];
			ir_orig4 = set_limit(ir_orig4, pixel_max_val, 0);
			//dst[PIX(j/2+1, i/2+1, width/2)] = ir_orig4;		
			dst[PIX(j+2, i+2, width)] = ir_orig4;	
			dst[RIGHT(j+2, i+2, width)] = ir_orig4;
			dst[BOTTOM(j+2, i+2, width)] = ir_orig4;
		}
	}
	perform_shift(dev, dst,  set_shift(*bpp));
	//cv::Mat rgbir_ir(height/2, width/2, CV_8UC1, dst);
	cv::Mat rgbir_ir(height, width, CV_8UC1, dst);
	cv::cvtColor(rgbir_ir, rgbir_ir, cv::COLOR_BayerBG2BGR);
	cv::cvtColor(rgbir_ir, rgbir_ir, cv::COLOR_BGR2GRAY);
	//cv::pyrUp(rgbir_ir, rgbir_ir,cv::Size(rgbir_ir.cols*2, rgbir_ir.rows*2));
	*gamma_val = 0.45;
	//*blc = 64;
	*sharpness = 10;
	apply_gamma_correction(rgbir_ir, *gamma_val);
	sharpness_control(rgbir_ir, *sharpness);
	cv::imshow("RGB-IR IR Channel", rgbir_ir);

	
}

/**
 * request, allocate and map buffers
 * 
 * args: 
 * 		struct device *dev - put buffers in
 * 		nbufs - number of buffer request, map
 * returns: 
 * 		errno 
 * 
 */
int video_alloc_buffers(
	struct device *dev, 
	int nbufs)
{
	struct buffer *buffers;

	/** request buffer */
	struct v4l2_requestbuffers bufrequest;
	struct v4l2_buffer querybuffer;
	CLEAR(bufrequest);
	bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufrequest.memory = V4L2_MEMORY_MMAP;
	bufrequest.count = nbufs;
	dev->nbufs = nbufs;
	dev->memtype = V4L2_MEMORY_MMAP;
	dev->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	int ret;
	ret = ioctl(dev->fd, VIDIOC_REQBUFS, &bufrequest);
	if (ret < 0)
	{
		printf("Unable to request buffers: %d.\n", errno);
		return ret;
	}
	printf("%u buffers requested.\n", bufrequest.count);

	/** allocate buffer */
	buffers = (buffer *)malloc(bufrequest.count * sizeof buffers[0]);
	dev->buffers = buffers;

	if (buffers == NULL)
		return -ENOMEM;

	/** map the buffers */
	for (size_t i = 0; i < dev->nbufs; i++)
	{
		CLEAR(querybuffer);
		querybuffer.type = bufrequest.type;
		querybuffer.memory = V4L2_MEMORY_MMAP;
		querybuffer.index = i;

		ret = ioctl(dev->fd, VIDIOC_QUERYBUF, &querybuffer);
		if (ret < 0)
		{
			printf("Unable to query buffer %zu (%d).\n", i, errno);
			return ret;
		}
		printf("length: %u offset: %u\n", querybuffer.length,
			   querybuffer.m.offset);

		buffers[i].length = querybuffer.length; /** remember for munmap() */

		buffers[i].start = mmap(
			NULL, 
			querybuffer.length, 
			PROT_READ | PROT_WRITE,
		 	MAP_SHARED, 
			dev->fd, 
			querybuffer.m.offset);

		if (buffers[i].start == MAP_FAILED)
		{
			/** If you do not exit here you should unmap() and free()
           	the buffers mapped so far. */
			printf("Unable to map buffer %zu (%d)\n", i, errno);
			return ret;
		}

		printf("Buffer mapped at address %p.\n", buffers[i].start);

		CLEAR(queuebuffer);
		queuebuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		queuebuffer.memory = V4L2_MEMORY_MMAP;
		queuebuffer.index = i; /** Queueing buffer index i. */

		/** Put the buffer in the incoming queue. */
		ret = ioctl(dev->fd, VIDIOC_QBUF, &queuebuffer);
		if (ret < 0)
		{
			printf("Unable to queue the buffer %d\n", errno);
			return ret;
		}
	}
	return 0;
}

/**
 * free, unmap buffers
 * 
 * args: 
 * 		struct device *dev - put buffers in
 * 
 * returns: 
 * 		errno 
 * 
 */
int video_free_buffers(struct device *dev)
{
	struct v4l2_requestbuffers requestbuf;
	int ret;

	if (dev->nbufs == 0)
		return 0;

	for (size_t i = 0; i < dev->nbufs; ++i)
	{
		ret = munmap(dev->buffers[i].start, dev->buffers[i].length);
		if (ret < 0)
		{
			printf("Unable to unmap buffer %zu (%d)\n", i, errno);
			return ret;
		}
	}

	CLEAR(requestbuf);
	requestbuf.count = 0;
	requestbuf.type = dev->type;
	requestbuf.memory = dev->memtype;

	ret = ioctl(dev->fd, VIDIOC_REQBUFS, &requestbuf);
	if (ret < 0)
	{
		printf("Unable to release buffers: %d.\n", errno);
		fflush(stdout);
		return ret;
	}

	printf("%u buffers released.\n", dev->nbufs);

	free(dev->buffers);
	dev->buffers = NULL;
	dev->nbufs = 0;

	return 0;
}

/**
 * callback for exit streaming from gui
 */
void set_loop(int exit)
{
	*loop = exit;
}

