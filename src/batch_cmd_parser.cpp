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
 *  This is the sample code for Leopard USB3.0 camera, mainly for             
 *  BatchCmd.txt parser. Using some STL functions for string manipulations.   
 *                                                                            
 *  Author: Danyu L                                                           
 *  Last edit: 2019/06                                                        
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/batch_cmd_parser.h"

/*****************************************************************************
**                      	External Callbacks
*****************************************************************************/
extern void generic_I2C_write(int fd, int rw_flag, int bufCnt,
                              int slaveAddr, int regAddr, unsigned char *i2c_data);
extern int generic_I2C_read(int fd, int rw_flag, int bufCnt,
                            int slaveAddr, int regAddr);
extern void sensor_reg_write(int fd, int regAddr, int regVal);
extern int sensor_reg_read(int fd, int regAddr);
extern void video_capture_save_raw();
extern void video_capture_save_bmp();

/******************************************************************************
**                           Function definition
*****************************************************************************/
/**
 * return the output string for a linux shell command 
 */
std::string
get_stdout_from_cmd(std::string cmd)
{

	std::string data;
	FILE *stream;
	const int max_buffer= 256;
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

/** 
 *   split the string by a delimiter
 *   this function puts the results in a pre-constructed vector
 */
template <typename Out>
void split(const std::string &s, char delim, Out result)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        *(result++) = item;
    }
}

/** 
 *   split the string by a delimiter
 *   this function return a new vector
 */
std::vector<std::string> 
split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

/* 
 * since c/c++ switch can only switch int/enum, which is really inconvenient. 
 * Use this helper function to switch over the result of a hash function that
 * uses the string as input, so that the code looks much cleaner 
 * */
option_string_code 
hashit(std::string const &inString)
{
    if (inString == "SubAddress")
        return eSubAddress;
    else if (inString == "RegAddress")
        return eRegAddress;
    else if (inString == "RegAddrWidth")
        return eRegAddrWidth;
    else if (inString == "Delay")
        return eDelay;
    else if (inString == "InterFrameDelay")
        return eInterFrameDelay;
    else if (inString == "Read")
        return eRead;
    else if (inString == "Write")
        return eWrite;
    else if (inString == "Capture")
        return eCapture;
    else if (inString == "FlashVal")
        return eFlashVal;
    return eNotExist;
}

/* 
 * add this helper function for interpreting hex and decimal number 
 * for all the batchCmd values
 */
int hex_or_dec_interpreter(std::string const &inString)
{
    int out_val;

    if (inString.find("0x") != std::string::npos)
        out_val = std::stoi(inString, 0, 16);
    else
        out_val = std::stoi(inString, 0, 10);
    return out_val;
    
}
/// well, let's use cpp stl library, i don't want to use c anymore for this...
void txt_file_parser(int fd, char *buf, long length)
{
    int sub_addr = 0, reg_addr = 0, reg_addr_width = 16;
    int read_count = 2, write_count = 2, delay_time = 0;
    int inter_frame_delay_time = 0; //TODO:
    std::string tmpstr(buf, length);
    std::istringstream is(tmpstr);
    std::string line;

    while (std::getline(is, line))
    {
        if (line[0] != '#') /// ignore the comment lines
        {
            /// split line words by space
            /// first word: key, second word: value, third word and so on: ignored
            std::vector<std::string> element = split(line, ' ');
            switch (hashit(element[0]))
            {
            case eSubAddress:
                sub_addr = hex_or_dec_interpreter(element[1]);
                break;

            case eRegAddress:
                reg_addr = hex_or_dec_interpreter(element[1]);
                break;

            case eRegAddrWidth:
                reg_addr_width = hex_or_dec_interpreter(element[1]);
                break;

            case eDelay:
                delay_time = hex_or_dec_interpreter(element[1]);
                printf("delay time = %d ms\r\n", delay_time);
                while (delay_time > 0)
                {
                    usleep(delay_time);
                }
                break;

            case eInterFrameDelay:
                inter_frame_delay_time = hex_or_dec_interpreter(element[1]);
                printf("set inter frame delay time to = %d ms\r\n", inter_frame_delay_time);
                while (inter_frame_delay_time > 0)
                {
                    usleep(inter_frame_delay_time);
                }
                break;
            case eRead:
                read_count = hex_or_dec_interpreter(element[1]);
                if (read_count <= 0 || read_count > 256)
                {
                    printf("ERROR! I2C read count must be 1-256, it is %d now\r\n", read_count);
                    return;
                }

                generic_I2C_read(
                    fd, 
                    (GENERIC_REG_READ_FLG | (reg_addr_width / 8)),
                    read_count, 
                    sub_addr, 
                    reg_addr);
                break;

            case eWrite:
                int reg_val;
                reg_val = hex_or_dec_interpreter(element[1]);
                unsigned char buf[2];
                buf[0] = reg_val & 0xff;
                buf[1] = (reg_val >> 8) & 0xff;
                generic_I2C_write(
                    fd, 
                    (GENERIC_REG_WRITE_FLG | (reg_addr_width / 8)), 
                    write_count, 
                    sub_addr, 
                    reg_addr, 
                    buf);
                break;

            case eCapture:
                int capture_number;
                capture_number = hex_or_dec_interpreter(element[1]);
                printf("capture %d frames\r\n", capture_number);
                for (int i = 1; i <= capture_number; i++)
                {
                    video_capture_save_raw();
                    video_capture_save_bmp();
                }
                break;

            case eFlashVal:
                //TODO: add that later
                break;
            case eNotExist:
                std::cout <<"ERROR! Script key value =[" << element[0]<< "] doesn\'t exist\r\n";
                break;
            }
          
        }
    }
}