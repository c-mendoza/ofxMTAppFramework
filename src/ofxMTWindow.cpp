//
//  ofxMTAppFramework.h
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 3/11/16.
//
//

#ifndef ofxMTWindow_h
#define ofxMTWindow_h

#include "ofxMTWindow.hpp"
#include "ofxMTAppMode.hpp"
#include "ofxMTApp.hpp"
#include "ofxMTModel.hpp"
#include "ofxMTView.hpp"


ofxMTWindow::ofxMTWindow(string name)
{
    contentView = shared_ptr<ofxMTView>(new ofxMTView("Content View"));
    contentView->window(std::make_shared(this));
    this->name.set("Window Name", name);
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

void ofxMTWindow::windowResized(ofResizeEventArgs & resize)
{
    contentView->setFrameSize(resize.width, resize.height);
    for (auto view : contentView->subviews)
    {
        view->windowResized(ofResizeEventArgs & resize);
    }
}

void ofxMTWindow::modelDidLoad()
{
    contentView->modelDidLoad();
}

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


#endif /* ofxMTWindow_h */
