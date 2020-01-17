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
  
 * This is the sample code for Leopard USB3.0 camera, shortcut.h stores all the
 * useful macro functions, common libraries, global data structures

 * Author: Danyu L
 * Last edit: 2019/07
*****************************************************************************/
#pragma once

//#include <pthread.h>
#include <sys/types.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h> 
#include <linux/videodev2.h>
#include <linux/usb/video.h>
#include <errno.h>
#include <linux/uvcvideo.h>
#include <sys/fcntl.h>      /** for open() syscall */ 
#include <sys/mman.h>       /** for using mmap */
#include <math.h>           /** pow */
#include "hardware.h"

#include <vector>
#include <sstream>
/*****************************************************************************
**                      	            Bool
*****************************************************************************/
typedef enum { FALSE, TRUE } BOOL;
/*****************************************************************************
**                      	            Bits
*****************************************************************************/
#define BIT(x)              (1<<(x))
#define SETBIT(x,mask)      ((x)|(1<<(mask)))
#define CLEARBIT(x,mask)    ((x)&(~(1<<(mask))))
#define GETBIT(x,mask)      (((x)>>(mask))&1)
#define TOGGLEBIT(x,mask)   ((x)^(1<<(mask)))

#define LOWBYTE(x)          ((unsigned char) (x))
#define HIGHBYTE(x)         ((unsigned char) (((unsigned int) (x)) >> 8))
#define BYTES_PER_BPP(bpp)  ((bpp -1)/8 + 1)
/** Converts an unaligned four-byte little-endian integer into an int32 */
#define DW_TO_INT(p) ((p)[0] | ((p)[1] << 8) | ((p)[2] << 16) | ((p)[3] << 24))
/** Converts an unaligned two-byte little-endian integer into an int16 */
#define SW_TO_SHORT(p) ((p)[0] | ((p)[1] << 8))
/** Converts an int16 into an unaligned two-byte little-endian integer */
#define SHORT_TO_SW(s, p) \
(p)[0] = (s); \
(p)[1] = (s) >> 8;
    /** Converts an int32 into an unaligned four-byte little-endian integer */
#define INT_TO_DW(i, p) \
(p)[0] = (i); \
(p)[1] = (i) >> 8; \
(p)[2] = (i) >> 16; \
(p)[3] = (i) >> 24;
/*****************************************************************************
**                      	           Arrays
*****************************************************************************/
#define CLEAR(a)            memset(&(a), 0, sizeof(a))
/** Get number of elements in an array */
#define SIZE(a)             (sizeof(a) / sizeof(*a)) 
#define IS_ARRAY(a)         ((void *)&a == (void *)a)
/*****************************************************************************
**                      	            Loops
*****************************************************************************/
/** Loop over an array of given size */
#define FOREACH_NELEM(array, nelem, iter)       \
	for (__typeof__(*(array)) *iter = (array);    \
		iter < (array) + (nelem);                   \
		iter++)

/** Loop over an array of known size */
#define FOREACH(array, iter)                    \
	FOREACH_NELEM(array, SIZE(array), iter)
/*****************************************************************************
**                      	           Maths
*****************************************************************************/
#define PI                  3.14159265

#define MIN(x,y)          ({ __typeof__ (x) _x = (x);   \
                             __typeof__ (y) _y = (y);   \
                            _x < _y ? _x : _y; })  
#define MAX(x,y)          ({ __typeof__ (x) _x = (x);   \
                             __typeof__ (y) _y = (y);   \
                            _x > _y ? _x : _y; })

#define ABS(x)              (((x) <  0)  ? -(x) : (x))
#define DIFF(x,y)           ABS((x)-(y))
#define SWAP(x, y)          do { x ^= y; y ^= x; x ^= y; } while ( 0 )
#define IS_BETWEEN(x,L,H)   ((unsigned char)((x) >= (L) && (n) <= (H)))
/** clip value between 0 and 255 */
#define CLIP(value) (uint8_t)(((value)>0xFF)?0xff:(((value)<0)?0:(value)))
/*****************************************************************************
**                      	           Pixels
*****************************************************************************/
/** x is column number, y is row number */ 
#define PIX(x, y, width)	        ((x) + (y) * (width))
#define LEFT(x, y, width)	        ((x) - 1 + (y) * (width))
#define RIGHT(x, y, width)	      ((x) + 1 + (y) * (width))
#define TOP(x, y, width)	        ((x) + ((y) - 1) * (width))
#define BOTTOM(x, y, width)	      ((x) + ((y) + 1) * (width))
#define TOP_LEFT(x, y, width)	    ((x) - 1 + ((y) - 1) * (width))
#define BOTTOM_LEFT(x, y, width)	((x) - 1 + ((y) + 1) * (width))
#define TOP_RIGHT(x, y, width)	  ((x) + 1 + ((y) - 1) * (width))
#define BOTTOM_RIGHT(x, y, width)	((x) + 1 + ((y) + 1) * (width))

#define AVE_DIAGONAL_CFA(in, x, y, width) \
 (uint16_t) ((in[PIX(x, y, width)] + in[BOTTOM_RIGHT(x, y, width)]) / 2)
#define AVE_DIAGONAL_PAN(in, x, y, width) \
 (uint16_t) ((in[RIGHT(x, y, width)] + in[BOTTOM(x, y, width)]) / 2)

/** interpolate vertically */
#define INTERPOLATE_V(in, x, y, width) \
	(uint16_t)((in[TOP(x, y, width)] + in[BOTTOM(x, y, width)]) / 2)

/** interpolate horizontally */
#define INTERPOLATE_H(in, x, y, width) \
	(uint16_t)((in[RIGHT(x, y, width)] + in[LEFT(x, y, width)]) / 2)

/** bilinear interpolation, horizontally and vertically */
#define INTERPOLATE_HV(in, x, y, width) \
	(uint16_t)((in[LEFT(x, y, width)] + in[RIGHT(x, y, width)] + \
		in[TOP(x, y, width)] + in[BOTTOM(x, y, width)]) / 4)
    
/**  bilinear interpolation, 4 edges */
#define INTERPOLATE_X(in, x, y, w) \
	(((uint32_t)in[TOP_LEFT(x, y, w)] + in[BOTTOM_LEFT(x, y, w)] + \
		in[TOP_RIGHT(x, y, w)] + in[BOTTOM_RIGHT(x, y, w)]) / 4)
/*****************************************************************************
**                      	     Threads
*****************************************************************************/
#define __THREAD_TYPE       pthread_t
#define __THREAD_CREATE(t,f,d) (pthread_create(t,NULL,f,d))
#define __THREAD_CREATE_ATTRIB(t,a,f,d) (pthread_create(t,a,f,d))
#define __THREAD_JOIN(t)    (pthread_join(t, NULL))

#define __MUTEX_TYPE        pthread_mutex_t
#define __STATIC_MUTEX_INIT PTHREAD_MUTEX_INITIALIZER
#define __INIT_MUTEX(m)     (pthread_mutex_init(m, NULL) )
#define __CLOSE_MUTEX(m)    (pthread_mutex_destroy(m) )
#define __LOCK_MUTEX(m)     (pthread_mutex_lock(m) )
#define __UNLOCK_MUTEX(m)   (pthread_mutex_unlock(m) )
/*****************************************************************************
**                      	   OpenCV GUI Shortcuts
*****************************************************************************/
#define _1MS                 (1)
#define _ESC_KEY_ASCII       (27)
#define _SPACE_KEY_ASCII     (32)
#define _Q_LETTER            (114)

// #define LOGD(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
// #define LOGE(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
// #define LOGW(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)

struct device
{
    int fd;
    struct buffer *buffers;
    enum v4l2_buf_type type;
    enum v4l2_memory memtype;
    unsigned int nbufs;
    unsigned int width;
    unsigned int height;
    unsigned int bytesperline;
    unsigned int imagesize;
    u_int32_t pixelformat;
};


