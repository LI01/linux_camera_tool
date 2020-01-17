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
 * This is the sample code for Leopard USB3.0 camera, mainly for loading     
 * from config.json to perform group register writes and captures.           
 *                                                                            
 * Author: Danyu L                                                           
 * Last edit: 2019/06                                                        
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/json_parser.h"
#include "../includes/core_io.h"

/*****************************************************************************
**                      	External Callbacks
*****************************************************************************/
extern char *get_product();
extern void generic_I2C_write(int fd, int rw_flag, int bufCnt,
                              int slaveAddr, int regAddr, unsigned char *i2c_data);
extern void sensor_reg_write(int fd, int regAddr, int regVal);
extern void video_capture_save_raw();
extern void video_capture_save_bmp();

/******************************************************************************
**                           Function definition
*****************************************************************************/

void json_parser(int fd, char *json_buffer)
{

    struct json_object *parsed_json;
    struct json_object *device_name;
    struct json_object *sub_address;
    struct json_object *reg_address_width;
    struct json_object *reg_value_width;
    struct json_object *reg_pairs;
    struct json_object *capture_raw_num;
    struct json_object *capture_bmp_num;

    char *reg_pair;
    char *reg_pair_string;

    char *dev_name;
    uint8_t sub_addr;
    uint8_t addr_width, val_width;
    uint32_t reg_addr = 0;
    uint32_t reg_val = 0;
    int execute_flg = 0;
    int i;

    /** parse the json file */
    parsed_json = json_tokener_parse(json_buffer);

    /** Get the json_object associated with a given object field */
    json_object_object_get_ex(parsed_json, "deviceName", &device_name);
    json_object_object_get_ex(parsed_json, "subAddress", &sub_address);
    json_object_object_get_ex(parsed_json, "regAddressWidth", &reg_address_width);
    json_object_object_get_ex(parsed_json, "regValueWidth", &reg_value_width);
    json_object_object_get_ex(parsed_json, "captureRAWNumber", &capture_raw_num);
    json_object_object_get_ex(parsed_json, "captureBMPNumber", &capture_bmp_num);

    json_object_object_get_ex(parsed_json, "regPair", &reg_pairs);

    dev_name = (char *)json_object_get_string(device_name);

    if (strcmp(get_product(), dev_name) == 0)
    {
        printf("Device is %s\r\n", dev_name);
        execute_flg = 1;
    }
    if (execute_flg)
    {
        sub_addr = strtol((char *)json_object_get_string(sub_address), NULL, 16);
        addr_width = json_object_get_int(reg_address_width);
        val_width = json_object_get_int(reg_value_width);

        //couldn't find a better way to parser nested json, so do it yourself
        reg_pair_string = (char *)json_object_to_json_string(reg_pairs);
        top_n_tail(reg_pair_string); // inplace remove the paranthesis
        reg_pair = strtok(reg_pair_string, ",: ");
        i = 0;

        while (reg_pair != NULL)
        {
            /** register address */
            if (i % 2 == 0)
            {
                top_n_tail(reg_pair); // remove double quote
                reg_addr = (uint32_t)strtol(reg_pair, NULL, 16);
            }
            /** register value */
            else
            {
                top_n_tail(reg_pair); // remove double quote
                reg_val = (uint32_t)strtol(reg_pair, NULL, 16);

                unsigned char buf[2];
                buf[0] = reg_val & 0xff;
                buf[1] = (reg_val >> 8) & 0xff;
                generic_I2C_write(fd, GENERIC_REG_WRITE_FLG | (addr_width / 8),
                                  (val_width / 8), sub_addr, reg_addr, buf);
            }

            reg_pair = strtok(NULL, ",: ");
            i++;
        }
        printf("Capture RAW Image Number: %d\n",
               json_object_get_int(capture_raw_num));
        // FIXME:block the capture threads using mutex, 
        // only capture 1/2 images for now
        for (int i = 1; i <= json_object_get_int(capture_raw_num); i++)
        {
            printf("capture #%d raw image\r\n", i);
            video_capture_save_raw();
        }
        printf("Capture BMP Image Number: %d\n",
               json_object_get_int(capture_bmp_num));
        for (int i = 1; i <= json_object_get_int(capture_bmp_num); i++)
        {
            printf("capture #%d bmp image\r\n", i);
            video_capture_save_bmp();
        }
        execute_flg = 0;
    }
    else
    {
        printf("Please update device name in config.json\r\n");
    }
}
