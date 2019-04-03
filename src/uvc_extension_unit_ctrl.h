#pragma once
#ifndef _UVC_EXTENSION_EXTRA_H_
#define _UVC_EXTENSION_EXTRA_H_
/****************************************************************************
**                      	Global data 
*****************************************************************************/

// define the Leopard Imaging USB3.0 Camera
// uvc extension id
#define LI_XU_SENSOR_MODES_SWITCH (0x01)
#define LI_XU_SENSOR_WINDOW_REPOSITION (0x02)
#define LI_XU_LED_MODES (0x03)
#define LI_XU_SENSOR_GAIN_CONTROL_RGB (0x04)
#define LI_XU_SENSOR_GAIN_CONTROL_A (0x05)
#define LI_XU_SENSOR_EXPOSURE_TIME (0x06)
#define LI_XU_SENSOR_UUID_HWFW_REV (0x07)
#define LI_XU_PTS_QUERY (0x08) // not suppoprted by all usb cameras
#define LI_XU_SOFT_TRIGGER (0x09)
#define LI_XU_TRIGGER_DELAY (0x0a)
#define LI_XU_EX_MAX_MIN_INFO (0x0b)
#define LI_XU_SENSOR_REGISTER_CONFIGURATION (0x0c) // not supported by all usb cameras
#define LI_XU_SENSOR_REG_RW (0x0e)
#define LI_XU_ERASE_EEPROM (0x0f)
#define LI_XU_GENERIC_I2C_RW (0x10)
#define LI_XU_SENSOR_DEFECT_PIXEL_TABLE (0x11)

// I2C slave address list
//  On-semi
#define AP020X_I2C_ADDR (0xBA)
#define AR0231_I2C_ADDR (0x20)
#define AR0144_I2C_ADDR (0x20)
// Sony
#define IMX334_I2C_ADDR (0x34)
#define IMX390_I2C_ADDR (0x34)
#define IMX324_I2C_ADDR (0x34)
// Omnivision
#define OV2311_I2C_ADDR (0xC0)
#define OS05A20_I2C_ADDR (0x6C)
// Maxim
#define MAX9295_SER_I2C_ADDR (0x80)
#define MAX9296_DESER_I2C_ADDR (0x90)

typedef struct reg_pair
{
	unsigned short reg_addr;
	unsigned short reg_val;
} reg_pair;

typedef struct reg_seq
{
	unsigned char reg_data_width;
	unsigned short reg_addr;
	unsigned short reg_val;
} reg_seq;

#define MAX_PAIR_FOR_SPI_FLASH (64)

//test registers on the fly
#ifdef AP0202_WRITE_REG_ON_THE_FLY
// TODO: used in helper function, modify it for different camera
const reg_seq ChangConfig[] =
	{
		{2, 0x098e, 0x7c00},
		{2, 0xfc00, 0x2800},
		{2, 0x0040, 0x8100},
		{2, 0x0058, 0x4444}};
const reg_seq TrigEnable[] =
	{
		{2, 0x098e, 0xc890},
		{1, 0xc890, 0x03},
		{1, 0xc891, 0x03},
		{1, 0xc892, 0x00}};
const reg_seq TrigDisable[] =
	{
		{2, 0x098e, 0xc890},
		{1, 0xc890, 0x00}};
#endif

#ifdef AP0202_WRITE_REG_IN_FLASH
// used for SPI flash
// notice it only support 16-bit data with SPI flash
// TODO: put the register you want to set in here
const reg_pair ChangConfigFromFlash[] =
	{
		{0x305e, 0x0222}, //customer_rev
};
#endif

//#define AR0231_MIPI_TESTING
#ifdef AR0231_MIPI_TESTING
const reg_seq AR0231_MIPI_REG_TESTING[] =
	{
		{2, 0x3064, 0x1802}, // SMIA_TEST
		{2, 0x3056, 0x0080}, // GREEN1_GAIN
		{2, 0x3058, 0x0080}, // BLUE_GAIN
		{2, 0x305a, 0x0080}, // RED_GAIN
		{2, 0x305c, 0x0080}, //GREEN2_GAIN
		{2, 0x3138, 0x000B}  //OTPM_TCFG_OPT
};
#endif

//#define IMX334_MONO_MIPI_TESTING
#ifdef IMX334_MONO_MIPI_TESTING
const reg_seq IMX334_MIPI_REG_TESTING[] =
	{
		{1, 0x30E8, 0x14}, // PROGRAMMABLE_GAIN_CONTROL
		{1, 0x3302, 0x32}, // BLKLEVEL[7:0]
		{1, 0x3303, 0x00}, // BLKLEVEL[1:0] range 0x0-0x3ff
};
#endif

/****************************************************************************
**							 Function declaration
*****************************************************************************/
/*
 * handle the error for extension unit control
 * args:
 * 		none
 * returns:
 * 		none
 */
void error_handle_extension_unit();
/*
 * helper function to commnunicate with FX3 UVC defined extension unit
 * args:
 * 		fd 			- file descriptor
 *  	property_id - defined Leopard Imaging USB3.0 Camera uvc extension id
 * 		length 		- size of defined extension unit
 * 		buffer		- pointer for buffer data
 *  
 */
void write_to_UVC_extension(struct device *dev, int property_id,
							int length, unsigned char *buffer);

/*
 * helper function to commnunicate with FX3 UVC defined extension unit
 * args:
 * 		fd 			- file descriptor
 *  	property_id - defined Leopard Imaging USB3.0 Camera uvc extension id
 * 		length 		- size of defined extension unit
 * 		buffer		- pointer for buffer data
 *  
 */
void read_from_UVC_extension(struct device *dev, int property_id,
							 int length, unsigned char *buffer);

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
void generic_I2C_write(struct device *dev, int rw_flag, int bufCnt,
					   int slaveAddr, int regAddr, unsigned char *i2c_data);

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
void generic_I2C_read(struct device *dev, int rw_flag, int bufCnt,
					  int slaveAddr, int regAddr);

/* 
 * I2C register read/write for the sensor 
 * args: 
 * 		fd 		- file descriptor
 * 		regAddr - register address want to access
 *		regVal  - register value to write
 */
void sensor_reg_write(struct device *dev, int regAddr, int regVal);

/* 
 * I2C register read/write for the sensor 
 * args: 
 * 		fd 		- file descriptor
 * 		regAddr - register address want to access
 */
void sensor_reg_read(struct device *dev, int regAddr);

/*
 *	save register to spi flash on FX3, load it automatically when boot time
 *  flash for storage is set to be 256 bytes
 *  args:
 * 		fd 		 - file descriptor
 * 		regCount - pairs of regAddr and regVal (up to 62)
 * 		buffer   - sensor register configuration
 */
void load_register_setting_from_configuration(struct device *dev, int regCount,
											  const struct reg_pair *buffer);

/*
 *	load register to spi flash on FX3 manually     
 *  #########FOR TEST ONLY###########
 *  flash for storage is set to be 256 bytes
 *  args:
 * 		fd 		 - file descriptor
 */
void load_register_setting_from_flash_manually(struct device *dev);

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
 * for different camera boards, crystall will slightly drift over time
 * PTS increments over time can be calculated by 
 * 		(1/frame_rate)/(1/25MHz) = 25MHz/frame_rate
 * 
 * this method is an add-on extension unit for query PTS info
 * args:
 * 		fd 		 - file descriptor
 */
void get_pts(struct device *dev);

/* set PTS counter initial value
 * args:
 * 		fd 		- file descriptor
 * 		initVal	- initial counter start value for PTS(4-byte long)
 *
 */
void set_pts(struct device *dev, unsigned long initVal);

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
void set_sensor_gain_rgb(struct device *dev, unsigned int rGain,
						 unsigned int grGain,
						 unsigned int gbGain,
						 unsigned int bGain);

#endif