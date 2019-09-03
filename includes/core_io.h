/*****************************************************************************
*  This program is free software; you can redistribute it and/or modify      *   
*  it under the terms of the GNU General Public License as published by      *
*  the Free Software Foundation; either version 2 of the License, or         *
*  (at your option) any later version.                                       *
*                                                                            *
*  This program is distributed in the hope that it will be useful,           *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*  GNU General Public License for more details.                              *
*                                                                            *
*  You should have received a copy of the GNU General Public License along   *
*  with this program; if not, write to the Free Software Foundation, Inc.,   * 
*                                                                            *
*  This is the sample code for Leopard USB3.0 camera, this file is mainly    *
*  for string and file manipulation.                                         *
*                                                                            *
*  Author: Danyu L                                                           *
*  Last edit: 2019/06                                                        *
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
  CONFIG_FILE_WRONG
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
