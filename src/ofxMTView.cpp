//
//  ofxMTView.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#include "ofxMTView.hpp"

//int	ofGetMouseY();

ofxMTView::ofxMTView(string _name)
{
    ofLogVerbose("View Construct: ") << _name;
    window = nullptr;
    name = _name;
//	contentPosition.set("Content Position", ofVec2f());
    contentScale.set("Content Scale", 1);
    scrollbarColor.set("Scrollbar Color", ofColor(110));
    bScrollbarsVisible = true;
}

ofxMTView::~ofxMTView()
{
    for (auto& el : eventListeners)
    {
        el.unsubscribe();
    }

    eventListeners.clear();
    removeAllEvents();
    ofLogVerbose("View Destruct: ") << getName();
}

void ofxMTView::setWindow(shared_ptr<ofAppBaseWindow> window)
{
    if (!this->window)
    {
        this->window = window;
        window->setWindowTitle(name);

        //Preliminarily set the Content Frame to the size of the window:
        setContentFrame(ofRectangle(0, 0, window->getWidth(), window->getHeight()));
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

//void ofxMTView::setModel(ofPtr<ofxMTModel> model)
//{
//	this->model = model;
//}

void ofxMTView::setName(string newName)
{
    name = newName;
    if(window)
    {
        window->setWindowTitle(name);
    }
}

shared_ptr<ofAppBaseWindow> ofxMTView::getWindow()
{
    return window;
}

void ofxMTView::update(ofEventArgs & args)
{
    while (!updateOpQueue.empty())
    {
        auto op = updateOpQueue.front();
        op();
        updateOpQueue.pop();
    }
    //I should do something here to update the size of the contentFrame and the scroll bars when necessary
    update(); //Call user's update()
}

void ofxMTView::draw(ofEventArgs & args)
{
//	ofPushView();
//	ofViewport(viewport);

    while (!drawOpQueue.empty())
    {
        auto op = drawOpQueue.front();
        op();
        drawOpQueue.pop();
    }

    ofPushView();
    ofSetupScreenOrtho(ofGetViewportWidth(), ofGetViewportHeight());
    ofLoadMatrix(transMatrix);

    //Call the user's draw() function
    draw();

    ofPopView();

    //Draw in screen coordinates:

    if (bScrollbarsVisible)
    {
        int hDiff = contentFrame.width - ofGetWidth();
        int vDiff = contentFrame.height - ofGetHeight();

        //If there is more content than window, draw the scrollbars:
        if (hDiff > 0)
        {
            float barWidth = ofGetWidth() * ((float)ofGetWidth() / contentFrame.width);
            ofFill();
            ofSetColor(scrollbarColor);
            ofDrawRectangle(-contentPosition.x, ofGetHeight() - 10, barWidth, 10);
        }

        if (vDiff > 0)
        {
            float barHeight = ofGetHeight() * ((float)ofGetHeight() / contentFrame.height);
            ofFill();
            ofSetColor(scrollbarColor);
            ofDrawRectangle(ofGetWidth() - 10, -contentPosition.y, 10, barHeight);
        }

    }
}

void ofxMTView::exit(ofEventArgs &args)
{
    removeAllEvents();
    exit();
    ofxMTApp::sharedApp->viewClosing(this);

}

void ofxMTView::updateMatrix()
{
    transMatrix.makeIdentityMatrix();
    transMatrix.scale(contentScale, contentScale, 1);
//	transMatrix.translate(ofGetViewportWidth()/2, ofGetViewportHeight()/2, 0);

//	transMatrix.translate(-ofGetViewportWidth()/2, -ofGetViewportHeight()/2, 0);
    transMatrix.translate(contentPosition.x, contentPosition.y, 0);
    invTransMatrix = transMatrix.getInverse();
}

void ofxMTView::scrollBy(float dx, float dy)
{
    contentPosition.x += dx;
    contentPosition.y += dy;
    updateMatrix();
}

void ofxMTView::scrollTo(float x, float y)
{
    contentPosition.x = x;
    contentPosition.y = y;
    updateMatrix();
}

void ofxMTView::zoomTo(float zoom)
{
    contentScale = zoom;
    updateMatrix();
}

void ofxMTView::zoomBy(float zoomChange)
{
    contentScale += zoomChange;
    updateMatrix();
}

void ofxMTView::addAllEvents()
{
    window->events().enable();
    ofAddListener(window->events().setup, this, &ofxMTView::setup, OF_EVENT_ORDER_APP);
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
    ofAddListener(ofxMTApp::appChangeModeEvent, this, &ofxMTView::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);

}
void ofxMTView::removeAllEvents()
{
    ofRemoveListener(window->events().setup, this, &ofxMTView::setup, OF_EVENT_ORDER_APP);
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
    ofRemoveListener(ofxMTApp::appChangeModeEvent, this, &ofxMTView::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);
}

