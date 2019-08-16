/*****************************************************************************
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc., 

  This is the sample code for image signal processing library that used
  in this application.

  Author: Danyu L
  Last edit: 2019/08
*****************************************************************************/
#include "../includes/shortcuts.h"
#include "../includes/isp_lib.h"


/** 
 *  Apply auto white balance in-place for a given image array
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
 *  the basic idea of Leopard AWB algorithm is to find the gray
 *  area of the image and apply Red, Green and Blue gains to make
 *  it gray, and then use the gray area to estimate the color 
 *  temperature.
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
	cv::cuda::equalizeHist(bgr_planes[0], bgr_planes[0]);
	cv::cuda::equalizeHist(bgr_planes[1], bgr_planes[1]);
	cv::cuda::equalizeHist(bgr_planes[2], bgr_planes[2]);
	cv::cuda::merge(bgr_planes, opencvImage);
	
#else
	/// ref: https://gist.github.com/tomykaira/94472e9f4921ec2cf582
	cv::Mat _opencvImage = opencvImage.getMat();
	double discard_ratio = 0.05;
	int hists[3][256];
	CLEAR(hists);

	for (int y = 0; y < _opencvImage.rows; ++y)
	{
		uchar *ptr = _opencvImage.ptr<uchar>(y);
		for (int x = 0; x < _opencvImage.cols; ++x)
		{
			for (int j = 0; j < 3; ++j)
			{
				hists[j][ptr[x * 3 + j]] += 1;
			}
		}
	}

	/// cumulative hist
	int total = _opencvImage.cols * _opencvImage.rows;
	int vmin[3], vmax[3];
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

	for (int y = 0; y < _opencvImage.rows; ++y)
	{
		uchar *ptr = _opencvImage.ptr<uchar>(y);
		for (int x = 0; x < _opencvImage.cols; ++x)
		{
			for (int j = 0; j < 3; ++j)
			{
				int val = ptr[x * 3 + j];
				if (val < vmin[j])
					val = vmin[j];
				if (val > vmax[j])
					val = vmax[j];
				ptr[x * 3 + j] = static_cast<uchar>((val - vmin[j]) * 255.0 / (vmax[j] - vmin[j]));
			}
		}
	}


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
 * 4. apply threshold with hysterysis
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
		0, 						// start_x
		0, 						// start_y
		_opencvImage.cols/2, 	// width
		_opencvImage.rows);		// height
	cv::Mat cropped_ref_left(_opencvImage, roi_left);
	cv::Mat cropped_left;
	cropped_ref_left.copyTo(cropped_left);
	cv::imshow("cam_left", cropped_left);

	cv::Rect roi_right(
		_opencvImage.cols/2,     // start_x 
		0, 						// start_y
		_opencvImage.cols/2, 	// width
		_opencvImage.rows);		// height
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
	cv::cuda::GpuMat opencvImage = _opencvImage.getGpuMat();
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
			apply_white_balance(opencvImage);
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
	int rb, int rg, int rr,
	int gb, int gg, int gr,
	int bb, int bg, int br)
{
	cv::Mat _opencvImage = opencvImage.getMat();
	uchar r,g,b;

	for (int i=0; i < _opencvImage.rows;++i) {
		/// point to first pixel in row
		cv::Vec3b* pixel = _opencvImage.ptr<cv::Vec3b>(i); 
		for (int j=0; j < _opencvImage.cols;++j) {
			r = pixel[j][2];
			g = pixel[j][1];
			b = pixel[j][0];
			pixel[j][2] = ((rb) * b + (rg) * g + (rr) * r)/256;
			pixel[j][1] = ((gb) * b + (gg) * g + (gr) * r)/256;
			pixel[j][0] = ((bb) * b + (bg) * g + (br) * r)/256;
		}
	}

}