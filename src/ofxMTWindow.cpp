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

#include "ofxMTAppMode.hpp"
#include "ofxMTApp.hpp"
#include "ofxMTModel.hpp"
#include "ofxMTView.hpp"
//#include "ofxDatGui.h"
#include "MTProcedure.h"

#endif /* ofxMTAppFramework_h */

ofxMTWindow::ofxMTWindow()
{
    contentView = shared_ptr<ofxMTView>(new ofxMTView("Content View"));
    contentView->window(std::make_shared(this));
}

void ofxMTWindow::setup(ofEventArgs & args)
{
    contentView->setup(args);
}

void ofxMTWindow::update(ofEventArgs & args)
{
    contentView->update(args);
}

void ofxMTWindow::draw(ofEventArgs & args)
{
    contentView->draw(args);
}

void ofxMTWindow::exit(ofEventArgs & args)
{
    contentView = nullptr;
}

void ofxMTWindow::windowResized(ofResizeEventArgs & resize);
void ofxMTWindow::keyPressed( ofKeyEventArgs & key );
void ofxMTWindow::keyReleased( ofKeyEventArgs & key );
void ofxMTWindow::mouseMoved( ofMouseEventArgs & mouse );
void ofxMTWindow::mouseDragged( ofMouseEventArgs & mouse );
void ofxMTWindow::mousePressed( ofMouseEventArgs & mouse );
void ofxMTWindow::mouseReleased(ofMouseEventArgs & mouse);
void ofxMTWindow::mouseScrolled( ofMouseEventArgs & mouse );
void ofxMTWindow::mouseEntered( ofMouseEventArgs & mouse );
void ofxMTWindow::mouseExited( ofMouseEventArgs & mouse );
void ofxMTWindow::dragged(ofDragInfo & drag);
void ofxMTWindow::messageReceived(ofMessage & message);

//TODO: Touch
void ofxMTWindow::touchDown(ofTouchEventArgs & touch);
void ofxMTWindow::touchMoved(ofTouchEventArgs & touch);
void ofxMTWindow::touchUp(ofTouchEventArgs & touch);
void ofxMTWindow::touchDoubleTap(ofTouchEventArgs & touch);
void ofxMTWindow::touchCancelled(ofTouchEventArgs & touch);
