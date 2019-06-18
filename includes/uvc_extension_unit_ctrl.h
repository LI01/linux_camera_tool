/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

  This is the sample code for Leopard USB3.0 camera for register control & 
  trigger/strobe & fw&hw revision under Linux using V4L2. For supporting more
  UVC extension unit features, firmware will need to get updated.
  
  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/

#pragma once
/*****************************************************************************
**                      	Global data 
*****************************************************************************/

/** 
 * define the Leopard Imaging USB3.0 Camera
 * uvc extension id selector
 */
#define LI_XU_SENSOR_MODES_SWITCH           (0x01)
#define LI_XU_SENSOR_WINDOW_REPOSITION      (0x02)
#define LI_XU_LED_MODES                     (0x03)
#define LI_XU_SENSOR_GAIN_CONTROL_RGB       (0x04)
#define LI_XU_SENSOR_GAIN_CONTROL_A         (0x05)

#define LI_XU_SENSOR_UUID_HWFW_REV          (0x07)
#define LI_XU_PTS_QUERY                     (0x08) 
#define LI_XU_SOFT_TRIGGER                  (0x09)
#define LI_XU_TRIGGER_DELAY                 (0x0a)
#define LI_XU_TRIGGER_MODE                  (0x0b)
#define LI_XU_SENSOR_REGISTER_CONFIGURATION (0x0c) 
#define LI_XU_SENSOR_REG_RW                 (0x0e)
#define LI_XU_ERASE_EEPROM                  (0x0f)
#define LI_XU_GENERIC_I2C_RW                (0x10)
#define LI_XU_SENSOR_DEFECT_PIXEL_TABLE     (0x11)
#define LI_XU_SENSOR_REGISTER_CONFIG        (0x1f) 
/** 
 * define the Leopard Imaging USB3.0 Camera
 * uvc extension id buffer size
 */
#define LI_XU_SENSOR_MODES_SWITCH_SIZE          (2)
#define LI_XU_SENSOR_WINDOW_REPOSITION_SIZE     (8)
#define LI_XU_LED_MODES_SIZE                    (1)
#define LI_XU_SENSOR_GAIN_CONTROL_RGB_SIZE      (8)
#define LI_XU_SENSOR_GAIN_CONTROL_A_SIZE        (2)

#define LI_XU_SENSOR_UUID_HWFW_REV_SIZE         (49)
#define LI_XU_PTS_QUERY_SIZE                    (4)
#define LI_XU_SOFT_TRIGGER_SIZE                 (2)
#define LI_XU_TRIGGER_DELAY_SIZE                (4)
#define LI_XU_TRIGGER_MODE_SIZE                 (2)
#define LI_XU_SENSOR_REGISTER_CONFIGURATION_SIZE (256) 
#define LI_XU_SENSOR_REG_RW_SIZE                (5)
#define LI_XU_ERASE_EEPROM_SIZE                 (2)
#define LI_XU_GENERIC_I2C_RW_SIZE               (262)
#define LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE    (33)

#define SERIAL_NUMBER_WR_FLG        (0xA5)

// for FX3 EEPROM functionality
#define HEADER_EEPROM_VERIFY	    (0x00F9)    // verify a page
#define HEADER_EEPROM_PAGE_PROG     (0x00FA)    // program one page
#define HEADER_EEPROM_BULK_ERASE    (0x00FB)    // send bulk erase command
#define HEADER_EEPROM_UPDATE_BUF    (0x00FC)    // update EEPROM page buffer, 256 bytes
#define HEADER_EEPROM_UPATE_ADDR    (0x00FD)    // update EEPROM page address, 3 bytes
#define HEADER_EEPROM_RDSR          (0x00FE)    // EEPROM read status

// uvc extension unit flag for FX3 EEPROM
#define ERASE_EEPROM_FLG    (0x9a)
#define REBOOT_CAM_FLG      (0x9b)
#define SET_SPI_PORT_SELECT(x) (0xa0|(x & 0x0f))

#define PACKET_SIZE (16)



/* Once a command has been written through the HCI, it will be 
 * executed by on-chip firmware. The results are reported back.
 * All commands are encoded with bit 15 set, which automatically
 * generates the 'host command' (doorbell) interrupt to ISP MCU
 */

// AP020X update defect pixel table flags
#define IMG_SENSOR_UPDATE_DPT_RST_FLG       (0)
#define IMG_SENSOR_UPDATE_DPT_RD_FLG        (1)
#define IMG_SENSOR_UPDATE_DPT_CMD_WR_FLG    (2)
#define IMG_SENSOR_UPDATE_DPT_DATA_WR_FLG   (3)

#define CMD_START_POS                       (2)

//AP020X HOST COMMAND INTERFACE(HCI) REG 
#define AP020X_HOST_CMD_PARAM_POOL  (0xFC00)
#define AP020X_SYSCTL_REG           (0x0040)


#define SENSOR_REG_WRITE_FLG    (1)
#define SENSOR_REG_READ_FLG     (0)

/** --- 8-bit I2C slave address list --- */
/**  On-semi Sensor */
#define AP020X_I2C_ADDR         (0xBA)
#define AR0231_I2C_ADDR         (0x20)// 0x30
#define AR0144_I2C_ADDR         (0x20)
/**   Sony Sensor   */
#define IMX334_I2C_ADDR         (0x34)
#define IMX390_I2C_ADDR         (0x34)
#define IMX324_I2C_ADDR         (0x34)
/** Omnivision Sensor */
#define OV2311_I2C_ADDR         (0xC0)
#define OS05A20_I2C_ADDR        (0x6C)
/** Toshiba Bridge */
#define TC_MIPI_BRIDGE_I2C_ADDR (0x1C)
/**  Maxim Serdes */
#define MAX96705_SER_I2C_ADDR   (0x80)
#define MAX9295_SER_I2C_ADDR    (0x80) //0x88, 0xC4
#define MAX9272_SER_I2C_ADDR    (0x90)
#define MAX9296_DESER_I2C_ADDR  (0x90) //0xD4
/** TI Serdes */
#define TI913_SER_I2C_ADDR      (0xB4)//0xB2
#define TI953_SER_I2C_ADDR      (0x60) 
#define TI914_DESER_I2C_ADDR    (0xC0)
#define TI954_DESER_I2C_ADDR    (0x30)


#define RAW_8_MODE					(0x1000)
#define RAW_10_MODE					(0x2000)
#define RAW_12_MODE					(0x3000)
#define YUY2_MODE					(0x4000)
#define RAW_8_DUAL_MODE             (0x5000)
#define JPEG_MODE                   (0x6000)


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


/* AP020X flash manger host command */
typedef enum {
    // TODO: this is from AP010X HCI command, check for AP020X compatibility
    CMD_GET_LOCK 		= 0x8500, // Request flash manager access lock
    CMD_LOCK_STATUS 	= 0x8501, // Retrive status of access lock request
	CMD_RELEASE_LOCK	= 0x8502, // Release flash maanger access lock
    CMD_FLASHMGR_CONFIG = 0x8503, // configure flash manager and underlying SPI NVM subsystem
    CMD_FLASHMGR_READ   = 0x8504, // Read data from the SPI NVM
    CMD_WRITE		    = 0x8505, // Write data to the SPI NVM
	CMD_ERASE_BLOCK		= 0x8506, // Erase a block of data from the SPI NVM
	CMD_QUERY_DEV		= 0x8508, // Query device−specific information
	CMD_FLASH_STATUS	= 0x8509, // Obtain status of current asynchronous operation
	CMD_CONFIG_DEV		= 0x850a, // Configure the attached SPI NVM device

    // CMD_CCIMGR_GET_LOCK     = 0x8D00,
    // CMD_CCIMGR_LOCK_STATUS  = 0x8D01,
    // CMD_CCIMGR_RELEASE_LOCK = 0x8D02,
    // CMD_CCIMGR_CONFIG       = 0x8D03,
    // CMD_CCIMGR_SET_DEVICE   = 0x8D04,
    // CMD_CCIMGR_READ         = 0x8D05,
    // CMD_CCIMGR_WRITE        =  0x8D06,
    // CMD_CCIMGR_WRITE_BITFIELD = 0x8D07,
    // CMD_CCIMGR_STATUS       = 0x8D08,

}AP0200_CMD;

/** --- test registers on the fly --- */
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

/*****************************************************************************
**							 Function declarations
*****************************************************************************/

/** --- helper function ---*/
void error_handle_extension_unit();

void write_to_UVC_extension(int fd, int property_id,
                            int length, unsigned char *buffer);

void read_from_UVC_extension(int fd, int property_id,
                             int length, unsigned char *buffer);

/**--------------------------------------------------------------------------- */

/** --- functions using extension unit call in camera driver --- */
/** these functions down below are the API you want to look at(paste) if you try 
 *  to implement some functionalities on your current software */
void set_sensor_mode(int fd, int mode);
void set_pos(int fd, int start_x, int start_y);
void get_led_status(int fd);
void set_led(int fd, int left_0, int left_1, int right_0, int right_1);
void set_sensor_gain_rgb(int fd,unsigned int rGain,
						 unsigned int grGain,
						 unsigned int gbGain,
						 unsigned int bGain);

int read_cam_uuid_hwfw_rev(int fd);
void sensor_set_serial_number(int fd, char *sn);

void get_pts(int fd);
void soft_trigger(int fd);
void trigger_delay_time(int fd, unsigned int delay_time);
void trigger_enable(int fd, int ena, int enb);

void load_register_setting_from_configuration(int fd,int regCount,
											  const struct reg_pair *buffer);
void load_register_setting_from_flash_manually(int fd);  

void sensor_reg_write(int fd,int regAddr, int regVal);
int sensor_reg_read(int fd,int regAddr);

void firmware_erase(int fd);
void reboot_camera(int fd);
void set_spi_port_select(int fd, int mode);

void generic_I2C_write(int fd,int rw_flag, int bufCnt,
					   int slaveAddr, int regAddr, unsigned char *i2c_data);
int generic_I2C_read(int fd,int rw_flag, int bufCnt,
					  int slaveAddr, int regAddr);                      

void write_cam_defect_pixel_table(int fd, unsigned char *w_buf);
void read_cam_defect_pixel_table(int fd, unsigned char *r_buf);

void ap020x_send_command(int fd, AP0200_CMD cmd, int time_out);
void ap020x_soft_reset(int fd);
void ap020x_read_data(int fd, int reg_addr, int count);
void ap020x_write_data(int fd, int buf_size, int pos, unsigned char *buf);
void ap020x_program_flash_eeprom(int fd, const unsigned char *bin_buf, int bin_size);

//void eeprom_fill_page_buffer(int fd, unsigned char *buf, int len);