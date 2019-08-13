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

  This is the sample code for Leopard USB3.0 camera use v4l2 and OpenCV for 
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
  Last edit: 2019/07
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/extend_cam_ctrl.h"

/** Include files to use OpenCV API */
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef HAVE_OPENCV_CUDA_SUPPORT
#include <opencv4/opencv2/cudaobjdetect.hpp>
#include <opencv4/opencv2/cudaimgproc.hpp>
#include <opencv4/opencv2/cudafilters.hpp>
#include <opencv4/opencv2/cudaarithm.hpp>
#endif

#include <omp.h> /**for openmp */

/*****************************************************************************
**                      	Global variables 
*****************************************************************************/
/** 
 * Since we use fork, variables btw two processes are not shared
 * use mmap for all variables you need to share between gui, videostreaming
 */
static int *save_bmp;				/// flag for saving bmp
static int *save_raw;				/// flag for saving raw
static int *bayer_flag;				/// flag for choosing bayer pattern
static int *bpp;					/// flag for datatype bits per pixel
static int *awb_flag;				/// flag for enable awb
static int *clahe_flag;				/// flag for enable CLAHE
static int *abc_flag;				/// flag for enable auto brightness&contrast
static float *gamma_val;			/// gamma correction value from gui
static int *black_level_correction; /// black level correction value from gui
static int *loop;					/// while (*loop)
static int *rgb_gain_offset_flag;   /// flag for rgb gain and offset enable
static int *r_gain;					/// values for rgb gain, offset correction
static int *b_gain;
static int *g_gain;
static int *r_offset;
static int *b_offset;
static int *g_offset;
static int *rgb_matrix_flag; 		/// flag for rgb2rgb matrix enable
static int *rr, *rg, *rb;			/// values for rgb2rgb matrix
static int *gr, *gg, *gb;
static int *br, *bg, *bb;
static int *soft_ae_flag; 			/// flag for software AE
static int *flip_flag, *mirror_flag;
static int *show_edge_flag;
static int *rgb_ir_color, *rgb_ir_ir;
static int *separate_dual_display;
static int *display_mat_info_ena;
static int *resize_window_ena;

static int *alpha, *beta, *sharpness;
static int *edge_low_thres;

int *cur_exposure; 					/// update current exposure for AE
int *cur_gain;	 					/// update current gain for AE

static int image_count;				/// image count number add to capture name
double cur_time = 0;				/// time measured in OpenCV for fps
struct v4l2_buffer queuebuffer; 	/// queuebuffer for enqueue, dequeue buffer
static constexpr const char* window_name = "Camera View"; 
/*****************************************************************************
**                      	External Callbacks
*****************************************************************************/
extern char *get_product(); 		/// put product name in captured image name
/// for software ae
extern void set_gain(int fd, int analog_gain);
extern void set_exposure_absolute(int fd, int exposure_absolute);
extern int get_current_height(int fd);
/******************************************************************************
**                           Function definition
*****************************************************************************/

/**
 * resize opencv image
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void resize_window_enable(int enable)
{
	switch (enable)
	{
	case 1:
		*resize_window_ena = TRUE;
		break;
	case 0:
		*resize_window_ena= FALSE;
		break;
	default:
		*resize_window_ena = TRUE;
		break;
	}
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
int v4l2_core_save_data_to_file(const void *data, int size)
{
	FILE *fp;
	int ret = 0;
	char buf_name[30];
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
static int save_frame_image_bmp(cv::InputOutputArray& opencvImage)
{

	printf("save one capture bmp\n");
	cv::imwrite(cv::format("captures_%s_%d.bmp",
						   get_product(), image_count),
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
inline int set_shift(int *bpp)
{
	int cmp = *bpp;
	switch (cmp)
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
 * callback for change sensor datatype shift flag
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
 * determine the sensor bayer pattern to correctly debayer the image
 *   CV_BayerBG2BGR =46   -> bayer_flag_increment = 0
 *   CV_BayerGB2BGR =47   -> bayer_flag_increment = 1
 *   CV_BayerRG2BGR =48   -> bayer_flag_increment = 2
 *   CV_BayerGR2BGR =49	  -> bayer_flag_increment = 3
 * default bayer pattern: RGGB
 * args:
 * 		bayer_flag - flag for determining bayer pattern 
 * returns:
 * 		bayer pattern flag increment
 */
inline int add_bayer_forcv(int *bayer_flag)
{

	int cmp = *bayer_flag;
	switch (cmp)
	{
	case CV_BayerBG2BGR_FLG:
		return CV_BayerBG2BGR_FLG-1;
	case CV_BayerGB2BGR_FLG:
		return CV_BayerGB2BGR_FLG-1;
	case CV_BayerRG2BGR_FLG:
		return CV_BayerRG2BGR_FLG-1;
	case CV_BayerGR2BGR_FLG:
		return CV_BayerGR2BGR_FLG-1;
	case CV_MONO_FLG:
		return CV_MONO_FLG-1;
	default:
		return CV_BayerBG2BGR_FLG-1;
	}
}

/**
 * callback for change sensor bayer pattern for debayering
 * args:
 * 		bayer - for updating *bayer_flag
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

/** callback for set black_level_correction from gui */
void add_black_level_correction(int blc_val_from_gui)
{
	*black_level_correction = blc_val_from_gui;
}

/** callback for set gamma_val for gamma correction from gui */
void add_gamma_val(float gamma_val_from_gui)
{
	*gamma_val = gamma_val_from_gui;
}

/** 
 *  Apply auto white balance in-place for a given image array
 *  When *gamma_val < 1, the original dark regions will be brighter 
 *  and the histogram will be shifted to the right 
 *  whereas it will be the opposite with *gamma_val > 1
 *  recommend *gamma_val: 0.45(1/2.2)
 *  args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 */
static void apply_gamma_correction(cv::InputOutputArray& opencvImage)
{
	cv::Mat look_up_table(1, 256, CV_8U);
	uchar *p = look_up_table.ptr();
	for (int i = 0; i < 256; i++)
	{
		p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, *gamma_val) * 255.0);
	}
#ifdef HAVE_OPENCV_CUDA_SUPPORT
	cv::Ptr<cv::cuda::LookUpTable> lutAlg =
		cv::cuda::createLookUpTable(look_up_table);
	lutAlg->transform(opencvImage, opencvImage);
#else
	LUT(opencvImage, look_up_table, opencvImage);
#endif
}

/**
 * set awb flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void awb_enable(int enable)
{
	switch (enable)
	{
	case 1:
		*awb_flag = TRUE;
		break;
	case 0:
		*awb_flag = FALSE;
		break;
	default:
		*awb_flag = FALSE;
		break;
	}
}

/** 
 *  Apply auto white balance in-place for a given image array
 *  the basic idea of Leopard AWB algorithm is to find the gray
 *  area of the image and apply Red, Green and Blue gains to make
 *  it gray, and then use the gray area to estimate the color 
 *  temperature.
 *  args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 */
static void apply_white_balance(cv::InputOutputArray& opencvImage)
{
	/// if it is grey image, do nothing
	if (opencvImage.type() == CV_8UC1)
		return;
#ifdef HAVE_OPENCV_CUDA_SUPPORT
	cv::cuda::GpuMat _opencvImage = opencvImage.getGpuMat();
	std::vector<cv::cuda::GpuMat> bgr_planes;
	cv::cuda::split(_opencvImage, bgr_planes);
	cv::cuda::equalizeHist(bgr_planes[0], bgr_planes[0]);
	cv::cuda::equalizeHist(bgr_planes[1], bgr_planes[1]);
	cv::cuda::equalizeHist(bgr_planes[2], bgr_planes[2]);
	cv::cuda::merge(bgr_planes, _opencvImage);
#else
	/// ref: https://gist.github.com/tomykaira/94472e9f4921ec2cf582
	cv::Mat _opencvImage = opencvImage.getMat();
	double discard_ratio = 0.05;
	int hists[3][256];
	CLEAR(hists);

	for (int y = 0; y < _opencvImage.rows; ++y)
	{
		uchar *ptr = _opencvImage.ptr<uchar>(y);
		for (int x = 0; x < _opencvImage.cols; ++x)
		{
			for (int j = 0; j < 3; ++j)
			{
				hists[j][ptr[x * 3 + j]] += 1;
			}
		}
	}

	/// cumulative hist
	int total = _opencvImage.cols * _opencvImage.rows;
	int vmin[3], vmax[3];
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 255; ++j)
		{
			hists[i][j + 1] += hists[i][j];
		}
		vmin[i] = 0;
		vmax[i] = 255;
		while (hists[i][vmin[i]] < discard_ratio * total)
			vmin[i] += 1;
		while (hists[i][vmax[i]] > (1 - discard_ratio) * total)
			vmax[i] -= 1;
		if (vmax[i] < 255 - 1)
			vmax[i] += 1;
	}

	for (int y = 0; y < _opencvImage.rows; ++y)
	{
		uchar *ptr = _opencvImage.ptr<uchar>(y);
		for (int x = 0; x < _opencvImage.cols; ++x)
		{
			for (int j = 0; j < 3; ++j)
			{
				int val = ptr[x * 3 + j];
				if (val < vmin[j])
					val = vmin[j];
				if (val > vmax[j])
					val = vmax[j];
				ptr[x * 3 + j] = static_cast<uchar>((val - vmin[j]) * 255.0 / (vmax[j] - vmin[j]));
			}
		}
	}
#endif
}
/**
 * set abc flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void abc_enable(int enable)
{
	switch (enable)
	{
	case 1:
		*abc_flag = TRUE;
		break;
	case 0:
		*abc_flag = FALSE;
		break;
	default:
		*abc_flag = FALSE;
		break;
	}
}

/**
 * set CLAHE flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void clahe_enable(int enable)
{
	switch (enable)
	{
	case 1:
		*clahe_flag = TRUE;
		break;
	case 0:
		*clahe_flag = FALSE;
		break;
	default:
		*clahe_flag = FALSE;
		break;
	}
}

/** 
 * In-place automatic brightness and contrast optimization with 
 * optional histogram clipping. Looking at histogram, alpha operates 
 * as color range amplifier, beta operates as range shift.
 * O(x,y) = alpha * I(x,y) + beta
 * Automatic brightness and contrast optimization calculates
 * alpha and beta so that the output range is 0..255.
 * Ref: http://answers.opencv.org/question/75510/how-to-make-auto-adjustmentsbrightness-and-contrast-for-image-android-opencv-image-correction/
 * args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 * 		float clipHistPercent 			 - cut wings of histogram at given percent
 * 			typical=>1, 0=>Disabled
 */

static void apply_auto_brightness_and_contrast(
	cv::InputOutputArray& opencvImage,
	float clipHistPercent = 0)
{
	int hist_size = 256;
	float alpha, beta;
	double min_gray = 0, max_gray = 0;

	/// to calculate grayscale histogram
	cv::Mat gray;
	if (opencvImage.type() != CV_8UC1)
		cv::cvtColor(opencvImage, gray, cv::COLOR_BGR2GRAY);
	else 
		gray = opencvImage.getMat();

	if (clipHistPercent == 0)
	{
		/// keep full available range
		cv::minMaxLoc(gray, &min_gray, &max_gray);
	}
	else
	{
		/// the grayscale histogram
		cv::Mat hist;

		float range[] = {0, 256};
		const float *histRange = {range};
		bool uniform = true;
		bool accumulate = false;
		// void calcHist(img_orig, n_images, channels(gray=0), mask(for ROi),
		//               mat hist, dimemsion, histSize=bins=256,
		//               ranges_for_pixel, bool uniform, bool accumulate);
		calcHist(&gray, 1, 0, cv::Mat(), hist, 1,
				 &hist_size, &histRange, uniform, accumulate);

		/// calculate cumulative distribution from the histogram
		std::vector<float> accumulator(hist_size);
		accumulator[0] = hist.at<float>(0);
		for (int i = 1; i < hist_size; i++)
		{
			accumulator[i] = accumulator[i - 1] + hist.at<float>(i);
		}

		/// locate points that cuts at required value
		float max = accumulator.back();
		clipHistPercent *= (max / 100.0); /// make percent as absolute
		clipHistPercent /= 2.0;			  /// left and right wings
		/// locate left cut
		min_gray = 0;
		while (accumulator[min_gray] < clipHistPercent)
			min_gray++;

		/// locate right cut
		max_gray = hist_size - 1;
		while (accumulator[max_gray] >= (max - clipHistPercent))
			max_gray--;
	}

	/// current range
	float input_range = max_gray - min_gray;
	/// alpha expands current range to histsize range
	alpha = (hist_size - 1) / input_range;
	/// beta shifts current range so that minGray will go to 0
	beta = -min_gray * alpha;

	/**
	 * Apply brightness and contrast normalization
	 * convertTo operates with saturate_cast
	 */
	cv::Mat _opencvImage = opencvImage.getMat();
	_opencvImage.convertTo(opencvImage, -1, alpha, beta);

}

/** 
 * Contrast Limited Adaptive Histogram Equalization) algorithm
 * ref: https://stackoverflow.com/questions/24341114/simple-illumination-correction-in-images-opencv-c
 */
static void apply_clahe(
	cv::InputOutputArray& opencvImage)
{

#ifdef HAVE_OPENCV_CUDA_SUPPORT
	
	if (opencvImage.type() != CV_8UC1)
	{
		cv::cuda::GpuMat lab_image;
		cv::cuda::cvtColor(opencvImage, lab_image, cv::COLOR_BGR2Lab);
		/// Extract the L channel
		std::vector<cv::cuda::GpuMat> lab_planes(3);
		cv::cuda::split(lab_image, lab_planes); // now we have the L image in lab_planes[0]

		/// apply the CLAHE algorithm to the L channel
		cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE();
		clahe->setClipLimit(4);
		cv::cuda::GpuMat dst;
		clahe->apply(lab_planes[0], dst);

		/// Merge the the color planes back into an Lab image
		dst.copyTo(lab_planes[0]);
		cv::cuda::merge(lab_planes, lab_image);

		/// convert back to RGB
		cv::cuda::cvtColor(lab_image, opencvImage, cv::COLOR_Lab2BGR);
	}
	else
	{
		cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE();
		clahe->setClipLimit(4);
		clahe->apply(opencvImage, opencvImage);		
	}
	
#else
	if (opencvImage.type() != CV_8UC1)
	{
		cv::Mat lab_image;
		cv::cvtColor(opencvImage, lab_image, cv::COLOR_BGR2Lab);
		/// Extract the L channel
		std::vector<cv::Mat> lab_planes(3);
		/// now we have the L image in lab_planes[0]
		cv::split(lab_image, lab_planes);

		/// apply the CLAHE algorithm to the L channel
		cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
		clahe->setClipLimit(4);
		cv::Mat dst;
		clahe->apply(lab_planes[0], dst);

		/// Merge the the color planes back into an Lab image
		dst.copyTo(lab_planes[0]);
		cv::merge(lab_planes, lab_image);

		/// convert back to RGB
		cv::cvtColor(lab_image, opencvImage, cv::COLOR_Lab2BGR);
	}
	else 
	{
		cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
		clahe->setClipLimit(4);
		clahe->apply(opencvImage, opencvImage);
	}
#endif

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
int open_v4l2_device(char *device_name, struct device *dev)
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

	mmap_variables();
	initialize_shared_memory_var();
	return v4l2_dev;
}

/** 
 * retrive device's capabilities
 * 
 * args: 
 * 		struct device *dev - device infomation
 * returns: 
 * 		error value
 */
int check_dev_cap(struct device *dev)
{
	struct v4l2_capability cap;
	CLEAR(cap);
	int ret;
	ret = (ioctl(dev->fd, VIDIOC_QUERYCAP, &cap));
	if (ret < 0)
	{
		printf("VIDIOC_QUERYCAP error:%s\n", strerror(errno));
		return -1;
	}

	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0)
	{
		printf("Error opening device: video capture not supported.\n");
		return -1;
	}
	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		printf("Does not support streaming\n");

		return -1;
	}
	if (!(cap.capabilities & V4L2_CAP_READWRITE))
	{
		printf("Does not support read, try with mmap\n");
		return -1;
	}
	return 0;
}

/** mmap the variables for processes share */
void mmap_variables()
{
	save_bmp = (int *)mmap(NULL, sizeof *save_bmp, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	save_raw = (int *)mmap(NULL, sizeof *save_raw, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	bpp = (int *)mmap(NULL, sizeof *bpp, PROT_READ | PROT_WRITE,
					  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	bayer_flag = (int *)mmap(NULL, sizeof *bayer_flag, PROT_READ | PROT_WRITE,
							 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	awb_flag = (int *)mmap(NULL, sizeof *awb_flag, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	clahe_flag = (int *)mmap(NULL, sizeof *clahe_flag, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	abc_flag = (int *)mmap(NULL, sizeof *abc_flag, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	gamma_val = (float *)mmap(NULL, sizeof *gamma_val, PROT_READ | PROT_WRITE,
							  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	black_level_correction = (int *)mmap(NULL, sizeof *black_level_correction,
										 PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	loop = (int *)mmap(NULL, sizeof *loop, PROT_READ | PROT_WRITE,
					   MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	rgb_gain_offset_flag = (int *)mmap(NULL, sizeof *rgb_gain_offset_flag,
									   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	rgb_matrix_flag = (int *)mmap(NULL, sizeof *rgb_matrix_flag,
								  PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	r_gain = (int *)mmap(NULL, sizeof *r_gain, PROT_READ | PROT_WRITE,
						 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	g_gain = (int *)mmap(NULL, sizeof *g_gain, PROT_READ | PROT_WRITE,
						 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	b_gain = (int *)mmap(NULL, sizeof *b_gain, PROT_READ | PROT_WRITE,
						 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	r_offset = (int *)mmap(NULL, sizeof *r_offset, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	g_offset = (int *)mmap(NULL, sizeof *g_offset, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	b_offset = (int *)mmap(NULL, sizeof *b_offset, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	rr = (int *)mmap(NULL, sizeof *rr, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	rg = (int *)mmap(NULL, sizeof *rg, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	rb = (int *)mmap(NULL, sizeof *rb, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	gr = (int *)mmap(NULL, sizeof *gr, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	gg = (int *)mmap(NULL, sizeof *gg, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	gb = (int *)mmap(NULL, sizeof *gb, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	br = (int *)mmap(NULL, sizeof *br, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	bg = (int *)mmap(NULL, sizeof *bg, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	bb = (int *)mmap(NULL, sizeof *bb, PROT_READ | PROT_WRITE,
					 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	cur_exposure = (int *)mmap(NULL, sizeof *cur_exposure, PROT_READ | PROT_WRITE,
							   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	cur_gain = (int *)mmap(NULL, sizeof *cur_gain, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);


	soft_ae_flag = (int *)mmap(NULL, sizeof *soft_ae_flag, PROT_READ | PROT_WRITE,
							   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	flip_flag = (int *)mmap(NULL, sizeof *flip_flag, PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	mirror_flag = (int *)mmap(NULL, sizeof *mirror_flag, PROT_READ | PROT_WRITE,
		 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	show_edge_flag = (int *)mmap(NULL, sizeof *show_edge_flag, 
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);	
	
	rgb_ir_color = (int *)mmap(NULL, sizeof *rgb_ir_color,
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	rgb_ir_ir = (int *)mmap(NULL, sizeof *rgb_ir_ir,
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	separate_dual_display = (int *)mmap(NULL, sizeof *separate_dual_display,
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	display_mat_info_ena = (int *)mmap(NULL, sizeof *display_mat_info_ena,
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	alpha = (int *)mmap(NULL, sizeof *alpha, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	beta = (int *)mmap(NULL, sizeof *beta, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	sharpness = (int *)mmap(NULL, sizeof *sharpness, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	edge_low_thres = (int *)mmap(NULL, sizeof *edge_low_thres, 
	PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	resize_window_ena = (int *)mmap(NULL, sizeof *resize_window_ena,
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

}
/** initialize share memory variables after declaration */
void initialize_shared_memory_var()
{
	*gamma_val = 1.0;
	*black_level_correction = 0;
	*bpp = RAW10_FLG;
	*bayer_flag = CV_BayerBG2BGR_FLG;
	*display_mat_info_ena = TRUE;
	*alpha = 1;
	*beta = 0;
	*sharpness = 1;
}
/** unmap all the variables after stream ends */
void unmap_variables()
{
	munmap(save_bmp, sizeof *save_bmp);
	munmap(save_raw, sizeof *save_raw);
	munmap(bpp, sizeof *bpp);
	munmap(bayer_flag, sizeof *bayer_flag);
	munmap(awb_flag, sizeof *awb_flag);
	munmap(clahe_flag, sizeof *clahe_flag);
	munmap(abc_flag, sizeof *abc_flag);
	munmap(gamma_val, sizeof *gamma_val);
	munmap(black_level_correction, sizeof *black_level_correction);
	munmap(loop, sizeof *loop);

	munmap(rgb_gain_offset_flag, sizeof *rgb_gain_offset_flag);
	munmap(r_gain, sizeof *r_gain);
	munmap(g_gain, sizeof *g_gain);
	munmap(b_gain, sizeof *b_gain);
	munmap(r_offset, sizeof *r_offset);
	munmap(g_offset, sizeof *g_offset);
	munmap(b_offset, sizeof *b_offset);

	munmap(rgb_matrix_flag, sizeof *rgb_matrix_flag);
	munmap(rr, sizeof *rr);
	munmap(rg, sizeof *rg);
	munmap(rb, sizeof *rb);
	munmap(gr, sizeof *gr);
	munmap(gg, sizeof *gg);
	munmap(gb, sizeof *gb);
	munmap(br, sizeof *br);
	munmap(bg, sizeof *bg);
	munmap(bb, sizeof *bb);

	munmap(cur_exposure, sizeof *cur_exposure);
	munmap(cur_gain, sizeof *cur_gain);

	munmap(soft_ae_flag, sizeof *soft_ae_flag);
	munmap(flip_flag, sizeof *flip_flag);
	munmap(mirror_flag, sizeof *mirror_flag);
	munmap(show_edge_flag, sizeof *show_edge_flag);

	munmap(rgb_ir_color, sizeof *rgb_ir_color);
	munmap(rgb_ir_ir, sizeof *rgb_ir_ir);
	munmap(separate_dual_display, sizeof *separate_dual_display);
	munmap(display_mat_info_ena, sizeof *display_mat_info_ena);

	munmap(alpha, sizeof *alpha);
	munmap(beta, sizeof *beta);
	munmap(sharpness, sizeof *sharpness);
	munmap(edge_low_thres, sizeof *edge_low_thres);
	
	munmap(resize_window_ena, sizeof *resize_window_ena);

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
 * 		struct device *dev - device infomation
 * 	  	width - resolution width
 * 		height - resolution height
 * 		pixelformat - V4L2_PIX_FMT_YUYV
 * 
 */
void video_set_format(struct device *dev, int width,
					  int height, int pixelformat)
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
	for (unsigned int i = 0; i < dev->nbufs; i++)
	{
		cur_time = (double)cv::getTickCount();
		queuebuffer.index = i;

		/// The buffer's waiting in the outgoing queue
		if (ioctl(dev->fd, VIDIOC_DQBUF, &queuebuffer) < 0)
		{
			perror("VIDIOC_DQBUF");
			return;
		}

		/// check the capture raw image flag, do this before decode a frame
		if (*(save_raw))
			v4l2_core_save_data_to_file(dev->buffers[i].start, dev->imagesize);

		decode_process_a_frame(dev, dev->buffers[i].start);

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
	switch (enable)
	{
	case 1:
		*soft_ae_flag = TRUE;
		break;
	case 0:
		*soft_ae_flag = FALSE;
		break;
	default:
		*soft_ae_flag = FALSE;
		break;
	}
}
/**
 * calculate the mean value of a given unprocessed image for a defined ROI
 * return the value used for software AE
 * args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 */
double calc_mean(struct device *dev, const void *p)
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
void apply_soft_ae(struct device *dev, const void *p)
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
 * 		struct device *dev - every infomation for camera
 * 		const void *p 		- camera streaming buffer pointer
 * 		int shift 			- shift bits
 * */
void perform_shift(struct device *dev, const void *p, int shift)
{
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
	for (unsigned int i = 0; i < dev->height; i++)
	{
		for (unsigned int j = 0; j < dev->width; j++)
		{
			ts = *(src_short++);
			if (ts > *black_level_correction)
				tmp = (ts - *black_level_correction) >> shift;
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

	for (unsigned int i = 0; i < dev->height; i++)
	{
		for (unsigned int j = 0; j < dev->width; j++)
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

	for (unsigned int i = 0; i < dev->height/2; i++)
	{
		for (unsigned int j = 0; j < dev->width/2; j++)
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
void enable_rgb_gain_offset(int red_gain, int green_gain, int blue_gain,
							int red_offset, int green_offset, int blue_offset)
{
	*r_gain = red_gain;
	*g_gain = green_gain;
	*b_gain = blue_gain;
	*r_offset = red_offset;
	*g_offset = green_offset;
	*b_offset = blue_offset;
	*rgb_gain_offset_flag = 1;

	printf("r gain = %d\tg gain = %d\tb gain = %d\r\n"
		   "r offset = %d\tg offset = %d\tb offset = %d\r\n ",
		   *r_gain, *g_gain, *b_gain, *r_offset, *g_offset, *b_offset);
}

/**
 * disable the flag on rgb_gain_offset
 */
void disable_rgb_gain_offset()
{
	*rgb_gain_offset_flag = 0;
}
/**
 * apply rgb gain and offset color correction before debayering
 * only support four bayer patterns now: GRBG, GBRG, RGGB, BGGR
 */
void apply_rgb_gain_offset_pre_debayer(struct device *dev, const void *p)
{
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
 */
void enable_rgb_matrix(int red_red, int red_green, int red_blue,
					   int green_red, int green_green, int green_blue,
					   int blue_red, int blue_green, int blue_blue)
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
	*rgb_matrix_flag = 1;
	printf("rgb matrix enabled\r\n");
	printf("rr = %d\trg = %d\trb = %d\r\n"
		   "gr = %d\tgg = %d\tgb = %d\r\n"
		   "br = %d\tbg = %d\tbb = %d\r\n",
		   *rr, *rg, *rb, *gr, *gg, *gb, *br, *bg, *bb);
}
/**
 * disable the flag on rgb to rgb matrix
 */
void disable_rgb_matrix()
{
	*rgb_matrix_flag = 0;
}

// bgr_planes[0];//blue channel
// bgr_planes[1];//green channel
// bgr_planes[2];//red channel
static void apply_rgb_matrix_post_debayer(cv::InputOutputArray& _opencvImage)
{

#ifdef HAVE_OPENCV_CUDA_SUPPORT
	cv::cuda::GpuMat opencvImage = _opencvImage.getGpuMat();
	cv::cuda::GpuMat bgr_planes[3];
	cv::cuda::GpuMat output[3];
	cv::cuda::split(opencvImage, bgr_planes);
	//TODO: multiply the matrix etc
	//cv::cuda::merge(output, 3, opencvImage);
	cv::cuda::merge(bgr_planes, 3, opencvImage);

#else
	cv::Mat opencvImage = _opencvImage.getMat();
	cv::Mat bgr_planes[3];
	cv::Mat output[3];
	cv::split(opencvImage, bgr_planes);

	output[0] = ((*bb) * bgr_planes[0] + (*bg) * bgr_planes[1] + (*br) * bgr_planes[2]) / 256; //blue
	// FIXME: why every time adding bgr_planes[2] multiple 0 can even make color wrong? - remove all green
	// so red-green and green-red don't work now because I couldn't add in
	//output[1] = ((*gb) * bgr_planes[0] + (*gg) * bgr_planes[1] + (*gr) * bgr_planes[2])/256; // green
	output[1] = ((*gb) * bgr_planes[0] + (*gg) * bgr_planes[1]) / 256;						   // green
	output[2] = ((*rb) * bgr_planes[0] + (*rg) * bgr_planes[1] + (*rr) * bgr_planes[2]) / 256; // red

	cv::merge(output, 3, opencvImage);
#endif
}
/**
 * set flip flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void flip_enable(int enable)
{
	switch (enable)
	{
	case 1:
		*flip_flag = TRUE;
		break;
	case 0:
		*flip_flag = FALSE;
		break;
	default:
		*flip_flag = FALSE;
		break;
	}
}
/**
 * set mirror flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void mirror_enable(int enable)
{
	switch (enable)
	{
	case 1:
		*mirror_flag = TRUE;
		break;
	case 0:
		*mirror_flag = FALSE;
		break;
	default:
		*mirror_flag = FALSE;
		break;
	}
}
/**
 * set canny filter flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void canny_filter_enable(int enable)
{
	switch (enable)
	{
	case 1:
		*show_edge_flag = TRUE;
		break;
	case 0:
		*show_edge_flag = FALSE;
		break;
	default:
		*show_edge_flag = FALSE;
		break;
	}
}

/**
 * set separate dual display enable flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void separate_dual_display_enable(int enable)
{
	switch (enable)
	{
	case 1:
		*separate_dual_display = TRUE;
		break;
	case 0:
		*separate_dual_display = FALSE;
		break;
	default:
		*separate_dual_display = FALSE;
		break;
	}
}
/** separate display right and left mat image from a dual stereo vision camera */
static void display_dual_stereo_separately(cv::InputOutputArray& _opencvImage)
{

	cv::Mat opencvImage = _opencvImage.getMat();
	/// define region of interest for cropped Mat for dual stereo
	cv::Rect roi_left(0, 0, opencvImage.cols/2, opencvImage.rows);
	cv::Mat cropped_ref_left(opencvImage, roi_left);
	cv::Mat cropped_left;
	cropped_ref_left.copyTo(cropped_left);
	cv::imshow("cam_left", cropped_left);
	cv::Rect roi_right(opencvImage.cols/2, 0, 
		opencvImage.cols/2, opencvImage.rows);
	cv::Mat cropped_ref_right(opencvImage, roi_right);
	cv::Mat cropped_right;
	cropped_ref_right.copyTo(cropped_right);
	cv::imshow("cam_right", cropped_right);
}
/**
 * set display mat info flag 
 * args:
 * 		flag - set/reset the flag when get check button toggle
 */
void display_mat_info_enable(int enable)
{
	switch (enable)
	{
	case 1:
		*display_mat_info_ena = TRUE;
		break;
	case 0:
		*display_mat_info_ena = FALSE;
		break;
	default:
		*display_mat_info_ena = TRUE;
		break;
	}
}
/**
 * unify putting text in opencv image
 * a wrapper for put_text()
 */
static void streaming_put_text(cv::Mat opencvImage,
							   const char *str, int cordinate_y)
{
	int scale = opencvImage.cols / 1000;
	cv::putText(opencvImage,
				str,
				cv::Point(scale * TEXT_SCALE_BASE, cordinate_y), // Coordinates
				cv::FONT_HERSHEY_SIMPLEX,						 // Font
				(float)scale,									 // Scale. 2.0 = 2x bigger
				cv::Scalar(255, 255, 255),						 // BGR Color - white
				2												 // Line Thickness (Optional)
	);															 // Anti-alias (Optional)
}
/** put mat info text in: res, fps, ESC*/
static void display_current_mat_stream_info(cv::InputOutputArray& _opencvImage)
{
	cv::Mat opencvImage = _opencvImage.getMat();
	int height_scale = (opencvImage.cols / 1000);
	streaming_put_text(opencvImage, "ESC: close application",
					   height_scale * TEXT_SCALE_BASE);

	char resolution[25];
	sprintf(resolution, "Current Res:%dx%d", opencvImage.rows, opencvImage.cols);
	streaming_put_text(opencvImage, resolution,
					   height_scale * TEXT_SCALE_BASE * 2);

	/** 
  	 * getTickcount: return number of ticks from OS
	 * getTickFrequency: returns the number of ticks per second
	 * t = ((double)getTickCount() - t)/getTickFrequency();
	 * fps is t's reciprocal
	 */
	cur_time = ((double)cv::getTickCount() - cur_time) /
			   cv::getTickFrequency();
	double fps = 1.0 / cur_time;			 // frame rate
	char string_frame_rate[10];				 // string to save fps
	sprintf(string_frame_rate, "%.2f", fps); // to 2 decimal places
	char fpsString[20];
	strcpy(fpsString, "Current Fps:");
	strcat(fpsString, string_frame_rate);
	streaming_put_text(opencvImage, fpsString,
					   height_scale * TEXT_SCALE_BASE * 3);
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
static void apply_brightness_and_contrast(cv::InputOutputArray& _opencvImage)
{
	cv::Mat opencvImage = _opencvImage.getMat();
 	if (opencvImage.type() == CV_8UC3) {
	 	for( int y = 0; y < opencvImage.rows; y++) {
        	for( int x = 0; x < opencvImage.cols; x++) {
            	for( int c = 0; c < 3; c++ ) {
                	opencvImage.at<cv::Vec3b>(y,x)[c] = cv::saturate_cast<uchar>
					((*alpha)*(opencvImage.at<cv::Vec3b>(y,x)[c] ) + (*beta));
				}
			}
		}	 
	}
	else { //mono
		for( int y = 0; y < opencvImage.rows; y++) {
        	for( int x = 0; x < opencvImage.cols; x++) {
                opencvImage.at<uchar>(y,x) = cv::saturate_cast<uchar>
					((*alpha)*(opencvImage.at<uchar>(y,x) ) + (*beta));
			}
		}
	}
}
/** callback for set sharpness for sharpness correction from gui */
void add_sharpness_val(int sharpness_val_from_gui)
{
	*sharpness = sharpness_val_from_gui;
}
/**
 * apply sharpness filter to a given image
 * if OpenCV CUDA is enabled, use createGaussianFilter, 
 */
static void sharpness_control(cv::InputOutputArray& _opencvImage)
{
#ifndef HAVE_OPENCV_CUDA_SUPPORT
	cv::Mat opencvImage = _opencvImage.getMat();
	cv::Mat blurred;
	cv::GaussianBlur(opencvImage, blurred, cv::Size(0, 0), *sharpness);
	cv::addWeighted(opencvImage, 1.5, blurred, -0.5, 0, blurred);
	blurred.copyTo(opencvImage);
#else
	if (*sharpness > 16) *sharpness = 16;
	int ksize = 2*(*sharpness)-1;
	cv::cuda::GpuMat opencvImage = _opencvImage.getGpuMat();
	cv::cuda::GpuMat blurred;
	cv::Ptr<cv::cuda::Filter> filter = 
		cv::cuda::createGaussianFilter(opencvImage.type(),blurred.type(), 
		cv::Size(ksize, ksize),1);
	filter->apply(opencvImage, blurred);
	cv::cuda::addWeighted(opencvImage, 1.5, blurred, -0.5, 0, blurred);
	blurred.copyTo(opencvImage);
#endif
}

void add_edge_thres_val(int edge_low_thres_val_from_gui)
{
	*edge_low_thres = edge_low_thres_val_from_gui;
}
/**
 * Reason not to use OpenCV CUDA acceleration is it doesn't make the process faster
 */
static cv::Mat canny_filter_control(cv::InputOutputArray& _opencvImage)
{

	cv::Mat opencvImage = _opencvImage.getMat();
	const int ratio = 3;
	const int kernel_size = 3;
	cv::Mat edges;
	cv::blur(opencvImage, edges, cv::Size(5,5));
	cv::cvtColor(edges, edges, cv::COLOR_BGR2GRAY);
	cv::Canny(edges, opencvImage, (*edge_low_thres), 
		(*edge_low_thres)*ratio, kernel_size);
	return opencvImage;

}

/**
 * group 3a ctrl flags for bayer cameras
 * auto exposure, auto white balance, CLAHE ctrl
 */
static void group_3a_ctrl_flags_for_raw_camera(
	cv::InputOutputArray& opencvImage)
{
	/** color output */
	if (*bayer_flag != CV_MONO_FLG)
	{
#ifdef HAVE_OPENCV_CUDA_SUPPORT
		// cv::cuda::cvtColor(opencvImage, opencvImage,
		// cv::COLOR_BayerBG2BGR + add_bayer_forcv(bayer_flag));
		cv::cuda::demosaicing(opencvImage, opencvImage,
			cv::cuda::COLOR_BayerBG2BGR_MHT + add_bayer_forcv(bayer_flag));
#else
		cv::cvtColor(opencvImage, opencvImage,
			cv::COLOR_BayerBG2BGR + add_bayer_forcv(bayer_flag));

#endif
		if (*rgb_matrix_flag == TRUE)
			apply_rgb_matrix_post_debayer(opencvImage);
	}
	/** mono output */
	if (*bayer_flag == CV_MONO_FLG && opencvImage.type() == CV_8UC3)
	{
#ifdef HAVE_OPENCV_CUDA_SUPPORT
			cv::cuda::cvtColor(opencvImage, opencvImage, cv::COLOR_BayerBG2BGR);
			cv::cuda::cvtColor(opencvImage, opencvImage, cv::COLOR_BGR2GRAY);
#else
			cv::cvtColor(opencvImage, opencvImage, cv::COLOR_BayerBG2BGR);
			cv::cvtColor(opencvImage, opencvImage, cv::COLOR_BGR2GRAY);	
#endif
	}

	/** check awb flag, awb functionality, only available for bayer camera */
	if (*(awb_flag) == TRUE)
		apply_white_balance(opencvImage);

}

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
void decode_process_a_frame(struct device *dev, const void *p)
{
	int height = dev->height;
	int width = dev->width;
	int shift = set_shift(bpp);
	cv::Mat share_img;
#ifdef HAVE_OPENCV_CUDA_SUPPORT
	cv::cuda::GpuMat gpu_img;
#endif

	if (*soft_ae_flag)
		apply_soft_ae(dev, p);

	/** --- for raw8, raw10, raw12 bayer camera ---*/
	if (shift != 0)
	{
		if (*rgb_gain_offset_flag)
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
		group_3a_ctrl_flags_for_raw_camera(gpu_img);
		gpu_img.download(img);
#else
		group_3a_ctrl_flags_for_raw_camera(img);	
#endif
		share_img = img;
	}

	/** --- for yuv camera ---*/
	else if (shift == 0)
	{	
		//swap_four_bytes(dev, p);
		cv::Mat img(height, width, CV_8UC2, (void *)p);
		cv::cvtColor(img, img, cv::COLOR_YUV2BGR_YUY2);
		if (*bayer_flag == CV_MONO_FLG && img.type() != CV_8UC1)
			cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
		share_img = img;
	}
#ifdef HAVE_OPENCV_CUDA_SUPPORT
	gpu_img.upload(share_img);
	if (*(gamma_val) != (float)1)
		apply_gamma_correction(gpu_img);
	
	if (*sharpness != (float)1)
		sharpness_control(gpu_img);

	if (*clahe_flag)
		apply_clahe(gpu_img);
	gpu_img.download(share_img);
#else
	if (*(gamma_val) != (float)1)
		apply_gamma_correction(share_img);
	
	if (*sharpness != (float)1)
		sharpness_control(share_img);

	if (*clahe_flag)
		apply_clahe(share_img);
#endif 
	if (*(alpha) != (float)1 || (*beta) != (float)0)
		apply_brightness_and_contrast(share_img);
	if (*flip_flag)
		cv::flip(share_img, share_img, 0);
	if (*mirror_flag)
		cv::flip(share_img, share_img, 1);

	if (*abc_flag)
		apply_auto_brightness_and_contrast(share_img, 1);
	if (*show_edge_flag) 
		share_img = canny_filter_control(share_img);
	if (*save_bmp)
		save_frame_image_bmp(share_img);
	if (*separate_dual_display) 
		display_dual_stereo_separately(share_img);
	if (*display_mat_info_ena)
		display_current_mat_stream_info(share_img);


	/** if image larger than 720p by any dimension, resize the window */
	if (*resize_window_ena) {
		if (width >= CROPPED_WIDTH || height >= CROPPED_HEIGHT)
			cv::resizeWindow(window_name, CROPPED_WIDTH, CROPPED_HEIGHT);
	}	

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
	switch (enable)
	{
	case 1:
		*rgb_ir_color = TRUE;
		break;
	case 0:
		*rgb_ir_color = FALSE;
		break;
	default:
		*rgb_ir_color = FALSE;
		break;
	}
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
void apply_color_correction_rgb_ir(struct device *dev, const void *p)
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
	switch (enable)
	{
	case 1:
		*rgb_ir_ir = TRUE;
		break;
	case 0:
		*rgb_ir_ir = FALSE;
		break;
	default:
		*rgb_ir_ir = FALSE;
		break;
	}
}
void display_rgbir_ir_channel(struct device *dev, const void *p)
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
	perform_shift(dev, dst,  set_shift(bpp));
	//cv::Mat rgbir_ir(height/2, width/2, CV_8UC1, dst);
	cv::Mat rgbir_ir(height, width, CV_8UC1, dst);
	cv::cvtColor(rgbir_ir, rgbir_ir, cv::COLOR_BayerBG2BGR);
	cv::cvtColor(rgbir_ir, rgbir_ir, cv::COLOR_BGR2GRAY);
	//cv::pyrUp(rgbir_ir, rgbir_ir,cv::Size(rgbir_ir.cols*2, rgbir_ir.rows*2));
	*gamma_val = 0.45;
	//*black_level_correction = 64;
	*sharpness = 10;
	apply_gamma_correction(rgbir_ir);
	sharpness_control(rgbir_ir);
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
int video_alloc_buffers(struct device *dev, int nbufs)
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
	for (unsigned int i = 0; i < dev->nbufs; i++)
	{
		CLEAR(querybuffer);
		querybuffer.type = bufrequest.type;
		querybuffer.memory = V4L2_MEMORY_MMAP;
		querybuffer.index = i;

		ret = ioctl(dev->fd, VIDIOC_QUERYBUF, &querybuffer);
		if (ret < 0)
		{
			printf("Unable to query buffer %u (%d).\n", i, errno);
			return ret;
		}
		printf("length: %u offset: %u\n", querybuffer.length,
			   querybuffer.m.offset);

		buffers[i].length = querybuffer.length; /** remember for munmap() */

		buffers[i].start = mmap(NULL, querybuffer.length,
								PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd,
								querybuffer.m.offset);

		if (buffers[i].start == MAP_FAILED)
		{
			/** If you do not exit here you should unmap() and free()
           	the buffers mapped so far. */
			printf("Unable to map buffer %u (%d)\n", i, errno);
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
	unsigned int i;
	int ret;

	if (dev->nbufs == 0)
		return 0;

	for (i = 0; i < dev->nbufs; ++i)
	{
		ret = munmap(dev->buffers[i].start, dev->buffers[i].length);
		if (ret < 0)
		{
			printf("Unable to unmap buffer %u (%d)\n", i, errno);
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