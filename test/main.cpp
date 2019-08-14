
/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, main.cpp checks the camera
  capabilities, open&close the camera device, forks two processes. For most things
  you want to try, put the function before fork() is main.cpp is a good idea,
  that way you won't be worry about variable memory sharing between two processes.
  To change resolution, frame rate, and number of buffers, you need to exit the
  program and re-enter leopard_cam with specific arguments. So far, this doesn't 
  support dynamically change resolutions etc because we use mmap for camera device.

  Author: Danyu L
  Last edit: 2019/08
*****************************************************************************/

#include "../includes/shortcuts.h"
#include <getopt.h>
#include <chrono> //high resolution clock
#include <iostream>

#include "../includes/uvc_extension_unit_ctrl.h"
#include "../includes/extend_cam_ctrl.h"
#include "../includes/ui_control.h"
#include "../includes/cam_property.h"
#include "../includes/v4l2_devices.h"
#include "../includes/json_parser.h"
/*****************************************************************************
**                      	Global data
*****************************************************************************/
int v4l2_dev; /** global variable, file descriptor for camera device */

static struct option opts[] = {

	{"nbufs", 1, 0, 'n'},
	{"size", 1, 0, 's'},
	{"time-per-frame", 1, 0, 't'},
	{"device", 1, 0, 'd'},
	{0, 0, 0, 0}};

/******************************************************************************
**                           Function definition
*****************************************************************************/

/**
 * timer used for benchmark ISP performance
 */
class Timer
{
public:
	Timer()
	{
		m_start_time_point = std::chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		Stop();
	}
	void Stop()
	{
		auto end_time_point = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>
			(m_start_time_point).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>
			(end_time_point).time_since_epoch().count();
		auto duration = end - start;
		double second = duration * 0.001 * 0.001;
		//std::cout << "Elapsed time："<< duration << "us (" << ms << "ms)\n";
		std::cout << "Elapsed time："<< second << "s\n";
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time_point;
};

/**
 * return the output string for a linux shell command 
 */
std::string
get_stdout_from_cmd(std::string cmd)
{

	std::string data;
	FILE *stream;
	const int max_buffer = 256;
	char buffer[max_buffer];
	cmd.append(" 2>&1");

	stream = popen(cmd.c_str(), "r");
	if (stream)
	{
		while (!feof(stream))
			if (fgets(buffer, max_buffer, stream) != NULL)
				data.append(buffer);
		pclose(stream);
	}
	return data;
}

/** main function */
int main(int argc, char **argv)
{
	struct device dev;
	char dev_name[64] = "/dev/video0";
	int do_set_format = 0;
	int do_set_time_per_frame = 0;
	char *endptr;
	dev.nbufs = V4L_BUFFERS_DEFAULT;
	int c;
	int sys_ret;
	struct v4l2_fract time_per_frame = {1, 15};
	char *ret_dev_name = enum_v4l2_device(dev_name);
	char *dev_number;
	
	{
		Timer timer;
		printf("********************Camera Tool Usages***********************\n");
		while ((c = getopt_long(argc, argv, "n:s:t:d:", opts, NULL)) != -1)
		{
			switch (c)
			{
			case 'n':
				/** set buffer number */
				dev.nbufs = atoi(optarg);
				if (dev.nbufs > V4L_BUFFERS_MAX)
					dev.nbufs = V4L_BUFFERS_MAX;
				printf("device nbuf %d\n", dev.nbufs);
				break;
			case 's':
				do_set_format = 1;
				dev.width = strtol(optarg, &endptr, 10);
				if (*endptr != 'x' || endptr == optarg)
				{
					printf("Invalid size '%s'\n", optarg);
					return 1;
				}
				dev.height = strtol(endptr + 1, &endptr, 10);
				if (*endptr != 0)
				{
					printf("Invalid size '%s'\n", optarg);
					return 1;
				}
				break;
			case 't':
				do_set_time_per_frame = 1;
				time_per_frame.denominator = atoi(optarg);
				break;
			case 'd':
				dev_number = optarg;
				if (dev_number[0] >= '0' && dev_number[0] <= '9' && strlen(dev_number) <= 3)
				{
					sprintf(ret_dev_name, "/dev/video%s", dev_number);
					printf("ret dev name = %s\r\n", ret_dev_name);
				}

				break;
			default:
				printf("Invalid option -%c\n", c);
				printf("Run %s -h for help.\n", argv[0]);
				return -1;
			}
		}

		if (optind >= argc)
			usage(argv[0]);
		
		std::string _dev_name(ret_dev_name);
		v4l2_dev = open_v4l2_device(ret_dev_name, &dev);

		if (v4l2_dev < 0)
		{
			printf("open camera %s failed,err code:%d\n\r", ret_dev_name, v4l2_dev);
			return -1;
		}

		printf("********************List Available Resolutions***************\n");
		/** 
	 	 * list all the resolutions 
	 	 * run a v4l2-ctl --list-formats-ext 
	 	 * to see the resolution and available frame rate 
	 	 */
		std::cout << get_stdout_from_cmd(
			"v4l2-ctl --device=" + _dev_name +
			" --list-formats-ext | grep Size | awk '{print $1 $3}'| sed 's/Size/Resolution/g'");

		/** Set the video format. */
		if (do_set_format)
			video_set_format(&dev, dev.width, dev.height, V4L2_PIX_FMT_YUYV);
		
		/** Set the frame rate. */
		if (do_set_time_per_frame)
			set_frame_rate(v4l2_dev, time_per_frame.denominator);
		

		printf("********************Device Infomation************************\n");
		/** try to get all the static camera info before fork */
		read_cam_uuid_hwfw_rev(v4l2_dev);
		get_gain(v4l2_dev);
		get_exposure_absolute(v4l2_dev);
		check_dev_cap(v4l2_dev);
		video_get_format(&dev);   /** list the current resolution etc */
		get_frame_rate(v4l2_dev); /** list the current frame rate */

		/** individual camera tests, detail info is in hardware.h */
#ifdef AP0202_WRITE_REG_ON_THE_FLY
		ap0202_write_reg_on_the_fly(v4l2_dev);
#endif

#ifdef AP0202_WRITE_REG_IN_FLASH
		ap0202_write_reg_in_flash(v4l2_dev);
#endif

#ifdef OS05A20_PTS_QUERY
		os05a20_pts_query(v4l2_dev);
#endif

#ifdef AR0231_MIPI_TESTING
		ar0231_mipi_testing(v4l2_dev);
#endif

#ifdef IMX334_MONO_MIPI_TESTING
		imx334_mipi_testing(v4l2_dev);
#endif

		printf("********************Allocate Buffer for Capturing************\n");
		video_alloc_buffers(&dev, dev.nbufs);

		printf("********************Control Logs*****************************\n");
		/** Activate streaming */
		start_camera(&dev);
		pid_t pid;
		pid = fork();
		if (pid == 0)
		{ /** child process */
			ctrl_gui_main();
		}
		else if (pid > 0)
		{ /** parent process */
			streaming_loop(&dev);
		}
		else
		{
			fprintf(stderr, "ERROR:fork() failed\n");
		}

		/** Deactivate streaming */
		stop_Camera(&dev);
		video_free_buffers(&dev);
		close(v4l2_dev);
	}


	//FIXME:why when close the window, it won't kill the process
	sys_ret = system("killall -9 leopard_cam");

	if (sys_ret < 0)
	{
		printf("fail to exit the leopard camera tool\r\n");
		return -1;
	}

	return 0;
}
