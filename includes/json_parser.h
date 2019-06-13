/****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly for reading from 
  config.json at camera startup to perform group register writes and captures.

  Author: Danyu L
  Last edit: 2019/04
*****************************************************************************/
#pragma once
#include<json-c/json.h>
#include<ctype.h>

#define BUFFER_MAX 1 << 16

void json_parser(int fd);

