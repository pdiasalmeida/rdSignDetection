#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <string>
#include <algorithm>

static std::string
toLowerCase( std::string in )
{
	std::transform( in.begin(), in.end(), in.begin(), ::tolower );

	return in;
}

#endif
