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
 * This is the sample code for Leopard USB3.0 camera for register control &  
 * trigger/strobe & fw&hw revision under Linux using V4L2. For supporting 	 
 * more UVC extension unit features, firmware will need to get updated.      
 * 																			 
 * Author: Danyu L 														     
 * Last edit: 2019/08 														 
*****************************************************************************/

#include "../includes/shortcuts.h"
#include "../includes/uvc_extension_unit_ctrl.h"

/*****************************************************************************
**                      	Global data 
*****************************************************************************/
static unsigned int m_rGain  = 0x1;
static unsigned int m_grGain = 0x1;
static unsigned int m_gbGain = 0x1;
static unsigned int m_bGain  = 0x1;

int hw_rev; 
int fw_rev;
int li_datatype;
char uuid[64];
/******************************************************************************
**                           Function definition
*****************************************************************************/

/**
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

/**
 * helper function to communicate with FX3 UVC defined extension unit
 * args:
 * 		fd 			- file descriptor
 *  	property_id - defined Leopard Imaging USB3.0 Camera uvc extension id
 * 		length 		- size of defined extension unit
 * 		buffer		- pointer for buffer data
 *  
 */
void write_to_UVC_extension(int fd, int property_id,
							int length, unsigned char *buffer)
{
	struct uvc_xu_control_query xu_query;
	CLEAR(xu_query);
	xu_query.unit = 3;			  //has to be unit 3
	xu_query.query = UVC_SET_CUR; //request code to send to the device
	xu_query.size = length;
	xu_query.selector = property_id;
	xu_query.data = buffer; //control buffer

	if ((ioctl(fd, UVCIOC_CTRL_QUERY, &xu_query)) != 0)
		error_handle_extension_unit();
}

/**
 * helper function to communicate with FX3 UVC defined extension unit
 * args:
 * 		fd 			- file descriptor
 *  	property_id - defined Leopard Imaging USB3.0 Camera uvc extension id
 * 		length 		- size of defined extension unit
 * 		buffer		- pointer for buffer data
 *  
 */
void read_from_UVC_extension(int fd, int property_id,
							 int length, unsigned char *buffer)
{
	struct uvc_xu_control_query xu_query;
	CLEAR(xu_query);
	xu_query.unit = 3;			  //has to be unit 3
	xu_query.query = UVC_GET_CUR; //request code to send to the device
	xu_query.size = length;
	xu_query.selector = property_id;
	xu_query.data = buffer; //control buffer

	if (ioctl(fd, UVCIOC_CTRL_QUERY, &xu_query) != 0)
		error_handle_extension_unit();
}


//////////////////////////////////////////////////////////////////////////////
//		LEOPARD FX3 USB FIRMWARE EXTENSION UNIT API				    		//
// 		Used in most control widgets in the first tab in GUI	   			//		
//////////////////////////////////////////////////////////////////////////////
/**
 * set sensor mode
 * most cameras don't support it in the driver
 * 
 * args:
 *      fd     - file descriptor
 *      mode   - mode
 */
void set_sensor_mode(int fd, int mode)
{
	unsigned char buf1[LI_XU_SENSOR_MODES_SWITCH_SIZE] = {0};
	CLEAR(buf1);
	buf1[0] = mode;
	write_to_UVC_extension(fd, LI_XU_SENSOR_MODES_SWITCH,
						   LI_XU_SENSOR_MODES_SWITCH_SIZE, buf1);
	printf("V4L2_CORE: set sensor mode to %d\n", mode);
}

/**
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
	unsigned char buf2[LI_XU_SENSOR_WINDOW_REPOSITION_SIZE] = {0};
	CLEAR(buf2);
	buf2[0] = LOWBYTE(start_x);
	buf2[1] = HIGHBYTE(start_x); 
	buf2[2] = LOWBYTE(start_y);
	buf2[3] = HIGHBYTE(start_y);
	write_to_UVC_extension(fd, LI_XU_SENSOR_WINDOW_REPOSITION,
						   LI_XU_SENSOR_WINDOW_REPOSITION_SIZE, buf2);
	printf("V4L2_CORE: set pos for start_x=%d, start_y=%d", start_x, start_y);
}

/**
 * get led status
 * most cameras don't support it in the driver
 * 
 * args: 
 *      fd      - file descriptor
 */
void get_led_status(int fd)
{
	unsigned char buf3[LI_XU_LED_MODES_SIZE] = {0};
	CLEAR(buf3);
	read_from_UVC_extension(fd, LI_XU_LED_MODES,
							LI_XU_LED_MODES_SIZE, buf3);
	printf("V4L2_CORE: led status is %d", buf3[0]);
}

/**
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
	unsigned char buf3[LI_XU_LED_MODES_SIZE] = {0};
	CLEAR(buf3);
	char val = {0};
	if (left_0)
		val |= 0x04;
	if (left_1)
		val |= 0x08;
	if (right_0)
		val |= 0x01;
	if (right_1)
		val |= 0x02;
	buf3[0] = val;
	write_to_UVC_extension(fd, LI_XU_LED_MODES,
						   LI_XU_LED_MODES_SIZE, buf3);
	printf("V4L2_CORE: set left led 0=%d,\r set left led 1=%d,\r\n	\
			set right led 0=%d\rset right led 1=%d\n",
		   left_0, left_1, right_0, right_1);
}

/** 
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
void set_sensor_gain_rgb(int fd, unsigned int rGain,
						 unsigned int grGain,
						 unsigned int gbGain,
						 unsigned int bGain)
{
	unsigned char buf4[LI_XU_SENSOR_GAIN_CONTROL_RGB_SIZE] = {0};
	CLEAR(buf4);
	if ((m_rGain != rGain) | (m_grGain != grGain) |
		(m_gbGain != gbGain) | (m_bGain != bGain))
	{
		m_rGain = rGain;
		m_grGain = grGain;
		m_gbGain = gbGain;
		m_bGain = bGain;

		buf4[0] = LOWBYTE(m_rGain);
		buf4[1] = HIGHBYTE(m_rGain);
		buf4[2] = LOWBYTE(m_grGain);
		buf4[3] = HIGHBYTE(m_grGain);
		buf4[4] = LOWBYTE(m_gbGain);
		buf4[5] = HIGHBYTE(m_gbGain);
		buf4[6] = LOWBYTE(m_bGain);
		buf4[7] = HIGHBYTE(m_bGain);
	}

	write_to_UVC_extension(fd, LI_XU_SENSOR_GAIN_CONTROL_RGB,
						   LI_XU_SENSOR_GAIN_CONTROL_RGB_SIZE, buf4);
}

void DS28C36_I2C_write(
    int fd, 
    int slaveAddr, 
    int length,
    int cmd,
    unsigned char *i2c_data)
{
    unsigned char buf[LI_XU_SENSOR_NO_REG_I2C_RW_SIZE] = {0};
    CLEAR(buf);
    buf[0] = 0x80;              //indicate read/write
	buf[1] = slaveAddr&0xff;    // slave addr
	buf[2] = (length+2) & 0xff;        // TX byte len
	buf[3] = cmd & 0xff;     // cmd
    buf[4] = (length)& 0xff; // length padding
    buf[5] = 0x00; 
    // TX data - 32 bytes
    if (length > 1)
    {
        printf("write in data [");
        for (int i=1; i < length; i++) 
        {
            buf[5+i] = *(i2c_data+i-1); 
            //printf("0x%x ", *(i2c_data+i));
        }
        for (int i=0; i< length+4; i++)
        {
            printf("0x%x ", buf[i]);
        }
        printf("] write finished\r\n");
    }
    write_to_UVC_extension(
        fd, LI_XU_SENSOR_NO_REG_I2C_RW,
		LI_XU_SENSOR_NO_REG_I2C_RW_SIZE, buf);

}

//unsigned char * 
void DS28C36_I2C_read(
    int fd, 
    int slaveAddr, 
    int length)
{
    unsigned char buf[LI_XU_SENSOR_NO_REG_I2C_RW_SIZE] = {0};
    CLEAR(buf);
    buf[0] = 0x00;              // indicate for read
	buf[1] = slaveAddr &0xff;   // slave addr
	buf[2] = length & 0xff;     // RX data length
	
    write_to_UVC_extension(
        fd, LI_XU_SENSOR_NO_REG_I2C_RW,
		LI_XU_SENSOR_NO_REG_I2C_RW_SIZE, buf);
    //unsigned char *i2c_data;
    read_from_UVC_extension(
		fd, LI_XU_SENSOR_NO_REG_I2C_RW,
		LI_XU_SENSOR_NO_REG_I2C_RW_SIZE, buf);
    printf("received data [");
    // for (int i=0; i < length; i++) 
    // {
    //     //*(i2c_data+i) = buf[5+i];
    //     printf("0x%x ", buf[5+i]); 
    // }
    for (int i=0; i < length+3; i++) 
    {
        //*(i2c_data+i) = buf[5+i];
        printf("0x%x ", buf[i]); 
    }
    printf("] read finished\r\n");
    //return i2c_data;
}

/* test */
void DS28C36_test(int fd)
{  /** write memory */
    unsigned char write_buf[WRITE_TX_BYTE_COUNT];
    for (int i=0; i<WRITE_TX_BYTE_COUNT; i++) {
        write_buf[i] = i+1;
    }
    DS28C36_I2C_write(
        fd,
        DS28C36_I2C_SLAVE_ADDR, 
        WRITE_TX_BYTE_COUNT,
        WRITE_MEM_CMD,
        write_buf);
    usleep(50); // delay 500ms
    DS28C36_I2C_read(
        fd,
        DS28C36_I2C_SLAVE_ADDR, 
        2);
    usleep(50); // delay 50ms
    /** read memory */
    unsigned char read_buf[READ_RX_BYTE_COUNT] = {0};
    DS28C36_I2C_write(
        fd,
        DS28C36_I2C_SLAVE_ADDR, 
        1,
        READ_MEM_CMD,
        read_buf);
    usleep(50); // delay 50ms
    DS28C36_I2C_read(
        fd,
        DS28C36_I2C_SLAVE_ADDR, 
        READ_RX_BYTE_COUNT);

}

/**
 * Getter for hardware revision
 * Most likely 0x0102
 */ 
int get_hw_rev()
{
	return hw_rev;
}

/**
 * Getter for firmware revision 
 * To make it work, need to compile the driver from svn. 
 * If your firmware revision is 0 or 65535, mostly that version hasn't been
 * compiled at svn...  
 */
int get_fw_rev()
{
	return fw_rev;
}
int get_li_datatype()
{
	return li_datatype;
}
/**
 * Getter for uuid
 */
char *get_uuid()
{
	return uuid;
}

/** 
 * read camera uuid hardware firmware revision
 * for uuid and fuseid, request for new driver to fit needs
 */
void read_cam_uuid_hwfw_rev(int fd)
{
	unsigned char buf7[LI_XU_SENSOR_UUID_HWFW_REV_SIZE] = {0};
	CLEAR(buf7);
	char uuidBuf[80];
	read_from_UVC_extension(fd, LI_XU_SENSOR_UUID_HWFW_REV,
							LI_XU_SENSOR_UUID_HWFW_REV_SIZE, buf7);
	/** upper 4 bits are for camera li_datatype, clear that flags */
	hw_rev = buf7[0] | (buf7[1] << 8);
	li_datatype = hw_rev & 0xf000;
	hw_rev &= ~(0xf000);
	fw_rev = buf7[2] | (buf7[3] << 8);
	for (int i = 0; i < (36 + 9); ++i)
	{
		uuidBuf[i] = buf7[4 + i];
	}
	strcpy(uuid, uuidBuf);
	printf("hardware rev=%x\n", hw_rev);
	printf("firmware rev=%d\n", fw_rev);
	printf("uuid=%s\n", uuid);
}

/**
 * set sensor serial number/ uuid number
 * used for mass production camera that got the official serial numbers
 * and use this for group program the serial number from a serial number doc 
 * This is not available for normal released camera.
 * If serial number is still needed, one can ask for a firmware update that 
 * read the fuse ID from sensor/FX3 for serial number.
 */
void sensor_set_serial_number(int fd, char *sn)
{
	unsigned char buf7[LI_XU_SENSOR_UUID_HWFW_REV_SIZE] = {0};
	CLEAR(buf7);

	buf7[0] = SERIAL_NUMBER_WR_FLG;
	buf7[1] = *sn;
	buf7[2] = *(sn + 1);
	buf7[3] = *(sn + 2);
	buf7[4] = *(sn + 3);
	buf7[5] = *(sn + 4);
	buf7[6] = *(sn + 5);
	buf7[7] = *(sn + 6);
	buf7[8] = *(sn + 7);
	buf7[9] = *(sn + 8);
	buf7[10] = *(sn + 9);
	write_to_UVC_extension(fd, LI_XU_SENSOR_UUID_HWFW_REV,
						   LI_XU_SENSOR_UUID_HWFW_REV_SIZE, buf7);
	printf("Sensor Set Serial Number = %s\n\r", sn);
}

/**
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
	unsigned char buf8[LI_XU_PTS_QUERY_SIZE] = {0};
	CLEAR(buf8);
	read_from_UVC_extension(fd, LI_XU_PTS_QUERY,
							LI_XU_PTS_QUERY_SIZE, buf8);

	unsigned long pts = buf8[0] | buf8[1] << 8 | buf8[2] << 16 | buf8[3] << 24;
	printf("V4L2_CORE: get PTS = %lu\r\n", pts);
}

/** set PTS counter initial value
 * args:
 * 		fd 		- file descriptor
 * 		initVal	- initial counter start value for PTS(4-byte long)
 *
 */
void set_pts(int fd, unsigned long initVal)
{
	unsigned char buf8[LI_XU_PTS_QUERY_SIZE] = {0};
	CLEAR(buf8);
	buf8[0] = (initVal >> 24) & 0xff;
	buf8[1] = (initVal >> 16) & 0xff;
	buf8[2] = (initVal >> 8) & 0xff;
	buf8[3] = initVal & 0xff;
	write_to_UVC_extension(fd, LI_XU_PTS_QUERY,
						   LI_XU_PTS_QUERY_SIZE, buf8);
	printf("V4L2_CORE: set init count value of PTS to %lu\r\n", initVal);
}

/**
 * send out the trigger pulse once when call once
 * need to check with driver it the sensor support trigger mode
 * args:
 *      fd  - file descriptor
 */
void soft_trigger(int fd)
{
	unsigned char buf9[LI_XU_SOFT_TRIGGER_SIZE] = {0};
	CLEAR(buf9);
	printf("V4L2_CORE: send one trigger\n");

	write_to_UVC_extension(fd, LI_XU_SOFT_TRIGGER,
						   LI_XU_SOFT_TRIGGER_SIZE, buf9);
}

void trigger_delay_time(int fd, unsigned int delay_time)
{
	unsigned char buf10[LI_XU_TRIGGER_DELAY_SIZE] = {0};
	CLEAR(buf10);
	buf10[0] = delay_time & 0xff;
	buf10[1] = (delay_time >> 8) & 0xff;
	buf10[2] = (delay_time >> 16) & 0xff;
	buf10[3] = (delay_time >> 24) & 0xff;

	printf("V4L2_CORE: trigger delay time %x", delay_time);

	write_to_UVC_extension(fd, LI_XU_TRIGGER_DELAY,
						   LI_XU_TRIGGER_DELAY_SIZE, buf10);
}

/**
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
	unsigned char buf11[LI_XU_TRIGGER_MODE_SIZE] = {0};
	CLEAR(buf11);
	if (ena)
	{
		if (enb)
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

/**
 *	save register to spi flash on FX3, load it automatically when boot time
 *  flash for storage is set to be 256 bytes
 *  args:
 * 		fd 		 - file descriptor
 * 		regCount - pairs of regAddr and regVal (up to 62)
 * 		buffer   - sensor register configuration
 */
void load_register_setting_from_configuration(int fd, int regCount,
											  const struct reg_pair *buffer)
{
	int i;
	unsigned char buf12[LI_XU_SENSOR_REGISTER_CONFIGURATION_SIZE] = {0};
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
			buf12[4 * i] = LOWBYTE(addr);
			buf12[4 * i + 1] = HIGHBYTE(addr);
			buf12[4 * i + 2] = LOWBYTE(val);
			buf12[4 * i + 3] = HIGHBYTE(val);
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

/**
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
	unsigned char buf12[LI_XU_SENSOR_REGISTER_CONFIGURATION_SIZE] = {0};
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

/** 
 * I2C register read/write for the sensor 
 * args: 
 * 		fd 		- file descriptor
 * 		regAddr - register address want to access
 *		regVal  - register value to write
 */
void sensor_reg_write(int fd, int regAddr, int regVal)
{
	unsigned char buf14[LI_XU_SENSOR_REG_RW_SIZE] = {0};
	CLEAR(buf14);

	buf14[0] = SENSOR_REG_WRITE_FLG;
	buf14[1] = HIGHBYTE(regAddr);
	buf14[2] = LOWBYTE(regAddr);
	buf14[3] = HIGHBYTE(regVal);
	buf14[4] = LOWBYTE(regVal);

	write_to_UVC_extension(fd, LI_XU_SENSOR_REG_RW,
						   LI_XU_SENSOR_REG_RW_SIZE, buf14);

	printf("V4L2_CORE: Write Sensor REG[0x%x]: 0x%x\r\n", regAddr, regVal);
}

/** 
 * I2C register read/write for the sensor 
 * args: 
 * 		fd 		- file descriptor
 * 		regAddr - register address want to access
 */
int sensor_reg_read(int fd, int regAddr)
{

	int regVal = 0;
	unsigned char buf14[LI_XU_SENSOR_REG_RW_SIZE] = {0};
	CLEAR(buf14);

	buf14[0] = SENSOR_REG_READ_FLG;
	buf14[1] = HIGHBYTE(regAddr);
	buf14[2] = LOWBYTE(regAddr);

	write_to_UVC_extension(fd, LI_XU_SENSOR_REG_RW,
						   LI_XU_SENSOR_REG_RW_SIZE, buf14);
	buf14[0] = SENSOR_REG_READ_FLG;
	buf14[3] = 0;
	buf14[4] = 0;
	read_from_UVC_extension(fd, LI_XU_SENSOR_REG_RW,
							LI_XU_SENSOR_REG_RW_SIZE, buf14);

	regVal = (buf14[3] << 8) + buf14[4];
	printf("V4L2_CORE: Read Sensor REG[0x%x] = 0x%x\r\n", regAddr, regVal);
	return regVal;
}

/** 
 * This call will erase firmware saved in FX3 EEPROM, and then issue a reset for FX3 
 * !!!!!!!!!!!CALL IT WITH CAUTION!!!!!!!!!!!! 
 * args: 
 * 		fd 		- file descriptor
 */
void firmware_erase(int fd)
{
	unsigned char buf15[LI_XU_ERASE_EEPROM_SIZE] = {0};
	CLEAR(buf15);

	buf15[0] = ERASE_EEPROM_FLG;
	buf15[1] = 0;

	write_to_UVC_extension(fd, LI_XU_ERASE_EEPROM,
						   LI_XU_ERASE_EEPROM_SIZE, buf15);
	printf("V4L2_CORE: Firmware Erased\r\n");
}

/** 
 * This call will issue a reset for FX3 
 * !!!!!!!!!!!CALL IT WITH CAUTION!!!!!!!!!!!!  
 * args: 
 * 		fd 		- file descriptor
 */
void reboot_camera(int fd)
{
	unsigned char buf15[LI_XU_ERASE_EEPROM_SIZE] = {0};
	CLEAR(buf15);

	buf15[0] = REBOOT_CAM_FLG;
	buf15[1] = 0;

	write_to_UVC_extension(fd, LI_XU_ERASE_EEPROM,
						   LI_XU_ERASE_EEPROM_SIZE, buf15);
	printf("V4L2_CORE: Reboot Camera\r\n");
}

/** 
 * ONLY some of YUV camera driver will support this
 * args: 
 * 		fd 		- file descriptor
 * 		mode
 */
void set_spi_port_select(int fd, int mode)
{
	unsigned char buf15[LI_XU_ERASE_EEPROM_SIZE] = {0};
	CLEAR(buf15);

	buf15[0] = SET_SPI_PORT_SELECT(mode);
	buf15[1] = 0;

	write_to_UVC_extension(fd, LI_XU_ERASE_EEPROM,
						   LI_XU_ERASE_EEPROM_SIZE, buf15);
	printf("V4L2_CORE: Set SPI Port Select\r\n");
}

/**
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
void generic_I2C_write(int fd, int rw_flag, int bufCnt,
					   int slaveAddr, int regAddr, unsigned char *i2c_data)
{
	int regVal = 0;
	unsigned char buf16[LI_XU_GENERIC_I2C_RW_SIZE] = {0};
	CLEAR(buf16);

	buf16[0] = rw_flag;
	buf16[1] = bufCnt - 1;
	buf16[2] = HIGHBYTE(slaveAddr);
	buf16[3] = LOWBYTE(slaveAddr);
	buf16[4] = HIGHBYTE(regAddr);
	buf16[5] = LOWBYTE(regAddr);

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

/**
 *  register read/write for different slaves on I2C line
 *
 *  Register data starts from Byte4(8bit address) or Byte5(16bit address)
 *  args:	
 * 		fd 		 - file descriptor
 * 		rw_flag	 - please read the above for details
 * 		bufCnt	 - define register data length(8-bit/16-bit)
 * 		slaveAddr- I2C address for slave
 * 		regAddr  - register address want to access
 * 		i2c_data - pointer to register value   
 */
int generic_I2C_read(int fd, int rw_flag, int bufCnt,
					 int slaveAddr, int regAddr)

{
	int regVal = 0;
	unsigned char buf16[LI_XU_GENERIC_I2C_RW_SIZE] = {0};
	CLEAR(buf16);
	buf16[0] = rw_flag;
	buf16[1] = bufCnt - 1;
	buf16[2] = HIGHBYTE(slaveAddr);
	buf16[3] = LOWBYTE(slaveAddr);
	buf16[4] = HIGHBYTE(regAddr);
	buf16[5] = LOWBYTE(regAddr);
	write_to_UVC_extension(
		fd, 
		LI_XU_GENERIC_I2C_RW,
		LI_XU_GENERIC_I2C_RW_SIZE, 
		buf16);
	buf16[6] = 0;
	buf16[7] = 0;
	read_from_UVC_extension(
		fd, 
		LI_XU_GENERIC_I2C_RW,
		LI_XU_GENERIC_I2C_RW_SIZE, 
		buf16);
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

/** 
 *  write to camera defect pixel table
 *  args:	
 * 		fd 		 - file descriptor
 * 		w_buf	 - write data buffer
 */
void write_cam_defect_pixel_table(int fd, unsigned char *w_buf)
{
	unsigned char buf17[LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE] = {0};
	CLEAR(buf17);
	memcpy(buf17, w_buf, LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE);
	write_to_UVC_extension(fd, LI_XU_SENSOR_DEFECT_PIXEL_TABLE,
						   LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE, buf17);
	printf("V4L2_CORE: Write Camera Defect Pixel Table: %s\r\n", w_buf);
}

/** 
 *  read from camera defect pixel table
 *  args:	
 * 		fd 		 - file descriptor
 * 		r_buf	 - read data buffer
 */
void read_cam_defect_pixel_table(int fd, unsigned char *r_buf)
{
	unsigned char buf17[LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE] = {0};
	CLEAR(buf17);
	read_from_UVC_extension(fd, LI_XU_SENSOR_DEFECT_PIXEL_TABLE,
							LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE, buf17);
	memcpy(r_buf, buf17, LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE);
	printf("V4L2_CORE: Read Camera Defect Pixel Table: %s\r\n", buf17);
}

#ifdef FOR_FPGA_UPDATE
//TODO: hasn't tested yet...
int eeprom_page_write(int fd)
{
	sensor_reg_write(fd, HEADER_EEPROM_PAGE_PROG, 0);
	return eeprom_process_done(fd, 10); // max 10ms
}
int eeprom_bulk_erase(int fd)
{
	sensor_reg_write(fd, HEADER_EEPROM_BULK_ERASE, 0);
	return eeprom_process_done(fd, 100 * 1000); // max 100s
}
int eeprom_process_done(int fd, int total_time)
{
	int reg_val = 1;

	// try 100 times
	for (int i = 0; i < total_time; i++)
	{
		sensor_reg_write(fd, HEADER_EEPROM_RDSR, 0);

		reg_val = sensor_reg_read(fd, HEADER_EEPROM_RDSR);
		if ((reg_val & 0x01) == 0x0)
			return 0; // true
	}
	return 1; //false
}
int eeprom_verify_page(int fd)
{
	int reg_val = 1;
	sensor_reg_write(fd, HEADER_EEPROM_VERIFY, 0);
	reg_val = sensor_reg_read(fd, HEADER_EEPROM_VERIFY);
	if ((reg_val & 0xff) == 0x0)
		return 0; //true
	return 1;	 // false
}
// set 3-byte address
void eeprom_set_page_addr(int fd, int page_addr)
{
	sensor_reg_write(fd, HEADER_EEPROM_UPDATE_ADDR, (page_addr >> 16) & 0xff);
	sensor_reg_write(fd, HEADER_EEPROM_UPDATE_ADDR, ((page_addr >> 8) & 0xff) | 0x0100);
	sensor_reg_write(fd, HEADER_EEPROM_UPDATE_ADDR, ((page_addr)&0xff) | 0x0200);
}

// fill 256-byte buffer in FX3
// for AR01335_ICP3:
// buf17[0] = 0xaa; // flag to trigger EEPROM reflash
void eeprom_fill_page_buffer(int fd, unsigned char *buf, int len)
{
	unsigned char buf17[LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE] = {0};
	CLEAR(buf17);
	int addr = 0;
#ifdef AR1335_ICP3
	int remain_length = len;
	int position = 0;

	while (remain_length > 0)
	{
		printf("..");
		fflush(stdout);
		for (int i = 0; i < LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE; i++)
			buf17[i] = 0;
		buf17[0] = 0xaa;

		memcpy(&buf17[1], buf + position, LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE - 1);
		write_cam_defect_pixel_table(fd, buf17);

		usleep(2000);
		remain_length -= 32;
		position += 32;
	}
#endif

	for (addr = 0; addr < len; addr += 32)
	{
		buf17[0] = 0x0;
		memcpy(&buf17[1], buf, LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE - 1);
		write_cam_defect_pixel_table(fd, buf17);
	}
	if (addr != len)
	{
		for (int i = 0; i < 256; i++)
			buf17[i] = 0xff;
		buf17[0] = (unsigned char)addr;
		memcpy(&buf17[1], buf + addr, len - addr);
	}
}
#endif
//////////////////////////////////////////////////////////////////////////////
//			AR0231 AP0200 HOST COMMAND API									//		
// 			FOR FLASHING AR0231 AP0200 BINARY ON THE FLY					//      	
//////////////////////////////////////////////////////////////////////////////
// s_buf[0] : type : 0: soft reset, 1: read data, 2,3: write command
// s_buf[1] : count ( including address & data)
// s_buf[2:3] : address{addrH, addrL}
// s_buf[4-] : data

/* AP020X software reset */
void ap020x_soft_reset(int fd)
{
	unsigned char s_buf[4];
	CLEAR(s_buf); // s_buf[0] = 0 for reset
	write_cam_defect_pixel_table(fd, s_buf);
	printf("Software Reset\r\n");
}

/* AP020X read data from register address */
void ap020x_read_data(int fd, int reg_addr, int count)
{
	unsigned char s_buf[4];
	s_buf[0] = IMG_SENSOR_UPDATE_DPT_RD_FLG;
	s_buf[1] = (unsigned char)count;
	s_buf[2] = HIGHBYTE(reg_addr);
	s_buf[3] = LOWBYTE(reg_addr);
	write_cam_defect_pixel_table(fd, s_buf);
	read_cam_defect_pixel_table(fd, s_buf);
	for (int i = 0; i < LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE; i++)
	{
		printf("buf17[%d] = 0x%x\r\n", i, s_buf[i]);
	}
}

void ap020x_write_data(int fd, int buf_size, int pos, unsigned char *buf)
{
	if (buf_size > 16)
		return;
	unsigned char s_buf[33];
	CLEAR(s_buf);
	s_buf[0] = IMG_SENSOR_UPDATE_DPT_DATA_WR_FLG;
	s_buf[1] = (unsigned char)(buf_size + 7 + 2);
	s_buf[CMD_START_POS] = (AP020X_HOST_CMD_PARAM_POOL >> 8) & 0xff;
	s_buf[CMD_START_POS + 1] = AP020X_HOST_CMD_PARAM_POOL & 0xff;
	s_buf[CMD_START_POS + 2] = (unsigned char)((pos >> 24) & 0xff);
	s_buf[CMD_START_POS + 3] = (unsigned char)((pos >> 16) & 0xff);
	s_buf[CMD_START_POS + 4] = (unsigned char)((pos >> 8) & 0xff);
	s_buf[CMD_START_POS + 5] = (unsigned char)(pos & 0xff);
	s_buf[CMD_START_POS + 6] = buf_size;

	memcpy(&s_buf[CMD_START_POS + 7], buf + pos, buf_size);
	write_cam_defect_pixel_table(fd, s_buf);
}

void ap020x_send_command(int fd, AP0200_CMD cmd, int time_out)
{

	int retry;
	unsigned char r_buf[33] = {0};
	unsigned char s_buf[6];

	s_buf[0] = IMG_SENSOR_UPDATE_DPT_CMD_WR_FLG;
	s_buf[1] = 4; // command length = 4 bytes
	s_buf[CMD_START_POS] = HIGHBYTE(AP020X_SYSCTL_REG);
	s_buf[CMD_START_POS + 1] = LOWBYTE(AP020X_SYSCTL_REG);
	s_buf[CMD_START_POS + 2] = HIGHBYTE(cmd);
	s_buf[CMD_START_POS + 3] = LOWBYTE(cmd);

	retry = 0;

	while (retry < time_out)
	{
		write_cam_defect_pixel_table(fd, s_buf);
		read_cam_defect_pixel_table(fd, r_buf);
		if (s_buf[0] == 0x00 && s_buf[1] == 0x00) // doorbell cleared
			break;
		retry++;
		usleep(10);
	}

	if (retry == time_out)
	{
		printf("Error: %d didn't go through\r\n", cmd);
	}
}

/*r945 r940 support AP0200/AP0202 isp file update on the fly.
	NOTES: ap0200/ap0202 flash update.
    do not reset the ap0200/ap0202 before update flash, 
	for some case if user flash incorrect flash file and
	want short circle to bypass the incorrect isp file. reboot is no need.

*/
void ap020x_program_flash_eeprom(int fd, unsigned char *bin_buf, int bin_size)
{
	int pos = 0;
	int page_remaining = 0;
	int step = 0;

	unsigned char s_buf[20];
	AP0200_CMD AP0200_CMD_EXP;
	//int flash_update_in_progress = 0;
	int flash_update_percentage = 0;

	usleep(800);
	ap020x_read_data(fd, 0x00, 2);
	ap020x_send_command(fd, AP0200_CMD_EXP = CMD_GET_LOCK, 5);
	usleep(50);
	ap020x_send_command(fd, AP0200_CMD_EXP = CMD_LOCK_STATUS, 5);
	usleep(50);

	CLEAR(s_buf);
	s_buf[0] = IMG_SENSOR_UPDATE_DPT_CMD_WR_FLG;
	s_buf[1] = (unsigned char)(8 + 2); //count
	s_buf[CMD_START_POS] = HIGHBYTE(AP020X_HOST_CMD_PARAM_POOL);
	s_buf[CMD_START_POS + 1] = LOWBYTE(AP020X_HOST_CMD_PARAM_POOL);
	s_buf[CMD_START_POS + 2] = 2;
	s_buf[CMD_START_POS + 3] = 0;
	s_buf[CMD_START_POS + 4] = 3;
	s_buf[CMD_START_POS + 5] = 0x18;
	s_buf[CMD_START_POS + 6] = 0;
	s_buf[CMD_START_POS + 7] = 0x2;
	s_buf[CMD_START_POS + 8] = 0;
	s_buf[CMD_START_POS + 9] = 0;
	write_cam_defect_pixel_table(fd, s_buf);
	usleep(50);
	ap020x_send_command(fd, AP0200_CMD_EXP = CMD_CONFIG_DEV, 5);
	usleep(50);
	ap020x_send_command(fd, AP0200_CMD_EXP = CMD_RELEASE_LOCK, 5);
	usleep(50);

	CLEAR(s_buf);
	s_buf[0] = IMG_SENSOR_UPDATE_DPT_CMD_WR_FLG;
	s_buf[1] = (unsigned char)(CMD_START_POS + 16); //count
	s_buf[CMD_START_POS] = HIGHBYTE(AP020X_HOST_CMD_PARAM_POOL);
	s_buf[CMD_START_POS + 1] = LOWBYTE(AP020X_HOST_CMD_PARAM_POOL);
	write_cam_defect_pixel_table(fd, s_buf);
	usleep(50);

	for (int i = 0; i < 15; i++)
	{
		s_buf[1] = (unsigned char)(CMD_START_POS + 16);
		s_buf[CMD_START_POS] = (AP020X_HOST_CMD_PARAM_POOL >> 8) & 0xff;
		s_buf[CMD_START_POS + 1] = (unsigned char)(i * 16);
		write_cam_defect_pixel_table(fd, s_buf);
		usleep(50);
	}

	ap020x_send_command(fd, AP0200_CMD_EXP = CMD_GET_LOCK, 5);
	usleep(50);
	ap020x_send_command(fd, AP0200_CMD_EXP = CMD_LOCK_STATUS, 5);
	usleep(50);
	ap020x_send_command(fd, AP0200_CMD_EXP = CMD_QUERY_DEV, 5);
	usleep(50);
	ap020x_send_command(fd, AP0200_CMD_EXP = CMD_FLASH_STATUS, 5);
	usleep(50);
	usleep(50);

	pos = 0;

	while (pos < bin_size)
	{
		if (bin_size - pos > PACKET_SIZE)
		{
			page_remaining = 0x0100 - (pos & 0x00ff);

			if (page_remaining > PACKET_SIZE)
			{
				ap020x_write_data(fd, PACKET_SIZE, pos, bin_buf);
				pos += PACKET_SIZE;
			}
			else
			{
				ap020x_write_data(fd, page_remaining, pos, bin_buf);
				pos += page_remaining;
			}
		}
		else
		{
			ap020x_write_data(fd, bin_size - pos, pos, bin_buf);
			pos = bin_size;
		}
		ap020x_send_command(fd, AP0200_CMD_EXP = CMD_WRITE, 50);
		ap020x_send_command(fd, AP0200_CMD_EXP = CMD_FLASH_STATUS, 50);

		if (pos > bin_size * step / 100)
			step++;
		flash_update_percentage = pos * 100 / bin_size;
		printf("flash update percentage = %d%%\r\n", flash_update_percentage);
	}
	usleep(50);
	ap020x_send_command(fd, AP0200_CMD_EXP = CMD_RELEASE_LOCK, 5);
	usleep(50);
}

//////////////////////////////////////////////////////////////////////////////
//			OV580 HOST COMMAND API						        			//		
// 			FOR CTRLS OF OV580                          					//      	
//////////////////////////////////////////////////////////////////////////////
/**
 * helper function to communicate with OV580 UVC defined extension unit
 * args:
 * 		fd 			- file descriptor
 * 		buffer		- pointer for buffer data
 *  
 */
void ov580_write_to_UVC_extension(
	int fd, 
	unsigned char *buffer)
{
	struct uvc_xu_control_query xu_query;
	CLEAR(xu_query);
	xu_query.unit =  OV580_XU_UNIT;			 
	xu_query.query = UVC_SET_CUR;
	xu_query.size = OV580_PACKET_SIZE;
	xu_query.selector = OV580_XU_SELECTOR;
	xu_query.data = buffer; 

	if ((ioctl(fd, UVCIOC_CTRL_QUERY, &xu_query)) != 0)
		error_handle_extension_unit();
}
/**
 * helper function to communicate with OV580 UVC defined extension unit
 * args:
 * 		fd 			- file descriptor
 * 		buffer		- pointer for buffer data
 *  
 */
void ov580_read_from_UVC_extension(
	int fd, 
	unsigned char *buffer)
{
	struct uvc_xu_control_query xu_query;
	CLEAR(xu_query);
	xu_query.unit = OV580_XU_UNIT;			  
	xu_query.query = UVC_GET_CUR; 
	xu_query.size = OV580_PACKET_SIZE;
	xu_query.selector = OV580_XU_SELECTOR;
	xu_query.data = buffer; 

	if (ioctl(fd, UVCIOC_CTRL_QUERY, &xu_query) != 0)
		error_handle_extension_unit();
}

void ov580_write_system_reg(
	int fd, 
	int reg_addr, 
	unsigned char reg_val)
{
	unsigned char w_buf[OV580_PACKET_SIZE] = {0};
	CLEAR(w_buf);
	w_buf[0] = OV580_WRITE_CMD_ID;
	w_buf[1] = OV580_SYSTEM_REG;
	w_buf[2] = 0x20;
	w_buf[3] = 0x4; 					// address width = 4 bytes
	w_buf[4] = 0x1; 					// data width = 1 byte
	w_buf[5] = 0x80; 					// reg addr, big endian, 0x8018xxxx
	w_buf[6] = 0x18;
	w_buf[7] = HIGHBYTE(reg_addr);
	w_buf[8] = LOWBYTE(reg_addr);
	w_buf[9] = 0x00; 					// reg number(big endian) = 1 byte
	w_buf[10] = 0x01;
	w_buf[11] = 0x00;
	w_buf[12] = 0x01;
	w_buf[16] = reg_val;
	
	ov580_write_to_UVC_extension(fd, w_buf); 
	printf("ov580_write_system_reg: regAddr=0x%x, regVal=0x%x\r\n", 
		reg_addr, reg_val);
}

unsigned char ov580_read_system_reg(
	int fd,
	int reg_addr)
{
	unsigned char w_buf[OV580_PACKET_SIZE] = {0};
	CLEAR(w_buf);
	w_buf[0] = OV580_READ_CMD_ID;
	w_buf[1] = OV580_SYSTEM_REG;
	w_buf[2] = 0x20;
	w_buf[3] = 0x04; 					// address width = 4 bytes
	w_buf[4] = 0x01; 					// data width = 1 byte
	w_buf[5] = 0x80; 					// reg addr, big endian, 0x8018xxxx
	w_buf[6] = 0x18;
	w_buf[7] = HIGHBYTE(reg_addr);
	w_buf[8] = LOWBYTE(reg_addr);
	w_buf[9] = 0x00; 					// reg number(big endian) = 1 byte
	w_buf[10] = 0x01;
	w_buf[11] = 0x00;
	w_buf[12] = 0x01;
	ov580_read_from_UVC_extension(
		fd, 
		w_buf); 
	unsigned char reg_val = w_buf[16];
	printf("ov580_read_system_reg: regAddr=0x%x, regVal=0x%x\r\n", 
		reg_addr, w_buf[16]);
	// printf("buf[0]=0x%x, buf[1]=0x%x, buf[2]=0x%x, buf[3]=0x%x, buf[4]=0x%x,"
	// 	"buf[5]=0x%x, buf[6]=0x%x, buf[7]=0x%x, buf[8]=0x%x,buf[9]=0x%x," 
	// 	"buf[10]=0x%x, buf[11]=0x%x, buf[12]=0x%x, buf[13]=0x%x, buf[16]=0x%x\r\n", 
	// 	w_buf[0], w_buf[1], w_buf[2], w_buf[3], w_buf[4],
	// 	w_buf[5], w_buf[6], w_buf[7], w_buf[8], w_buf[9],
	// 	w_buf[10], w_buf[11], w_buf[12], w_buf[13], w_buf[16]);
	return reg_val;
}

void ov580_write_sccb0_reg(
	int fd, 
	unsigned char slave_addr,
	int reg_addr, 
	unsigned char reg_val)
{
	unsigned char w_buf[OV580_PACKET_SIZE] = {0};
	CLEAR(w_buf);
	w_buf[0] = OV580_WRITE_CMD_ID;
	w_buf[1] = OV580_SENSOR_SCCB0_REG;
	w_buf[2] = slave_addr;
	w_buf[3] = 2; 					// address width, 1->8-bit, 2->16-bit
	w_buf[4] = 1; 					// data width = 1 byte
	w_buf[7] = HIGHBYTE(reg_addr);	// reg addr, big endian, 0xXXXX
	w_buf[8] = LOWBYTE(reg_addr);
	w_buf[9] = 0x00; 				// reg number(big endian) = 1 byte
	w_buf[10] = 0x01;
	w_buf[16] = reg_val;
	
	ov580_write_to_UVC_extension(
		fd, 
		w_buf); 
}

unsigned char ov580_read_sccb0_reg(
	int fd,
	unsigned char slave_addr,
	int reg_addr)
{
	unsigned char w_buf[OV580_PACKET_SIZE] = {0};
	CLEAR(w_buf);
	w_buf[0] = OV580_READ_CMD_ID;
	w_buf[1] = OV580_SENSOR_SCCB0_REG;
	w_buf[2] = slave_addr;
	w_buf[3] = 2; 					// address width = 2 bytes
	w_buf[4] = 1; 					// data width = 1 byte
	w_buf[7] = HIGHBYTE(reg_addr);
	w_buf[8] = LOWBYTE(reg_addr);
	w_buf[9] = 0x00; 				// reg number(big endian) = 1 byte
	w_buf[10] = 0x01;

	ov580_read_from_UVC_extension(
		fd, 
		w_buf); 
	printf("ov580_read_slave_reg: slaveAddr=0x%x, regAddr=0x%x, regVal=0x%x\r\n", 
		slave_addr, reg_addr, w_buf[16]);
	return w_buf[16];
}

void ov580_write_sccb1_reg(
	int fd, 
	unsigned char slave_addr,
	int reg_addr, 
	unsigned char reg_val)
{
	unsigned char w_buf[OV580_PACKET_SIZE] = {0};
	CLEAR(w_buf);
	w_buf[0] = OV580_WRITE_CMD_ID;
	w_buf[1] = OV580_SENSOR_SCCB1_REG;
	w_buf[2] = slave_addr;
	w_buf[3] = 2; 					// address width, 1->8-bit, 2->16-bit
	w_buf[4] = 1; 					// data width = 1 byte
	w_buf[7] = HIGHBYTE(reg_addr);	// reg addr, big endian, 0xXXXX
	w_buf[8] = LOWBYTE(reg_addr);
	w_buf[9] = 0x00; 				// reg number(big endian) = 1 byte
	w_buf[10] = 0x01;
	w_buf[16] = reg_val;
	
	ov580_write_to_UVC_extension(
		fd, 
		w_buf); 
}

unsigned char ov580_read_sccb1_reg(
	int fd,
	unsigned char slave_addr,
	int reg_addr)
{
	unsigned char w_buf[OV580_PACKET_SIZE] = {0};
	CLEAR(w_buf);
	w_buf[0] = OV580_READ_CMD_ID;
	w_buf[1] = OV580_SENSOR_SCCB1_REG;
	w_buf[2] = slave_addr;
	w_buf[3] = 4; 					// address width = 4 bytes
	w_buf[4] = 1; 					// data width = 1 byte
	w_buf[7] = HIGHBYTE(reg_addr);
	w_buf[8] = LOWBYTE(reg_addr);
	w_buf[9] = 0x00; 				// reg number(big endian) = 1 byte
	w_buf[10] = 0x01;

	ov580_read_from_UVC_extension(
		fd, 
		w_buf); 
	return w_buf[16];
}
//////////////////////////////////////////////////////////////////////////////
//			TESTING MACROS													//
//			Some macros for testing different sensor XU features			//
//			Enabling them in ../includes/hardware.h							//			
//////////////////////////////////////////////////////////////////////////////
#ifdef AP0202_WRITE_REG_ON_THE_FLY
void ap0202_write_reg_on_the_fly(int fd)
{
	printf("**********AP0202 write reg on the fly************************\n");
	unsigned int i;
	for (i = 0; i < sizeof(ChangConfig) / sizeof(reg_seq); i++)
		generic_I2C_write(fd, 0x82, ChangConfig[i].reg_data_width,
						  AP020X_I2C_ADDR, ChangConfig[i].reg_addr,
						  (unsigned char *)&(ChangConfig[i].reg_val));
	for (i = 0; i < sizeof(ChangConfig) / sizeof(reg_seq); i++)
	{
		generic_I2C_read(fd, 0x02, ChangConfig[i].reg_data_width,
						 AP020X_I2C_ADDR, ChangConfig[i].reg_addr);
	}
	generic_I2C_read(fd, 0x02, 1, MAX9295_SER_I2C_ADDR, 0x0000);
	generic_I2C_read(fd, 0x02, 1, MAX9296_DESER_I2C_ADDR, 0x0000);
}
#endif

#ifdef AP0202_WRITE_REG_IN_FLASH
void ap0202_write_reg_in_flash(int fd)
{
	printf("**********AP0202 write reg in flash**************************\n");
	load_register_setting_from_configuration(fd,
		SIZE(ChangConfigFromFlash), ChangConfigFromFlash);

	sleep(1);
	//generic_I2C_read(v4l2_dev, 0x02, 2, AP020X_I2C_ADDR, 0x0058);
	sensor_reg_write(fd, 0x5080, 0x00);
	sensor_reg_read (fd, 0x4308);
	sensor_reg_read (fd, 0x5080);
}
#endif
#ifdef OS05A20_PTS_QUERY
void os05a20_pts_query(int fd)
{
	printf("**********OS05A20 pts query**********************************\n");
	set_pts(fd, 0);
	get_pts(fd);
}
#endif 
#ifdef AR0231_MIPI_TESTING
void ar0231_mipi_testing(int fd)
{
	printf("**********AR0231 reg mipi testing****************************\n");
	unsigned int i;
	for (i = 0; i < sizeof(AR0231_MIPI_REG_TESTING) / sizeof(reg_seq); i++)
	{
		//choose either one of the function below for register read
		generic_I2C_read(fd, 0x02, AR0231_MIPI_REG_TESTING[i].reg_data_width,
						 AR0231_I2C_ADDR, AR0231_MIPI_REG_TESTING[i].reg_addr);
		sensor_reg_read(fd, AR0231_MIPI_REG_TESTING[i].reg_addr);
	}
}
#endif

#ifdef IMX334_MONO_MIPI_TESTING
void imx334_mipi_testing(int fd)
{
	printf("**********IMX334 reg mipi testing****************************\n");
	unsigned int i;
	for (i = 0; i < sizeof(IMX334_MIPI_REG_TESTING) / sizeof(reg_seq); i++)
	{
		//TODO: choose either one of the function below for register read
		generic_I2C_read(fd, 0x02, IMX334_MIPI_REG_TESTING[i].reg_data_width,
						 IMX334_I2C_ADDR, IMX334_MIPI_REG_TESTING[i].reg_addr);
		sensor_reg_read(fd, IMX334_MIPI_REG_TESTING[i].reg_addr);
	}
}
#endif