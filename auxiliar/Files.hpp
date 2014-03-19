#ifndef FILES_HPP_
#define FILES_HPP_

#include <vector>
#include <string>
#include <fstream>

#include <opencv2/core/core.hpp>

class Files {
public:
	Files();
	Files( std::vector< std::string > validExtensions );
	~Files();

	std::vector< std::string > getValidExtensions();
	void setValidExtensions( std::vector< std::string > validExtensions );
	void addValidExtendion( std::string extension );

	std::vector< std::string > streamSplit( std::istringstream& instream, char delimiter );
	int getFilesInDirectory( const std::string& dirName, std::vector< std::string >& fileNames );

	void parseGroundTruth( std::string file, std::vector< std::pair< std::string, cv::Rect > >& signs );

	void saveDescriptorVectorToFile( int label, std::vector< float >& descriptorVector, std::ofstream& File );
	void saveSignBlobToFile( std::string image, cv::Rect boundingRect, int signType, float time, std::ofstream& File );

protected:
	std::vector< std::string > _validExtensions;

};

#endif
