#pragma once
// #define AP0202_WRITE_REG_ON_THE_FLY
// #define AP0202_WRITE_REG_IN_FLASH
// #define OS05A20_PTS_QUERY
// #define AR0231_MIPI_TESTING
// #define IMX334_MONO_MIPI_TESTING

/** 
 * ---------------- 8-bit I2C slave address list -----------------
 * Put a list here so you don't ask me what is the slave address.
 * Note that for different hardware and revsion, we might have 
 * different slave address for same sensors, so try with the address
 * in the comment if possible.
 * If you still have problem to access the sensor registers, you can 
 * contact the support for a firmware update since most likely that 
 * driver wasn't enabling generic i2c slave register access.
 * Disclaimer: this list doesn't include all our USB3 slave address,
 * just all the cameras I got in touch before.
 */

/**  
 * On-semi Sensor 
 * 16-bit addr, 16-bit value
 * */
#define AP020X_I2C_ADDR         (0xBA)
#define AR0231_I2C_ADDR         (0x20)// 0x30
#define AR0144_I2C_ADDR         (0x20)// 0x30
#define AR0234_I2C_ADDR         (0x20)

/**   
 * Sony Sensor   
 * 16-bit addr, 8-bit value
 */
#define IMX334_I2C_ADDR         (0x34)
#define IMX390_I2C_ADDR         (0x34)
#define IMX324_I2C_ADDR         (0x34)
#define IMX477_I2C_ADDR         (0x20)
/** 
 * Omnivision Sensor
 * 16-bit addr, 8-bit value
 */
#define OV2311_I2C_ADDR         (0xC0)
#define OS05A20_I2C_ADDR        (0x6C)
#define OV5640_I2C_ADDR         (0x78)
#define OV7251_I2C_ADDR         (0xE7)
/** 
 * Toshiba Bridge 
 * 16-bit addr, 16-bit value
 */
#define TC_MIPI_BRIDGE_I2C_ADDR (0x1C)

/**  
 * Maxim Serdes 
 * GMSL1: 8-bit addr, 8-bit value
 * GMSL2: 16-bit addr, 8-bit value
*/
#define MAX96705_SER_I2C_ADDR   (0x80)
#define MAX9295_SER_I2C_ADDR    (0x80) //0x88, 0xC4
#define MAX9272_SER_I2C_ADDR    (0x90)
#define MAX9296_DESER_I2C_ADDR  (0x90) //0xD4

/** 
 * TI Serdes
 * 16-bit addr, 8-bit value 
 * */
#define TI913_SER_I2C_ADDR      (0xB4)//0xB2
#define TI953_SER_I2C_ADDR      (0x60) 
#define TI914_DESER_I2C_ADDR    (0xC0)
#define TI954_DESER_I2C_ADDR    (0x30)