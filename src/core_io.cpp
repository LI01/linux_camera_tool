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
 * This is the sample code for Leopard USB3.0 camera, this file is mainly    
 * for string and file manipulation.                                         
 *                                                                            
 * Author: Danyu L                                                           
 * Last edit: 2019/06                                                        
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/core_io.h"
#include "../includes/uvc_extension_unit_ctrl.h"
#include "../includes/json_parser.h"
#include "../includes/batch_cmd_parser.h"

//extern void ap020x_program_flash_eeprom(int fd, const unsigned char *bin_buf, int bin_size);

/** 
 * Remove the first and last character from c string 
 * for removing the paranthesis, double quote etc
 */
void top_n_tail(char *str)
{
    if (str == NULL || strlen(str) < 1)
        return;
    size_t len = strlen(str);
    memmove(str, str + 1, len - 2);
    str[len - 2] = 0;
}

/**
 * trim trailing white spaces and control chars (\n) from source string
 * args:
 *    src - source string
 *
 * asserts:
 *    none
 *
 * returns: error code
 */
void trim_trailing_whitespaces(char *src)
{
    if (src == NULL || strlen(src) < 1)
        return;

    /**move to end of string*/
    char *srcp = src + strlen(src);

    while ((isspace(*(srcp - 1)) || iscntrl(*(srcp - 1))) && (srcp - 1) > src)
        srcp--;

    /**end string*/
    *srcp = '\0';
}

/**
 * get the filename basename
 * args:
 *    filename - string with filename (full path)
 *
 * asserts:
 *    none
 *
 * returns: new string with basename (must free it)
 */
char *get_file_basename(char *filename)
{
    char *name = strrchr(filename, '/');
    char *basename = NULL;
    if (name != NULL)
        basename = strdup(name + 1); // skip '/'
    else
        basename = strdup(filename);
    //printf("basename for %s is %s\n", filename, basename);
    return basename;
}

/**
 * get the filename extension
 * args:
 *    filename - string with filename (full path)
 *
 * asserts:
 *    none
 *
 * returns: new string with extension (must free it)
 *      or NULL if no extension found
 */
char *get_file_extension(char *filename)
{
    char *basename = get_file_basename(filename);

    char *name = strrchr(basename, '.');

    char *extname = NULL;

    if (name)
        extname = strdup(name + 1);

    //printf("extension for %s is %s\n", filename, extname);

    free(basename);

    return extname;
}

config_file_type 
config_file_identifier(char *filename)
{
    char *file_type = get_file_extension(filename);
    if (strcmp(file_type, "bin") == 0)
        return CONFIG_FILE_BIN;
    else if (strcmp(file_type, "json") == 0)
        return CONFIG_FILE_JSON;
    else if (strcmp(file_type, "txt") == 0)
        return CONFIG_FILE_TXT;
    else 
        return CONFIG_FILE_WRONG;
}


void load_control_profile(int fd, char *filename)
{
    FILE *fp;
    long l_size;
    char *buffer;
    config_file_type cfg_file = CONFIG_FILE_TXT;
    fp = fopen(filename, "rb");

    if (!fp)
    {
        printf("File is NULL\r\n");
        return;
    }
    fseek(fp, 0L, SEEK_END);
    l_size = ftell(fp);
    rewind(fp);

    /// allocate memory for entire content
    buffer = (char *)calloc(1, l_size + 1);
    if (!buffer)
    {
        fclose(fp);
        printf("memory alloc failure\r\n");
        return;
    }
    /// copy file into the buffer
    if (1 != fread(buffer, l_size, 1, fp))
    {
        fclose(fp);
        free(buffer);
        printf("read file failure\r\n");
        return;
    }
    cfg_file = config_file_identifier(filename);
    switch (cfg_file)
    {
    case CONFIG_FILE_TXT:
        printf("*******************Commands Load From BatchCmd.txt***********\n");
        txt_file_parser(fd, buffer, l_size);
        printf("*******************Commands Executed From BatchCmd.txt*******\n");
        break;
    case CONFIG_FILE_JSON:
        printf("*******************Commands Load From JSON*******************\n");
        json_parser(fd, buffer);
        printf("*******************Commands Executed From JSON***************\n");
        break;
    case CONFIG_FILE_BIN:
        printf("*******************Configs Load From BIN file*****************\n");
        //printf("%x\r\n", buffer);
        //ap020x_program_flash_eeprom(fd, (unsigned char *)buffer, (int)l_size);
        printf("*******************Flash Finish From BIN file*****************\n");
        break;
    case CONFIG_FILE_WRONG:
    default: 
        printf("*************Choose another type of file**********************\n");
        break;
   
    }

    fclose(fp);

    free(buffer);
}