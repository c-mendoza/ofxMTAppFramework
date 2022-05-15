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

   backgroundColor.set("Background Color", ofFloatColor(1.0, 1.0, 1.0, 1.0));
   //	currentViewMode = std::make_shared<MTViewModeVoid>(nullptr);
   setFrameSize(200, 200);
}

MTView::~MTView()
{
   subviews.clear();
   ofLogVerbose("MTView") << name << " destroyed";
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
   for (auto &sv : subviews)
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

void MTView::setFrameOrigin(const glm::vec2 &pos)
{
   frame.setPosition(pos.x, pos.y);
   frameChangedInternal();
}

void MTView::shiftFrameOrigin(const glm::vec2 &shiftAmount)
{
   setFrameOrigin(frame.getPosition() + shiftAmount);
}

void MTView::setFrameFromCenter(const glm::vec2 &pos, const glm::vec2 &size)
{
   frame.setFromCenter(pos.x, pos.y, size.x, size.y);
}

void MTView::setFrameCenter(const glm::vec2 &pos)
{
   frame.setFromCenter(pos, frame.width, frame.height);
   frameChangedInternal();
}

void MTView::setFrameSize(const glm::vec2 &size)
{
   setFrameSize(size.x, size.y);
}

void MTView::setFrameSize(float width, float height)
{
   frame.setSize(width, height);
   frameChangedInternal();
   frameChanged();
}

const glm::vec3 &MTView::getFrameOrigin()
{
   return frame.getPosition();
}

glm::vec2 MTView::getFrameSize()
{
   auto size = glm::vec2(frame.getWidth(), frame.getHeight());
   return size;
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

void MTView::setContentOrigin(const glm::vec2 &pos)
{
   // Making sure that we are not accidentally adding a z-coordinate here:
   content.setPosition(pos.x, pos.y);
   contentChangedInternal();
}

const glm::vec3 &MTView::getContentOrigin()
{
   return content.getPosition();
}

void MTView::shiftContentOrigin(const glm::vec2 &shiftAmount)
{
   setContentOrigin(content.position + shiftAmount);
}

void MTView::setContentSize(const glm::vec2 &size)
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

void MTView::setPosition(float x, float y)
{
   setFrameOrigin(x, y);
   setContentOrigin({0, 0});
}

void MTView::setSize(const glm::vec2 &size)
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
   for (const auto &sv : subviews)
   {
      sv->superviewFrameChangedInternal();
      //			sv->superviewFrameChangedInternal();
   }
}

void MTView::contentChangedInternal()
{
   updateMatrices();
   updateScreenFrame();  //?

   contentChanged();

   for (const auto &sv : subviews)
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
   for (const auto &sv : subviews)
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
   if (!superview) return;

   switch (resizePolicy)
   {
   case ResizePolicySuperview: setFrameSize(superview->getFrameSize()); break;
   case ResizePolicyNone: break;
   case ResizePolicyKeepExact: ofLogNotice("MTView") << "ResizePolicyKeepExact not yet implemented"; break;
   case ResizePolicyKeepProportional: ofLogNotice("MTView") << "ResizePolicyKeepProportional not yet implemented"; break;
   case ResizePolicyAspectRatio: ofLogNotice("MTView") << "ResizePolicyAspectRatio not yet implemented"; break;
   }
}

void MTView::updateScreenFrame()
{
   if (superview)
   {
      glm::vec4 screenFramePosition = superview->contentMatrix * glm::vec4(frame.getPosition(), 1);
      screenFrame.setPosition(screenFramePosition);
      auto size = glm::vec4(getFrameSize(), 0, 0) * superview->contentMatrix;  //TODO: check if this is correct
      screenFrame.setSize(size.x, size.y);
   }
   else
   {
      screenFrame = frame;
   }
}

//TODO: Check to see if transformPoint works
glm::vec2 MTView::transformPoint(glm::vec2 &coords, const MTView *toView)
{
   auto windowCoords = frameMatrix * glm::vec4(coords.x, coords.y, 1, 1);
   return toView->invFrameMatrix * windowCoords;
}

//glm::vec2 MTView::transformPoint(glm::vec2 &coords,
//								 MTView* toView)
//{
//	return transformPoint(coords, toView);
//}

glm::vec2 MTView::transformFramePointToContent(glm::vec2 &coords)
{
   auto windowCoords = frameMatrix * glm::vec4(coords.x, coords.y, 1, 1);
   auto result = invContentMatrix * windowCoords;
   return glm::vec2(result.x, result.y);
}

/// \brief Transforms the passed point from frame
/// coordinates to content coordinates.
glm::vec2 MTView::transformFramePointToScreen(glm::vec2 &coords)
{
   auto windowCoords = frameMatrix * glm::vec4(coords.x, coords.y, 1, 1);
   return glm::vec2(windowCoords);
}


//------------------------------------------------------//
// VIEW HEIRARCHY                                       //
//------------------------------------------------------//

MTView *MTView::getSuperview()
{
   return superview;
}

void MTView::setSuperview(MTView *view)
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
   subview->setSuperview(this);
   if (this->isSetUp)  // If setupInternal has run already, then call the subview's setup
   {
      // Enqueue it in update() so that we may call setup() under the right
      // OpenGL context.
      //		MTView* subviewPtr = subview.get();
      enqueueUpdateOperation(
          [subview]()
          {
             auto args = ofEventArgs();
             subview->setup(args);
          });
   }
   subviews.push_back(subview);
}

const std::vector<std::shared_ptr<MTView>> &MTView::getSubviews() const
{
   return subviews;
}

/// \returns True if successful.
std::shared_ptr<MTView> MTView::removeFromSuperview()
{
   if (superview != NULL)
   {
      if (auto view = superview->removeSubview(this))
      {
         ofEventArgs voidArgs;
         removedFromSuperviewEvent.notify(voidArgs);
         return view;
      }
   }

   return nullptr;
}

/**
 * @brief
 * returns true if there was a view to be removed.
 */
std::shared_ptr<MTView> MTView::removeLastSubview()
{
   if (!subviews.empty())
   {
      return removeSubview(subviews.back().get());
   }
   else
   {
      return nullptr;
   }
}

void MTView::resetWindowPointer()
{
   window.reset();
   ofEventArgs args;
   removedFromWindowEvent.notify(args);
   removedFromWindow();
   for (auto &sv : subviews)
   {
      sv->resetWindowPointer();
   }
}

std::shared_ptr<MTView> MTView::removeSubview(MTView *view)
{
   auto iter = std::find_if(subviews.begin(), subviews.end(), [&](std::shared_ptr<MTView> &p) { return p.get() == view; });
   if (iter <= subviews.end())
   {
      view->superview = nullptr;
      view->resetWindowPointer();
      auto res = *iter;
      subviews.erase(iter);
      return res;
   }

   return nullptr;
}

void MTView::removeAllSubviews(bool recursive)
{
   for (auto &view : subviews)
   {
      if (recursive) view->removeAllSubviews();
      view->superview = nullptr;
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

void MTView::setup(ofEventArgs &args)
{
   currentViewMode = std::make_shared<MTViewModeVoid>(this);
   eventListeners.unsubscribeAll();
   addEventListener(MTApp::GetApp()->appModeChangedEvent.newListener(
       [this](MTAppModeChangeArgs &args)
       {
          if (currentViewMode)
          {
             currentViewMode->exit();
          }
          appModeChanged(args);
       },
       -100));
   setup();
   onSetup(this);
   isSetUp = true;
   for (const auto &sv : subviews)
   {
      sv->setup(args);
   }
}

void MTView::update(ofEventArgs &args)
{
   while (!updateOpQueue.empty())
   {
      auto op = updateOpQueue.front();
      op();
      updateOpQueue.pop();
   }

   //Call user's update()
   update();
   onUpdate(this);

   if (MTApp::Instance()->autoUpdateAppModes) currentViewMode->update();

   for (const auto &sv : subviews)
   {
      sv->update(args);
   }
}

void MTView::draw(ofEventArgs &args)
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
      ofSetRectMode(OF_RECTMODE_CORNER);
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
   onDraw(this);

   // Should I fire a drawEvent here instead? It would make sense...
   if (MTApp::Instance()->autoDrawViewModes)
   {
      if (currentViewMode != nullptr)
      {
         currentViewMode->draw();
      }
   }

   ofPopMatrix();

   // Draw subviews:
   for (const auto &sv : subviews)
   {
      sv->draw(args);
   }

   if (clipToFrame)
   {
      glDisable(GL_SCISSOR_TEST);
   }
}

void MTView::drawGuiInternal()
{
   drawGui();
   onDrawGui();
}

void MTView::exit(ofEventArgs &args)
{
   currentViewMode->exit();
   exit();
   onExit(this);
   for (const auto &sv : subviews)
   {
      sv->exit(args);
   }
}

void MTView::windowResized(ofResizeEventArgs &resize)
{
   //	updateMatrices();
   //	layoutInternal();
   windowResized(resize.width, resize.height);
   onWindowResized(this, resize.width, resize.height);
   for (const auto &sv : subviews)
   {
      sv->windowResized(resize);
   }
}
//#pragma mark KEYBOARD EVENTS

void MTView::keyPressedInternal(ofKeyEventArgs &key)
{
   ofLogVerbose("MTView") << "keyPressed: " << name.get() + " " << (char) key.key;
   keyPressed(key.key);
   keyPressed(key);
   onKeyPressed(this, key.key);
   keyPressedEvent.notify(this, key);
}

void MTView::keyReleasedInternal(ofKeyEventArgs &key)
{
   ofLogVerbose("MTView") << "keyReleasedInternal: " << name.get() + " " << (char) key.key;
   keyReleased(key.key);
   keyReleased(key);
   onKeyReleased(this, key.key);
   keyReleasedEvent.notify(this, key);
}

//#pragma mark MOUSE EVENTS

void MTView::updateMousePositionsWithWindowCoordinate(glm::vec2 windowCoord)
{
   prevContentMouse = contentMouse;
   contentMouse = glm::vec2(invContentMatrix * glm::vec4(windowCoord.x, windowCoord.y, 1, 1));
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

void MTView::mouseMoved(ofMouseEventArgs &mouse)
{
   updateMousePositionsWithWindowCoordinate(mouse);
   ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Moved, contentMouse.x, contentMouse.y, mouse.button);
   mouseMoved(contentMouse.x, contentMouse.y);
   onMouseMoved(this, contentMouse.x, contentMouse.y);
   mouseMovedEvent.notify(this, localArgs);
}

void MTView::mouseDragged(ofMouseEventArgs &mouse)
{
   updateMousePositionsWithWindowCoordinate(mouse);
   if (!isMouseDragging)
   {
      isMouseDragging = true;
      contentMouseDragStart = contentMouseDown;
      windowMouseDragStart = windowMouseDown;
   }

   ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Dragged, contentMouse.x, contentMouse.y, mouse.button);
   mouseDragged(contentMouse.x, contentMouse.y, mouse.button);
   onMouseDragged(this, contentMouse.x, contentMouse.y, mouse.button);
   mouseDraggedEvent.notify(this, localArgs);
}

void MTView::mousePressed(ofMouseEventArgs &mouse)
{
   ofLogVerbose("MTView") << "mousePressed: " << name.get();
   updateMouseDownPositionsWithWindowCoordinate(mouse);

   ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Pressed, contentMouse.x, contentMouse.y, mouse.button);
   mousePressed(contentMouse.x, contentMouse.y, mouse.button);
   onMousePressed(this, contentMouse.x, contentMouse.y, mouse.button);
   mousePressedEvent.notify(this, localArgs);
}

void MTView::mouseReleased(ofMouseEventArgs &mouse)
{
   ofLogVerbose("MTView") << "mouseReleased: " << name.get();
   updateMouseUpPositionsWithWindowCoordinate(mouse);

   ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Released, contentMouse.x, contentMouse.y, mouse.button);
   if (isMouseDragging)
   {
      isMouseDragging = false;
      mouseDraggedEndEvent.notify(this, localArgs);
   }

   isMouseDown = false;
   mouseReleased(contentMouse.x, contentMouse.y, mouse.button);
   onMouseReleased(this, contentMouse.x, contentMouse.y, mouse.button);
   mouseReleasedEvent.notify(this, localArgs);
}

void MTView::mouseScrolled(ofMouseEventArgs &mouse)
{
   updateMousePositionsWithWindowCoordinate(mouse);
   ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Scrolled, contentMouse.x, contentMouse.y, mouse.button);
   mouseWheel = mouse.scrollY;
   //    ofLogNotice("MTView::mouseScrolled") << "scrollX and scrollY are in Window coordinates"
   mouseScrolled(contentMouse.x, contentMouse.y, mouse.scrollX, mouse.scrollY);
   onMouseScrolled(this, contentMouse.x, contentMouse.y, mouse.scrollX, mouse.scrollY);
   mouseScrolledEvent.notify(this, localArgs);
}

void MTView::mouseEntered(ofMouseEventArgs &mouse)
{
   ofLogVerbose("MTView") << "mouseEntered: " << name.get();
   ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Entered, contentMouse.x, contentMouse.y, mouse.button);
   mouseEntered(contentMouse.x, contentMouse.y);
   onMouseEntered(this, contentMouse.x, contentMouse.y);
   mouseEnteredEvent.notify(this, localArgs);
}

void MTView::mouseExited(ofMouseEventArgs &mouse)
{
   ofLogVerbose("MTView") << "mouseExited: " << name.get();
   ofMouseEventArgs localArgs = ofMouseEventArgs(ofMouseEventArgs::Exited, contentMouse.x, contentMouse.y, mouse.button);
   mouseExited(contentMouse.x, contentMouse.y);
   onMouseExited(this, contentMouse.x, contentMouse.y);
   mouseExitedEvent.notify(this, localArgs);
}

void MTView::dragged(ofDragInfo &drag)
{
   ofLogNotice() << "ofxMTView::dragged not yet implemented";
}

void MTView::messageReceived(ofMessage &message)
{
   ofLogNotice() << "ofxMTView::messageReceived not yet implemented";
}

void MTView::modelLoaded(ofEventArgs &args)
{
   enqueueUpdateOperation(
       [this]()
       {
          modelLoaded();
          onModelLoaded(this);
       });

   // Recurse:
   for (const auto &sv : subviews)
   {
      sv->modelLoaded(args);
   }
}

MTView *MTView::hitTest(glm::vec2 &windowCoord)
{
   if (subviews.size() > 0)
   {
      for (int i = subviews.size() - 1; i >= 0; i--)
      {
         auto &sv = subviews[i];
         if (sv->screenFrame.inside(windowCoord))
         {
            return sv->hitTest(windowCoord);
         }
      }

      //for (auto it = subviews.end()-1; it >= subviews.begin(); it--)
      //{
      //	auto sv = it->get();
      //	if (sv->screenFrame.inside(windowCoord))
      //	{
      //		return sv->hitTest(windowCoord);
      //	}
      //}
   }

   return this;
}

bool MTView::hasFocus()
{
   return isFocused;
}

void MTView::updateMatrices()
{
   if (superview)
   {
      frameMatrix = glm::translate(superview->contentMatrix, frame.getPosition());
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

   for (const auto &sv : subviews)
   {
      sv->updateMatrices();
   }
}

const ofParameter<ofFloatColor> &MTView::getBackgroundColor() const
{
   return backgroundColor;
}

const glm::mat4 &MTView::getContentMatrix() const
{
   return contentMatrix;
}

const glm::mat4 &MTView::getInvContentMatrix() const
{
   return invContentMatrix;
}

const glm::mat4 &MTView::getInvFrameMatrix() const
{
   return invFrameMatrix;
}

const glm::mat4 &MTView::getFrameMatrix() const
{
   return frameMatrix;
}

void MTView::setViewMode(std::shared_ptr<MTViewMode> mode)
{
   currentViewMode = mode;
   currentViewMode->setup();
}
