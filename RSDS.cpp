#include "RSDS.hpp"
#include "ihls_nhs/ihls.h"
#include "ihls_nhs/nhs.h"

#include <fstream>
#include <algorithm>

int RSDS::nColorModes = 2;

RSDS::RSDS( cv::Mat image )
{
	_baseImage = image.clone();

	_aspect_are = 1500;
	_low_ratio = 0.25;
	_high_ratio = 1.3;
	_dist_threshold = 30;

	_minSignArea = 750;
	_maxSignArea = 5000;
}

RSDS::RSDS()
{
	_baseImage = cv::Mat();

	_aspect_are = 1500;
	_low_ratio = 0.25;
	_high_ratio = 1.3;
	_dist_threshold = 30;

	_minSignArea = 750;
	_maxSignArea = 5000;
}

void RSDS::colorSegmentation( int colorCode, cv::Mat& result )
{
	cv::Mat ihls_image = convert_rgb_to_ihls( _baseImage );
	result = convert_ihls_to_nhs( ihls_image, colorCode );
}

void RSDS::noiseElimination( cv::Mat inImage, cv::Mat& outImage )
{
	_pp = PostProcess( inImage );
	cv::Mat fimg = _pp.filterImage();
	cv::Mat fimg_save = fimg.clone();

	outImage = _pp.elimination( fimg, _copyCont, _aspect_are, _low_ratio, _high_ratio );
	cv::Mat eimg_save = outImage.clone();
}

void RSDS::convexHullContours( cv::Mat inImage, cv::Mat& outImage, std::vector< std::vector< cv::Point > >& contours )
{
	std::vector< std::vector< cv::Point > >hull( _copyCont.size() );
	cv::Mat himg = _pp.convex( inImage, hull, _copyCont );
	cv::Mat himg_save = himg.clone();

	outImage = _pp.thresholdedContour( hull, _copyCont, contours, _dist_threshold );
}

void RSDS::processCountours( cv::Mat image, std::vector< std::vector< cv::Point > > contours, std::vector< cv::Rect >& signs )
{
	std::vector< cv::RotatedRect > pSigns( contours.size() );

	for( unsigned int i = 0; i < contours.size(); i++ )
	{
		pSigns[i] = minAreaRect( cv::Mat(contours[i]) );
		cv::Rect pSign = pSigns[i].boundingRect();
		bool exists = false;

		for( std::vector< cv::Rect >::iterator rit = signs.begin(); !exists && rit != signs.end(); rit++ )
		{
			if( (*rit) == pSign )
			{
				exists = true;
			}
		}

		if( !exists && validateSign( image, pSign ) )
		{
			signs.push_back( pSign );
		}
	}
}

bool RSDS::validateSign( cv::Mat image, cv::Rect pSign )
{
	bool result = false;

	float area = (float)pSign.height * (float)pSign.width;
	float ratio = (float)pSign.height / (float)pSign.width;

	result = pSign.x > 0 && pSign.y > 0;
	// signs found have to be approximately symmetric
	result = result && ( ratio > 0.8f && ratio < 1.5f );
	// eliminate signs in top third of the image
	result = result && ( pSign.y > 0.30 * image.rows );
	result = result && ( pSign.y < image.rows - ( 0.20 * image.rows ) );
	// sign area has to be between a certain percentage of the image area
	result = result && ( area < _maxSignArea  );
	result = result && ( area > _minSignArea  );

	return result;
}

cv::Mat RSDS::drawPoints( const cv::Mat &image, const std::vector< Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d> > &data )
{
	//    Mat dest = Mat::zeros( image.size(), CV_8U );
	cv::Mat dest = image;

	int size = data.size();
	for( int i = 0; i < size; i++ )
	{
		circle( dest, cv::Point( data[i][0], data[i][1] ), 0, cvScalar( 0, 255, 0 ), 2 ); //Green
	}

	return dest;
}

RSDS::~RSDS(){}
