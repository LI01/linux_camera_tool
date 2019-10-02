
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
  Last edit: 2019/10
*****************************************************************************/
#include <getopt.h>
#include "../includes/shortcuts.h"
#include "../includes/utility.h"

#include "../includes/uvc_extension_unit_ctrl.h"
#include "../includes/extend_cam_ctrl.h"
#include "../includes/ui_control.h"
#include "../includes/cam_property.h"
#include "../includes/v4l2_devices.h"
#include "../includes/json_parser.h"
#include "../includes/fd_socket.h"
/*****************************************************************************
**                      	Global data
*****************************************************************************/
static struct option opts[] = {
	{"nbufs", 			required_argument, 0, 'n'},
	{"size", 			required_argument, 0, 's'},
	{"time-per-frame", 	required_argument, 0, 't'},
	{"device", 			required_argument, 0, 'd'},
	{"help", 			no_argument, 	   0, 'h'},
	{0, 				0, 0,  0}
};

/******************************************************************************
**                           Function definition
*****************************************************************************/
/**
 * return the output string for a linux shell command 
 */
std::string
get_stdout_from_cmd(std::string cmd)
{

	std::string data;
	FILE *stream;
	const int max_buffer= 256;
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

/** individual camera tests, detail info is in hardware.h */
void individual_sensor_test(int fd)
{
		
#ifdef AP0202_WRITE_REG_ON_THE_FLY
	ap0202_write_reg_on_the_fly(fd);
#elif AP0202_WRITE_REG_IN_FLASH
	ap0202_write_reg_in_flash(fd);
#elif OS05A20_PTS_QUERY
	os05a20_pts_query(fd);
#elif AR0231_MIPI_TESTING
	ar0231_mipi_testing(fd);
#elif IMX334_MONO_MIPI_TESTING
	imx334_mipi_testing(fd);
#endif

}


int main(int argc, char **argv)
{

	{
		Timer timer; /** measure time spent in this scope and print out once out*/
		int v4l2_dev; /** global variable, file descriptor for camera device */
		struct device dev;
		dev.nbufs = V4L_BUFFERS_DEFAULT;
		// assign 20 bytes in case user input jibberish
		char dev_name[20] = "/dev/video0";
		int do_set_format = 0;
		int do_set_time_per_frame = 0;
		uint32_t time_per_frame  = 15;
		int choice;

		enum_v4l2_device(dev_name);
		printf("********************Camera Tool Usages***********************\n");
		while ((choice = getopt_long(argc, argv, "n:s:t:d:h", opts, NULL)) != -1)
		{
			switch (choice)
			{
				case 'n':
				{
					/** set buffer number */
					dev.nbufs = atoi(optarg);
					if (dev.nbufs > V4L_BUFFERS_MAX)
						dev.nbufs = V4L_BUFFERS_MAX;
					printf("device nbuf %d\n", dev.nbufs);
					break;
				}
				case 's':
				{
					do_set_format = 1;
					char *endptr;
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
				}
				case 't':
				{
					do_set_time_per_frame = 1;
					time_per_frame = atoi(optarg);
					break;
				}
				case 'd':
				{
					char *dev_number = optarg;
					if (dev_number[0] >= '0' 
						&& dev_number[0] <= '9' 
						&& strlen(dev_number) <= 3)
						sprintf(dev_name, "/dev/video%s", dev_number);
					else if (strstr(dev_number, "/dev/video") 
					&& dev_number[strlen(dev_number)-1] >= '0'
					&& dev_number[strlen(dev_number)-1] <= '9')
						strcpy(dev_name, dev_number);
					else {
						printf("Invalid argument: %s\n", dev_number);
						usage(argv[0]);
						exit(EXIT_FAILURE);
					}

					printf("ret dev name = %s\r\n", dev_name);
					update_dev_info(dev_name);
					break;
				}
				case 'h':
				{
					usage(argv[0]);
					exit(0);
				}
				default:
				{
					printf("Invalid option -%c\n", choice);
					usage(argv[0]);
					printf("Run %s -h for help.\n", argv[0]);
					/// refer to cat /usr/includes/sysexits.h for exit code
					exit(EXIT_FAILURE);
				}
			}
		}
		
		if (optind >= argc)
			usage(argv[0]);
		
		v4l2_dev = open_v4l2_device(dev_name, &dev);
		if (v4l2_dev < 0)
		{
			printf("open camera %s failed,err code:%d\n\r", dev_name, v4l2_dev);
			return -1;
		}
	
		printf("********************List Available Resolutions***************\n");
		/** 
	 	 * list all the resolutions 
	 	 * run a v4l2-ctl --list-formats-ext 
	 	 * to see the resolution and available frame rate 
	 	 */
		std::cout << get_stdout_from_cmd(
			"v4l2-ctl --device=" 
			+ std::string(dev_name) 
			+" --list-formats-ext | grep Size | "
			"awk '{print $1 $3}'| sed 's/Size/Resolution/g'");

		/** Set the video format. */
		if (do_set_format)
			video_set_format(&dev);
		/** Set the frame rate. */
		if (do_set_time_per_frame)
			set_frame_rate(dev.fd, time_per_frame);

		printf("********************Device Infomation************************\n");
		/** try to get all the static camera info before fork */
		
		if (!is_ov580_stereo())
			read_cam_uuid_hwfw_rev(dev.fd);
		
		check_dev_cap(dev.fd);
		video_get_format(&dev);   /** list the current resolution etc */
		get_frame_rate(dev.fd); /** list the current frame rate */
		individual_sensor_test(dev.fd);

		// ov580_write_system_reg(v4l2_dev, 0x1033, 0x2);
		// printf("!!!!!!!!!!!!!ae default value = 0x%x\r\n", 
		// 	ov580_read_system_reg(v4l2_dev, 0x1033));
		// printf("!!!!!!!!!!!!!gain value = 0x%x\r\n", 
		// 	ov580_read_sccb0_reg(v4l2_dev, 0xc0, 0x3508));	
		// printf("v4l2 dev = %d\r\n", v4l2_dev);
		// ov580_write_sccb0_reg(v4l2_dev, 0xC0, 0x3508, 0xff);
		// printf("!!!!!!!!!!!!!gain value = 0x%x\r\n", 
		// 	ov580_read_sccb0_reg(v4l2_dev, 0xc0, 0x3508));

		printf("********************Allocate Buffer for Capturing************\n");
		video_alloc_buffers(&dev);
		printf("********************Control Logs*****************************\n");
		/** Activate streaming */
		start_camera(&dev); 	// need to be put before fork for shared memory

		int socket_pair[2];
      	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, socket_pair) < 0) {
	    	std::cout << "socketpair failed" << std::endl;
        	return 1;
    	}
		pid_t pid = fork();
		if (pid == 0)
		{ 	/** child process */
			close(socket_pair[0]); /** read end is unused */
			streaming_loop(&dev, socket_pair[1]);
		}
		else if (pid > 0)
		{ 	/** parent process */
			close(socket_pair[1]); /** write end is unused */
			ctrl_gui_main(socket_pair[0]);
		}
		else
		{
			close(socket_pair[1]);
			close(socket_pair[0]);
			fprintf(stderr, "ERROR:fork() failed\n");
		}
	}

	int sys_ret = system("killall -9 leopard_cam");	
	if (sys_ret < 0)	
	{	
		printf("fail to exit the leopard camera tool\r\n");	
		return -1;	
	}
	return 0;
}
