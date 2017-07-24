//
//  ofxMTView.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#include "ofxMTView.hpp"
#include "ofxMTApp.hpp"
#include "ofxMTModel.hpp"
#include "ofxMTWindow.hpp"

ofxMTView::ofxMTView(string _name)
{
    ofLogVerbose("View Construct: ") << _name;
//    window = nullptr;
    name.set("View Name", _name);
//    superview = nullptr;
//	contentPosition.set("Content Position", ofVec2f());
    contentScale.set("Content Scale", 1);
    backgroundColor.set("Background Color",
                        ofFloatColor(1.0, 1.0, 1.0, 1.0));
}

ofxMTView::~ofxMTView()
{
    // Not sure if I need to be that explicit here
    // TODO check ~MTView
    eventListeners.clear();
    removeAllEvents();
    subviews.clear();
    superview = nullptr;
    ofLogVerbose("View Destruct: ") << name;
}

//------------------------------------------------------//
// FRAME AND CONTENT                                    //
//------------------------------------------------------//

void ofxMTView::setFrame(ofRectangle newFrame)
{
    frame = newFrame;
    frameChangedInternal();
}

void ofxMTView::setFrameOrigin(glm::vec3 pos)
{
    frame.setPosition(pos);
    frameChangedInternal();
}

void ofxMTView::setFrameSize(glm::vec2 size)
{
    setFrameSize(size.x, size.y);
}

void ofxMTView::setFrameSize(float width, float height)
{
    setFrameSize(width, height);
    frameChanged();
}

const glm::vec3& ofxMTView::getFrameOrigin()
{
    return frame.getPosition();
}

glm::vec2 ofxMTView::getFrameSize()
{
    return glm::vec2(frame.getWidth(), frame.getHeight());
}

void ofxMTView::setContent(ofRectangle newContentRect)
{
    content = newContentRect;
    contentChangedInternal();
}

void ofxMTView::setContentOrigin(glm::vec3 pos)
{
    content.setPosition(pos);
    contentChangedInternal();
}

const glm::vec3& ofxMTView::getContentOrigin()
{
    return content.getPosition();
}

void ofxMTView::setContentSize(glm::vec2 size)
{
    setContentSize(size.x, size.y);
}

void ofxMTView::setContentSize(float width, float height)
{
    content.setSize(width, height);
    contentChangedInternal();
}

glm::vec2 ofxMTView::getContentSize()
{
    return glm::vec2(content.getWidth(), content.getHeight());
}

void ofxMTView::frameChangedInternal()
{
    frameChanged();

    for (auto sv : subviews)
    {
        sv->superviewFrameChanged();
    }
}

void ofxMTView::contentChangedInternal()
{
    contentChanged();

    for (auto sv : subviews)
    {
        sv->superviewContentChanged();
    }
}

//------------------------------------------------------//
// VIEW HEIRARCHY                                       //
//------------------------------------------------------//

std::weak_ptr<ofxMTView> getSuperview()
{
    return superview;
}

/// \brief Adds a subview.

void ofxMTView::addSubview(shared_ptr<ofxMTView> subview)
{
    subviews.push_back(subview);
    subview->thisView = subview;
    subview->superview = thisView;
}

vector<shared_ptr<ofxMTView>>& getSubviews()
{
    return subviews;
}

/// \returns True if successful.
bool removeFromSuperview()
{
    if (superview != nullptr)
    {
        auto sv = getSuperview()->getSubviews();
        auto iter = std::find(sv.begin(), sv.end(), thisView);
        if (iter != sv.end())
        {
            superview = nullptr;
            sv.erase(iter);
            return true;
        }
    }

    return false;
}

/// \returns True if there was a view to be removed.
bool removeLastSubview()
{

}

void removeAllSubviews();

shared_ptr<ofxMTWindow> ofxMTView::getWindow()
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

    if (ofxMTApp::sharedApp->autoUpdateAppModes) currentAppMode->update();
}

void ofxMTView::draw(ofEventArgs & args)
{
//	ofPushView();
//	ofViewport(viewport);

    ofPushView();
    ofViewport(frame);

    ofSetupScreen();
//    ofSetupScreenOrtho(ofGetViewportWidth(),
//                       ofGetViewportHeight());

    ofLoadIdentityMatrix();
    ofLoadMatrix(frameMatrix * contentMatrix);
    ofBackground(0);

    while (!drawOpQueue.empty())
    {
        auto op = drawOpQueue.front();
        op();
        drawOpQueue.pop();
    }

    //Call the user's draw() function
    draw();

    if (ofxMTApp::sharedApp->autoDrawAppModes) currentAppMode->draw();

    ofPopView();
}

void ofxMTView::exit(ofEventArgs &args)
{
    removeAllEvents();
    exit();
}

void ofxMTView::updateMatrices()
{
    frameMatrix = glm::translate(glm::mat4(), frame.getPosition());

    contentMatrix = glm::translate(frameMatrix, content.getPosition());
    if (contentScale > 1)
    {
        contentMatrix = glm::scale(contentMatrix, glm::vec3(contentScale, contentScale, 1));
    }
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

