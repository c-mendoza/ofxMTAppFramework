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
#define OFXMTAPP_USE_LOCAL_MOUSE 1


//#ifdef OFXMTAPP_USE_LOCAL_MOUSE
//	#define ofGetMouseX mtGetLocalMouseX
//	#define ofGetMouseY mtGetLocalMouseY
//#endif

#include "ofMain.h"


class MTEventListenerStore
{
public:
//    MTEventListenerStore(){}
//    ~MTEventListenerStore()
//    {
//        eventListeners.clear(); //Check to see if this is necessary
//    }

    /// \brief Adds event listeners that should be destroyed when this
    /// object is destroyed.
    void addEventListener(ofEventListener &&e)
    {
        eventListeners.push_back(std::move(e));
    }

    void clearEventListeners()
    {
        eventListeners.clear();
    }

protected:
    vector<ofEventListener> eventListeners;
};

#include "ofxMTAppMode.hpp"
#include "ofxMTApp.hpp"
#include "ofxMTModel.hpp"
#include "ofxMTView.hpp"
#include "ofxMTWindow.hpp"
#include "MTProcedure.h"


#endif
