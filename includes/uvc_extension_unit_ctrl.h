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
 *  This is the sample code for Leopard USB3.0 camera for register control &  
 *  trigger/strobe & fw&hw revision under Linux using V4L2. For supporting 	 
 *  more UVC extension unit features, firmware will need to get updated.      
 *  																			 *
 *  Author: Danyu L 														     *
 *  Last edit: 2019/08 														 
*****************************************************************************/
#pragma once
/*****************************************************************************
**                      	Global data 
*****************************************************************************/

/** 
 * define the Leopard Imaging USB3.0 Camera
 * uvc extension id selector
 */
typedef enum
{
    LI_XU_SENSOR_MODES_SWITCH           = 0x01,
    LI_XU_SENSOR_WINDOW_REPOSITION      = 0x02,
    LI_XU_LED_MODES                     = 0x03,
    LI_XU_SENSOR_GAIN_CONTROL_RGB       = 0x04,
  /*  LI_XU_SENSOR_GAIN_CONTROL_A         = 0x05, */
    LI_XU_SENSOR_NO_REG_I2C_RW          = 0x05,////////////////////
    LI_XU_SENSOR_UUID_HWFW_REV          = 0x07,
    LI_XU_PTS_QUERY                     = 0x08,
    LI_XU_SOFT_TRIGGER                  = 0x09,
    LI_XU_TRIGGER_DELAY                 = 0x0a,
    LI_XU_TRIGGER_MODE                  = 0x0b,
    LI_XU_SENSOR_REGISTER_CONFIGURATION = 0x0c,
    LI_XU_SENSOR_REG_RW                 = 0x0e,
    LI_XU_ERASE_EEPROM                  = 0x0f,
    LI_XU_GENERIC_I2C_RW                = 0x10,
    LI_XU_SENSOR_DEFECT_PIXEL_TABLE     = 0x11,
    LI_XU_SENSOR_REGISTER_CONFIG        = 0x1f,
} ext_unit_ctrl;

/** 
 * define the Leopard Imaging USB3.0 Camera
 * uvc extension id buffer size
 */
#define LI_XU_SENSOR_MODES_SWITCH_SIZE              (2)
#define LI_XU_SENSOR_WINDOW_REPOSITION_SIZE         (8)
#define LI_XU_LED_MODES_SIZE                        (1)
#define LI_XU_SENSOR_GAIN_CONTROL_RGB_SIZE          (8)
/*#define LI_XU_SENSOR_GAIN_CONTROL_A_SIZE            (2) */
#define LI_XU_SENSOR_NO_REG_I2C_RW_SIZE             (259)
#define LI_XU_SENSOR_UUID_HWFW_REV_SIZE             (49)
#define LI_XU_PTS_QUERY_SIZE                        (4)
#define LI_XU_SOFT_TRIGGER_SIZE                     (2)
#define LI_XU_TRIGGER_DELAY_SIZE                    (4)
#define LI_XU_TRIGGER_MODE_SIZE                     (2)
#define LI_XU_SENSOR_REGISTER_CONFIGURATION_SIZE    (256)
#define LI_XU_SENSOR_REG_RW_SIZE                    (5)
#define LI_XU_ERASE_EEPROM_SIZE                     (2)
#define LI_XU_GENERIC_I2C_RW_SIZE                   (262)
#define LI_XU_SENSOR_DEFECT_PIXEL_TABLE_SIZE        (33)

#define SERIAL_NUMBER_WR_FLG                        (0xA5)

/** --- for LI_XU_SENSOR_REG_RW  --- */
typedef struct reg_seq
{
    unsigned char reg_data_width;
    unsigned short reg_addr;
    unsigned short reg_val;
} reg_seq;

#define SENSOR_REG_WRITE_FLG    (1)
#define SENSOR_REG_READ_FLG     (0)

/** --- for LI_XU_SENSOR_REGISTER_CONFIGURATION --- */
typedef struct reg_pair
{
    unsigned short reg_addr;
    unsigned short reg_val;
} reg_pair;
#define MAX_PAIR_FOR_SPI_FLASH (64)

/** --- for FX3 EEPROM --- */
typedef enum
{
    HEADER_EEPROM_VERIFY     = 0x00F9,      // verify a page
    HEADER_EEPROM_PAGE_PROG  = 0x00FA,      // program one page
    HEADER_EEPROM_BULK_ERASE = 0x00FB,      // send bulk erase command
    HEADER_EEPROM_UPDATE_BUF = 0x00FC,      // update EEPROM page buffer, 256 bytes
    HEADER_EEPROM_UPDATE_ADDR = 0x00FD,      // update EEPROM page address, 3 bytes
    HEADER_EEPROM_RDSR       = 0x00FE,      // EEPROM read status
} fx3_eeprom_func;

#define ERASE_EEPROM_FLG        (0x9a)
#define REBOOT_CAM_FLG          (0x9b)
#define SET_SPI_PORT_SELECT(x)  (0xa0 | (x & 0x0f))

/* test for no registry I2C slave*/
#define DS28C36_I2C_SLAVE_ADDR   (0x36) 

#define WRITE_MEM_CMD            (0x96)
#define WRITE_READBACK_BYTE1      (0x1)
#define WRITE_READBACK_BYTE2     (0xAA)
#define WRITE_TX_BYTE_COUNT      (0x21) // 33 bytes

#define READ_MEM_CMD             (0x69)
#define READ_READBACK_BYTE1      (0x33)
#define READ_READBACK_BYTE2      (0xAA)
#define READ_RX_BYTE_COUNT       (0x22) //34 bytes
////////////////////////////////////////////////////////////////////////
/** --- For AP020x firmware update --- */
/* Once a command has been written through the HCI, it will be 
 * executed by on-chip firmware. The results are reported back.
 * All commands are encoded with bit 15 set, which automatically
 * generates the 'host command' (doorbell) interrupt to ISP MCU
 */
typedef enum
{
    IMG_SENSOR_UPDATE_DPT_RST_FLG       = 0,
    IMG_SENSOR_UPDATE_DPT_RD_FLG        = 1,
    IMG_SENSOR_UPDATE_DPT_CMD_WR_FLG    = 2,
    IMG_SENSOR_UPDATE_DPT_DATA_WR_FLG   = 3,
} ap0202_dpt_flag;


//AP020X HOST COMMAND INTERFACE(HCI) REG
#define AP020X_HOST_CMD_PARAM_POOL  (0xFC00)
#define AP020X_SYSCTL_REG           (0x0040)
#define PACKET_SIZE                 (16)

/* AP020X flash manger host command */
typedef enum
{
    // TODO: this is from AP010X HCI command, check for AP020X compatibility
    CMD_GET_LOCK                = 0x8500, // Request flash manager access lock
    CMD_LOCK_STATUS             = 0x8501, // Retrive status of access lock request
    CMD_RELEASE_LOCK            = 0x8502, // Release flash manager access lock
    CMD_FLASHMGR_CONFIG         = 0x8503, // configure flash manager and underlying SPI NVM subsystem
    CMD_FLASHMGR_READ           = 0x8504, // Read data from the SPI NVM
    CMD_WRITE                   = 0x8505, // Write data to the SPI NVM
    CMD_ERASE_BLOCK             = 0x8506, // Erase a block of data from the SPI NVM
    CMD_QUERY_DEV               = 0x8508, // Query device−specific information
    CMD_FLASH_STATUS            = 0x8509, // Obtain status of current asynchronous operation
    CMD_CONFIG_DEV              = 0x850a, // Configure the attached SPI NVM device

    // CMD_CCIMGR_GET_LOCK     = 0x8D00,
    // CMD_CCIMGR_LOCK_STATUS  = 0x8D01,
    // CMD_CCIMGR_RELEASE_LOCK = 0x8D02,
    // CMD_CCIMGR_CONFIG       = 0x8D03,
    // CMD_CCIMGR_SET_DEVICE   = 0x8D04,
    // CMD_CCIMGR_READ         = 0x8D05,
    // CMD_CCIMGR_WRITE        =  0x8D06,
    // CMD_CCIMGR_WRITE_BITFIELD = 0x8D07,
    // CMD_CCIMGR_STATUS       = 0x8D08,

} AP0200_CMD;

/////////////////////////////////////////////////////////////////////
// for USB3.0 packet size
#define OV580_PACKET_SIZE     (384)
#define CMD_START_POS         (2)
#define OV580_XU_UNIT         (4)
#define OV580_XU_SELECTOR     (2)
     
#define SCCB_ADDR_9782        (0x20)
#define SCCB_ADDR_928x        (0xC0)
#define SCCB_ADDR_7251        (0xC0)
#define SCCB_ADDR_OG01A1B     (0xC0)

/* OV580 host command API */
typedef enum
{
    OV580_SYSTEM_REG            = 0xA2,
    OV580_SENSOR_SCCB0_REG      = 0xA3,
    OV580_SENSOR_SCCB1_REG      = 0xA5,
    OV580_SLAVE_SCCB2_REG       = 0xAB,
    OV580_SPI_FLASH             = 0xA1,
    OV580_GPIO_DIRECTION        = 0x10,
    OV580_GPIO_INTERRPUT_ENA    = 0X11,
    OV580_SET_GPIO_VAL          = 0x12,
    OV580_GET_GPIO_VAL          = 0x13,
    OV580_IC_REV_RO             = 0x01,
    OV580_FW_REV_RO             = 0x02
}OV580_HOST_CMD;

typedef enum
{
    OV580_WRITE_CMD_ID          = 0x50,
    OV580_READ_CMD_ID           = 0x51
}OV580_RW_CMD_ID;


////////////////////////////////////////////////////////////////////
/** --- test registers on the fly --- */
#ifdef AP0202_WRITE_REG_ON_THE_FLY
// TODO: used in helper function, modify it for different camera
const reg_seq ChangConfig[] =
{
    {2, 0x098e, 0x7c00},
    {2, 0xfc00, 0x2800},
    {2, 0x0040, 0x8100},
    {2, 0x0058, 0x4444}
};
const reg_seq TrigEnable[] =
{
    {2, 0x098e, 0xc890},
    {1, 0xc890, 0x03},
    {1, 0xc891, 0x03},
    {1, 0xc892, 0x00}
};

const reg_seq TrigDisable[] =
{
    {2, 0x098e, 0xc890},
    {1, 0xc890, 0x00}
};
void ap0202_write_reg_on_the_fly(int fd);
#endif

#ifdef AP0202_WRITE_REG_IN_FLASH
/**  used for SPI flash
 *  notice it only support 16-bit data with SPI flash
 *   TODO: put the register you want to set in here
 */
const reg_pair ChangConfigFromFlash[] =
{
    {0x305e, 0x0222}, //customer_rev
};
void ap0202_write_reg_in_flash(int fd);
#endif

#ifdef OS05A20_PTS_QUERY
void os05a20_pts_query(int fd);
#endif

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
void ar0231_mipi_testing(int fd);
#endif

#ifdef IMX334_MONO_MIPI_TESTING
const reg_seq IMX334_MIPI_REG_TESTING[] =
{
    {1, 0x30E8, 0x14}, // PROGRAMMABLE_GAIN_CONTROL
    {1, 0x3302, 0x32}, // BLKLEVEL[7:0]
    {1, 0x3303, 0x00}, // BLKLEVEL[1:0] range 0x0-0x3ff
};
void imx334_mipi_testing(int fd);
#endif

/*****************************************************************************
**							 Function declarations
*****************************************************************************/

/** --------------------------- helper function ----------------------------*/
void error_handle_extension_unit();

void write_to_UVC_extension(
    int fd, 
    int property_id,
    int length, 
    unsigned char *buffer);

void read_from_UVC_extension(
    int fd, 
    int property_id,
    int length, 
    unsigned char *buffer);

/**------------------------------------------------------------------------ */

/** ------- functions using extension unit call in camera driver ---------- 
 * these functions down below are the API you want to look at(paste) if you 
 * try to implement some functionalities on your current software 
 */
//////////////////////////////////////////////////////////////////////////////
//		LEOPARD FX3 USB FIRMWARE EXTENSION UNIT API				    		//
// 		Used in most control widgets in the first tab in GUI	   			//		
//////////////////////////////////////////////////////////////////////////////
void set_sensor_mode(int fd, int mode);

void set_pos(int fd, int start_x, int start_y);

void get_led_status(int fd);
void set_led(
    int fd, 
    int left_0, 
    int left_1, 
    int right_0, 
    int right_1);

void set_sensor_gain_rgb(
    int fd, 
    unsigned int rGain,
    unsigned int grGain,
    unsigned int gbGain,
    unsigned int bGain);

// can used for other no registry I2C slave but I only tested on SD28C36
void DS28C36_I2C_write(
    int fd, 
    int slaveAddr, 
    int length,
    int cmd,
    unsigned char *i2c_data);

void DS28C36_I2C_read(
    int fd, 
    int slaveAddr,
    int length);

int get_hw_rev();
int get_li_datatype();
int get_fw_rev();
char *get_uuid();
void read_cam_uuid_hwfw_rev(int fd);
void sensor_set_serial_number(int fd, char *sn);

void get_pts(int fd);
void set_pts(int fd, unsigned long initVal);

void soft_trigger(int fd);
void trigger_delay_time(
    int fd, 
    unsigned int delay_time);
void trigger_enable(
    int fd, 
    int ena, 
    int enb);

void load_register_setting_from_configuration(
    int fd, 
    int regCount,
    const struct reg_pair *buffer);
void load_register_setting_from_flash_manually(int fd);

void sensor_reg_write(
    int fd, 
    int regAddr, 
    int regVal);
int sensor_reg_read(
    int fd, 
    int regAddr);

void firmware_erase(int fd);
void reboot_camera(int fd);
void set_spi_port_select(
    int fd, 
    int mode);

void generic_I2C_write(
    int fd, 
    int rw_flag, 
    int bufCnt,
    int slaveAddr, 
    int regAddr, 
    unsigned char *i2c_data);
int generic_I2C_read(
    int fd, 
    int rw_flag, 
    int bufCnt,
    int slaveAddr, 
    int regAddr);

void write_cam_defect_pixel_table(
    int fd, 
    unsigned char *w_buf);
void read_cam_defect_pixel_table(
    int fd, 
    unsigned char *r_buf);
//void eeprom_fill_page_buffer(int fd, unsigned char *buf, int len);
//////////////////////////////////////////////////////////////////////////////
//			AR0231 AP0200 HOST COMMAND API									//		
// 			FOR FLASHING AR0231 AP0200 BINARY ON THE FLY					//      	
//////////////////////////////////////////////////////////////////////////////
void ap020x_send_command(
    int fd, 
    AP0200_CMD cmd, 
    int time_out);
void ap020x_soft_reset(int fd);
void ap020x_read_data(
    int fd, 
    int reg_addr, 
    int count);
void ap020x_write_data(
    int fd, 
    int buf_size, 
    int pos, 
    unsigned char *buf);
void ap020x_program_flash_eeprom(
    int fd, 
    const unsigned char *bin_buf, 
    int bin_size);
//////////////////////////////////////////////////////////////////////////////
//			OV580 HOST COMMAND API						        			//		
// 			FOR CTRLS OF OV580                          					//      	
//////////////////////////////////////////////////////////////////////////////
void ov580_write_to_UVC_extension(
	int fd, 
	unsigned char *buffer);
void ov580_read_from_UVC_extension(
	int fd, 
	unsigned char *buffer);

void ov580_write_system_reg(
	int fd, 
	int reg_addr, 
	unsigned char reg_val);
unsigned char ov580_read_system_reg(
	int fd,
	int reg_addr);
void ov580_write_sccb0_reg(
	int fd, 
    unsigned char slave_addr,
	int reg_addr, 
	unsigned char reg_val);
unsigned char ov580_read_sccb0_reg(
	int fd,
	unsigned char slave_addr,
	int reg_addr);
void ov580_write_sccb1_reg(
	int fd, 
    unsigned char slave_addr,
	int reg_addr, 
	unsigned char reg_val);
unsigned char ov580_read_sccb1_reg(
	int fd,
    unsigned char slave_addr,
	int reg_addr);



