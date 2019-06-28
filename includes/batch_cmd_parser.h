/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly for BatchCmd.txt
  parser. Using some STL functions for string manipulations.

  Author: Danyu L
  Last edit: 2019/06
*****************************************************************************/
#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
/****************************************************************************
**                      	Global data 
*****************************************************************************/
typedef enum
{
  eSubAddress,
  eRegAddress,
  eRegAddrWidth,
  eDelay,
  eInterFrameDelay,
  eRead,
  eWrite,
  eCapture,
  eFlashVal,
  eNotExist
}option_string_code;
/****************************************************************************
**							 Function declaration
*****************************************************************************/
template<typename Out>
void split(const std::string &s, char delim, Out result);

std::vector<std::string> split(const std::string &s, char delim);

option_string_code hashit(std::string const &inString);
int hex_or_dec_interpreter(std::string const &inString);

void txt_file_parser(int fd, char *buf, long length);

