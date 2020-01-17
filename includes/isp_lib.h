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
 * This is the sample code for image signal processing library that used     
 * in this application.														 
 *																			 
 * Author: Danyu L														     
 * Last edit: 2019/08														 
*****************************************************************************/
#pragma once
/** Include files to use OpenCV API */
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#ifdef HAVE_OPENCV_CUDA_SUPPORT
#include <opencv4/opencv2/cudaobjdetect.hpp>
#include <opencv4/opencv2/cudaimgproc.hpp>
#include <opencv4/opencv2/cudafilters.hpp>
#include <opencv4/opencv2/cudaarithm.hpp>
#endif

// Define a pixel
typedef cv::Point3_<uint8_t> Pixel;

typedef enum
{
   CV_BayerBG2BGR_FLG = 0,
   CV_BayerGB2BGR_FLG,
   CV_BayerRG2BGR_FLG,
   CV_BayerGR2BGR_FLG,
   CV_MONO_FLG
} pixel_order_flag;

void tic(double &t);
double toc(double &t);

void apply_gamma_correction(
	cv::InputOutputArray& opencvImage,
    float gamma_value);

void apply_white_balance(
	cv::InputOutputArray& opencvImage);

void apply_auto_brightness_and_contrast(
	cv::InputOutputArray& opencvImage,
	float clipHistPercent);

void apply_clahe(
	cv::InputOutputArray& opencvImage);

void sharpness_control(
	cv::InputOutputArray& opencvImage,
	int sharpness_val);

void canny_filter_control(
	cv::InputOutputArray& opencvImage,
	int edge_low_threshold);

void display_histogram(
	cv::InputOutputArray& opencvImage);

void motion_detector(
	cv::InputOutputArray& opencvImage);
	
void display_dual_stereo_separately(
	cv::InputOutputArray& opencvImage);

void apply_brightness_and_contrast(
	cv::InputOutputArray& opencvImage,
	int alpha_val,
	int beta_val);

void streaming_put_text(
	cv::Mat& opencvImage,
	const char *str, 
	int cordinate_y);

void display_current_mat_stream_info(
	cv::InputOutputArray& opencvImage,
	double *cur_time);

void debayer_awb_a_frame(
	cv::InputOutputArray& opencvImage,
	int bayer_flg,
    int awb_flg);

void apply_rgb_matrix_post_debayer(
	cv::InputOutputArray& opencvImage,
	int* ccm);