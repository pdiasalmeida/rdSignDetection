#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include <eigen2/Eigen/Core>

#include "auxiliar/Log.h"
#include "auxiliar/Files.hpp"
#include "RSDS.hpp"

const std::string benchmarkDir = "test/";
const char* benchmarkFile = "result/benchmark.txt";

int
main( int argc, char** argv )
{
	clock_t totalTime;
	totalTime = clock();

	std::map< std::string, std::vector< cv::Rect > > showSigns;

	// --------------------------------------------- read image --------------------------------------------- //
	std::vector< std::string > validExtensions;
	validExtensions.push_back("jpg");
	validExtensions.push_back("png");
	validExtensions.push_back("ppm");
	validExtensions.push_back("jp2");
	Files fh = Files( validExtensions );

	std::vector< std::string > fileNames;
	int nFiles = fh.getFilesInDirectory( benchmarkDir, fileNames );

	std::ofstream File;
	File.open( benchmarkFile );
	int n = 1;

	for( std::vector< std::string >::iterator it = fileNames.begin(); it != fileNames.end(); it++, n++ )
	{
		std::string fileName( (*it) );
		Log::notice( "Processing image: '" + fileName + "' (" + Log::to_string(n) + " of " + Log::to_string(nFiles) + " ).." );

		clock_t t;
		t = clock();

		cv::Mat image = cv::imread( fileName );
		std::vector< std::vector< cv::Point > > data;
		std::vector< cv::Rect > signs;

		RSDS rsd = RSDS( image );

		for( int nm = 0; nm < RSDS::nColorModes; nm ++ )
		{
			// ------------------------------------------- convert to nhs ------------------------------------------- //
			// The second argument of convert_ihls_to_nhs means if it's red or blue. 0 is red, blue is 1.
			// You can put 2 here for others, but then you have to provide the hue max and min, sat min values.
			cv::Mat nhs_image;
			rsd.colorSegmentation( nm, nhs_image );

			// ------------------------------------------- PostProcessing ------------------------------------------- //
			cv::Mat nhs_image_for_post = nhs_image.clone();
			cv::Mat eimg;
			rsd.noiseElimination( nhs_image_for_post, eimg );

			// -------------------------------------- PostProcessing - Convex --------------------------------------- //
			// ------------------------------------ PostProcessing - Get contour ------------------------------------ //
			cv::Mat cimg;
			rsd.convexHullContours( eimg, cimg, data );
		}
		rsd.processCountours( image, data, signs );

		t = clock() - t;
		float calcDuration = ( (float) t ) / CLOCKS_PER_SEC;

		for( std::vector< cv::Rect >::iterator sit = signs.begin(); sit != signs.end(); sit ++ )
		{
//			std::cout << fileName << ";" << (*sit).x << ";" << (*sit).y << ";" << (*sit).x + (*sit).width << ";"
//					<< (*sit).y + (*sit).height << ";" << "-1" << ";" << calcDuration << std::endl;

			fh.saveSignBlobToFile( fileName, (*sit), -1, calcDuration, File );

			std::map< std::string, std::vector< cv::Rect > >::iterator it = showSigns.find(fileName);
			if( it != showSigns.end() )
			{
				it->second.push_back( (*sit) );
			}
			else
			{
				std::vector< cv::Rect > sr;
				sr.push_back( (*sit) );
				showSigns.insert( std::pair< std::string, std::vector< cv::Rect > >( fileName, sr ) );
			}
		}
	}

	File.flush();
	File.close();

	totalTime = clock() - totalTime;
	float calcDuration = ( (float) totalTime ) / CLOCKS_PER_SEC;
	std::cout << "Benchmark processed in " << calcDuration << " seconds." << std::endl;

	std::cout << "The following images have signs in them:" << std::endl;
	for( std::map< std::string, std::vector< cv::Rect > >::iterator it = showSigns.begin(); it != showSigns.end(); it++ )
	{
		cv::Mat result = cv::imread( it->first );

		for( std::vector< cv::Rect >::iterator sit = it->second.begin(); sit != it->second.end(); sit ++ )
		{
			cv::Point2f rect_points[4];
			rect_points[0] = cv::Point2f( (*sit).x, (*sit).y );
			rect_points[1] = cv::Point2f( (*sit).x, (*sit).y + (*sit).width );
			rect_points[2] = cv::Point2f( (*sit).x + (*sit).height, (*sit).y + (*sit).width );
			rect_points[3] = cv::Point2f( (*sit).x + (*sit).height, (*sit).y );
			for( int j = 0; j < 4; j++ )
				line( result, rect_points[j], rect_points[(j+1)%4], cv::Scalar(0, 255, 0), 2, 8 );
		}

		cv::namedWindow( "result", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED );
		cv::imshow( "result", result );

		std::cout << '\t' << it->first << std::endl;

		cv::waitKey(0);
	}

	return 0;
}
