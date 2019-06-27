/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, this file is mainly for 
  string manipulation.

  Author: Danyu L
  Last edit: 2019/06
*****************************************************************************/
#pragma once
#include<ctype.h>
/****************************************************************************
**                      	Global data 
*****************************************************************************/
typedef enum 
{
  CONFIG_FILE_TXT = 0,
  CONFIG_FILE_JSON,
  CONFIG_FILE_BIN, 
}config_file_type;
/****************************************************************************
**							 Function declaration
*****************************************************************************/
void top_n_tail(char *str);
void trim_trailing_whitespaces(char *src);
char *get_file_basename(char *filename);
char *get_file_extension(char *filename);

config_file_type config_file_identifier (char *filename);
void load_control_profile(int fd, char *filename);
