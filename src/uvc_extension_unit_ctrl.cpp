/****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

  This is the sample code for Leopard USB3.0 camera for register control & 
  trigger/strobe & fw&hw revision under Linux using V4L2. For supporting more
  UVC extension unit features, firmware will need to get updated.
  
  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/

#include "../includes/shortcuts.h"
#include "uvc_extension_unit_ctrl.h"

/****************************************************************************
**                      	Global data 
*****************************************************************************/
struct uvc_xu_control_query xu_query;

// define the buffer for storage
unsigned char buf1[LI_XU_SENSOR_MODES_SWITCH_SIZE] = {0}; 
unsigned char buf2[LI_XU_SENSOR_WINDOW_REPOSITION_SIZE] = {0}; 
unsigned char buf3[LI_XU_LED_MODES_SIZE] = {0}; 
unsigned char buf4[LI_XU_SENSOR_GAIN_CONTROL_RGB_SIZE] = {0}; 

unsigned char buf7[LI_XU_SENSOR_UUID_HWFW_REV_SIZE] = {0}; 
unsigned char buf8[LI_XU_PTS_QUERY_SIZE] = {0};		 
unsigned char buf9[LI_XU_SOFT_TRIGGER_SIZE] = {0};		  
unsigned char buf10[LI_XU_TRIGGER_DELAY_SIZE] = {0};	
unsigned char buf11[LI_XU_TRIGGER_MODE_SIZE] = {0};
unsigned char buf12[LI_XU_SENSOR_REGISTER_CONFIGURATION_SIZE] = {0};		
unsigned char buf14[LI_XU_SENSOR_REG_RW_SIZE] = {0};		 

unsigned char buf16[LI_XU_GENERIC_I2C_RW_SIZE] = {0}; 
unsigned char buf17[LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE] = {0};		


unsigned int m_rGain = 0x1;
unsigned int m_grGain = 0x1;
unsigned int m_gbGain = 0x1;
unsigned int m_bGain = 0x1;

int hw_rev;
 char uuid[64];

/*****************************************************************************
**                           Function definition
*****************************************************************************/
/*
 * handle the error for extension unit control
 * args:
 * 		none
 * returns:
 * 		none
 */
void error_handle_extension_unit()
{
	int res = errno;

	const char *err;
	switch (res)
	{
	case ENOENT:
		err = "Extension unit or control not found";
		break;
	case ENOBUFS:
		err = "Buffer size does not match control size";
		break;
	case EINVAL:
		err = "Invalid request code";
		break;
	case EBADRQC:
		err = "Request not supported by control";
		break;
	default:
		err = strerror(res);
		break;
	}

	printf("failed %s. (System code: %d) \n", err, res);

	return;
}

/*
 * helper function to commnunicate with FX3 UVC defined extension unit
 * args:
 * 		fd 			- file descriptor
 *  	property_id - defined Leopard Imaging USB3.0 Camera uvc extension id
 * 		length 		- size of defined extension unit
 * 		buffer		- pointer for buffer data
 *  
 */
void write_to_UVC_extension(int fd,int property_id,
							int length, unsigned char *buffer)
{

	CLEAR(xu_query);
	xu_query.unit = 3;			  //has to be unit 3
	xu_query.query = UVC_SET_CUR; //request code to send to the device
	xu_query.size = length;
	xu_query.selector = property_id;
	xu_query.data = buffer; //control buffer



	if ((ioctl(fd, UVCIOC_CTRL_QUERY, &xu_query)) != 0)
		error_handle_extension_unit();

	
}

/*
 * helper function to commnunicate with FX3 UVC defined extension unit
 * args:
 * 		fd 			- file descriptor
 *  	property_id - defined Leopard Imaging USB3.0 Camera uvc extension id
 * 		length 		- size of defined extension unit
 * 		buffer		- pointer for buffer data
 *  
 */
void read_from_UVC_extension(int fd,int property_id,
							 int length, unsigned char *buffer)
{
	CLEAR(xu_query);
	xu_query.unit = 3;			  //has to be unit 3
	xu_query.query = UVC_GET_CUR; //request code to send to the device
	xu_query.size = length;
	xu_query.selector = property_id;
	xu_query.data = buffer; //control buffer
	
	if (ioctl(fd, UVCIOC_CTRL_QUERY, &xu_query) != 0)
		error_handle_extension_unit();
}

/*--------------------------------------------------------------------------- */

/*
 * set sensor mode
 * most cameras don't support it in the driver
 * 
 * args:
 *      fd     - file descriptor
 *      mode   - mode
 */
void set_sensor_mode(int fd, int mode)
{
    CLEAR(buf1);
    buf1[0] = mode;
    write_to_UVC_extension(fd, LI_XU_SENSOR_MODES_SWITCH, 
        LI_XU_SENSOR_MODES_SWITCH_SIZE, buf1);
    printf("V4L2_CORE: set sensor mode to %d\n", mode);
}

/*
 * set pos
 * most cameras don't support it in the driver
 * 
 * args:
 *      fd      - file descriptor
 *      start_x 
 *      start_y 
 */
void set_pos(int fd, int start_x, int start_y)
{
    CLEAR(buf2);
    buf2[0] = start_x & 0xff;
    buf2[1] = (start_x >> 8) & 0xff;
    buf2[2] = start_y & 0xff;
    buf2[3] = (start_y >> 8) & 0xff;
    write_to_UVC_extension(fd, LI_XU_SENSOR_WINDOW_REPOSITION,
        LI_XU_SENSOR_WINDOW_REPOSITION_SIZE, buf2);
    printf("V4L2_CORE: set pos for start_x=%d, start_y=%d", start_x, start_y);
}


/*
 * get led status
 * most cameras don't support it in the driver
 * 
 * args: 
 *      fd      - file descriptor
 */
void get_led_status(int fd)
{
    CLEAR(buf3);
    read_from_UVC_extension(fd, LI_XU_LED_MODES,
        LI_XU_LED_MODES_SIZE, buf3);
    printf("V4L2_CORE: led status is %d", buf3[0]);
}

/*
 * set led status
 * most cameras don't support it in the driver
 * 
 * args: 
 *      fd      - file descriptor
 *      left_0  
 *      left_1
 *      right_0
 *      right_1
 */
void set_led(int fd, int left_0, int left_1, int right_0, int right_1)
{
    CLEAR(buf3);
    char val = {0};
    if (left_0) val |= 0x04;
    if (left_1) val |= 0x08;
    if (right_0) val |= 0x01;
    if (right_1) val |= 0x02;
    buf3[0] = val;
    write_to_UVC_extension(fd, LI_XU_LED_MODES,
        LI_XU_LED_MODES_SIZE, buf3);
    printf("V4L2_CORE: set left led 0=%d,\r set left led 1=%d,\r\n	\
			set right led 0=%d\rset right led 1=%d\n",
        left_0, left_1, right_0, right_1);
}

/* 
 * set sensor gain value, 
 * need to enable this feature in USB camera driver 
 * 
 * args:
 * 		fd 		- file descriptor
 * 		rGain   - better to consult sensor datasheet before performing
 * 		grGain
 * 		gbGain
 * 		bGain
 *
 */
void set_sensor_gain_rgb(int fd,unsigned int rGain,
						 unsigned int grGain,
						 unsigned int gbGain,
						 unsigned int bGain)
{
	CLEAR(buf4);
	if ((m_rGain != rGain) | (m_grGain != grGain) |
		(m_gbGain != gbGain) | (m_bGain != bGain))
	{
		m_rGain = rGain;
		m_grGain = grGain;
		m_gbGain = gbGain;
		m_bGain = bGain;

		buf4[0] = m_rGain & 0xff;
		buf4[1] = m_rGain >> 8;
		buf4[2] = m_grGain & 0xff;
		buf4[3] = m_grGain >> 8;
		buf4[4] = m_rGain & 0xff;
		buf4[5] = m_rGain >> 8;
		buf4[6] = m_grGain & 0xff;
		buf4[7] = m_grGain >> 8;
	}
	
	write_to_UVC_extension(fd, LI_XU_SENSOR_GAIN_CONTROL_RGB, 
        LI_XU_SENSOR_GAIN_CONTROL_RGB_SIZE, buf4);
}

/* 
 * read camera uuid hardware firmware revision
 * for uuid and fuseid, request for new driver to fit needs
 */
int read_cam_uuid_hwfw_rev(int fd)
{
    CLEAR(buf7);
	
    char uuidBuf[80];
    read_from_UVC_extension(fd, LI_XU_SENSOR_UUID_HWFW_REV,
        LI_XU_SENSOR_UUID_HWFW_REV_SIZE, buf7);
	/* upper 4 bits are for camera datatype, clear that flags */
    hw_rev = buf7[0] | (buf7[1] << 8);
	hw_rev &= ~(0xf000); 
    int local_fw_rev = buf7[2] | (buf7[3] << 8);
    for (int i=0; i < (36+9); ++i)
    {
        uuidBuf[i] = buf7[4+i];
    }
    strcpy(uuid, uuidBuf);
    printf("hardware rev=%x\n", hw_rev);
    printf("firmware rev=%d\n", local_fw_rev);
    printf("uuid=%s\n", uuid);
	return local_fw_rev;
}

/*
 * currently PTS information are placed in 2 places
 * 1. UVC video data header 
 * 	- a 33-bit PTS timestamp as defined in ITU-REC-H.222.0/ISO/IEC 13818-1
 * 	- you can get the PTS info by looking at usb sniffing data
 * 	- uvc header byte 1(BFH) bit 2 for PTS indicator
 *  - PTS is stored in byte 2-5 of a 12-byte uvc header
 *  - PTS value stays the same in a frame
 * 2.  first 4 bytes in a given frame
 *  - you can get the PTS by grabbing the first 4 bytes of each frame
 *  - byte order: little endian
 *  
 * PTS info:
 * currently on FX3, PTS counter is sampling at frequency 403M/16 ~ 25MHz
 * 
 * Clock frequency can be read off from usb descriptor: 25187500Hz 
 * e.g.	lsusb -D /dev/bus/usb/002/019 | grep dwClockFrequency
 * 
 * for different camera boards, crystall will slightly drift over time
 * PTS increments over time can be calculated by 
 * 		(1/frame_rate)/(1/25MHz) = 25MHz/frame_rate
 * 
 * this method is an add-on extension unit for query PTS info
 * args:
 * 		fd 		 - file descriptor
 */
void get_pts(int fd)
{
	CLEAR(buf8);
	read_from_UVC_extension(fd, LI_XU_PTS_QUERY, 
        LI_XU_PTS_QUERY_SIZE, buf8);
	
	unsigned long pts = buf8[0] | buf8[1] << 8 | buf8[2] << 16 | buf8[3] << 24;
	printf("V4L2_CORE: get PTS = %lu\r\n", pts);
}

/* set PTS counter initial value
 * args:
 * 		fd 		- file descriptor
 * 		initVal	- initial counter start value for PTS(4-byte long)
 *
 */
void set_pts(int fd,unsigned long initVal)
{
	CLEAR(buf8);
	buf8[0] = (initVal >> 24) & 0xff;
	buf8[1] = (initVal >> 16) & 0xff;
	buf8[2] = (initVal >> 8) & 0xff;
	buf8[3] = initVal & 0xff;
	write_to_UVC_extension(fd, LI_XU_PTS_QUERY, 
        LI_XU_PTS_QUERY_SIZE, buf8);
	printf("V4L2_CORE: set init count value of PTS to %lu\r\n", initVal);
}


/*
 * send out the trigger pulse once when call once
 * need to check with driver it the sensor support trigger mode
 * args:
 *      fd  - file descriptor
 */
void soft_trigger(int fd)
{
    CLEAR(buf9);

    printf("V4L2_CORE: send one trigger\n");

    write_to_UVC_extension(fd, LI_XU_SOFT_TRIGGER, 
        LI_XU_SOFT_TRIGGER_SIZE, buf9);
    
}


void trigger_delay_time(int fd, unsigned int delay_time)
{
    CLEAR(buf10);
    buf10[0] = delay_time & 0xff;
    buf10[1] = (delay_time >> 8) & 0xff;
    buf10[2] = (delay_time >> 16) & 0xff;
    buf10[3] = (delay_time >> 24) & 0xff;

    printf("V4L2_CORE: trigger delay time %x", delay_time);

    write_to_UVC_extension(fd, LI_XU_TRIGGER_DELAY,
        LI_XU_TRIGGER_DELAY_SIZE, buf10);
    
}

/*
 * Change camera sensor from free running master mode to trigger mode.
 * Different sensors will have different triggering mode configurations.
 * Need to check with the sensor and driver support for triggering functionality.
 * Once trigger_enable is called, streaming will freeze.
 * There will be frames coming out for each trigger pulse you've been sent.
 * soft_trigger will generate one trigger for each call.
 * However, a dedicated triggering pin to generate the desired frame rate is prefered 
 * to sync with more than one camera.
 */
void trigger_enable(int fd, int ena, int enb)
{
    CLEAR(buf11);
    if(ena)
    {
        if(enb) 
            buf11[0] = 0x01;
        else 
            buf11[0] = 0x03;
    }
    else
        buf11[0] = 0x00;
    buf11[1] = 0x00;
   	write_to_UVC_extension(fd, LI_XU_TRIGGER_MODE,
        LI_XU_TRIGGER_MODE_SIZE, buf11);
    //printf("trigger mode enable\n");
}

/*
 *	save register to spi flash on FX3, load it automatically when boot time
 *  flash for storage is set to be 256 bytes
 *  args:
 * 		fd 		 - file descriptor
 * 		regCount - pairs of regAddr and regVal (up to 62)
 * 		buffer   - sensor register configuration
 */
void load_register_setting_from_configuration(int fd,int regCount,
											  const struct reg_pair *buffer)
{
	int i;
	CLEAR(buf12);
	printf("V4L2_CORE: save to flash\r\n");
	//set flags to match definition in firmware
	buf12[0] = 0x11;
	buf12[1] = 0x22;
	buf12[2] = 0x33;
	buf12[3] = 0x44;

	//regCount can be less than 62, match with firmware
	buf12[4] = regCount & 0xff;
	buf12[5] = regCount >> 8;
	buf12[6] = regCount >> 16;
	buf12[7] = regCount >> 24;

	//max reg, addr pair #= (256 -8)/4 = 62
	for (i = 2; i < MAX_PAIR_FOR_SPI_FLASH; i++)
	{

		int addr = buffer[i - 2].reg_addr;
		int val = buffer[i - 2].reg_val;

		if ((i - 2) < regCount)
		{
			if (addr != 0xffff && val != 0xffff)
			{
				sensor_reg_write(fd, addr, val);
			}
			buf12[4 * i] = addr & 0xff;
			buf12[4 * i + 1] = addr >> 8;
			buf12[4 * i + 2] = val & 0xff;
			buf12[4 * i + 3] = val >> 8;
		}
		else
		{
			buf12[4 * i] = 0xff;
			buf12[4 * i + 1] = 0xff;
			buf12[4 * i + 2] = 0xff;
			buf12[4 * i + 3] = 0xff;
		}

		if ((i + 1) % MAX_PAIR_FOR_SPI_FLASH == 0)
		{
			//store it to SPI flash
			write_to_UVC_extension(fd, LI_XU_SENSOR_REGISTER_CONFIGURATION,
				LI_XU_SENSOR_REGISTER_CONFIGURATION_SIZE, buf12);
			sleep(1);
		}
	}
}

/*
 *	load register to spi flash on FX3 manually     
 *  #########FOR TEST ONLY###########
 *  flash for storage is set to be 256 bytes
 *  args:
 * 		fd 		 - file descriptor
 */
void load_register_setting_from_flash_manually(int fd)
{
	int reg_flash_length, addr, val, i;
	printf("V4L2_CORE: load from flash\r\n");
	CLEAR(buf12);
	read_from_UVC_extension(fd, LI_XU_SENSOR_REGISTER_CONFIGURATION,
		LI_XU_SENSOR_REGISTER_CONFIGURATION_SIZE, buf12);
	if (buf12[0] != 0x11 && buf12[1] != 0x22 && buf12[2] != 0x33 && buf12[3] != 0x44)
	{
		return;
	}
	reg_flash_length = buf12[4] | buf12[5] << 8 | buf12[6] << 16 | buf12[7] << 24;

	if (reg_flash_length > 63)
		return;

	for (i = 0; i < reg_flash_length; i++)
	{
		addr = buf12[i * 4 + 8] | buf12[i * 4 + 9] << 8;
		val = buf12[i * 4 + 10] | buf12[i * 4 + 11] << 8;
		if (addr != 0xffff && val != 0xffff)
		{
			sensor_reg_write(fd, addr, val);
		}
	}
}

/* 
 * I2C register read/write for the sensor 
 * args: 
 * 		fd 		- file descriptor
 * 		regAddr - register address want to access
 *		regVal  - register value to write
 */
void sensor_reg_write(int fd,int regAddr, int regVal)
{

	CLEAR(buf14);

	buf14[0] = 1; //1 indicates for write
	buf14[1] = (regAddr >> 8) & 0xff;
	buf14[2] = regAddr & 0xff;
	buf14[3] = (regVal >> 8) & 0xff;
	buf14[4] = regVal & 0xff;

	write_to_UVC_extension(fd, LI_XU_SENSOR_REG_RW, 
        LI_XU_SENSOR_REG_RW_SIZE, buf14);

	printf("V4L2_CORE: Write Sensor REG[0x%x]: 0x%x\r\n", regAddr, regVal);
}

/* 
 * I2C register read/write for the sensor 
 * args: 
 * 		fd 		- file descriptor
 * 		regAddr - register address want to access
 */
int sensor_reg_read(int fd,int regAddr)
{

	int regVal = 0;

	CLEAR(buf14);

	buf14[0] = 0; //0 indicates for read
	buf14[1] = (regAddr >> 8) & 0xff;
	buf14[2] = regAddr & 0xff;

	write_to_UVC_extension(fd, LI_XU_SENSOR_REG_RW, 
        LI_XU_SENSOR_REG_RW_SIZE, buf14);
	buf14[0] = 0; //0 indicates for read
	buf14[3] = 0;
	buf14[4] = 0;
	read_from_UVC_extension(fd, LI_XU_SENSOR_REG_RW, 
        LI_XU_SENSOR_REG_RW_SIZE, buf14);

	regVal = (buf14[3] << 8) + buf14[4];
	printf("V4L2_CORE: Read Sensor REG[0x%x] = 0x%x\r\n", regAddr, regVal);
	return regVal;
}

/*
 *  register read/write for different slaves on I2C line
 * 
 *	Byte0: bit7 0:read;1:write. bit[6:0] regAddr width, 1:8-bit register addr;
 														2:16-bit register addr
 *		   0x81: write, regAddr is 8-bit; 0x82: write, regAddr is 16-bit
 *		   0x01: read,  regAddr is 8-bit; 0x02: read,  regAddr is 16-bit
 *	Byte1: Length of register data,1~256
 *	Byte2: i2c salve address 8bit
 *	Byte3: register address
 *	Byte4: register address(16bit) or register data
 *
 *  Register data starts from Byte4(8bit address) or Byte5(16bit address)
 * args:	
 * 		fd 		 - file descriptor
 * 		rw_flag	 - please read the above
 * 		bufCnt	 - define register data length(8-bit/16-bit)
 * 		slaveAddr- I2C address for slave
 * 		regAddr  - register address want to access
 * 		i2c_data - pointer to register value   
 */
void generic_I2C_write(int fd,int rw_flag, int bufCnt,
					   int slaveAddr, int regAddr, unsigned char *i2c_data)
{
	int regVal = 0;

	CLEAR(buf16);

	buf16[0] = rw_flag;
	buf16[1] = bufCnt - 1;
	buf16[2] = slaveAddr >> 8;
	buf16[3] = slaveAddr & 0xff;
	buf16[4] = regAddr >> 8;
	buf16[5] = regAddr & 0xff;

	if (bufCnt == 1)
	{
		buf16[6] = *i2c_data;
		regVal = buf16[6];
	}
	else
	{
		buf16[6] = *(i2c_data + 1);
		buf16[7] = *i2c_data;
		regVal = (buf16[6] << 8) + buf16[7];
	}

	write_to_UVC_extension(fd, LI_XU_GENERIC_I2C_RW, 
        LI_XU_GENERIC_I2C_RW_SIZE, buf16);
	printf("V4L2_CORE: I2C slave ADDR[0x%x], Write REG[0x%x]: 0x%x\r\n",
		   slaveAddr, regAddr, regVal);
}

/*
 *  register read/write for different slaves on I2C line
 *
 *  Register data starts from Byte4(8bit address) or Byte5(16bit address)
 * args:	
 * 		fd 		 - file descriptor
 * 		rw_flag	 - please read the above for details
 * 		bufCnt	 - define register data length(8-bit/16-bit)
 * 		slaveAddr- I2C address for slave
 * 		regAddr  - register address want to access
 * 		i2c_data - pointer to register value   
 */
int generic_I2C_read(int fd,int rw_flag, int bufCnt,
					  int slaveAddr, int regAddr)

{
	int regVal = 0;

	CLEAR(buf16);
	buf16[0] = rw_flag;
	buf16[1] = bufCnt - 1;
	buf16[2] = slaveAddr >> 8;
	buf16[3] = slaveAddr & 0xff;
	buf16[4] = regAddr >> 8;
	buf16[5] = regAddr & 0xff;
	write_to_UVC_extension(fd, LI_XU_GENERIC_I2C_RW, 
        LI_XU_GENERIC_I2C_RW_SIZE, buf16);
	buf16[6] = 0;
	buf16[7] = 0;
	read_from_UVC_extension(fd, LI_XU_GENERIC_I2C_RW, 
        LI_XU_GENERIC_I2C_RW_SIZE, buf16);
	if (bufCnt == 1)
	{
		regVal = buf16[6];
	}
	else
	{
		regVal = (buf16[6] << 8) + buf16[7];
	}
	printf("V4L2_CORE: I2C slave ADDR[0x%x], Read REG[0x%x]: 0x%x\r\n",
		   slaveAddr, regAddr, regVal);
	return regVal;
}
