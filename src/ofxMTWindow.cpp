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
	contentView = ofxMTView::createView("Content View");
    this->name.set("Window Name", name);
}

ofxMTWindow::~ofxMTWindow()
{

}

//void ofxMTWindow::setup(ofEventArgs & args)
//{
//    contentView->setup(args);
//}

#ifndef TARGET_OPENGLES
void ofxMTWindow::setup(const ofGLFWWindowSettings & settings)
{
    ofAppGLFWWindow::setup(settings);
}
#else
void ofxMTWindow::setup(const ofGLESWindowSettings & settings)
{
    ofAppGLESWindow::setup(settings);
}
#endif

//void ofxMTWindow::update()
//{
//	ofAppGLFWWindow::update();
//}
//
//void ofxMTWindow::draw()
//{
//	ofAppGLFWWindow::draw();
//}

void ofxMTWindow::setupInternal(ofEventArgs & args)
{
	contentView->setSize(this->getWidth(), this->getHeight());
	contentView->setup(args);
}

void ofxMTWindow::update(ofEventArgs & args)
{
	contentView->update(args);
}

void ofxMTWindow::draw(ofEventArgs & args)
{
	ofBackground(0);
	contentView->draw(args);
}

void ofxMTWindow::exit(ofEventArgs & args)
{
	contentView->exit(args);
	contentView = nullptr;
}

void ofxMTWindow::windowResized(ofResizeEventArgs & resize)
{
    contentView->setFrameSize(resize.width, resize.height);
    contentView->windowResized(resize);
}

void ofxMTWindow::modelDidLoad()
{
    contentView->modelDidLoad();
}

void ofxMTWindow::keyPressed( ofKeyEventArgs & key )
{
    auto fv = focusedView.lock();
    if (fv)
    {
        fv->keyPressed(key);
    }
}

void ofxMTWindow::keyReleased( ofKeyEventArgs & key )
{
    auto fv = focusedView.lock();
    if (fv)
    {
        fv->keyReleased(key);
    }
    this->keyReleased(key.key);
}

void ofxMTWindow::mouseMoved( ofMouseEventArgs & mouse )
{

}

void ofxMTWindow::mouseDragged( ofMouseEventArgs & mouse ){}
void ofxMTWindow::mousePressed( ofMouseEventArgs & mouse )
{
    contentView->mousePressed(mouse);
}

void ofxMTWindow::mouseReleased(ofMouseEventArgs & mouse){}
void ofxMTWindow::mouseScrolled( ofMouseEventArgs & mouse ){}
void ofxMTWindow::mouseEntered( ofMouseEventArgs & mouse ){}
void ofxMTWindow::mouseExited( ofMouseEventArgs & mouse ){}
void ofxMTWindow::dragged(ofDragInfo & drag){}
void ofxMTWindow::messageReceived(ofMessage & message){}

//TODO: Touch
void ofxMTWindow::touchDown(ofTouchEventArgs & touch){}
void ofxMTWindow::touchMoved(ofTouchEventArgs & touch){}
void ofxMTWindow::touchUp(ofTouchEventArgs & touch){}
void ofxMTWindow::touchDoubleTap(ofTouchEventArgs & touch){}
void ofxMTWindow::touchCancelled(ofTouchEventArgs & touch){}

void ofxMTWindow::removeAllEvents()
{

}

void ofxMTWindow::addAllEvents()
{

}

#endif /* ofxMTWindow_h */
