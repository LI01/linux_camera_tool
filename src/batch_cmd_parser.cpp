/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly for BatchCmd.txt
  parser. Using some STL functions for string manipulations.

  Author: Danyu L
  Last edit: 2019/06
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/batch_cmd_parser.h"

/*****************************************************************************
**                      	External Callbacks
*****************************************************************************/
extern void generic_I2C_write(int fd, int rw_flag, int bufCnt,
                              int slaveAddr, int regAddr, unsigned char *i2c_data);
extern int generic_I2C_read(int fd,int rw_flag, int bufCnt,
					  int slaveAddr, int regAddr);  
extern void sensor_reg_write(int fd, int regAddr, int regVal);
extern int sensor_reg_read(int fd,int regAddr);
extern void video_capture_save_raw();
extern void video_capture_save_bmp();

/******************************************************************************
**                           Function definition
*****************************************************************************/
/** 
 *   split the string by a delimiter
 *   this function puts the results in a pre-constructed vector
 */
template<typename Out>
void split(const std::string &s, char delim, Out result) 
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

/** 
 *   split the string by a delimiter
 *   this function return a new vector
 */
std::vector<std::string> split(const std::string &s, char delim) 
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

/// well, let's use cpp stl library, i don't want to use c anymore for this...
void txt_file_parser(int fd, char *buf, int length)
{
    int sub_addr = 0, reg_addr = 0, reg_addr_width = 16;
    int read_count = 2, write_count = 2, delay_time = 0;
    int inter_frame_delay_time = 0; //TODO:
    std::string tmpstr(buf, length);
    std::istringstream is(tmpstr);
    std::string line; 
    
    while (std::getline(is, line)) {
        if (line[0] != '#') /// ignore the comment lines
        {
            std::vector<std::string> element  = split(line, ' ');
            /// couldn't use switch for this, so bear with it...
            if (element[0].compare("SubAddress") == 0) 
            {
                if (element[1].find("0x") != std::string::npos)             
                    sub_addr = std::stoi(element[1], 0, 16);
                else 
                    sub_addr = std::stoi(element[1], 0, 10);
            }
            else if (element[0].compare("RegAddress") == 0) 
            {
                if (element[1].find("0x") != std::string::npos)  
                    reg_addr = std::stoi(element[1], 0, 16);
                else
                    reg_addr = std::stoi(element[1], 0, 10);
            }
            else if (element[0].compare("RegAddrWidth") == 0) 
            {
                if (element[1].find("0x") != std::string::npos)  
                    reg_addr_width = std::stoi(element[1], 0, 16);
                else
                    reg_addr_width = std::stoi(element[1], 0, 10);
            }
            else if (element[0].compare("Delay") == 0) 
            {
                if (element[1].find("0x") != std::string::npos)  
                    delay_time = std::stoi(element[1], 0, 16);
                else
                    delay_time = std::stoi(element[1], 0, 10);
        
                printf ("delay time = %d ms\r\n", delay_time);
                while (delay_time > 0)
                {
                    usleep(delay_time);
                }
            }
            else if (element[0].compare("InterFrameDelay") == 0) 
            {
                if (element[1].find("0x") != std::string::npos)  
                    inter_frame_delay_time = std::stoi(element[1], 0, 16);
                else
                    inter_frame_delay_time = std::stoi(element[1], 0, 10);
        
                printf ("set inter frame delay time to = %d ms\r\n", delay_time);
                while (delay_time > 0)
                {
                    usleep(delay_time);
                }
            }

            else if (element[0].compare("Read") == 0) 
            {
                if (element[1].find("0x") != std::string::npos)  
                    read_count = std::stoi(element[1], 0, 16);
                else
                    read_count = std::stoi(element[1], 0, 10);
                
                if (read_count <= 0 || read_count > 256)
                {    
                    printf ("ERROR! I2C read count must be 1-256, it is %d now\r\n", read_count);
                    return;
                }
               
                generic_I2C_read(fd, (GENERIC_REG_READ_FLG | (reg_addr_width/8)), 
                read_count, sub_addr, reg_addr);
            }
            else if (element[0].compare("Write") == 0) 
            {
                int reg_val;
                if (element[1].find("0x") != std::string::npos)  
                    reg_val = std::stoi(element[1], 0, 16);
                else
                    reg_val = std::stoi(element[1], 0, 10);
                        
                unsigned char buf[2];
                buf[0] = reg_val & 0xff;
                buf[1] = (reg_val >> 8) & 0xff;
                generic_I2C_write(fd, (GENERIC_REG_WRITE_FLG 
                | (reg_addr_width/8)), write_count, sub_addr, reg_addr, buf);

            }
            else if (element[0].compare("Capture") == 0) 
            {
                int capture_number;
                if (element[1].find("0x") != std::string::npos)  
                    capture_number = std::stoi(element[1], 0, 16);
                else
                    capture_number = std::stoi(element[1], 0, 10);
                printf("capture %d frames\r\n", capture_number);
                for (int i=1; i <= capture_number; i++) 
                {
                    video_capture_save_raw();
                    video_capture_save_bmp();
                }   

            }

            else if (element[0].compare("FlashVal") == 0) 
            {
                //TODO: add that later
            }
            
        }
    }
}