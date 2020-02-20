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

#include <opencv2/video/background_segm.hpp> /// for motion detector

#include <opencv2/aruco.hpp>				 /// for camera calibration
#include <opencv2/calib3d/calib3d.hpp>		 
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


const float calibrationSquareDimension = 0.01905f;  // meters
const float arucoSquareDimension = 0.1016f; 		// meters
const cv::Size chessboard_dimensions = cv::Size(6, 8);

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
	cv::InputOutputArray& opencvImage);

void debayer_awb_a_frame(
	cv::InputOutputArray& opencvImage,
	int bayer_flg,
    int awb_flg);

void apply_rgb_matrix_post_debayer(
	cv::InputOutputArray& opencvImage,
	int* ccm);

cv::Mat decode_mpeg_img(
	cv::InputOutputArray opencvImage);


///////////////////////////////////////////
void create_checkboard(
	cv::Mat &checkboard_output, 
	int num_row, 
	int num_col, 
	int square_size, 
	cv::Scalar color1, 
	cv::Scalar color2);
void generate_checkboard(int row, int col);

void create_aruco_markers(int cap_num);
void create_known_board_position(
	cv::Size board_size, 
	float square_edge_length, 
	std::vector<cv::Point3f>& corners);

void get_chessboard_corners(
	cv::Size board_size, 
	std::vector<cv::Mat> images, 
	std::vector<std::vector<cv::Point2f>>& all_found_corners, 
	bool show_results /*= false*/);

void camera_calibration(
	std::vector<cv::Mat> calibration_images, 
	cv::Size board_size, 
	float square_edge_length, 
	cv::Mat& camera_matrix, 
	cv::Mat& distance_coefficients);

bool save_camera_calibration(
	std::string name, 
	cv::Mat camera_matrix, 
	cv::Mat distance_coefficients);

