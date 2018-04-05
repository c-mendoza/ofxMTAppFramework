//
//  ofxMTView.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#include "MTView.hpp"
#include "ofGraphics.h"

MTView::MTView(std::string _name)
{
	name.set("View Name", _name);
	contentScaleX.set("Content Scale X", 1);
	contentScaleY.set("Content Scale Y", 1);

	backgroundColor.set("Background Color",
						ofFloatColor(1.0, 1.0, 1.0, 1.0));
	ofAddListener(MTApp::appStateChangedEvent,
				  this,
				  &MTView::appStateChanged,
				  OF_EVENT_ORDER_AFTER_APP);
	currentViewMode = std::make_shared<MTViewModeVoid>(nullptr);
}

MTView::~MTView()
{
	ofRemoveListener(MTApp::appStateChangedEvent,
					 this,
					 &MTView::appStateChanged,
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
	ofEventArgs args;
	addedToWindowEvent.notify(args);
	addedToWindow();
	for (auto& sv : subviews)
	{
		sv->setWindow(window);
	}
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

void MTView::setFrameOrigin(glm::vec2 pos)
{
	frame.setPosition(glm::vec3(pos, 0));
	frameChangedInternal();
}

void MTView::shiftFrameOrigin(glm::vec2 shiftAmount)
{
	setFrameOrigin(frame.getPosition() + shiftAmount);
}

void MTView::setFrameFromCenter(glm::vec2 pos, glm::vec2 size)
{
	frame.setFromCenter(pos, size.x, size.y);
}

void MTView::setFrameCenter(glm::vec2 pos)
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
	frameChangedInternal();
	frameChanged();
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

void MTView::setContentOrigin(glm::vec2 pos)
{
	// Making sure that we are not accidentally adding a z-coordinate here:
	content.setPosition(glm::vec3(pos, 0));
	contentChangedInternal();
}

const glm::vec3& MTView::getContentOrigin()
{
	return content.getPosition();
}

void MTView::shiftContentOrigin(glm::vec2 shiftAmount)
{
	setContentOrigin(content.position + shiftAmount);
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
	contentChangedInternal();
}

void MTView::setSize(float width, float height)
{
	setFrameSize(width, height);
	setContentSize(width, height);
//	updateMatrices(); //?
}

void MTView::setSize(glm::vec2 size)
{
	setSize(size.x, size.y);
}

void MTView::frameChangedInternal()
{
	updateMatrices();
	updateScreenFrame();
//    ofLogVerbose() << name << " " << screenFrame;

//	layoutInternal();

	// Call User's frameChanged:
	frameChanged();

	// Notify listeners:
	auto args = ofEventArgs();
	frameChangedEvent.notify(this, args);

	// Notify the rest of the hierarchy:
	for (auto sv : subviews)
	{
		sv->superviewFrameChangedInternal();
		//			sv->superviewFrameChangedInternal();
	}
}

void MTView::contentChangedInternal()
{
	updateMatrices();
	updateScreenFrame(); //?

	contentChanged();

	for (auto sv : subviews)
	{
//		sv->superviewFrameChangedInternal();
		sv->superviewContentChangedInternal();
	}
}

void MTView::superviewFrameChangedInternal()
{
	updateMatrices();
	performResizePolicy();  // setFrameSize will call frameChangedInternal
	layoutInternal();
	// call the users function:
	superviewFrameChanged();
}

void MTView::superviewContentChangedInternal()
{
	updateMatrices();
	updateScreenFrame();
	for (auto sv : subviews)
	{
		sv->superviewContentChangedInternal();
	}

	// Call the user's function:
	superviewContentChanged();
}

void MTView::layoutInternal()
{
	onLayout();
	layout();
}

void MTView::performResizePolicy()
{
	auto super = superview.lock();
	if (!super) return;

	switch (resizePolicy)
	{
		case ResizePolicySuperview:
			setFrameSize(super->getFrameSize());
			break;

		case ResizePolicyNone:
			break;
		case ResizePolicyKeepExact:
			break;
		case ResizePolicyKeepProportional:
			break;
		case ResizePolicyAspectRatio:
			break;
	}
}

void MTView::updateScreenFrame()
{
	if (auto super = superview.lock())
	{
		glm::vec4 screenFramePosition = super->contentMatrix * glm::vec4(frame.getPosition(), 1);
		screenFrame.setPosition(screenFramePosition.xyz());
		auto size = getFrameSize().xyyy() * super->contentMatrix; //TODO: check if this is correct
		screenFrame.setSize(size.x, size.y);
	}
	else
	{
		screenFrame = frame;
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

glm::vec2 MTView::transformFramePointToContent(glm::vec2& coords)
{

	auto windowCoords = frameMatrix * glm::vec4(coords.x, coords.y, 1, 1);
	return (invContentMatrix * windowCoords).xy();
}

/// \brief Transforms the passed point from frame
/// coordinates to content coordinates.
glm::vec2 MTView::transformFramePointToScreen(glm::vec2& coords)
{
	auto windowCoords = frameMatrix * glm::vec4(coords.x, coords.y, 1, 1);
	return windowCoords.xy();
}



//------------------------------------------------------//
// VIEW HEIRARCHY                                       //
//------------------------------------------------------//

std::shared_ptr<MTView> MTView::getSuperview()
{
	return superview.lock();
}

void MTView::setSuperview(std::shared_ptr<MTView> view)
{
	superview = view;
	setWindow(view->window);
	frameChangedInternal();
	performResizePolicy();
	layoutInternal();
	ofEventArgs voidArgs;
	addedToSuperviewEvent.notify(voidArgs);
}
/// \brief Adds a subview.

void MTView::addSubview(std::shared_ptr<MTView> subview)
{
	//	subview->window = window; // window for the subview is set in setSuperview
	subview->setSuperview(shared_from_this());

	if (this->isSetUp) // If setupInternal has run already, then call the subview's setup
	{
		// Enqueue it in update() so that we may call setup() under the right
		// OpenGL context.
		enqueueUpdateOperation([this, subview]()
							   {
								   auto args = ofEventArgs();
								   subview->setup(args);
							   });
	}
	subviews.push_back(subview);
}

std::vector<std::shared_ptr<MTView>>& MTView::getSubviews()
{
	return subviews;
}

/// \returns True if successful.
bool MTView::removeFromSuperview()
{
	if (auto s = superview.lock())
	{
		if(s->removeSubview(shared_from_this()))
		{
			ofEventArgs voidArgs;
			removedFromSuperviewEvent.notify(voidArgs);
		}
	}

	return false;
}

/**
 * @brief
 * returns true if there was a view to be removed.
 */
bool MTView::removeLastSubview()
{
	if (!subviews.empty())
	{
		return removeSubview(subviews.back());
	}
	else
	{
		return false;
	}
}

void MTView::resetWindowPointer()
{
	window.reset();
	ofEventArgs args;
	removedFromWindowEvent.notify(args);
	removedFromWindow();
	for (auto& sv : subviews)
	{
		sv->resetWindowPointer();
	}
}
bool MTView::removeSubview(std::shared_ptr<MTView> view)
{
	auto iter = std::find(subviews.begin(), subviews.end(), view);
	if (iter < subviews.end())
	{
		view->superview.reset();
		view->resetWindowPointer();
		subviews.erase(iter);
		return true;
	}

	return false;
}

void MTView::removeAllSubviews()
{
	for (auto& view : subviews)
	{
		view->superview.reset();
		view->resetWindowPointer();
	}
	subviews.clear();
}

std::weak_ptr<MTWindow> MTView::getWindow()
{
	return window;
}

int MTView::getWindowWidth()
{
	if (auto w = window.lock())
	{
		return (int) w->getWindowSize().x;
	}
	else
	{
		return 200;
	}
}

int MTView::getWindowHeight()
{
	if (auto w = window.lock())
	{
		return (int) w->getWindowSize().y;
	}
	else
	{
		return 0;
	}
}

//------------------------------------------------------//
// INTERNAL EVENT LISTENERS
//
// You do not need to call these methods
//------------------------------------------------------//

void MTView::setup(ofEventArgs& args)
{
	currentViewMode = std::make_shared<MTViewModeVoid>(shared_from_this());
	setup();
	isSetUp = true;
	for (auto sv : subviews)
	{
		sv->setup(args);
	}
}

void MTView::update(ofEventArgs& args)
{
	while (!updateOpQueue.empty())
	{
		auto op = updateOpQueue.front();
		op();
		updateOpQueue.pop();
	}

	//Call user's update()
	update();
	onUpdate();

	if (MTApp::sharedApp->autoUpdateAppModes) currentViewMode->update();

	for (auto sv : subviews)
	{
		sv->update(args);
	}
}

void MTView::draw(ofEventArgs& args)
{
//	ofPushView();
//	ofViewport(screenFrame);
	if (!isRenderingEnabled) return;
	ofSetBackgroundAuto(true);

	auto w = window.lock();
	//					glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
	if (clipToFrame)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(screenFrame.x,
				  w->getWindowSize().y - (screenFrame.y + screenFrame.height),
				  screenFrame.width,
				  screenFrame.height);
	}
	ofPushMatrix();
	ofSetMatrixMode(ofMatrixMode::OF_MATRIX_MODELVIEW);

	// Draw the background
	if (isDrawingBackground)
	{
		ofPushMatrix();
		// The background is drawn in Frame coordinates:
		ofMultMatrix(frameMatrix);
		ofFill();
		ofSetColor(backgroundColor.get());
		ofDrawRectangle(0, 0, frame.width, frame.height);
		ofPopMatrix();
	}

	// Load the content coordinates:
//    ofLoadIdentityMatrix();
	ofMultMatrix(contentMatrix);

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
	if (MTApp::sharedApp->autoDrawViewModes)
	{
		if (currentViewMode != nullptr)
		{
			currentViewMode->draw();
		}

	}

	ofPopMatrix();

	// Draw subviews:
	for (auto sv : subviews)
	{
		sv->draw(args);
	}

	if (clipToFrame)
	{
		glDisable(GL_SCISSOR_TEST);
	}
}

void MTView::exit(ofEventArgs& args)
{
	exit();
	onExit();
	for (auto sv : subviews)
	{
		sv->exit(args);
	}
}

void MTView::windowResized(ofResizeEventArgs& resize)
{
//	updateMatrices();
//	layoutInternal();
	windowResized(resize.width, resize.height);
	onWindowResized(resize.width, resize.height);
	for (auto view : subviews)
	{
		view->windowResized(resize);
	}
}
//#pragma mark KEYBOARD EVENTS

void MTView::keyPressed(ofKeyEventArgs& key)
{
	ofLogVerbose("MTView") << "keyPressed: " << name.get() + " " << (char) key.key;
	keyPressed(key.key);
	onKeyPressed(key.key);
	keyPressedEvent.notify(this, key);
}

void MTView::keyReleased(ofKeyEventArgs& key)
{
	ofLogVerbose("MTView") << "keyReleased: " << name.get() + " " << (char) key.key;
	keyReleased(key.key);
	onKeyReleased(key.key);
	keyReleasedEvent.notify(this, key);
}

//#pragma mark MOUSE EVENTS

void MTView::updateMousePositionsWithWindowCoordinate(glm::vec2 windowCoord)
{
	prevContentMouse = contentMouse;
	contentMouse = (invContentMatrix * glm::vec4(windowCoord.x, windowCoord.y, 1, 1)).xy();
	prevWindowMouse = windowMouse;
	windowMouse = windowCoord;
}

void MTView::updateMouseDownPositionsWithWindowCoordinate(glm::vec2 windowCoord)
{
	updateMousePositionsWithWindowCoordinate(windowCoord);
	contentMouseDown = contentMouse;
	windowMouseDown = windowMouse;
}

void MTView::updateMouseUpPositionsWithWindowCoordinate(glm::vec2 windowCoord)
{
	updateMousePositionsWithWindowCoordinate(windowCoord);
	contentMouseUp = contentMouse;
//    windowMouseUp = windowMouse;
}

void MTView::mouseMoved(ofMouseEventArgs& mouse)
{
	updateMousePositionsWithWindowCoordinate(mouse);
	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Moved,
												  contentMouse.x,
												  contentMouse.y,
												  mouse.button);
	mouseMoved(contentMouse.x, contentMouse.y);
	onMouseMoved(contentMouse.x, contentMouse.y);
	mouseMovedEvent.notify(this, localArgs);
}

void MTView::mouseDragged(ofMouseEventArgs& mouse)
{
	updateMousePositionsWithWindowCoordinate(mouse);
	if (!isMouseDragging)
	{
		isMouseDragging = true;
		contentMouseDragStart = contentMouseDown;
		windowMouseDragStart = windowMouseDown;
	}

	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Dragged,
												  contentMouse.x,
												  contentMouse.y,
												  mouse.button);
	mouseDragged(contentMouse.x, contentMouse.y, mouse.button);
	onMouseDragged(contentMouse.x, contentMouse.y, mouse.button);
	mouseDraggedEvent.notify(this, localArgs);
}

void MTView::mousePressed(ofMouseEventArgs& mouse)
{
//	ofLogVerbose("MTView") << "mousePressed: " << name.get();
	updateMouseDownPositionsWithWindowCoordinate(mouse);

	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Pressed,
												  contentMouse.x,
												  contentMouse.y,
												  mouse.button);
	mousePressed(contentMouse.x, contentMouse.y, mouse.button);
	onMousePressed(contentMouse.x, contentMouse.y, mouse.button);
	mousePressedEvent.notify(this, localArgs);
}

void MTView::mouseReleased(ofMouseEventArgs& mouse)
{
	//	ofLogVerbose("MTView") << "mouseReleased: " << name.get();
	updateMouseUpPositionsWithWindowCoordinate(mouse);

	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Released,
												  contentMouse.x,
												  contentMouse.y,
												  mouse.button);
	if (isMouseDragging)
	{
		isMouseDragging = false;
		mouseDraggedEndEvent.notify(this, localArgs);
	}

	isMouseDown = false;
	mouseReleased(contentMouse.x, contentMouse.y, mouse.button);
	onMouseReleased(contentMouse.x, contentMouse.y, mouse.button);
	mouseReleasedEvent.notify(this, localArgs);
}

void MTView::mouseScrolled(ofMouseEventArgs& mouse)
{
	updateMousePositionsWithWindowCoordinate(mouse);
	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Scrolled,
												  contentMouse.x,
												  contentMouse.y,
												  mouse.button);
	mouseWheel = mouse.scrollY;
//    ofLogNotice("MTView::mouseScrolled") << "scrollX and scrollY are in Window coordinates"
	mouseScrolled(contentMouse.x, contentMouse.y, mouse.scrollX, mouse.scrollY);
	onMouseScrolled(contentMouse.x, contentMouse.y, mouse.scrollX, mouse.scrollY);
	mouseScrolledEvent.notify(this, localArgs);
}

void MTView::mouseEntered(ofMouseEventArgs& mouse)
{
	ofLogVerbose("MTView") << "mouseEntered: " << name.get();
	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Entered,
												  contentMouse.x,
												  contentMouse.y,
												  mouse.button);
	mouseEntered(contentMouse.x, contentMouse.y);
	onMouseEntered(contentMouse.x, contentMouse.y);
	mouseEnteredEvent.notify(this, localArgs);
}

void MTView::mouseExited(ofMouseEventArgs& mouse)
{
	ofLogVerbose("MTView") << "mouseExited: " << name.get();
	ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Exited,
												  contentMouse.x,
												  contentMouse.y,
												  mouse.button);
	mouseExited(contentMouse.x, contentMouse.y);
	onMouseExited(contentMouse.x, contentMouse.y);
	mouseExitedEvent.notify(this, localArgs);
}

void MTView::dragged(ofDragInfo& drag)
{
	ofLogNotice() << "ofxMTView::dragged not yet implemented";
}

void MTView::messageReceived(ofMessage& message)
{
	ofLogNotice() << "ofxMTView::messageReceived not yet implemented";
}

void MTView::modelLoaded(ofEventArgs& args)
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

std::shared_ptr<MTView> MTView::hitTest(glm::vec2& windowCoord)
{
	if (subviews.size() > 0)
	{
		for (auto it = subviews.end() - 1; it >= subviews.begin(); --it)
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
		frameMatrix = glm::translate(sv->contentMatrix, frame.getPosition());
	}
	else
	{
		frameMatrix = glm::translate(glm::mat4(1.0f), frame.getPosition());
	}

	invFrameMatrix = glm::inverse(frameMatrix);
	auto scaleMatrix = glm::scale(glm::vec3(contentScaleX.get(), contentScaleY.get(), 1));
	auto transMatrix = glm::translate(frameMatrix, content.getPosition());
	contentMatrix = transMatrix * scaleMatrix;
	invContentMatrix = glm::inverse(contentMatrix);

	for (auto& sv : subviews)
	{
		sv->updateMatrices();
	}
}

const ofParameter<ofFloatColor>& MTView::getBackgroundColor() const
{
	return backgroundColor;
}

const glm::mat4& MTView::getContentMatrix() const
{
	return contentMatrix;
}

const glm::mat4& MTView::getInvContentMatrix() const
{
	return invContentMatrix;
}

const glm::mat4& MTView::getInvFrameMatrix() const
{
	return invFrameMatrix;
}

const glm::mat4& MTView::getFrameMatrix() const
{
	return frameMatrix;
}
