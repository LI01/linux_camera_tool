
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
int v4l2_dev;	 /** global variable, file descriptor for camera device */
int gain_max;	 /** global variable, update gain range in GUI, read from v4l2 */
int exposure_max; /** global variable, update exposure range in GUI, read from v4l2 */

struct v4l2_fract time_per_frame = {1, 15};
static struct option opts[] = {

	{"nbufs", 1, 0, 'n'},
	{"size", 1, 0, 's'},
	{"time-per-frame", 1, 0, 't'},
	{0, 0, 0, 0}};

/******************************************************************************
**                           Function definition
*****************************************************************************/
/**
 * return the output string for a linux shell command 
 */
std::string get_stdout_from_cmd(std::string cmd)
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

	// record start time
	auto start = std::chrono::high_resolution_clock::now();

	char *ret_dev_name = enum_v4l2_device(dev_name);
	std::string _dev_name(ret_dev_name);
	v4l2_dev = open_v4l2_device(ret_dev_name, &dev);

	if (v4l2_dev < 0)
	{
		printf("open camera %s failed,err code:%d\n\r", dev_name, v4l2_dev);
		return -1;
	}

	/**
	 * this part is to get the gain and exposure maximum value defined in firmware
	 * and update the range in GUI
	 * if MAX_EXPOSURE_TIME is undefined in firmware, will put a upper limit for 
	 * exposure time range so the range won't be as large as 65535 which confuses people
	 */
	gain_max = std::stoi(get_stdout_from_cmd(
		"v4l2-ctl --device=" + _dev_name +
		" --all | grep gain | awk '{print $6}'| sed 's/max=//g'"));
	exposure_max = std::stoi(get_stdout_from_cmd(
		"v4l2-ctl --device=" + _dev_name +
		" --all | grep exposure_absolute | awk '{print $6}'| sed 's/max=//g'"));
	int height = std::stoi(get_stdout_from_cmd(
		"v4l2-ctl --device=" + _dev_name +
		" --all | grep Bounds | awk '{print $10}'"));
	/**
	 *  if UNSET_MAX_EXPOSURE_LINE, firmware didn't define MAX_EXPOSURE_TIME...
	 * 	place holder = 3, it should be enough for most cases
	 *  if you adjust exposure to maximum, and image is still dark 
	 *  try to increase 3 to 4, 5, 6 etc
	 *  or ask for a firmware update, firmware exposure time mapping might be wrong
	 */
	if (exposure_max == UNDEFINED_MAX_EXPOSURE_LINE)
		exposure_max = height * 3;

	printf("********************List Available Resolutions***************\n");
	/** 
	 * list all the resolutions 
	 * run a v4l2-ctl --list-formats-ext 
	 * to see the resolution and available frame rate 
	 */
	std::cout << get_stdout_from_cmd(
		"v4l2-ctl --device=" + _dev_name +
		" --list-formats-ext | grep Size | awk '{print $1 $3}'| sed 's/Size/Resolution/g'");

	printf("********************Camera Tool Usages***********************\n");
	while ((c = getopt_long(argc, argv, "n:s:t:", opts, NULL)) != -1)
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
		default:
			printf("Invalid option -%c\n", c);
			printf("Run %s -h for help.\n", argv[0]);
			return -1;
		}
	}

	if (optind >= argc)
	{
		usage(argv[0]);
	}

	/** Set the video format. */
	if (do_set_format)
	{
		video_set_format(&dev, dev.width, dev.height, V4L2_PIX_FMT_YUYV);
	}
	/** Set the frame rate. */
	if (do_set_time_per_frame)
	{
		set_frame_rate(v4l2_dev, time_per_frame.denominator);
	}

	printf("********************Device Infomation************************\n");
	/** try to get all the static camera info before fork */
	read_cam_uuid_hwfw_rev(v4l2_dev);
	check_dev_cap(&dev);
	video_get_format(&dev);   /** list the current resolution etc */
	get_frame_rate(v4l2_dev); /** list the current frame rate */
/** individual camera tests, detail info is in hardware.h */
#ifdef AP0202_WRITE_REG_ON_THE_FLY
	unsigned int i;
	for (i = 0; i < sizeof(ChangConfig) / sizeof(reg_seq); i++)
		generic_I2C_write(v4l2_dev, 0x82, ChangConfig[i].reg_data_width,
			AP020X_I2C_ADDR, ChangConfig[i].reg_addr,
			(unsigned char *)&(ChangConfig[i].reg_val));

	for (i = 0; i < sizeof(ChangConfig) / sizeof(reg_seq); i++)
	{
		generic_I2C_read(v4l2_dev, 0x02, ChangConfig[i].reg_data_width,
			AP020X_I2C_ADDR, ChangConfig[i].reg_addr);
	}

	generic_I2C_read(v4l2_dev, 0x02, 1, MAX9295_SER_I2C_ADDR, 0x0000);
	generic_I2C_read(v4l2_dev, 0x02, 1, MAX9296_DESER_I2C_ADDR, 0x0000);
#endif

#ifdef AP0202_WRITE_REG_IN_FLASH
	load_register_setting_from_configuration(v4l2_dev,
		SIZE(ChangConfigFromFlash), ChangConfigFromFlash);

	sleep(1);
	//generic_I2C_read(v4l2_dev, 0x02, 2, AP020X_I2C_ADDR, 0x0058);
	sensor_reg_write(v4l2_dev, 0x5080, 0x00);
	sensor_reg_read(v4l2_dev, 0x4308);
	sensor_reg_read(v4l2_dev, 0x5080);
#endif

#ifdef OS05A20_PTS_QUERY
	set_pts(v4l2_dev, 0);
	get_pts(v4l2_dev);
#endif

#ifdef AR0231_MIPI_TESTING
	unsigned int i;
	for (i = 0; i < sizeof(AR0231_MIPI_REG_TESTING) / sizeof(reg_seq); i++)
	{
		//choose either one of the function below for register read
		generic_I2C_read(v4l2_dev, 0x02, AR0231_MIPI_REG_TESTING[i].reg_data_width,
						 AR0231_I2C_ADDR, AR0231_MIPI_REG_TESTING[i].reg_addr);

		sensor_reg_read(v4l2_dev, AR0231_MIPI_REG_TESTING[i].reg_addr);
	}
#endif

#ifdef IMX334_MONO_MIPI_TESTING
	unsigned int i;
	for (i = 0; i < sizeof(IMX334_MIPI_REG_TESTING) / sizeof(reg_seq); i++)
	{
		//TODO: choose either one of the function below for register read
		generic_I2C_read(v4l2_dev, 0x02, IMX334_MIPI_REG_TESTING[i].reg_data_width,
						 IMX334_I2C_ADDR, IMX334_MIPI_REG_TESTING[i].reg_addr);

		sensor_reg_read(v4l2_dev, IMX334_MIPI_REG_TESTING[i].reg_addr);
	}
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

	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	std::cout << "pid:" << getpid() << "\tElapsed time： " << elapsed.count() << "s\r\n";

	//FIXME:why when close the window, it won't kill the process
	sys_ret = system("killall -9 leopard_cam");

	if (sys_ret < 0)
	{
		printf("fail to exit the leopard camera tool\r\n");
		return -1;
	}

	return 0;
}
