#include <vector>
#include <set>
#include <utility>
#include <iostream>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../auxiliar/Files.hpp"
#include "../auxiliar/key_iterator.hpp"

const std::string benchmarkFile = "../test/gt.txt";
const std::string fileToEvaluate = "../result/benchmark.txt";

struct eval_result
{
	std::vector< cv::Rect > correctMatches;
	std::vector< cv::Rect > falsePositives;
	std::vector< cv::Rect > falseNegatives;

	float detectionRate;
	float falsePositiveRate;
	float falseNegativeRate;
};

// string to correct image paths
const std::string relPath = "";

// print results in image
bool printInImage = false;

int buildStructure( std::map< std::string, std::vector< cv::Rect > >& baseStruc,
		std::vector< std::pair< std::string, cv::Rect > >& signs );
void printStructure( std::map< std::string, std::vector< cv::Rect > >& baseStruc );

void evaluate( std::map< std::string, eval_result >& evaluation, std::map< std::string, std::vector< cv::Rect > >& baseStruc,
		std::map< std::string, std::vector< cv::Rect > >& evalStruc );
bool acceptRect( cv::Rect a, cv::Rect b);
void printEvaluation( std::map< std::string, eval_result >& evaluation, int nSigns );
void drawRects( cv::Mat& image, std::vector< cv::Rect > rects, cv::Scalar color );

int main( int argc, char** argv )
{
	std::map< std::string, std::vector< cv::Rect > > gtSigns;
	std::map< std::string, std::vector< cv::Rect > > bcSigns;

	std::vector< std::pair< std::string, cv::Rect > > signs;
	std::vector< std::pair< std::string, cv::Rect > > psigns;

	std::map< std::string, eval_result > evaluation;

	Files fh = Files();

	// base benchmark ground truth
	fh.parseGroundTruth( benchmarkFile, signs );

	// ground truth to evaluate
	fh.parseGroundTruth( fileToEvaluate, psigns );

	int nSigns = buildStructure( gtSigns, signs );
	int nPSigns = buildStructure( bcSigns, psigns );

	printStructure( gtSigns );
	printStructure( bcSigns );

	evaluate( evaluation, gtSigns, bcSigns );
	printEvaluation( evaluation, nSigns );

	return 0;
}

int buildStructure( std::map< std::string, std::vector< cv::Rect > >& baseStruc,
		std::vector< std::pair< std::string, cv::Rect > >& signs )
{
	int nSigns = 0;
	for( std::vector< std::pair< std::string, cv::Rect > >::iterator it = signs.begin(); it != signs.end(); it++ )
	{
		std::map< std::string, std::vector< cv::Rect > >::iterator element = baseStruc.find( (*it).first );

		if( element == baseStruc.end() )
		{
			std::vector< cv::Rect > vectorSigns;
			vectorSigns.push_back( (*it).second ); nSigns++;
			baseStruc.insert( std::pair< std::string, std::vector< cv::Rect > >( (*it).first, vectorSigns ) );
		}
		else
		{
			element->second.push_back( (*it).second ); nSigns++;
		}
	}

	return nSigns;
}

// TODO ensure that referenced filenames are equal
void evaluate( std::map< std::string, eval_result >& evaluation, std::map< std::string, std::vector< cv::Rect > >& baseStruc,
		std::map< std::string, std::vector< cv::Rect > >& evalStruc )
{
	for( std::map< std::string, std::vector< cv::Rect > >::iterator it = evalStruc.begin(); it != evalStruc.end(); it++ )
	{
		// necessary to parse file name
		unsigned found = (*it).first.find_last_of("/\\");
		std::string fileName = (*it).first.substr(found+1);
		// expected signs for this image
		std::map< std::string, std::vector< cv::Rect > >::iterator element = baseStruc.find( fileName );

		// signs already identified
		std::vector< cv::Rect > idtSigns;

		// auxiliary set with all expected signs, to calculate false negatives
		int size = (element != baseStruc.end()) ? element->second.size() : 0;
		std::vector< cv::Rect > fneg( size );

		// structure to store evaluation results
		eval_result evr = eval_result();

		if( element != baseStruc.end() )
		{
			std::copy( element->second.begin(), element->second.end(), fneg.begin() );

			for( std::vector< cv::Rect >::iterator itev = (*it).second.begin(); itev != (*it).second.end(); itev++ )
			{
				bool foundMatch = false;
				for( std::vector< cv::Rect >::iterator itgt = element->second.begin();
						!foundMatch && itgt != element->second.end(); itgt++ )
				{
					if( std::find(idtSigns.begin(), idtSigns.end(), (*itgt) ) == idtSigns.end()
							&& acceptRect( (*itgt), (*itev) ) )
					{
						// correct
						evr.correctMatches.push_back( (*itev) );
						foundMatch = true;
						idtSigns.push_back( (*itgt) );

						std::vector< cv::Rect >::iterator sf = std::find( fneg.begin(), fneg.end(), (*itgt) );
						if( sf != fneg.end() )
						{
							fneg.erase( sf );
						}
					}
					if( acceptRect( (*itgt), (*itev) ) ) foundMatch = true;
				}

				if( foundMatch == false  )
				{
					// false positive
					evr.falsePositives.push_back( (*itev) );
				}
			}
		}
		else
		{
			//they're all false positives
			for( std::vector< cv::Rect >::iterator itev = (*it).second.begin(); itev != (*it).second.end(); itev++ )
			{
				evr.falsePositives.push_back( (*itev) );
			}
		}

		evr.falseNegatives.resize(fneg.size());
		std::copy( fneg.begin(), fneg.end(), evr.falseNegatives.begin() );

		evr.detectionRate = (float) evr.correctMatches.size() / (float) element->second.size();
		evr.falsePositiveRate = (float) evr.falsePositives.size() / (float) (*it).second.size();
		evr.falseNegativeRate = (float) evr.falseNegatives.size() / (float) element->second.size();

		evaluation.insert( std::pair< std::string, eval_result >( fileName, evr ) );
	}

	for( std::map< std::string, std::vector< cv::Rect > >::iterator it = baseStruc.begin(); it != baseStruc.end(); it++ )
	{
		// necessary to parse file name
		std::string fileName = relPath + (*it).first;
		// expected signs for this image
		std::map< std::string, std::vector< cv::Rect > >::iterator element = evalStruc.find( fileName );

		if( element == evalStruc.end() )
		{
			eval_result evr = eval_result();
			std::vector< cv::Rect > fneg( it->second.size() );
			std::copy( it->second.begin(), it->second.end(), fneg.begin() );

			evr.falseNegatives = fneg;
			evr.detectionRate = 0.0f;
			evr.falsePositiveRate = 0.0f;
			evr.falseNegativeRate = 1.0f;

			evaluation.insert( std::pair< std::string, eval_result >( fileName, evr ) );
		}
	}
}

bool acceptRect( cv::Rect a, cv::Rect b)
{
	cv::Rect intersect = a & b;
	float unionArea = a.area() + b.area() - intersect.area();

	return ( intersect.area() / unionArea ) > 0.5;
}

void printStructure( std::map< std::string, std::vector< cv::Rect > >& baseStruc )
{
	std::cout << "Found " << baseStruc.size() << " images to evaluate." << std::endl;
	for( std::map< std::string, std::vector< cv::Rect > >::iterator it = baseStruc.begin(); it != baseStruc.end(); it++ )
	{
		std::cout << "In the image file, '" << (*it).first  << "', there are the following signs:" << std::endl;
		for( std::vector< cv::Rect >::iterator itv = (*it).second.begin(); itv != (*it).second.end(); itv++ )
		{
			std::cout << '\t' << itv->x << " " << itv->y << " " << itv->width << " " << itv->height << std::endl;
		}
	}
}

void printEvaluation( std::map< std::string, eval_result >& evaluation, int nSigns )
{
	int numberDetections = 0;
	int numberMatches = 0;
	int numberFP = 0;
	int numberFN = 0;

	cv::Mat result;

	std::cout << "Found " << evaluation.size() << " images evaluated." << std::endl;
	for( std::map< std::string, eval_result >::iterator it = evaluation.begin(); it != evaluation.end(); it++ )
	{
		if( printInImage ) result = cv::imread( "../test/" + (*it).first );

		std::cout << "In the image file, '" << (*it).first  << "', the following results were obtained:" << std::endl;
		std::cout << "Overall results:" << "\t Detection Rate = " << (*it).second.detectionRate <<
				"\t False Positive Rate = " << (*it).second.falsePositiveRate <<
				"\t False Negative Rate = " << (*it).second.falseNegativeRate << std::endl;

		//-----------------------------Correct_Matches---------------------------------------------//
		numberDetections += (*it).second.correctMatches.size();
		numberMatches += (*it).second.correctMatches.size();
		std::cout << '\t' << (*it).second.correctMatches.size() << " correct Matches: ";
		for( std::vector< cv::Rect >::iterator itv = (*it).second.correctMatches.begin();
				itv != (*it).second.correctMatches.end(); itv++ )
		{
			std::cout << '\t' << itv->x << " " << itv->y << " " << itv->width << " " << itv->height;
		}
		std::cout << std::endl;
		if( printInImage ) drawRects( result, (*it).second.correctMatches, cv::Scalar(0,255,0) );

		//-----------------------------False_Positives---------------------------------------------//
		numberDetections += (*it).second.falsePositives.size();
		numberFP += (*it).second.falsePositives.size();
		std::cout << '\t' << (*it).second.falsePositives.size() << " false positives: ";
		for( std::vector< cv::Rect >::iterator itv = (*it).second.falsePositives.begin();
				itv != (*it).second.falsePositives.end(); itv++ )
		{
			std::cout << '\t' << itv->x << " " << itv->y << " " << itv->width << " " << itv->height;
		}
		std::cout << std::endl;
		if( printInImage ) drawRects( result, (*it).second.falsePositives, cv::Scalar(255,0,0) );

		//-----------------------------False_Negatives---------------------------------------------//
		numberFN += (*it).second.falseNegatives.size();
		std::cout << '\t' << (*it).second.falseNegatives.size() << " false negatives: ";
		for( std::vector< cv::Rect >::iterator itv = (*it).second.falseNegatives.begin();
				itv != (*it).second.falseNegatives.end(); itv++ )
		{
			std::cout << '\t' << itv->x << " " << itv->y << " " << itv->width << " " << itv->height;
		}
		std::cout << std::endl;
		if( printInImage ) drawRects( result, (*it).second.falseNegatives, cv::Scalar(0,0,255) );

		std::cout << std::endl;

		if( printInImage )
		{
			cv::namedWindow( "result", CV_WINDOW_NORMAL );
			cv::imshow( "result", result );

			cv::waitKey(0);
		}
	}

	std::cout << std::endl << "Overall results for complete image set, containing " << nSigns  << " traffic signs." << std::endl;
	std::cout << "\t Detection Rate " << "(found " << numberMatches << " traffic signs out of " << nSigns << ") = " <<
				(float) numberMatches / (float) nSigns << std::endl <<
			"\t False Positive Rate " << "(there were " << numberFP << " false positives out of " << numberDetections <<
			" potencial signs) = " <<
				(float) numberFP / (float) numberDetections << std::endl <<
			"\t False Negative Rate " << "(missed " << numberFN << " traffic signs out of " << nSigns << ") = " <<
				(float) numberFN / (float) nSigns << std::endl;
}

void drawRects( cv::Mat& image, std::vector< cv::Rect > rects, cv::Scalar color )
{
for( std::vector< cv::Rect >::iterator sit = rects.begin(); sit != rects.end(); sit ++ )
		{
			cv::Point2f rect_points[4];
			rect_points[0] = cv::Point2f( (*sit).x, (*sit).y );
			rect_points[1] = cv::Point2f( (*sit).x, (*sit).y + (*sit).width );
			rect_points[2] = cv::Point2f( (*sit).x + (*sit).height, (*sit).y + (*sit).width );
			rect_points[3] = cv::Point2f( (*sit).x + (*sit).height, (*sit).y );
			for( int j = 0; j < 4; j++ )
				line( image, rect_points[j], rect_points[(j+1)%4], color, 2, 8 );
		}
}
