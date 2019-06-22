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

template<typename Out>
void split(const std::string &s, char delim, Out result);

std::vector<std::string> split(const std::string &s, char delim);

void txt_file_parser(int fd, char *buf, int length);

