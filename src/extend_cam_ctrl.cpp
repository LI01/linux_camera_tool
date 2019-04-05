/*
 * Common implementation of a v4l2 application
 * 1. open a descriptor to the device
 * 2. retrieve and analyse the device's capabilities.
 * 3. set the capture format(YUYV etc)
 * 4. prepare the device for buffering handling. 
 *    when capturing a frame, you have to submit a buffer to the device(queue)
 *    and retrieve it once it's been filled with data(dequeue)
 *    Before you can do this, you must inform the cdevice about 
 *    your buffer(buffer request)
 * 5. For each buffer you wish to use, you must negotiate characteristics with 
 *    the device(buffer size, frame start offset in memory), and create a new
 *    memory mapping for it
 * 6. Put the device into streaming mode
 * 7. Once your buffers are ready, all you have to do is keep queueing and
 *    dequeueing your buffer repeatedly, and every call will bring you a new 
 *    frame. The delay you set between each frames by putting your program to
 *    sleep is what determines your fps
 * 8. Turn off streaming mode
 * 9. Unmap the buffer
 * 9. Close your descriptor to the device 
 */

/* for mmap */
//#include <sys/mman.h>

/* Include files to use OpenCV API */
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../includes/shortcuts.h"
#include "extend_cam_ctrl.h"
#include <string.h>
#include <omp.h> //for openmp
/****************************************************************************
**                      	Global data 
*****************************************************************************/
static int *save_bmp;
static int *save_raw;
static int *bayer_flag;
static int *shift_flag;

int image_count;

/*****************************************************************************
**                           Function definition
*****************************************************************************/
static int save_frame_image_bmp(cv::Mat opencvImage)
{

	printf("save one capture bmp\n");
	cv::imwrite(cv::format("captures_%d.bmp",
						   image_count),
				opencvImage);

	return 0;
}

void video_capture_save_bmp()
{
	set_save_bmp_flag(1);
}

void video_capture_save_raw()
{
	set_save_raw_flag(1);
}

void set_save_bmp_flag(int flag)
{
	*save_bmp = flag;
}

void set_save_raw_flag(int flag)
{
	*save_raw = flag;
}

int set_shift(int *shift_flag)
{
	if (*shift_flag == 1)
		return 2;
	if (*shift_flag == 2)
		return 4;
	if (*shift_flag == 3)
		return 0;
	return 2;
}
int add_bayer_forcv(int *bayer_flag)
{
	if (*bayer_flag == 1)
		return 0;
	if (*bayer_flag == 2)
		return 1;
	if (*bayer_flag == 3)
		return 2;
	if (*bayer_flag == 4)
		return 3;
	return 2;
}

void change_datatype(void *datatype)
{
	if (strcmp((char *)datatype, "1") == 0)
		*shift_flag = 1;
	if (strcmp((char *)datatype, "2") == 0)
		*shift_flag = 2;
	if (strcmp((char *)datatype, "3") == 0)
		*shift_flag = 3;
}

void change_bayerpattern(void *bayer)
{
	if (strcmp((char *)bayer, "1") == 0)
		*bayer_flag = 1;
	if (strcmp((char *)bayer, "2") == 0)
		*bayer_flag = 2;
	if (strcmp((char *)bayer, "3") == 0)
		*bayer_flag = 3;
	if (strcmp((char *)bayer, "4") == 0)
		*bayer_flag = 4;
}

// static __THREAD_TYPE capture_thread;

// /*video buffer data mutex*/
// static __MUTEX_TYPE mutex = __STATIC_MUTEX_INIT;
// #define __PMUTEX &mutex

struct v4l2_buffer queuebuffer;

/* 
 * open the /dev/video* uvc camera device
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

	save_bmp = (int *)mmap(NULL, sizeof *save_bmp, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	save_raw = (int *)mmap(NULL, sizeof *save_raw, PROT_READ | PROT_WRITE,
						   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	shift_flag = (int *)mmap(NULL, sizeof *shift_flag, PROT_READ | PROT_WRITE,
							 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	bayer_flag = (int *)mmap(NULL, sizeof *bayer_flag, PROT_READ | PROT_WRITE,
							 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	return v4l2_dev;
}

/* 
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
		printf("does not support streaming\n");

		return -1;
	}
	if (!(cap.capabilities & V4L2_CAP_READWRITE))
	{
		printf("does not support read, try with mmap\n");
		return -1;
	}
	return 0;
}
/*
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
		printf("couldn't start camera streaming\n");
		return;
	}
	return;
}
/*
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
		printf("couldn't stop camera streaming\n");
		return;
	}
	return;
}
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
	printf("Get Video format: %c%c%c%c (%08x) %ux%u\n 	\
			byte per line:%d\nsize image:%ud\n",
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

/*
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

	printf("Get Video format: %c%c%c%c (%08x) %ux%u\nbyte per line:%d\nsize image:%ud\n",
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
int video_alloc_buffers(struct device *dev, int nbufs)
{
	struct buffer *buffers;

	//request buffer
	struct v4l2_requestbuffers bufrequest;
	struct v4l2_buffer querybuffer;
	CLEAR(bufrequest);
	bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufrequest.memory = V4L2_MEMORY_MMAP;
	bufrequest.count = nbufs;
	dev->nbufs = nbufs;

	int ret;
	ret = ioctl(dev->fd, VIDIOC_REQBUFS, &bufrequest);
	if (ret < 0)
	{
		printf("Unable to request buffers: %d.\n", errno);
		return ret;
	}
	printf("%u buffers requested.\n", bufrequest.count);

	//allocate buffer
	buffers = (buffer *)malloc(bufrequest.count * sizeof buffers[0]);
	if (buffers == NULL)
		return -ENOMEM;

	//map the buffers
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

		buffers[i].length = querybuffer.length; /* remember for munmap() */

		buffers[i].start = mmap(NULL, querybuffer.length,
								PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd,
								querybuffer.m.offset);

		if (buffers[i].start == MAP_FAILED)
		{
			/* If you do not exit here you should unmap() and free()
           	the buffers mapped so far. */
			printf("Unable to map buffer %u (%d)\n", i, errno);
			return ret;
		}

		printf("Buffer mapped at address %p.\n", buffers[i].start);

		CLEAR(queuebuffer);
		queuebuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		queuebuffer.memory = V4L2_MEMORY_MMAP;
		queuebuffer.index = i; /* Queueing buffer index i. */

		/* Put the buffer in the incoming queue. */
		ret = ioctl(dev->fd, VIDIOC_QBUF, &queuebuffer);
		if (ret < 0)
		{
			printf("Unable to queue the buffer %d\n", errno);
			return ret;
		}
	}
	dev->buffers = buffers;

	return 0;
}

/* 
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
int streaming_loop(struct device *dev)
{
	cv::namedWindow("cam", CV_WINDOW_NORMAL);
	image_count = 0;
	while (1)
	{
		get_a_frame(dev);
	}

	munmap(save_bmp, sizeof *save_bmp);
	munmap(save_raw, sizeof *save_raw);
	munmap(shift_flag, sizeof *shift_flag);
	munmap(bayer_flag, sizeof *bayer_flag);
}

/* 
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

		queuebuffer.index = i;
		// The buffer's waiting in the outgoing queue.
		if (ioctl(dev->fd, VIDIOC_DQBUF, &queuebuffer) < 0)
		{
			perror("VIDIOC_DQBUF");
			return;
		}
		if (*(save_raw))
		{
			printf("save a raw\n");
			char buf_name[16];
        	snprintf(buf_name, sizeof(buf_name), "captures_%d.raw",image_count);
			v4l2_core_save_data_to_file(buf_name,
										dev->buffers->start, dev->imagesize);
			image_count++;
			set_save_raw_flag(0);
		}
		decode_a_frame(dev, dev->buffers->start, set_shift(shift_flag));

		if (ioctl(dev->fd, VIDIOC_QBUF, &queuebuffer) < 0)
		{
			perror("VIDIOC_QBUF");
			return;
		}
	}
	return;
}

/*
 * save data to file
 * args:
 *   filename - string with filename
 *   data - pointer to data
 *   size - data size in bytes = sizeof(uint8_t)
 *
 * asserts:
 *   none
 *
 * returns: error code
 */
int v4l2_core_save_data_to_file(const char *filename, const void *data, int size)
{
	FILE *fp;
	int ret = 0;

	if ((fp = fopen(filename, "wb")) != NULL)
	{
		ret = fwrite((uint8_t *)data, size, 1, fp);

		if (ret < 1)
			ret = 1; /*write error*/
		else
			ret = 0;

		fflush(fp); /*flush data stream to file system*/
		if (fsync(fileno(fp)) || fclose(fp))
			fprintf(stderr, "V4L2_CORE: (save_data_to_file) error \
				- couldn't write buffer to file: %s\n",
					strerror(errno));
		else
			printf("V4L2_CORE: saved data to %s\n", filename);
	}
	else
		ret = 1;

	return (ret);
}
/* 
 * opencv only support debayering 8 and 16 bits 
 * 
 * decode the frame, move each pixel by certain bits,
 * and mask it for 8 bits, render a frame using opencv
 * args: 
 * 		struct device *dev - every infomation for camera
 * 		const void *p - pointer for the buffer
 * 		int shift - values to shift(RAW10 - 2, RAW12 - 4, YUV422 - 0) 
 * 
 */
void decode_a_frame(struct device *dev, const void *p, int shift)
{
	int height = dev->height;
	int width = dev->width;
	unsigned char tmp;
	unsigned short *srcShort = (unsigned short *)p;
	unsigned char *dst = (unsigned char *)p;

	/* --- for bayer camera ---*/
	if (shift != 0)
	{
#pragma omp for
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				tmp = *(srcShort++) >> shift; // todo Macro define
				*(dst++) = (unsigned char)tmp;
			}
		}

		cv::Mat img(height, width, 0, (void *)p);
		cv::cvtColor(img, img, CV_BayerBG2BGR + add_bayer_forcv(bayer_flag));
		
		if (*(save_bmp))
		{
			printf("save a bmp\n");
			save_frame_image_bmp(img);
			image_count++;
			set_save_bmp_flag(0);
		}

		cv::resizeWindow("cam", 640, 480);
		cv::imshow("cam", img);
	}
	/* --- for yuv camera ---*/
	else
	{
		cv::Mat img(height, width, CV_8UC2, (void *)p);
		cv::cvtColor(img, img, cv::COLOR_YUV2BGR_YUY2);
		if (*(save_bmp))
		{
			printf("save a bmp\n");
			save_frame_image_bmp(img);
			image_count++;
			set_save_bmp_flag(0);
		}

		cv::resizeWindow("cam", 640, 480);
		cv::imshow("cam", img);
	}

	if (cv::waitKey(_1MS) == _ESC_KEY)
	{
		cv::destroyWindow("cam");
		exit(0);
	}
}
/*
 * free, unmap buffers
 * 
 * args: 
 * 		struct device *dev - put buffers in
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
		return ret;
	}

	printf("%u buffers released.\n", dev->nbufs);

	free(dev->buffers);
	dev->buffers = NULL;
	dev->nbufs = 0;

	return 0;
}
