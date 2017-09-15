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

MTView::MTView(string _name)
{
	name.set("View Name", _name);
	contentScaleX.set("Content Scale X", 1);
	contentScaleY.set("Content Scale Y", 1);

	backgroundColor.set("Background Color",
						ofFloatColor(1.0, 1.0, 1.0, 1.0));
	ofAddListener(MTApp::appChangeModeEvent,
				  this,
				  &MTView::appModeChanged,
				  OF_EVENT_ORDER_AFTER_APP);
}

MTView::~MTView()
{
	ofRemoveListener(MTApp::appChangeModeEvent,
				  this,
				  &MTView::appModeChanged,
				  OF_EVENT_ORDER_AFTER_APP);
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

void MTView::shiftFrameOrigin(glm::vec3 shiftAmount)
{
	setFrameOrigin(frame.getPosition() + shiftAmount);
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

glm::vec3 MTView::getFrameCenter()
{
	return frame.getCenter();
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

/// \brief Sets the scale of the content matrix.
/// 1 means no scaling, 0.5 means half scale, -1 means inverse scale.
void MTView::setContentScale(float xs, float ys)
{
	contentScaleX = xs;
	contentScaleY = ys;
	updateMatrices();
}

void MTView::setSize(float width, float height)
{
	content.setSize(width, height);
	frame.setSize(width, height);
	updateMatrices();
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
	auto windowCoords = frameMatrix * glm::vec4(coords.x, coords.y, 1, 1);
	return (toView->invFrameMatrix * windowCoords).xy();
}

glm::vec2 MTView::transformPoint(glm::vec2& coords,
								 std::shared_ptr<MTView> toView)
{
	return transformPoint(coords, toView.get());
}

glm::vec2 MTView::frameToContent(glm::vec2& coords)
{

	auto windowCoords = frameMatrix * glm::vec4(coords.x, coords.y, 1, 1);
	return (invContentMatrix * windowCoords).xy();
}

//------------------------------------------------------//
// VIEW HEIRARCHY                                       //
//------------------------------------------------------//

std::shared_ptr<MTView> MTView::getSuperview()
{
	return superview.lock();
}

void MTView::setSuperview(shared_ptr<MTView> view)
{
	superview = view;
	frameChangedInternal();
}
/// \brief Adds a subview.

void MTView::addSubview(shared_ptr<MTView> subview)
{
	subview->setSuperview(shared_from_this());
	subview->window = window;
	auto args = ofEventArgs();
	subview->setup(args);
	subviews.push_back(subview);
}

vector<shared_ptr<MTView>>& MTView::getSubviews()
{
	return subviews;
}

/// \returns True if successful.
bool MTView::removeFromSuperview()
{
//    if (auto s = superview.lock())
//    {
//        auto sv = s->getSubviews();
//        auto iter = std::find(sv.begin(), sv.end(), shared_from_this());
//        if (iter != sv.end())
//        {
//            superview.reset();
//            sv.erase(iter);
//            return true;
//        }
//    }

//    return false;
	if (auto s = superview.lock())
	{
		return s->removeSubview(shared_from_this());
	}

	return false;
}

/// \returns True if there was a view to be removed.
bool MTView::removeLastSubview()
{
	if (subviews.size() > 0)
	{
		auto sv = subviews.back();
		sv->superview.reset();
		subviews.pop_back();
		return true;
	}
	else
	{
		return false;
	}
}

bool MTView::removeSubview(std::shared_ptr<MTView> view)
{
	auto iter = std::find(subviews.begin(), subviews.end(), view);
	if (iter < subviews.end())
	{
		view->superview.reset();
		subviews.erase(iter);
		return true;
	}

	return false;
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
	currentAppMode = std::make_shared<MTAppModeVoid>(shared_from_this());
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
//	ofPushView();
//	ofViewport(screenFrame);
	ofSetMatrixMode(ofMatrixMode::OF_MATRIX_MODELVIEW);
	ofLoadIdentityMatrix();

	// Draw the background
	if (isDrawingBackground)
	{
		ofPushMatrix();
		// The background is drawn in Frame coordinates:
		ofLoadMatrix(ofGetCurrentViewMatrix() * frameMatrix);
		ofFill();
		ofSetColor(backgroundColor.get());
		ofDrawRectangle(0, 0, frame.width, frame.height);
		ofPopMatrix();
	}

	// Load the content coordinates:
	ofLoadMatrix(ofGetCurrentViewMatrix() * contentMatrix);

	// Execute operations in the draw queue:
	while (!drawOpQueue.empty())
	{
		auto op = drawOpQueue.front();
		op();
		drawOpQueue.pop();
	}

	// Call the user's draw() function(s)
	draw();
	onDraw();

	// Should I fire a drawEvent here instead? It would make sense...
	if (MTApp::sharedApp->autoDrawAppModes) currentAppMode->draw();

//	ofPopView();
	// Draw subviews:
	for (auto sv : subviews)
	{
		sv->draw(args);
	}
}

void MTView::exit(ofEventArgs &args)
{
	exit();
	onExit();
}

void MTView::windowResized(ofResizeEventArgs & resize)
{
	updateMatrices();
	windowResized(resize.width, resize.height);
	onWindowResized(resize.width, resize.height);
	for (auto view : subviews)
	{
		view->windowResized(resize);
	}
}

void MTView::keyPressed(ofKeyEventArgs & key)
{
	ofLogVerbose("MTView") << "keyPressed: " << name.get() + " " << (char)key.key;
	keyPressed(key.key);
	onKeyPressed(key.key);
	keyPressedEvent.notify(this, key);
}

void MTView::keyReleased(ofKeyEventArgs & key)
{
	ofLogVerbose("MTView") << "keyReleased: " << name.get() + " " << (char)key.key;
	keyReleased(key.key);
	onKeyReleased(key.key);
	keyReleasedEvent.notify(this, key);
}

void MTView::mouseMoved(ofMouseEventArgs & mouse)
{
	frameMouse = (invFrameMatrix * glm::vec4(mouse.x, mouse.y, 1, 1)).xy();
	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Moved,
												  frameMouse.x,
												  frameMouse.y,
												  mouse.button);
	mouseMoved(frameMouse.x, frameMouse.y);
	onMouseMoved(frameMouse.x, frameMouse.y);
	mouseMovedEvent.notify(this, localArgs);
}

void MTView::mouseDragged(ofMouseEventArgs & mouse)
{
	frameMouse = (invFrameMatrix * glm::vec4(mouse.x, mouse.y, 1, 1)).xy();
	if (!isDragging)
	{
		isDragging = true;
		frameMouseDragStart = frameMouseDown;
	}

	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Dragged,
												  frameMouse.x,
												  frameMouse.y,
												  mouse.button);
	mouseDragged(frameMouse.x, frameMouse.y, mouse.button);
	onMouseDragged(frameMouse.x, frameMouse.y, mouse.button);
	mouseDraggedEvent.notify(this, localArgs);
}

void MTView::mousePressed(ofMouseEventArgs & mouse)
{
	ofLogVerbose("MTView") << "mousePressed: " << name.get();
	frameMouseDown =  (invFrameMatrix * glm::vec4(mouse.x, mouse.y, 1, 1)).xy();
	frameMouse = frameMouseDown;

	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Pressed,
												  frameMouse.x,
												  frameMouse.y,
												  mouse.button);
	mousePressed(frameMouse.x, frameMouse.y, mouse.button);
	onMousePressed(frameMouse.x, frameMouse.y, mouse.button);
	mousePressedEvent.notify(this, localArgs);
}

void MTView::mouseReleased(ofMouseEventArgs & mouse)
{
	ofLogVerbose("MTView") << "mouseReleased: " << name.get();
	frameMouseUp = (invFrameMatrix * glm::vec4(mouse.x, mouse.y, 1, 1)).xy();
	frameMouse = frameMouseUp;
	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Released,
												  frameMouse.x,
												  frameMouse.y,
												  mouse.button);
	if (isMouseDragging)
	{
		isMouseDragging = false;
		mouseDraggedEndEvent.notify(this, localArgs);
	}

	isMouseDown = false;
	mouseReleased(frameMouse.x, frameMouse.y, mouse.button);
	onMouseReleased(frameMouse.x, frameMouse.y, mouse.button);
	mouseReleasedEvent.notify(this, localArgs);
}

void MTView::mouseScrolled( ofMouseEventArgs & mouse )
{
	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Scrolled,
												  frameMouse.x,
												  frameMouse.y,
												  mouse.button);
//    ofLogNotice("MTView::mouseScrolled") << "scrollX and scrollY are in Window coordinates"
	mouseScrolled(frameMouse.x, frameMouse.y, mouse.scrollX, mouse.scrollY);
	onMouseScrolled(frameMouse.x, frameMouse.y, mouse.scrollX, mouse.scrollY);
	mouseScrolledEvent.notify(this, localArgs);
}

void MTView::mouseEntered( ofMouseEventArgs & mouse )
{
	ofLogVerbose("MTView") << "mouseEntered: " << name.get();
	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Entered,
												  frameMouse.x,
												  frameMouse.y,
												  mouse.button);
	mouseEntered(frameMouse.x, frameMouse.y);
	onMouseEntered(frameMouse.x, frameMouse.y);
	mouseEnteredEvent.notify(this, localArgs);
}

void MTView::mouseExited( ofMouseEventArgs & mouse )
{
	ofLogVerbose("MTView") << "mouseExited: " << name.get();
	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Exited,
												  frameMouse.x,
												  frameMouse.y,
												  mouse.button);
	mouseExited(frameMouse.x, frameMouse.y);
	onMouseExited(frameMouse.x, frameMouse.y);
	mouseExitedEvent.notify(this, localArgs);
}

void MTView::dragged(ofDragInfo & drag)
{
	ofLogNotice() << "ofxMTView::dragged not yet implemented";
}

void MTView::messageReceived(ofMessage & message)
{
	ofLogNotice() << "ofxMTView::messageReceived not yet implemented";
}

void MTView::modelLoaded(ofEventArgs &args)
{
	enqueueUpdateOperation([this]()
	{
		modelLoaded();
		onModelLoaded();
	});

	// Recurse:
	for (auto sv : subviews)
	{
		sv->modelLoaded(args);
	}
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
		frameMatrix = glm::translate(glm::mat4(1.0f), frame.getPosition());
	}

	invFrameMatrix = glm::inverse(frameMatrix);

	auto pos = glm::translate(frameMatrix, content.getPosition());
	contentMatrix = glm::scale(pos, glm::vec3(contentScaleX.get(), contentScaleY.get(), 1));
	invContentMatrix = glm::inverse(contentMatrix);

	// TODO: Should updateMatrices recurse?
}


//void MTView::addAllEvents()
//{
//    if (auto w = window.lock())
//    {
//        w->events().enable();
//        ofAddListener(w->events().setup, this, &MTView::setup, OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().update, this, &MTView::update,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().draw, this, &MTView::draw,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().exit,this, &MTView::exit,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().keyPressed,this, &MTView::keyPressed,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().keyReleased,this, &MTView::keyReleased,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().mouseMoved,this, &MTView::mouseMoved,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().mouseDragged,this, &MTView::mouseDragged,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().mousePressed,this, &MTView::mousePressed,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().mouseReleased,this, &MTView::mouseReleased,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().mouseScrolled,this, &MTView::mouseScrolled,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().mouseEntered,this, &MTView::mouseEntered,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().mouseExited,this, &MTView::mouseExited,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().windowResized,this, &MTView::windowResized,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().messageEvent,this, &MTView::messageReceived,OF_EVENT_ORDER_APP);
//        ofAddListener(w->events().fileDragEvent,this, &MTView::dragged,OF_EVENT_ORDER_APP);
//        //    ofAddListener(w->events().touchCancelled,this, &ofxMTView::touchCancelled,OF_EVENT_ORDER_APP);
//        //    ofAddListener(w->events().touchDoubleTap,this, &ofxMTView::touchDoubleTap,OF_EVENT_ORDER_APP);
//        //    ofAddListener(w->events().touchDown,this, &ofxMTView::touchDown,OF_EVENT_ORDER_APP);
//        //    ofAddListener(w->events().touchMoved,this, &ofxMTView::touchMoved,OF_EVENT_ORDER_APP);
//        //    ofAddListener(w->events().touchUp,this, &ofxMTView::touchUp,OF_EVENT_ORDER_APP);
//        ofAddListener(MTApp::modelLoadedEvent, this, &MTView::modelLoaded,OF_EVENT_ORDER_AFTER_APP);
//        ofAddListener(MTApp::appChangeModeEvent, this, &MTView::appModeChanged,OF_EVENT_ORDER_AFTER_APP);
//    }

//}
//void MTView::removeAllEvents()
//{
//    if (auto w = window.lock()) //Acquire the shared_ptr if it exists
//    {
//        ofRemoveListener(w->events().setup, this, &MTView::setup, OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().update, this, &MTView::update,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().draw, this, &MTView::draw,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().exit,this, &MTView::exit,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().keyPressed,this, &MTView::keyPressed,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().keyReleased,this, &MTView::keyReleased,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().mouseMoved,this, &MTView::mouseMoved,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().mouseDragged,this, &MTView::mouseDragged,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().mousePressed,this, &MTView::mousePressed,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().mouseReleased,this, &MTView::mouseReleased,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().mouseScrolled,this, &MTView::mouseScrolled,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().mouseEntered,this, &MTView::mouseEntered,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().mouseExited,this, &MTView::mouseExited,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().windowResized,this, &MTView::windowResized,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().messageEvent,this, &MTView::messageReceived,OF_EVENT_ORDER_APP);
//        ofRemoveListener(w->events().fileDragEvent,this, &MTView::dragged,OF_EVENT_ORDER_APP);
////        ofRemoveListener(w->events().touchCancelled,this, &ofxMTView::touchCancelled,OF_EVENT_ORDER_APP);
////        ofRemoveListener(w->events().touchDoubleTap,this, &ofxMTView::touchDoubleTap,OF_EVENT_ORDER_APP);
////        ofRemoveListener(w->events().touchDown,this, &ofxMTView::touchDown,OF_EVENT_ORDER_APP);
////        ofRemoveListener(w->events().touchMoved,this, &ofxMTView::touchMoved,OF_EVENT_ORDER_APP);
////        ofRemoveListener(w->events().touchUp,this, &ofxMTView::touchUp,OF_EVENT_ORDER_APP);
//        ofRemoveListener(MTApp::modelLoadedEvent, this, &MTView::modelLoaded,OF_EVENT_ORDER_AFTER_APP);
//        ofRemoveListener(MTApp::appChangeModeEvent, this, &MTView::appModeChanged,OF_EVENT_ORDER_AFTER_APP);
//    }
//}

