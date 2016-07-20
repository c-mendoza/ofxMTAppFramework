//
//  ofxMTView.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#include "ofxMTView.hpp"

ofxMTView::ofxMTView()
{
	window = nullptr;
	model = ofxMTApp::sharedApp->getModel();
}

ofxMTView::~ofxMTView()
{
	
}

void ofxMTView::setWindow(shared_ptr<ofAppBaseWindow> window)
{
	if (!this->window)
	{
		this->window = window;
		window->setWindowTitle(name);
		addAllEvents();
	}
	else
	{
		//TODO: this currently crashes
		ofEventArgs args;
		exit(args);
		this->window->close();
		removeAllEvents();
		this->window = window;
		addAllEvents();
	}
}

void ofxMTView::setModel(ofPtr<ofxMTModel> model)
{
	this->model = model;
}

void ofxMTView::setName(string newName)
{
	name = newName;
	if(window)
	{
		window->setWindowTitle(name);
	}
}

ofPtr<ofAppBaseWindow> ofxMTView::getWindow()
{
	return window;
}

void ofxMTView::draw()
{
	ofBackground(255);
	
	ofSetColor(255, 0, 0);
	ofDrawRectangle(100, 100, 400, 200);
}

void ofxMTView::addAllEvents()
{
	window->events().enable();
//	window->events().setup.disable();
//	ofAddListener(window->events().setup, this, &ofxMTView::setup, OF_EVENT_ORDER_APP);
	ofAddListener(window->events().update, this, &ofxMTView::update,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().draw, this, &ofxMTView::draw,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().exit,this, &ofxMTView::exit,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().keyPressed,this, &ofxMTView::keyPressed,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().keyReleased,this, &ofxMTView::keyReleased,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().mouseMoved,this, &ofxMTView::mouseMoved,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().mouseDragged,this, &ofxMTView::mouseDragged,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().mousePressed,this, &ofxMTView::mousePressed,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().mouseReleased,this, &ofxMTView::mouseReleased,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().mouseScrolled,this, &ofxMTView::mouseScrolled,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().mouseEntered,this, &ofxMTView::mouseEntered,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().mouseExited,this, &ofxMTView::mouseExited,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().windowResized,this, &ofxMTView::windowResized,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().messageEvent,this, &ofxMTView::messageReceived,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().fileDragEvent,this, &ofxMTView::dragged,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().touchCancelled,this, &ofxMTView::touchCancelled,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().touchDoubleTap,this, &ofxMTView::touchDoubleTap,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().touchDown,this, &ofxMTView::touchDown,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().touchMoved,this, &ofxMTView::touchMoved,OF_EVENT_ORDER_APP);
	ofAddListener(window->events().touchUp,this, &ofxMTView::touchUp,OF_EVENT_ORDER_APP);
	ofAddListener(ofxMTApp::appChangeModeEvent, this, &ofxMTView::appModeChanged,OF_EVENT_ORDER_APP);

}
void ofxMTView::removeAllEvents()
{
//	ofRemoveListener(window->events().setup, this, &ofxMTView::setup, OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().update, this, &ofxMTView::update,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().draw, this, &ofxMTView::draw,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().exit,this, &ofxMTView::exit,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().keyPressed,this, &ofxMTView::keyPressed,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().keyReleased,this, &ofxMTView::keyReleased,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().mouseMoved,this, &ofxMTView::mouseMoved,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().mouseDragged,this, &ofxMTView::mouseDragged,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().mousePressed,this, &ofxMTView::mousePressed,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().mouseReleased,this, &ofxMTView::mouseReleased,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().mouseScrolled,this, &ofxMTView::mouseScrolled,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().mouseEntered,this, &ofxMTView::mouseEntered,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().mouseExited,this, &ofxMTView::mouseExited,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().windowResized,this, &ofxMTView::windowResized,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().messageEvent,this, &ofxMTView::messageReceived,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().fileDragEvent,this, &ofxMTView::dragged,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().touchCancelled,this, &ofxMTView::touchCancelled,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().touchDoubleTap,this, &ofxMTView::touchDoubleTap,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().touchDown,this, &ofxMTView::touchDown,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().touchMoved,this, &ofxMTView::touchMoved,OF_EVENT_ORDER_APP);
	ofRemoveListener(window->events().touchUp,this, &ofxMTView::touchUp,OF_EVENT_ORDER_APP);
	ofRemoveListener(ofxMTApp::appChangeModeEvent, this, &ofxMTView::appModeChanged,OF_EVENT_ORDER_APP);
}