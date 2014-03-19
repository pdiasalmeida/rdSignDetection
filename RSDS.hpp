#ifndef RSDS_HPP_
#define RSDS_HPP_

#include "postProcess/PostProcess.h"

#include <opencv2/core/core.hpp>
#include <eigen2/Eigen/Core>

#include <vector>


class RSDS
{
public:
	RSDS( cv::Mat image );
	~RSDS();

	void colorSegmentation( int colorCode, cv::Mat& result );
	void noiseElimination( cv::Mat inImage, cv::Mat& outImage );
	void convexHullContours( cv::Mat inImage, cv::Mat& outImage, std::vector< std::vector< cv::Point > >& contours );

	void processCountours( cv::Mat image, std::vector< std::vector< cv::Point > > contours, std::vector< cv::Rect >& signs );
	cv::Mat drawPoints( const cv::Mat &image, const std::vector< Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d> > &data );

	static int nColorModes;

private:
	RSDS();

	bool validateSign( cv::Mat image, cv::Rect pSign );

	PostProcess _pp;
	std::vector< std::vector<cv::Point> > _copyCont;

protected:
	cv::Mat _baseImage;
	long _aspect_are;
	double _low_ratio;
	double _high_ratio;
	float _dist_threshold;

	float _minSignArea;
	float _maxSignArea;

};

#endif
