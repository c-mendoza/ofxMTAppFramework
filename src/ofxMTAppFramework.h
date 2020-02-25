//
//  ofxMTAppFramework.h
//
//  Created by Cristobal Mendoza on 3/11/16.
//
//

#ifndef ofxMTAppFramework_h
#define ofxMTAppFramework_h

#define MTAF_VERSION_MAJOR 0
#define MTAF_VERSION_MINOR 2
#define MTAF_VERSION_PATCH 0


#include "MTAppFrameworkUtils.hpp"
#include "MTViewMode.hpp"
#include "MTApp.hpp"
#include "MTModel.hpp"
#include "MTView.hpp"
#include "MTWindow.hpp"

inline std::ostream& operator<<(std::ostream& os, const ofPath& path)
{
	os << MTApp::pathToString(path);
	return os;
}

inline std::istream& operator>>(std::istream& is, ofPath& path)
{
	std::string inString;
	std::getline(is, inString);
	path = MTApp::pathFromString(inString);
	return is;
}

#endif
