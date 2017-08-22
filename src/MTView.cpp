//
//  ofxMTView.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#include "MTView.hpp"
#include "MTApp.hpp"
#include "MTModel.hpp"
#include "MTWindow.hpp"
#include "MTAppMode.hpp"

//std::shared_ptr<ofxMTView> ofxMTView::createView(string name)
//{
//    auto view = shared_ptr<ofxMTView>(new ofxMTView(name));
//    view->thisView = view;
//    return view;
//}

MTView::MTView(string _name)
{
    ofLogVerbose("View Construct: ") << _name;
//    window = nullptr;
    name.set("View Name", _name);
//    superview = nullptr;
//	contentPosition.set("Content Position", ofVec2f());
    contentScale.set("Content Scale", 1);
    backgroundColor.set("Background Color",
                        ofFloatColor(1.0, 1.0, 1.0, 1.0));
    currentAppMode = std::shared_ptr<MTAppMode>(new MTAppModeVoid);
}

MTView::~MTView()
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

void MTView::setWindow(std::weak_ptr<MTWindow> window)
{
    this->window = window;
}

void MTView::setFrame(ofRectangle newFrame)
{
    frame = newFrame;
    frameChangedInternal();
}

void MTView::setFrameOrigin(float x, float y)
{
    frame.setPosition(x, y);
    frameChangedInternal();
}

void MTView::setFrameOrigin(glm::vec3 pos)
{
    frame.setPosition(pos);
    frameChangedInternal();
}

void MTView::setFrameFromCenter(glm::vec3 pos, glm::vec2 size)
{
    frame.setFromCenter(pos.xy(), size.x, size.y);
}

void MTView::setFrameCenter(glm::vec3 pos)
{
    frame.setFromCenter(pos, frame.width, frame.height);
    frameChangedInternal();
}

void MTView::setFrameSize(glm::vec2 size)
{
    setFrameSize(size.x, size.y);
}

void MTView::setFrameSize(float width, float height)
{
    frame.setSize(width, height);
    frameChanged();
    frameChangedInternal();
}

const glm::vec3& MTView::getFrameOrigin()
{
    return frame.getPosition();
}

glm::vec2 MTView::getFrameSize()
{
    return glm::vec2(frame.getWidth(), frame.getHeight());
}

glm::vec2 MTView::getFrameCenter()
{
    return frame.getCenter().xy();
}

void MTView::setContent(ofRectangle newContentRect)
{
    content = newContentRect;
    contentChangedInternal();
}

void MTView::setContentOrigin(glm::vec3 pos)
{
    content.setPosition(pos);
    contentChangedInternal();
}

const glm::vec3& MTView::getContentOrigin()
{
    return content.getPosition();
}

void MTView::setContentSize(glm::vec2 size)
{
    setContentSize(size.x, size.y);
}

void MTView::setContentSize(float width, float height)
{
    content.setSize(width, height);
    contentChangedInternal();
}

glm::vec2 MTView::getContentSize()
{
    return glm::vec2(content.getWidth(), content.getHeight());
}

void MTView::setSize(float width, float height)
{
    setContentSize(width, height);
    setFrameSize(width, height);
}

void MTView::setSize(glm::vec2 size)
{
    setSize(size.x, size.y);
}

void MTView::frameChangedInternal()
{
    updateMatrices();

    if (auto super = superview.lock())
    {
        glm::vec4 screenFramePosition = super->frameMatrix * glm::vec4(frame.getPosition(), 1);
        screenFrame.setPosition(screenFramePosition.xyz());
        /// TODO: Scale
        screenFrame.setSize(frame.width, frame.height);
    }
    else
    {
        screenFrame = frame;
    }

//    ofLogVerbose() << name << " " << screenFrame;

    // Call User's frameChanged:
    frameChanged();

    // Notify listeners:
    auto args =  ofEventArgs();
    frameChangedEvent.notify(this, args);

    // Notify the rest of the hierarchy:
    for (auto sv : subviews)
    {
        sv->frameChangedInternal();
        sv->superviewFrameChanged();
    }
}

void MTView::contentChangedInternal()
{
    updateMatrices();

    //TODO: content changed internal?

    contentChanged();

    for (auto sv : subviews)
    {
        sv->superviewContentChanged();
    }
}

//TODO: Check to see if transformPoint works
glm::vec2 MTView::transformPoint(glm::vec2& coords,
                                    const MTView* toView)
{
    auto windowCoords = invFrameMatrix * glm::vec4(coords.x, coords.y, 1, 1);
    return (toView->frameMatrix * windowCoords).xy();
}
//------------------------------------------------------//
// VIEW HEIRARCHY                                       //
//------------------------------------------------------//

std::weak_ptr<MTView> MTView::getSuperview()
{
    return superview;
}

void MTView::setSuperview(shared_ptr<MTView> view)
{
    superview = view;
    frameChangedInternal();
}
/// \brief Adds a subview.

void MTView::addSubview(shared_ptr<MTView> subview)
{
    subview->thisView = subview;
    subview->setSuperview(shared_from_this());
    subview->window = window;
    subviews.push_back(subview);
}

vector<shared_ptr<MTView>>& MTView::getSubviews()
{
    return subviews;
}

/// \returns True if successful.
bool MTView::removeFromSuperview()
{
    if (auto s = superview.lock())
    {
        auto sv = s->getSubviews();
        auto iter = std::find(sv.begin(), sv.end(), shared_from_this());
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
bool MTView::removeLastSubview()
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

void MTView::removeAllSubviews()
{
    subviews.clear();
}

std::weak_ptr<MTWindow> MTView::getWindow()
{
    return window;
}

//------------------------------------------------------//
// INTERNAL EVENT LISTENERS
//
// You do not need to call these methods
//------------------------------------------------------//

void MTView::setup(ofEventArgs & args)
{
    setup();
    for (auto sv : subviews)
    {
        sv->setup(args);
    }
}

void MTView::update(ofEventArgs & args)
{
    while (!updateOpQueue.empty())
    {
        auto op = updateOpQueue.front();
        op();
        updateOpQueue.pop();
    }
    //I should do something here to update the size of the contentFrame and the scroll bars when necessary

    update(); //Call user's update()
    onUpdate();

    if (MTApp::sharedApp->autoUpdateAppModes) currentAppMode->update();

    for (auto sv : subviews)
    {
        sv->update(args);
    }
}

void MTView::draw(ofEventArgs & args)
{

//	glEnable(GL_SCISSOR_TEST);
//	glScissor(screenFrame.x,
//			  screenFrame.y + screenFrame.height,
//			  screenFrame.width,
//			  screenFrame.height);

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
    onDraw();
//    glDisable(GL_SCISSOR_TEST);
    if (MTApp::sharedApp->autoDrawAppModes) currentAppMode->draw();
    for (auto sv : subviews)
    {
        sv->draw(args);
    }
//    ofPopView();



//	ofPopView();
}

void MTView::exit(ofEventArgs &args)
{
    removeAllEvents();
    exit();
    onExit();
}

void MTView::windowResized(ofResizeEventArgs & resize)
{
    windowResized(resize.width, resize.height);
    for (auto view : subviews)
    {
        view->windowResized(resize);
    }
    onWindowResized(resize.width, resize.height);
}

void MTView::keyPressed(ofKeyEventArgs & key)
{
    ofLogVerbose(name, "Pressed: %c", (char)key.key);
    keyPressed(key.key);
    onKeyPressed(key.key);
}

void MTView::keyReleased(ofKeyEventArgs & key)
{
    ofLogVerbose(name, "Released: %c", (char)key.key);
    keyReleased(key.key);
    onKeyReleased(key.key);
}

void MTView::mouseMoved(ofMouseEventArgs & mouse)
{
    localMouse = (invFrameMatrix * glm::vec4(mouse.x, mouse.y, 1, 1)).xy();
    ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Moved,
                                                  localMouse.x,
                                                  localMouse.y,
                                                  mouse.button);
    mouseMovedEvent.notify(localArgs);
    mouseMoved(mouse.x, mouse.y);
    onMouseMoved(mouse.x, mouse.y);
}

void MTView::mouseDragged(ofMouseEventArgs & mouse)
{
    localMouse = (invFrameMatrix * glm::vec4(mouse.x, mouse.y, 1, 1)).xy();
    if (!isDragging)
    {
        isDragging = true;
        localMouseDragStart = localMouseDown;
    }

    ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Dragged,
                                                  localMouse.x,
                                                  localMouse.y,
                                                  mouse.button);
    mouseDraggedEvent.notify(localArgs);
    mouseDragged(localMouse.x, localMouse.y, mouse.button);
    onMouseDragged(mouse.x, mouse.y, mouse.button);
}

void MTView::mousePressed(ofMouseEventArgs & mouse)
{
    localMouseDown =  (invFrameMatrix * glm::vec4(mouse.x, mouse.y, 1, 1)).xy();
    localMouse = localMouseDown;

    ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Pressed,
                                                  localMouse.x,
                                                  localMouse.y,
                                                  mouse.button);
    mousePressedEvent.notify(localArgs);
    mousePressed(localMouse.x, localMouse.y, mouse.button);
    onMousePressed(localMouse.x, localMouse.y, mouse.button);
}

std::shared_ptr<MTView> MTView::hitTest(glm::vec2 &windowCoord)
{
    if (subviews.size() > 0)
    {
        for (auto it = subviews.end()-1; it >= subviews.begin(); --it)
        {
            auto sv = it->get();
            if (sv->screenFrame.inside(windowCoord))
            {
                return sv->hitTest(windowCoord);
            }
        }
    }

    return shared_from_this();
}

void MTView::mouseReleased(ofMouseEventArgs & mouse)
{
    localMouseUp = (invFrameMatrix * glm::vec4(mouse.x, mouse.y, 1, 1)).xy();
    localMouse = localMouseUp;
    ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Released,
                                                  localMouse.x,
                                                  localMouse.y,
                                                  mouse.button);
    mouseReleasedEvent.notify(localArgs);
    mouseReleased(mouse.x, mouse.y, mouse.button);
    onMouseReleased(mouse.x, mouse.y, mouse.button);
}

void MTView::mouseScrolled( ofMouseEventArgs & mouse )
{
    ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Scrolled,
                                                  localMouse.x,
                                                  localMouse.y,
                                                  mouse.button);
//    ofLogNotice("MTView::mouseScrolled") << "scrollX and scrollY are in Window coordinates"
    mouseScrolledEvent.notify(localArgs);
    mouseScrolled(localMouse.x, localMouse.y, mouse.scrollX, mouse.scrollY);
    onMouseScrolled(localMouse.x, localMouse.y, mouse.scrollX, mouse.scrollY);
}

void MTView::mouseEntered( ofMouseEventArgs & mouse )
{
    ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Entered,
                                                  localMouse.x,
                                                  localMouse.y,
                                                  mouse.button);
    mouseEnteredEvent.notify(localArgs);
    mouseEntered(localMouse.x, localMouse.y);
    onMouseEntered(localMouse.x, localMouse.y);
}

void MTView::mouseExited( ofMouseEventArgs & mouse )
{
    ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Exited,
                                                  localMouse.x,
                                                  localMouse.y,
                                                  mouse.button);
    mouseExitedEvent.notify(localArgs);
    mouseExited(localMouse.x, localMouse.y);
    onMouseExited(localMouse.x, localMouse.y);
}

void MTView::dragged(ofDragInfo & drag)
{
    ofLogNotice() << "ofxMTView::dragged not yet implemented";
}

void MTView::messageReceived(ofMessage & message)
{
    ofLogNotice() << "ofxMTView::messageReceived not yet implemented";
}

bool MTView::hasFocus()
{
    return isFocused;
}

void MTView::updateMatrices()
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


void MTView::addAllEvents()
{
    if (auto w = window.lock())
    {
    w->events().enable();
    ofAddListener(w->events().setup, this, &MTView::setup, OF_EVENT_ORDER_APP);
    ofAddListener(w->events().update, this, &MTView::update,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().draw, this, &MTView::draw,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().exit,this, &MTView::exit,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().keyPressed,this, &MTView::keyPressed,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().keyReleased,this, &MTView::keyReleased,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseMoved,this, &MTView::mouseMoved,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseDragged,this, &MTView::mouseDragged,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mousePressed,this, &MTView::mousePressed,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseReleased,this, &MTView::mouseReleased,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseScrolled,this, &MTView::mouseScrolled,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseEntered,this, &MTView::mouseEntered,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().mouseExited,this, &MTView::mouseExited,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().windowResized,this, &MTView::windowResized,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().messageEvent,this, &MTView::messageReceived,OF_EVENT_ORDER_APP);
    ofAddListener(w->events().fileDragEvent,this, &MTView::dragged,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchCancelled,this, &ofxMTView::touchCancelled,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchDoubleTap,this, &ofxMTView::touchDoubleTap,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchDown,this, &ofxMTView::touchDown,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchMoved,this, &ofxMTView::touchMoved,OF_EVENT_ORDER_APP);
//    ofAddListener(w->events().touchUp,this, &ofxMTView::touchUp,OF_EVENT_ORDER_APP);
    ofAddListener(MTApp::appChangeModeEvent, this, &MTView::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);
    }

}
void MTView::removeAllEvents()
{
    if (auto w = window.lock()) //Acquire the shared_ptr if it exists
    {
        ofRemoveListener(w->events().setup, this, &MTView::setup, OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().update, this, &MTView::update,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().draw, this, &MTView::draw,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().exit,this, &MTView::exit,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().keyPressed,this, &MTView::keyPressed,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().keyReleased,this, &MTView::keyReleased,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseMoved,this, &MTView::mouseMoved,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseDragged,this, &MTView::mouseDragged,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mousePressed,this, &MTView::mousePressed,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseReleased,this, &MTView::mouseReleased,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseScrolled,this, &MTView::mouseScrolled,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseEntered,this, &MTView::mouseEntered,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().mouseExited,this, &MTView::mouseExited,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().windowResized,this, &MTView::windowResized,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().messageEvent,this, &MTView::messageReceived,OF_EVENT_ORDER_APP);
        ofRemoveListener(w->events().fileDragEvent,this, &MTView::dragged,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchCancelled,this, &ofxMTView::touchCancelled,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchDoubleTap,this, &ofxMTView::touchDoubleTap,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchDown,this, &ofxMTView::touchDown,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchMoved,this, &ofxMTView::touchMoved,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().touchUp,this, &ofxMTView::touchUp,OF_EVENT_ORDER_APP);
        ofRemoveListener(MTApp::appChangeModeEvent, this, &MTView::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);
    }
}

