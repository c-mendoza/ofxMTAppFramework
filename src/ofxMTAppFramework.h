//
//  ofxMTAppFramework.h
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 3/11/16.
//
//

#ifndef ofxMTAppFramework_h
#define ofxMTAppFramework_h


//This is sadly the only way that I can think of overriding "of..." globals.
//This will cause ofGetMouseX and ofGetMouseY to return "local" view coordinates.
//In addition to this define, you must enable Prefix headers and use the supplied .pch file.
//#define OFXMTAPP_USE_LOCAL_MOUSE 1


//#ifdef OFXMTAPP_USE_LOCAL_MOUSE
//	#define ofGetMouseX mtGetLocalMouseX
//	#define ofGetMouseY mtGetLocalMouseY
//#endif

//#include "ofMain.h"
#include "MTAppFrameworkUtils.hpp"
#include "MTViewMode.hpp"
#include "MTApp.hpp"
#include "MTModel.hpp"
#include "MTView.hpp"
#include "MTWindow.hpp"


#endif
