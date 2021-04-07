//
//  ofxMTAppFramework.h
//
//  Created by Cristobal Mendoza on 3/11/16.
//
//

#ifndef ofxMTAppFramework_h
#define ofxMTAppFramework_h

#define MTAF_VERSION_MAJOR 0
#define MTAF_VERSION_MINOR 5
#define MTAF_VERSION_PATCH 0

//#define ImTextureID unsigned int

#include "MTAppFrameworkUtils.hpp"
#include "MTViewMode.hpp"
#include "MTApp.hpp"
#include "MTModel.hpp"
#include "MTView.hpp"
#include "MTWindow.hpp"

inline std::ostream& operator<<(std::ostream& os, const ofPath& path)
{
	os << MTAppFramework::PathToString2(path);
	return os;
}

inline std::istream& operator>>(std::istream& is, ofPath& path)
{
	std::string line;
	std::string result;
	while (std::getline(is, line))
	{
		if (!line.empty())
			result += line;
	}

	path = MTAppFramework::PathFromString2(result);
	return is;
}

#endif
