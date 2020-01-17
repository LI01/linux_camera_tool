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
#pragma once
#include<json-c/json.h>
#include<ctype.h>


void json_parser(int fd, char *json_buffer);

