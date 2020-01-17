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
#include "../includes/shortcuts.h"
#include "../includes/isp_lib.h"
#include "../includes/utility.h"
#include <opencv2/video/background_segm.hpp>

void tic(double &t)
{
	t = (double)cv::getTickCount();
}

/** 
 * getTickcount: return number of ticks from OS
 * getTickFrequency: returns the number of ticks per second
 * t = ((double)getTickCount() - t)/getTickFrequency();
 * fps is t's reciprocal
 */
double toc(double &t)
{
	return ((double)cv::getTickCount() - t) / cv::getTickFrequency();
}

/** 
 *  Apply gamma correction in-place for a given image array
 *  When *gamma_val < 1, the original dark regions will be brighter 
 *  and the histogram will be shifted to the right 
 *  Whereas it will be the opposite with *gamma_val > 1
 *  Recommend gamma_value: 0.45(1/2.2)
 *  args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 * 		int gamma_value					- gamma value
 */
void apply_gamma_correction(
	cv::InputOutputArray& opencvImage,
    float gamma_value)
{
	cv::Mat look_up_table(1, 256, CV_8U);
	uchar *p = look_up_table.ptr();
	for (int i = 0; i < 256; i++)
	{
		p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma_value) * 255.0);
	}

#ifdef HAVE_OPENCV_CUDA_SUPPORT
	cv::Ptr<cv::cuda::LookUpTable> lutAlg =
		cv::cuda::createLookUpTable(look_up_table);
	lutAlg->transform(opencvImage, opencvImage);
#else
	LUT(opencvImage, look_up_table, opencvImage);
#endif
}

/** 
 *  Apply auto white balance in-place for a given image array
 *  Ref: https://gist.github.com/tomykaira/94472e9f4921ec2cf582
 * 
 *  The basic idea of this AWB algorithm is 
 *  1. defining white as the highest values of R,G,B observed in the image 
 *  2. stretch as much as it can for rgb channels, so that they occupy
 *  maximal range by applying an affine transform ax+b to each channel.
 *  args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 */
void apply_white_balance(
	cv::InputOutputArray& opencvImage)
{
	
	/// if it is gray image, do nothing
	if (opencvImage.type() == CV_8UC1)
		return;
#ifdef HAVE_OPENCV_CUDA_SUPPORT
	
	cv::cuda::GpuMat _opencvImage = opencvImage.getGpuMat();
	std::vector<cv::cuda::GpuMat> bgr_planes;
	cv::cuda::split(_opencvImage, bgr_planes);
	// cv::cuda::GpuMat hist[3];
	// cv::Mat histogram[3];
	// for (int i=0; i< 3; i++)
	// {
	// 	cv::cuda::calcHist(bgr_planes[i], hist[i]);
	// 	hist[i].download(histogram[i]);
	// }
	// float accumulator[3][256];
	// double discard_ratio = 0.05;
	// int vmin[3], vmax[3];
	// int total = _opencvImage.cols * _opencvImage.rows;
	// for (int i = 0; i < 3; i++) {
	// 	for (int j=0; j < (256-1); j++)
	// 	{
	// 		accumulator[i][j+1] = accumulator[i][j] + histogram[i].at<float>(j);
	// 	}
	// 	vmin[i] = 0;
	// 	vmax[i] = 255;
	// 	while (accumulator[i][vmin[i]] < discard_ratio * total)
	// 		vmin[i] += 1;
	// 	while (accumulator[i][vmax[i]] > (1 - discard_ratio) * total)
	// 		vmax[i] -= 1;
	// 	if (vmax[i] < 255 - 1)
	// 		vmax[i] += 1;-
		
	// }
	// for(int i=0; i< 3; i++)
	// {
	// 	printf("vmin[%d]=%d\r\n", i, vmin[i]);
	// 	//printf("vmax[%d]=%d\r\n", i, vmax[i]);
	// }

	cv::cuda::equalizeHist(bgr_planes[0], bgr_planes[0]);
	cv::cuda::equalizeHist(bgr_planes[1], bgr_planes[1]);
	cv::cuda::equalizeHist(bgr_planes[2], bgr_planes[2]);
	
	cv::cuda::merge(bgr_planes, opencvImage);
	
#else
	
	cv::Mat _opencvImage = opencvImage.getMat();
	double discard_ratio = 0.05;
	int vmin[3], vmax[3];
	int total = _opencvImage.cols * _opencvImage.rows;
	/// build cumulative histogram
	/// method 1: naive pixel access
	int hists[3][256];
	CLEAR(hists);
	for (int y = 0; y < _opencvImage.rows; y++)
	{
		uchar *ptr = _opencvImage.ptr<uchar>(y);
		for (int x = 0; x < _opencvImage.cols; x++) 
		{
			for (int j = 0; j < 3; ++j)
			{
				hists[j][ptr[x * 3 + j]] += 1;
			}
		}
 	}

	/// method 2: pointer arithmetic 
	// int hists[3][256];
	// Pixel* pixel = _opencvImage.ptr<Pixel>(0,0);
	// const Pixel* endPixel = pixel + _opencvImage.cols * _opencvImage.rows;
	// for (; pixel!= endPixel; pixel++) 
	// {
	// 	hists[0][pixel->x] ++;
	// 	hists[1][pixel->y] ++;
	// 	hists[2][pixel->z] ++;
	// }

	/// method 3: use OpenCV calcHist to accelerate build histogram
	// cv::Mat bgr_planes[3];
	// cv::Mat hist[3];
	// cv::split(_opencvImage, bgr_planes);
	// float range[] = {0.0f,256.0f};
	// const float *histRange = {range};
	// int hist_size = 256;
	// for (int i=0; i<3; i++)
	// {
	// 	cv::calcHist(
	// 		&bgr_planes[i],  // src(const Mat*)
	// 		1, 				 // n_images
	// 		0, 			     // channels(gray = 0)
	// 		cv::Mat(), 		 // mask (for ROI)
	// 		hist[i], 		 // Mat hist
	// 		1, 				 // dimension
	// 		&hist_size, 	 // histSize = bins = 256
	// 		&histRange);     // ranges for pixel
	// }
	

	/// cumulative hist and search for vmin and vmax
	/// method 1&2: use int hists[3][256] to accumulate and compare
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 255; ++j)
		{
			hists[i][j + 1] += hists[i][j];
		}
		vmin[i] = 0;
		vmax[i] = 255;
		while (hists[i][vmin[i]] < discard_ratio * total)
			vmin[i] += 1;
		while (hists[i][vmax[i]] > (1 - discard_ratio) * total)
			vmax[i] -= 1;
		if (vmax[i] < 255 - 1)
			vmax[i] += 1;
	}

	/// method 3: use float accumulator[3][256] to accumulate and compare
	// float accumulator[3][256];
	// for (int i = 0; i < 3; i++) {
	// 	for (int j=0; j < (hist_size-1); j++)
	// 	{
	// 		accumulator[i][j+1] = accumulator[i][j] + hist[i].at<float>(j);
	// 	}
	// 	vmin[i] = 0;
	// 	vmax[i] = 255;
	// 	while (accumulator[i][vmin[i]] < discard_ratio * total)
	// 		vmin[i] += 1;
	// 	while (accumulator[i][vmax[i]] > (1 - discard_ratio) * total)
	// 		vmax[i] -= 1;
	// 	if (vmax[i] < 255 - 1)
	// 		vmax[i] += 1;
		
	// }

	/// original raw pointer access, slower
	// for (int y = 0; y < _opencvImage.rows; ++y)
	// {
	// 	uchar *ptr = _opencvImage.ptr<uchar>(y);
	// 	for (int x = 0; x < _opencvImage.cols; ++x)
	// 	{
	// 		for (int j = 0; j < 3; ++j)
	// 		{
	// 			int val = ptr[x * 3 + j];
	// 			if (val < vmin[j])
	// 				val = vmin[j];
	// 			if (val > vmax[j])
	// 				val = vmax[j];
	// 			ptr[x * 3 + j] = static_cast<uchar>((val - vmin[j]) * 255.0 / (vmax[j] - vmin[j]));
	// 		}
	// 	}
	// }

	/// use lambda, faster 
	_opencvImage.forEach<Pixel> (
		[&vmin, &vmax] (Pixel &ptr, const int *position) -> void {
			/// saturate the pixels	
			int b = ptr.x < vmin[0] ? vmin[0] : ptr.x > vmax[0] ? vmax[0] : ptr.x;
			int g = ptr.y < vmin[1] ? vmin[1] : ptr.y > vmax[1] ? vmax[1] : ptr.y;
			int r = ptr.z < vmin[2] ? vmin[2] : ptr.z > vmax[2] ? vmax[2] : ptr.z;

			/// rescale the pixels
			ptr.x = static_cast<uchar>((b-vmin[0])*255.0/(vmax[0] - vmin[0]));
			ptr.y = static_cast<uchar>((g-vmin[1])*255.0/(vmax[1] - vmin[1]));
			ptr.z = static_cast<uchar>((r-vmin[2])*255.0/(vmax[2] - vmin[2]));

		}
	);

#endif
}

/** 
 * In-place automatic brightness and contrast optimization with 
 * optional histogram clipping. Looking at histogram, alpha operates 
 * as color range amplifier, beta operates as range shift.
 * O(x,y) = alpha * I(x,y) + beta
 * Automatic brightness and contrast optimization calculates
 * alpha and beta so that the output range is 0..255.
 * Ref: http://answers.opencv.org/question/75510/how-to-make-auto-adjustmentsbrightness-and-contrast-for-image-android-opencv-image-correction/
 * args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 			that can be modified inside the functions
 * 		float clipHistPercent 			 - cut wings of histogram at given percent
 * 			typical=>1, 0=>Disabled
 */

void apply_auto_brightness_and_contrast(
	cv::InputOutputArray& opencvImage,
	float clipHistPercent = 0)
{
	int hist_size = 256;
	float alpha, beta;
	double min_gray = 0, max_gray = 0;

#ifndef HAVE_OPENCV_CUDA_SUPPORT

	/// to calculate grayscale histogram
	cv::Mat gray;
	if (opencvImage.type() != CV_8UC1)
		cv::cvtColor(opencvImage, gray, cv::COLOR_BGR2GRAY);
	else 
		gray = opencvImage.getMat();

	if (clipHistPercent == 0)
	{
		/// keep full available range
		cv::minMaxLoc(
			gray, 		// src
			&min_gray,  // double* minVal
			&max_gray); // double* maxVal
	}
	else
	{
		/// the grayscale histogram
		cv::Mat hist;

		float range[] = {0, 256};
		const float *histRange = {range};
		cv::calcHist(
			&gray, 			// src(const Mat*)
			1, 				// n_images
			0, 				// channels(gray = 0)
			cv::Mat(), 		// mask (for ROI)
			hist, 			// Mat hist
			1,				// dimension
			&hist_size, 	// histSize = bins = 256
			&histRange, 	// ranges_for_pixel
			true, 			// bool uniform
			false);			// bool accumulate

		/// calculate cumulative distribution from the histogram
		std::vector<float> accumulator(hist_size);
		accumulator[0] = hist.at<float>(0);
		for (int i = 1; i < hist_size; i++)
			accumulator[i] = accumulator[i - 1] + hist.at<float>(i);
		
		/// locate points that cuts at required value
		float max = accumulator.back();
		clipHistPercent *= (max / 100.0); /// make percent as absolute
		clipHistPercent /= 2.0;			  /// left and right wings
		
		/// locate left cut
		min_gray = 0;
		while (accumulator[min_gray] < clipHistPercent)
			min_gray++;

		/// locate right cut
		max_gray = hist_size - 1;
		while (accumulator[max_gray] >= (max - clipHistPercent))
			max_gray--;
	}

	cv::Mat _opencvImage = opencvImage.getMat();

#else
	
	/// to calculate grayscale histogram
	cv::cuda::GpuMat gray;
	gray = opencvImage.getGpuMat();
	if (opencvImage.type() != CV_8UC1)
		cv::cuda::cvtColor(gray, gray, cv::COLOR_BGR2GRAY);
	
	cv::Point minLoc, maxLoc;
	if (clipHistPercent == 0)
	{
		/// keep full available range
		cv::cuda::minMaxLoc(
			gray,		// src 
			&min_gray, 	// minVal
			&max_gray,  // maxVal
			NULL,		// Point* minLoc
			NULL);		// Point* maxLoc
	}
	else
	{
		/// the grayscale histogram
		cv::cuda::GpuMat hist;
		/**
		 * calculate histogram for 8-bit gray image
		 * hist is a one row, 256 column, CV_32SC1 type
		 */ 
		cv::cuda::calcHist(
			gray, 			// src
			hist);			// output histogram

		/// calculate cumulative distribution from the histogram
		cv::Mat histogram;
		hist.download(histogram);
		
		std::vector<float> accumulator(hist_size);
		accumulator[0] = histogram.at<float>(0);
		for (int i = 1; i < hist_size; i++)
			accumulator[i] = accumulator[i - 1] + histogram.at<float>(i);
		
		/// locate points that cuts at required value
		float max = accumulator.back();
		clipHistPercent *= (max / 100.0); /// make percent as absolute
		clipHistPercent /= 2.0;			  /// left and right wings
		/// locate left cut
		min_gray = 0;
		while (accumulator[min_gray] < clipHistPercent)
			min_gray++;

		/// locate right cut
		max_gray = hist_size - 1;
		while (accumulator[max_gray] >= (max - clipHistPercent))
			max_gray--;
	}

	cv::cuda::GpuMat _opencvImage = opencvImage.getGpuMat();
#endif
	/// current range
	float input_range = max_gray - min_gray;
	/// alpha expands current range to histsize range
	alpha = (hist_size - 1) / input_range;
	/// beta shifts current range so that minGray will go to 0
	beta = -min_gray * alpha;	

	/**
	 * Apply brightness and contrast normalization
	 * convertTo operates with saturate_cast
	 */
	_opencvImage.convertTo(
		opencvImage,	// dst 
		-1, 			// rtype, if negative, same as input matrix type
		alpha, 				
		beta);	
}

/** 
 * Contrast Limited Adaptive Histogram Equalization) algorithm
 * The algorithm used for OpenCV CUDA and normal is the same
 * Steps: 
 * 1. if it is RGB image, convert image to lab color-space, jump to step 2
 * 2. separate and get L channel of lab planes, jump tp step 4
 * 3. if it is monochrome image, jump to step 4
 * 4. apply adaptive historgram equalization(cv::createCLAHE etc)
 * 5. convert the resulting Lab back to RGB, if it is mono, do nothing
 * Ref: https://stackoverflow.com/questions/24341114/simple-illumination-correction-in-images-opencv-c
 * args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 			that can be modified inside the functions
 */
void apply_clahe(
	cv::InputOutputArray& opencvImage)
{
#ifdef HAVE_OPENCV_CUDA_SUPPORT
	
	if (opencvImage.type() != CV_8UC1)
	{
		cv::cuda::GpuMat lab_image;
		cv::cuda::cvtColor(opencvImage, lab_image, cv::COLOR_BGR2Lab);
		/// Extract the L channel
		std::vector<cv::cuda::GpuMat> lab_planes(3);
		/// now we have the L image in lab_planes[0]
		cv::cuda::split(lab_image, lab_planes); 

		/// apply the CLAHE algorithm to the L channel
		cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE();
		clahe->setClipLimit(4);
		cv::cuda::GpuMat dst;
		clahe->apply(lab_planes[0], dst);

		/// Merge the the color planes back into an Lab image
		dst.copyTo(lab_planes[0]);
		cv::cuda::merge(lab_planes, lab_image);

		/// convert back to RGB
		cv::cuda::cvtColor(lab_image, opencvImage, cv::COLOR_Lab2BGR);
	}
	else
	{
		cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE();
		clahe->setClipLimit(4);
		clahe->apply(opencvImage, opencvImage);		
	}
	
#else
	if (opencvImage.type() != CV_8UC1)
	{
		cv::Mat lab_image;
		cv::cvtColor(opencvImage, lab_image, cv::COLOR_BGR2Lab);
		/// Extract the L channel
		std::vector<cv::Mat> lab_planes(3);
		/// now we have the L image in lab_planes[0]
		cv::split(lab_image, lab_planes);

		/// apply the CLAHE algorithm to the L channel
		cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
		clahe->setClipLimit(4);
		cv::Mat dst;
		clahe->apply(lab_planes[0], dst);

		/// Merge the the color planes back into an Lab image
		dst.copyTo(lab_planes[0]);
		cv::merge(lab_planes, lab_image);

		/// convert back to RGB
		cv::cvtColor(lab_image, opencvImage, cv::COLOR_Lab2BGR);
	}
	else 
	{
		cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
		clahe->setClipLimit(4);
		clahe->apply(opencvImage, opencvImage);
	}
	
#endif
}

/**
 * apply sharpness filter to a given image
 * 
 * ref: https://stackoverflow.com/questions/4993082/how-to-sharpen-an-image-in-opencv
 * use the algorithm of un-sharp mask: 
 * 1. apply a Gaussian smoothing filter
 * 2. subtract the smoothed version from the original image
 * (in a weighted way so the values of a constant area remain constant)
 * sharpness slide will work as the Gaussian blur parameter sigma
 *
 * args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions,
 * 		int sharpness_val - sharpness value
 */
void sharpness_control(
	cv::InputOutputArray& opencvImage,
	int sharpness_val)
{
#ifndef HAVE_OPENCV_CUDA_SUPPORT
	cv::Mat _opencvImage = opencvImage.getMat();
	cv::Mat blurred;
	cv::GaussianBlur(
		_opencvImage,	// src 
		blurred, 		// dst
		cv::Size(0, 0), // ksize, if 0, will compute from sigma
		sharpness_val);	// sigma, Gaussian kernel std in X direction
	
	/// dst = src1*alpha + src2*beta + gamma;
	cv::addWeighted(
		_opencvImage, 	// src1
		1.5, 			// alpha
		blurred, 		// src2
		-0.5, 			// beta
		0, 				// gamma
		opencvImage);	// dst

#else
	if (sharpness_val > 16) sharpness_val = 16;
	int ksize = 2*(sharpness_val)-1;
	cv::cuda::GpuMat _opencvImage = opencvImage.getGpuMat();
	cv::cuda::GpuMat blurred;
	cv::Ptr<cv::cuda::Filter> filter = 
		cv::cuda::createGaussianFilter(
			_opencvImage.type(),		// srcType
			blurred.type(), 		// dstType
			cv::Size(ksize, ksize),	// ksize
			sharpness_val);			// sigma, Gaussian kernel std in X
	filter->apply(_opencvImage, blurred);
	cv::cuda::addWeighted(
		_opencvImage, 	// src1
		1.5, 			// alpha
		blurred, 		// src2
		-0.5, 			// beta
		0, 				// gamma
		opencvImage);	// dst
#endif
}

/**
 * Canny edge detecter
 * 
 * Ref: https://docs.opencv.org/3.4/da/d5c/tutorial_canny_detector.html
 * Steps:
 * 1. convert the image to monochrome
 * 2. smooth the image using a box filter blur for noise reduction(ksize = 5)
 * 3. compute gradient intensity representations of the image
 * 4. apply threshold with hysteresis
 * args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 * 		int edge_low_threshold 			 - edge low threshold
 */
void canny_filter_control(
	cv::InputOutputArray& opencvImage,
	int edge_low_threshold)
{
	const int ratio = 3;
	const int kernel_size = 3;
#ifndef HAVE_OPENCV_CUDA_SUPPORT
	cv::Mat _opencvImage = opencvImage.getMat();
	cv::Mat edges;
	cv::cvtColor(_opencvImage, edges, cv::COLOR_BGR2GRAY);
	/// blur an image use normalized box filter
	cv::blur(
		edges,						// src 
		edges, 						// dst
		cv::Size(5,5));				// ksize
	
	cv::Canny(							
		edges, 						// src
		opencvImage, 				// dst
		(edge_low_threshold), 		// threshold 1
		(edge_low_threshold)*ratio, // threshold 2
		kernel_size);			    // aperture size

#else
	cv::cuda::GpuMat _opencvImage = opencvImage.getGpuMat();
	cv::cuda::GpuMat edges;
	cv::cuda::cvtColor(_opencvImage, edges, cv::COLOR_BGR2GRAY);
	/// blur an image use normalized box filter
	cv::Ptr<cv::cuda::Filter> blur = 
	cv::cuda::createBoxFilter(
		edges.type(),				// srcType
		edges.type(),				// dstType
		cv::Size(5,5));				// ksize
	blur->apply(
		edges,						// src 
		edges);						// dst

	cv::Ptr<cv::cuda::CannyEdgeDetector> canny = 
	cv::cuda::createCannyEdgeDetector(
		(edge_low_threshold), 	   // threshold 1
		(edge_low_threshold)*ratio,// threshold 2
		kernel_size); 			   // aperture size	
	canny->detect(
		edges,					   // src
	 	opencvImage);			   // dst

#endif
}

/**
 * ref: https://github.com/microsoft/Windows-universal-samples/blob/master/Samples/CameraOpenCV/shared/OpenCVBridge/OpenCVHelper.cpp
 */
void display_histogram(
	cv::InputOutputArray& opencvImage)
{
	/// if it is gray image, do nothing
	if (opencvImage.type() == CV_8UC1)
		return;

	cv::Mat _opencvImage = opencvImage.getMat();
	cv::Mat outputMat(_opencvImage.cols, _opencvImage.rows,CV_8UC3);
	std::vector<cv::Mat> bgr_planes;
    split(opencvImage, bgr_planes);
    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    bool uniform = true; bool accumulate = false;

    cv::Mat b_hist, g_hist, r_hist;
	
    calcHist(&bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);
   
    int hist_w = outputMat.cols; int hist_h = outputMat.rows;
    double bin_w = (double)outputMat.cols / histSize;

    normalize(b_hist, b_hist, 0, outputMat.rows, cv::NORM_MINMAX, -1, cv::Mat());
    normalize(g_hist, g_hist, 0, outputMat.rows, cv::NORM_MINMAX, -1, cv::Mat());
    normalize(r_hist, r_hist, 0, outputMat.rows, cv::NORM_MINMAX, -1, cv::Mat());
    for (int i = 1; i < histSize; i++)
    {
        int x1 = cvRound(bin_w * (i - 1));
        int x2 = cvRound(bin_w * i);
        line(outputMat, cv::Point(x1, hist_h - cvRound(b_hist.at<float>(i - 1))),
            cv::Point(x2, hist_h - cvRound(b_hist.at<float>(i))),
            cv::Scalar(255, 0, 0, 255), 2, 8, 0);
        line(outputMat, cv::Point(x1, hist_h - cvRound(g_hist.at<float>(i - 1))),
            cv::Point(x2, hist_h - cvRound(g_hist.at<float>(i))),
            cv::Scalar(0, 255, 0, 255), 2, 8, 0);
        line(outputMat, cv::Point(x1, hist_h - cvRound(r_hist.at<float>(i - 1))),
            cv::Point(x2, hist_h - cvRound(r_hist.at<float>(i))),
            cv::Scalar(0, 0, 255, 255), 2, 8, 0);
    }
	cv::namedWindow("histogram",0);
	cv::resizeWindow("histogram", 640, 480);
	cv::imshow("histogram", outputMat);
}
/**
 * ref: https://github.com/microsoft/Windows-universal-samples/blob/master/Samples/CameraOpenCV/shared/OpenCVBridge/OpenCVHelper.cpp
 */
void motion_detector(
	cv::InputOutputArray& opencvImage)
{
	cv::Ptr<cv::BackgroundSubtractor> pMOG2; //MOG Background subtractor
	//Creates mixture-of-gaussian background subtractor
	pMOG2 = cv::createBackgroundSubtractorMOG2(); //MOG2 approach
	cv::Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method

	pMOG2->apply(opencvImage, fgMaskMOG2);
   // int type = fgMaskMOG2.type();
    cv::Mat temp;
    cv::cvtColor(fgMaskMOG2, temp, cv::COLOR_GRAY2BGRA);

    cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    erode(temp, temp, element);
    temp.copyTo(opencvImage);
}

/** 
 * Separate display right and left mat image from a dual stereo vision camera
 * args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 */
void display_dual_stereo_separately(
	cv::InputOutputArray& opencvImage)
{
	cv::Mat _opencvImage = opencvImage.getMat();
	/// define region of interest for cropped Mat for dual stereo
	cv::Rect roi_left(
		0, 						/// start_x
		0, 						/// start_y
		_opencvImage.cols/2, 	/// width
		_opencvImage.rows);		/// height
	cv::Mat cropped_ref_left(_opencvImage, roi_left);
	cv::Mat cropped_left;
	cropped_ref_left.copyTo(cropped_left);
	cv::imshow("cam_left", cropped_left);

	cv::Rect roi_right(
		_opencvImage.cols/2,     /// start_x 
		0, 						 /// start_y
		_opencvImage.cols/2, 	 /// width
		_opencvImage.rows);		 /// height
	cv::Mat cropped_ref_right(_opencvImage, roi_right);
	cv::Mat cropped_right;
	cropped_ref_right.copyTo(cropped_right);
	cv::imshow("cam_right", cropped_right);
}

/**
 * unify putting text in opencv image
 * a wrapper for put_text()
 * args:
 * 		cv::Mat opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 * 		str 				- text you will put in
 * 		cordinate_y 		- vertical location that will put this line  
 */
void streaming_put_text(
	cv::Mat &opencvImage,
	const char *str, 
	int cordinate_y)
{
	int scale = opencvImage.cols / 1000;
	cv::putText(
		opencvImage,
		str,
		cv::Point(scale * TEXT_SCALE_BASE, cordinate_y), // Coordinates
		cv::FONT_HERSHEY_SIMPLEX,						 // Font
		(float)scale,									 // Scale. 2.0 = 2x bigger
		cv::Scalar(255, 255, 255),						 // BGR Color - white
		2												 // Line Thickness
	);													 // Anti-alias (Optional)
}

/** 
 * put a given stream's info text in: 
 * res, fps, ESC, 
 * automatically adjust text size with different camera resolutions
 *  args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
*/
void display_current_mat_stream_info(
	cv::InputOutputArray& opencvImage,
	double *cur_time)
{
	cv::Mat _opencvImage = opencvImage.getMat();
	int height_scale = (_opencvImage.cols / 1000);
	streaming_put_text(_opencvImage, "ESC: close application",
					   height_scale * TEXT_SCALE_BASE);

	char resolution[25];
	sprintf(resolution, "Current Res:%dx%d", 
		_opencvImage.cols, _opencvImage.rows);
	streaming_put_text(_opencvImage, resolution,
					   height_scale * TEXT_SCALE_BASE * 2);

	double fps = 1.0 / toc(*cur_time);
	char string_frame_rate[10];				 // string to save fps
	sprintf(string_frame_rate, "%.2f", fps); // to 2 decimal places
	char fpsString[20];
	strcpy(fpsString, "Current Fps:");
	strcat(fpsString, string_frame_rate);
	streaming_put_text(_opencvImage, fpsString,
					   height_scale * TEXT_SCALE_BASE * 3);
}

/**
 * Apply brightness(alpha) and contrast(beta) control from slider val
 * for both color and mono camera stream
 * histogram clipping. Looking at histogram, alpha operates 
 * as color range amplifier, beta operates as range shift.
 * O(x,y) = alpha * I(x,y) + beta
 * args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 */
void apply_brightness_and_contrast(
	cv::InputOutputArray& opencvImage,
	int alpha_val,
	int beta_val)
{
#ifndef HAVE_OPENCV_CUDA_SUPPORT
	cv::Mat _opencvImage = opencvImage.getMat();
#else
	cv::cuda::GpuMat _opencvImage = opencvImage.getGpuMat();
#endif 
	_opencvImage.convertTo(
	opencvImage,	// dst 
	-1, 			// rtype, if negative, same as input matrix type
	alpha_val, 				
	beta_val);
}

/**
 * Debayer and apply AWB for a given frame(support both OpenCV CUDA and not)
 * args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 */
void debayer_awb_a_frame(
	cv::InputOutputArray& opencvImage,
	int bayer_flg,
    int awb_flg)
{
	/** color output */
	if (bayer_flg != CV_MONO_FLG)
	{
#ifdef HAVE_OPENCV_CUDA_SUPPORT
		// cv::cuda::cvtColor(opencvImage, opencvImage,
		// cv::COLOR_BayerBG2BGR + bayer_flg);
		cv::cuda::demosaicing(opencvImage, opencvImage,
			cv::cuda::COLOR_BayerBG2BGR_MHT + bayer_flg);
#else
		cv::cvtColor(opencvImage, opencvImage,
			cv::COLOR_BayerBG2BGR + bayer_flg);
	
#endif
		if (awb_flg)
		{	
			//Timer timer;
			apply_white_balance(opencvImage);
		}
	}
	/** mono output */
	if (bayer_flg == CV_MONO_FLG && opencvImage.type() == CV_8UC3)
	{
#ifdef HAVE_OPENCV_CUDA_SUPPORT
			cv::cuda::cvtColor(opencvImage, opencvImage, cv::COLOR_BayerBG2BGR);
			cv::cuda::cvtColor(opencvImage, opencvImage, cv::COLOR_BGR2GRAY);
#else
			cv::cvtColor(opencvImage, opencvImage, cv::COLOR_BayerBG2BGR);
			cv::cvtColor(opencvImage, opencvImage, cv::COLOR_BGR2GRAY);	
#endif
	}

}

/**
 * In-place apply rgb color correction matrix for a given mat
 * since this requires to access individual pixel of a given mat, OpenCV CUDA
 * acceleration is not used
 * For color correction matrix, please request from Leopard support
 * Refs: http://www.imatest.com/docs/colormatrix/
 * args:
 * 		cv::InputOutputArray opencvImage - camera stream buffer array
 * 		that can be modified inside the functions
 */
void apply_rgb_matrix_post_debayer(
	cv::InputOutputArray& opencvImage,
	int* ccm)
{
	
	int rb = *ccm;
	int rg = *(ccm+1);
	int rr = *(ccm+2);
	int gb = *(ccm+3);
	int gg = *(ccm+4);
	int gr = *(ccm+5);
	int bb = *(ccm+6);
	int bg = *(ccm+7);
	int br = *(ccm+8);

	cv::Mat _opencvImage = opencvImage.getMat();

	/// method 1: raw pointer access, 150ms
	// uchar r,g,b;
	// for (int i=0; i < _opencvImage.rows;++i) {
	// 	/// point to first pixel in row
	// 	cv::Vec3b* pixel = _opencvImage.ptr<cv::Vec3b>(i); 
	// 	for (int j=0; j < _opencvImage.cols;++j) {
	// 		r = pixel[j][2];
	// 		g = pixel[j][1];
	// 		b = pixel[j][0];
	// 		pixel[j][2] = ((rb) * b + (rg) * g + (rr) * r)/256;
	// 		pixel[j][1] = ((gb) * b + (gg) * g + (gr) * r)/256;
	// 		pixel[j][0] = ((bb) * b + (bg) * g + (br) * r)/256;
	// 	}
	// }

	/// method 2: raw pointer access, 60ms
	// uchar r,g,b;
	// for (int i=0; i < _opencvImage.rows;++i) {
	// 	/// point to first pixel in row
	// 	Pixel* ptr = _opencvImage.ptr<Pixel>(i);
	// 	const Pixel* ptr_end = ptr + _opencvImage.cols;
    // 	for (; ptr != ptr_end; ++ptr) {
	// 		b = ptr->x;
	// 		g = ptr->y;
	// 		r = ptr->z;
	// 		ptr->z = ((rb) * b + (rg) * g + (rr) * r)/256;
	// 		ptr->y = ((gb) * b + (gg) * g + (gr) * r)/256;
	// 		ptr->x = ((bb) * b + (bg) * g + (br) * r)/256;

	// 	}
	// }

	/// method 3: using MatIterator, 150ms
	// uchar r,g,b;
	// for (Pixel &ptr : cv::Mat_<Pixel>(_opencvImage)) {
	// 	b = ptr.x;
	//  	g = ptr.y;
	//  	r = ptr.z;
	//  	ptr.z = ((rb) * b + (rg) * g + (rr) * r)/256;
	//  	ptr.y = ((gb) * b + (gg) * g + (gr) * r)/256;
	//  	ptr.x = ((bb) * b + (bg) * g + (br) * r)/256;
	// }

	/// method 4: using lambda, 18ms
	/// capture everything by value
	/// Pixel (x,y,z) = (1,2,3) is (b,g,r) = (1,2,3).
	_opencvImage.forEach<Pixel>( 
		[=] (Pixel &ptr, const int *position) -> void {

			uchar b = ptr.x;
	 		uchar g = ptr.y;
	 		uchar r = ptr.z;
	 		ptr.z = ((rb) * b + (rg) * g + (rr) * r)/256;
	 		ptr.y = ((gb) * b + (gg) * g + (gr) * r)/256;
	 		ptr.x = ((bb) * b + (bg) * g + (br) * r)/256;			
		}
	);

}