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
#include "ofxMTAppMode.hpp"

std::shared_ptr<ofxMTView> ofxMTView::createView(string name)
{
	auto view = shared_ptr<ofxMTView>(new ofxMTView(name));
	view->thisView = view;
	return view;
}

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
	currentAppMode = std::shared_ptr<ofxMTAppMode>(new MTAppModeVoid);
}

ofxMTView::~ofxMTView()
{
    // Not sure if I need to be that explicit here
    // TODO check ~MTView
    eventListeners.clear();
    removeAllEvents();
    subviews.clear();
    ofLogVerbose("View Destruct: ") << name;
}

//------------------------------------------------------//
// FRAME AND CONTENT                                    //
//------------------------------------------------------//

void ofxMTView::setWindow(std::weak_ptr<ofxMTWindow> window)
{
    this->window = window;
}

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
    frame.setSize(width, height);
    frameChanged();
	frameChangedInternal();
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

void ofxMTView::setSize(float width, float height)
{
    setContentSize(width, height);
    setFrameSize(width, height);
}

void ofxMTView::setSize(glm::vec2 size)
{
    setSize(size.x, size.y);
}

void ofxMTView::frameChangedInternal()
{
	updateMatrices();
	
	if (auto super = superview.lock())
	{
		glm::vec4 screenFramePosition = super->frameMatrix * glm::vec4(frame.getPosition(), 1);
		screenFrame.setPosition(screenFramePosition.xyz());
		///TODO: Scale
		screenFrame.setSize(frame.width, frame.height);
	}
	else
	{
		screenFrame = frame;
	}

	ofLogVerbose() << name << " " << screenFrame;
	
	//Call User's frameChanged:
	frameChanged();

    for (auto sv : subviews)
    {
		sv->frameChangedInternal();
        sv->superviewFrameChanged();
    }
}

void ofxMTView::contentChangedInternal()
{
	updateMatrices();
	
	///TODO
	
    contentChanged();

    for (auto sv : subviews)
    {
        sv->superviewContentChanged();
    }
}

//------------------------------------------------------//
// VIEW HEIRARCHY                                       //
//------------------------------------------------------//

std::weak_ptr<ofxMTView> ofxMTView::getSuperview()
{
    return superview;
}

void ofxMTView::setSuperview(shared_ptr<ofxMTView> view)
{
	superview = view;
	frameChangedInternal();
}
/// \brief Adds a subview.

void ofxMTView::addSubview(shared_ptr<ofxMTView> subview)
{
    subview->thisView = subview;
	subview->setSuperview(thisView.lock());
    subview->window = window;
    subviews.push_back(subview);
}

vector<shared_ptr<ofxMTView>>& ofxMTView::getSubviews()
{
    return subviews;
}

/// \returns True if successful.
bool ofxMTView::removeFromSuperview()
{
    if (auto s = superview.lock())
    {
        auto sv = s->getSubviews();
        auto iter = std::find(sv.begin(), sv.end(), thisView.lock());
        if (iter != sv.end())
        {
//            superview = nullptr;
            sv.erase(iter);
            return true;
        }
    }

    return false;
}

/// \returns True if there was a view to be removed.
bool ofxMTView::removeLastSubview()
{
    if (subviews.size() > 0)
    {
        subviews.pop_back();
        return true;
    }
    else
    {
        return false;
    }
}

void ofxMTView::removeAllSubviews()
{
    subviews.clear();
}

std::weak_ptr<ofxMTWindow> ofxMTView::getWindow()
{
    return window;
}

//------------------------------------------------------//
// INTERNAL EVENT LISTENERS
//
// You do not need to call these methods
//------------------------------------------------------//

void ofxMTView::setup(ofEventArgs & args)
{
    setup();
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
	
	for (auto sv : subviews)
	{
		sv->update(args);
	}
}

void ofxMTView::draw(ofEventArgs & args)
{

	ofPushMatrix();
	ofSetMatrixMode(ofMatrixMode::OF_MATRIX_MODELVIEW);
	ofLoadMatrix(ofGetCurrentViewMatrix() * frameMatrix);
	
	ofFill();
	ofSetColor(backgroundColor.get());
	ofDrawRectangle(0, 0, frame.width, frame.height);

    while (!drawOpQueue.empty())
    {
        auto op = drawOpQueue.front();
        op();
        drawOpQueue.pop();
    }

    //Call the user's draw() function
    draw();

    if (ofxMTApp::sharedApp->autoDrawAppModes) currentAppMode->draw();

//    ofPopView();
	ofPopMatrix();
	for (auto sv : subviews)
	{
		sv->draw(args);
	}
	
//	ofPopView();
}

void ofxMTView::exit(ofEventArgs &args)
{
    removeAllEvents();
    exit();
}

void ofxMTView::windowResized(ofResizeEventArgs & resize)
{
    windowResized(resize.width, resize.height);
    for (auto view : subviews)
    {
        view->windowResized(resize);
    }
}

void ofxMTView::keyPressed(ofKeyEventArgs & key)
{
	keyPressed(key.key);
}

void ofxMTView::keyReleased(ofKeyEventArgs & key)
{
	keyReleased(key.key);
}

void ofxMTView::mouseMoved(ofMouseEventArgs & mouse)
{
	mouseMoved(mouse.x, mouse.y);
}

void ofxMTView::mouseDragged(ofMouseEventArgs & mouse)
{
	mouseDragged(mouse.x, mouse.y, mouse.button);
}

void ofxMTView::mousePressed(ofMouseEventArgs & mouse)
{
	if (subviews.size() > 0)
	{
		for (auto it = subviews.end()-1; it >= subviews.begin(); --it)
		{
			auto sv = it->get();
			if (sv->screenFrame.inside(mouse))
			{
				sv->mousePressed(mouse);
				return;
			}
		}
	}

    //If we haven't found another hit in the subviews,
    //this view should consume the event and become key:
	if (auto w = window.lock())
	{
		w->setFocusedView(thisView.lock());
	}
    mousePressed(mouse.x, mouse.y, mouse.button);
	onMousePressed(mouse.x, mouse.y, mouse.button);
}

void ofxMTView::mouseReleased(ofMouseEventArgs & mouse)
{

}

void ofxMTView::mouseScrolled( ofMouseEventArgs & mouse )
{

}

void ofxMTView::mouseEntered( ofMouseEventArgs & mouse )
{

}

void ofxMTView::mouseExited( ofMouseEventArgs & mouse )
{

}

void ofxMTView::dragged(ofDragInfo & drag)
{

}

void ofxMTView::messageReceived(ofMessage & message)
{

}

bool ofxMTView::hasFocus()
{
	return isFocused;
}

void ofxMTView::updateMatrices()
{
	if (auto sv = superview.lock())
	{
		frameMatrix = glm::translate(sv->frameMatrix, frame.getPosition());
	}
	else
	{
		frameMatrix = glm::translate(glm::mat4(), frame.getPosition());
	}
	
	invFrameMatrix = glm::inverse(frameMatrix);

    contentMatrix = glm::translate(frameMatrix, content.getPosition());
    if (contentScale > 1)
    {
        contentMatrix = glm::scale(contentMatrix, glm::vec3(contentScale, contentScale, 1));
    }
	
	invContentMatrix = glm::inverse(contentMatrix);
}


void ofxMTView::addAllEvents()
{
    if (auto w = window.lock())
    {
    w->events().enable();
    ofAddListener(w->events().setup, this, &ofxMTView::setup, OF_EVENT_ORDER_APP);
    ofAddListener(w->events().update, this, &ofxMTView::update,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().draw, this, &ofxMTView::draw,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().exit,this, &ofxMTView::exit,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().keyPressed,this, &ofxMTView::keyPressed,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().keyReleased,this, &ofxMTView::keyReleased,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseMoved,this, &ofxMTView::mouseMoved,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseDragged,this, &ofxMTView::mouseDragged,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mousePressed,this, &ofxMTView::mousePressed,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseReleased,this, &ofxMTView::mouseReleased,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseScrolled,this, &ofxMTView::mouseScrolled,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseEntered,this, &ofxMTView::mouseEntered,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseExited,this, &ofxMTView::mouseExited,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().windowResized,this, &ofxMTView::windowResized,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().messageEvent,this, &ofxMTView::messageReceived,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().fileDragEvent,this, &ofxMTView::dragged,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchCancelled,this, &ofxMTView::touchCancelled,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchDoubleTap,this, &ofxMTView::touchDoubleTap,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchDown,this, &ofxMTView::touchDown,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchMoved,this, &ofxMTView::touchMoved,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchUp,this, &ofxMTView::touchUp,OF_EVENT_ORDER_APP);
    ofAddListener(ofxMTApp::appChangeModeEvent, this, &ofxMTView::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);
    }

}
void ofxMTView::removeAllEvents()
{
    if (auto w = window.lock()) //Acquire the shared_ptr if it exists
    {
        ofRemoveListener(w->events().setup, this, &ofxMTView::setup, OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().update, this, &ofxMTView::update,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().draw, this, &ofxMTView::draw,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().exit,this, &ofxMTView::exit,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().keyPressed,this, &ofxMTView::keyPressed,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().keyReleased,this, &ofxMTView::keyReleased,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseMoved,this, &ofxMTView::mouseMoved,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseDragged,this, &ofxMTView::mouseDragged,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mousePressed,this, &ofxMTView::mousePressed,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseReleased,this, &ofxMTView::mouseReleased,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseScrolled,this, &ofxMTView::mouseScrolled,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseEntered,this, &ofxMTView::mouseEntered,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseExited,this, &ofxMTView::mouseExited,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().windowResized,this, &ofxMTView::windowResized,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().messageEvent,this, &ofxMTView::messageReceived,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().fileDragEvent,this, &ofxMTView::dragged,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchCancelled,this, &ofxMTView::touchCancelled,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchDoubleTap,this, &ofxMTView::touchDoubleTap,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchDown,this, &ofxMTView::touchDown,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchMoved,this, &ofxMTView::touchMoved,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchUp,this, &ofxMTView::touchUp,OF_EVENT_ORDER_APP);
        ofRemoveListener(ofxMTApp::appChangeModeEvent, this, &ofxMTView::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);
    }
}

