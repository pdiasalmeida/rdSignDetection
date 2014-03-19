#include "Files.hpp"
#include "Common.hpp"
#include "Log.h"

#include <dirent.h>

Files::Files()
{
	std::vector< std::string > _validExtensions();
}

Files::Files( std::vector< std::string > validExtensions )
{
	_validExtensions = validExtensions;
}

std::vector< std::string >
Files::getValidExtensions()
{
	std::vector< std::string > copy_validExtensions;
	for( std::vector< std::string >::iterator it = _validExtensions.begin(); it != _validExtensions.end(); it++ )
	{
		copy_validExtensions.push_back( std::string(*it) );
	}

	return copy_validExtensions;
}

void
Files::setValidExtensions( std::vector< std::string > validExtensions )
{
	for( std::vector< std::string >::iterator it = validExtensions.begin(); it != validExtensions.end(); it++ )
	{
		_validExtensions.push_back( std::string(*it) );
	}
}

void
Files::addValidExtendion( std::string ext )
{
	_validExtensions.push_back(ext);
}

int
Files::getFilesInDirectory( const std::string& dirName, std::vector< std::string >& fileNames )
{
    Log::notice( "Opening directory '" + dirName + "'..." );

    int count = 0;
    struct dirent* ep;
    size_t extensionLocation;

    DIR* dp = opendir( dirName.c_str() );
    if( dp != NULL )
    {
    	while( (ep = readdir(dp)) )
    	{
            if( ep->d_type & DT_DIR )
            {
                continue;
            }

            if( _validExtensions.size() > 0 )
            {
            	extensionLocation = std::string( ep->d_name ).find_last_of( "." );
            	std::string tempExt = toLowerCase( std::string( ep->d_name ).substr( extensionLocation + 1 ) );

            	if( find( _validExtensions.begin(), _validExtensions.end(), tempExt ) != _validExtensions.end() )
            	{
            		Log::notice( "Found matching data file '" + std::string( ep->d_name ) + "'." );
            		fileNames.push_back( (std::string) dirName + ep->d_name );
            		count++;
            	}
            	else
            	{
            		Log::notice( "Found file does not match required file type, skipping: '" +
            				std::string( ep->d_name ) + "'!" );
            	}
            }
            else
            {
            	Log::warning( "Accepting all files in directory '" + dirName + "'." );
            	Log::notice( "Found matching data file '" + std::string( ep->d_name ) + "'." );
            	fileNames.push_back( (std::string) dirName + ep->d_name );
            	count++;
            }
        }
        closedir( dp );
    }
    else
    {
    	Log::warning( "Error opening directory '" + dirName + "'!" );
    }

    Log::notice( "Found " + Log::to_string( count ) + " files in directory: '" +
    		Log::to_string( dirName ) + "'." );
    return count;
}

void
Files::parseGroundTruth( std::string file, std::vector< std::pair< std::string, cv::Rect > >& signs )
{
	std::ifstream infile;
	std::vector< std::string > tokens;

	infile.open( file.c_str() );
	if( infile.is_open() )
	{
		while( infile )
		{
			std::string line;
			getline( infile, line );
			std::istringstream iss( line );

			tokens = streamSplit( iss, ';' );

			if( tokens.size() >= 4 )
			{
				std::string image( tokens.at(0) );
				int x = atoi( tokens.at(1).c_str() );
				int y = atoi( tokens.at(2).c_str() );
				int w = atoi( tokens.at(3).c_str() ) - x;
				int h = atoi( tokens.at(4).c_str() ) - y;
				cv::Rect imR = cv::Rect( x, y, w, h );

				signs.push_back( std::make_pair( image, imR ) );
			}
		}
	}

	infile.close();
}

void
Files::saveDescriptorVectorToFile( int label, std::vector< float >& descriptorVector, std::ofstream& File )
{
    Log::notice( "Saving descriptor vector to file." );
    std::string separator = " ";

    if( File.good() && File.is_open() )
    {
        File << label << separator;
        for( int feature = 0; feature < (int) descriptorVector.size(); ++feature )
        {
            File << descriptorVector.at( feature ) << separator;
        }

        File << std::endl;
        File.flush();
    }
}

void
Files::saveSignBlobToFile( std::string image, cv::Rect boundingRect, int signType, float time, std::ofstream& File )
{
    Log::notice( "Saving image blob to file." );
    std::string separator = ";";

    if( File.good() && File.is_open() )
    {
    	File << image << ";" << boundingRect.x << ";" << boundingRect.y << ";" << boundingRect.x + boundingRect.width << ";"
    			<< boundingRect.y + boundingRect.height << ";" << signType << ";" << time;

        File << std::endl;
        File.flush();
    }
}

std::vector< std::string >
Files::streamSplit( std::istringstream& instream, char delimiter )
{
	std::string token;
	std::vector< std::string > result;

	while( getline(instream, token, delimiter) )
	{
		result.push_back( token );
	}

	return result;
}

Files::~Files(){ }
