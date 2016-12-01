//
//  ofxMTAppFramework.h
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 3/11/16.
//
//

#ifndef ofxMTAppFramework_h
#define ofxMTAppFramework_h



#define OFXMTAPP_USE_LOCAL_MOUSE 1

//This is sadly the only way that I can think of overriding "of..." globals:
//#ifdef OFXMTAPP_USE_LOCAL_MOUSE
//	#define ofGetMouseX mtGetLocalMouseX
//	#define ofGetMouseY mtGetLocalMouseY
//#endif

#include "ofMain.h"

#include "ofxMTAppMode.hpp"
#include "ofxMTApp.hpp"
#include "ofxMTModel.hpp"
#include "ofxMTView.hpp"
#include "ofxDatGui.h"

#endif /* ofxMTAppFramework_h */
