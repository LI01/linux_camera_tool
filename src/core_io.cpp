
/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, this file is mainly for 
  string and file manipulation.

  Author: Danyu L
  Last edit: 2019/06
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/core_io.h"

/** 
 * Remove the first and last character from c string 
 * for removing the paranthesis, double quote etc
 */
void top_n_tail(char *str)
{
    if(str == NULL || strlen(str) < 1)
		return;
     size_t len = strlen(str);
     memmove(str, str+1, len-2);
     str[len-2] = 0;
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
	if(src == NULL || strlen(src) < 1)
		return;

	/**move to end of string*/
	char *srcp = src + strlen(src);

	while((isspace(*(srcp-1)) || iscntrl(*(srcp-1))) && (srcp - 1) > src)
		srcp--;

	/**end string*/
	*srcp = '\0';

}

/*
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
    printf("basename for %s is %s\n", filename, basename);
    return basename;
    
}

/*
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

	if(name)
		extname = strdup(name + 1);

	
	printf("extension for %s is %s\n", filename, extname);

	free(basename);

	return extname;
}

void load_control_profile(char *filename)
{
    FILE *fp;
    long l_size;
    char *buffer;

    if (strcmp(get_file_extension(filename), "bin") == 0)
    {    
        printf("in bin!!!!!!!!!!!!!!!!!!\r\n");
        fp = fopen(filename, "rb");
    }
    else
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
    buffer = (char *)calloc(1, l_size+1);
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
    printf("%s\r\n", buffer);
    fclose(fp);
    //TODO: remember to do it somewhere
    free(buffer);


}