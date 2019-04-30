/****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly for reading from 
  config.json at camera startup to perform group register writes and captures.

  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/json_parser.h"
/****************************************************************************
**                      	External Callbacks
*****************************************************************************/
extern void generic_I2C_write(int fd, int rw_flag, int bufCnt,
            int slaveAddr, int regAddr, unsigned char *i2c_data);
extern void sensor_reg_write(int fd, int regAddr, int regVal);
/*****************************************************************************
**                           Function definition
*****************************************************************************/
/* 
 * Remove the first and last character from c string 
 * for removing the paranthesis, double quote etc
 */
void top_n_tail(char *str)
{
     size_t len = strlen(str);
     memmove(str, str+1, len-2);
     str[len-2] = 0;
}

void json_parser(int fd)
{
	FILE *fp;
	char json_buffer[BUFFER_MAX];
	struct json_object *parsed_json;
	struct json_object *device_name;
	struct json_object *sub_address;
    struct json_object *reg_address_width;
	struct json_object *reg_pairs;    
    struct json_object *capture_num;
    struct json_object *capture_name;

    char *reg_pair;
    char* reg_pair_string;
    
    char* dev_name;
    uint8_t sub_addr;
    int addr_width;
    uint32_t reg_addr;
    uint32_t reg_val;

    int i;
    int errnum;
    char path[100] = "config.json";

	fp = fopen(path,"r");
    if(fp == NULL) {
        errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("Error printed by perror");
        fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
    }
    else {
	    fread(json_buffer, BUFFER_MAX, 1, fp);
	    fclose(fp);
    }

    /* parse the json file */
	parsed_json = json_tokener_parse(json_buffer);

    /* Get the json_object associated with a given object field */
	json_object_object_get_ex(parsed_json, "deviceName", &device_name);
	json_object_object_get_ex(parsed_json, "subAddress", &sub_address);
    json_object_object_get_ex(parsed_json, "regAddressWidth", &reg_address_width);
    json_object_object_get_ex(parsed_json, "captureNumber", &capture_num);
    json_object_object_get_ex(parsed_json, "capturedName", &capture_name);
	json_object_object_get_ex(parsed_json, "regPair", &reg_pairs);

    dev_name = (char *)json_object_get_string(device_name);
	printf("Name: %s\n", dev_name);
    sub_addr = strtol((char *)json_object_get_string(sub_address), NULL, 16);
    printf("8-bit I2C Address: 0x%x\n", sub_addr);
    addr_width = json_object_get_int(reg_address_width);
	printf("Register Address Width: %d\n", addr_width);
    printf("Capture Image Number: %d\n", json_object_get_int(capture_num));
    printf("Capture Image Name: %s\n", json_object_get_string(capture_name));


    //couldn't find a better way to parser nested json, so do it yourself
    reg_pair_string = (char *)json_object_to_json_string(reg_pairs);
    top_n_tail(reg_pair_string); // inplace remove the paranthesis
    reg_pair = strtok (reg_pair_string,",: ");
    i = 0;

    while (reg_pair!= NULL)
    {   
        /* register address */
        if(i % 2 == 0) 
        {   
            top_n_tail(reg_pair); // remove double quote
            reg_addr = (uint32_t)strtol(reg_pair, NULL, 16);
            printf("reg addr = 0x%x\n", reg_addr);
        }
        /* register value */
        else 
        {
            top_n_tail(reg_pair); // remove double quote
            reg_val = (uint32_t)strtol(reg_pair, NULL, 16);
            printf("reg val = 0x%x\n", reg_val);
        }

        reg_pair = strtok (NULL, ",: ");
        i++;
    }
    //TODO: finish this later
    //generic_I2C_write(fd, 0x82, 2, slaveAddr, regAddr, buf);

}
