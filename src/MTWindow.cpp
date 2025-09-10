//
//  ofxMTAppFramework.h
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 3/11/16.
//
//


#include "MTWindow.hpp"
#include "GLFW/glfw3.h"
#include "ofGraphics.h"
#include "ofAppRunner.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

static ofEventArgs voidEventArgs;

MTWindow::MTWindow(const std::string& name)
{
   contentView = MTView::CreateView("root");
   contentView->backgroundColor = ofColor(0);
   focusedView = contentView.get();
   mouseOverView = contentView.get();
   this->name.set("Window Name", name);
   imCtx = nullptr;
}

MTWindow::~MTWindow()
{
   if (gui)
   {
      bindImGuiContext();
      gui.reset();
   }
   //	contentView->removeAllSubviews();
   glfwSetCursorPosCallback(getGLFWWindow(), NULL);
   glfwSetWindowFocusCallback(getGLFWWindow(), NULL);
   ofLogVerbose("MTWindow") << name.get() << " destroyed";
}

void MTWindow::close()
{
   if (contentView)
   {
      contentView->removeAllSubviews();
      contentView.reset();
   }

   glfwSetCursorPosCallback(getGLFWWindow(), NULL);
   glfwSetWindowFocusCallback(getGLFWWindow(), NULL);

   if (MTApp::Instance())
   {
      MTApp::Instance()->closeWindow(shared_from_this());
   }

   //// This will destroy the gui:
   bindImGuiContext();
   gui.reset();
}

// void MTWindow::setup(ofEventArgs & args)
//{
//    contentView->setup(args);
//}

#ifndef TARGET_RASPBERRY_PI

void MTWindow::setup(ofGLFWWindowSettings& settings)
{
   ofAppGLFWWindow::setup(settings);
   contentView->setWindow(shared_from_this());
   //	addEventListener(ofEvents().exit.newListener([this](ofEventArgs& args)
   //												 {
   //													 if (contentView)
   //													 {
   //														 contentView->removeAllSubviews();
   //														 contentView.reset();
   //													 }
   //												 }));
   glfwSetCursorPosCallback(getGLFWWindow(), nullptr);
   glfwSetCursorPosCallback(getGLFWWindow(), &MTWindow::mt_motion_cb);
   glfwSetWindowFocusCallback(getGLFWWindow(), &MTWindow::mt_focus_callback);

   //
   //addEventListener(events().windowMoved.newListener(
   //           [this](ofWindowPosEventArgs& args) {
   //           	if (MTApp::Instance())
   //			{
   //				MTApp::Instance()->saveAppPreferences();
   //			}
   //}));
}

#else
void MTWindow::setup(ofGLESWindowSettings& settings)
{
   ofAppEGLWindow::setup(settings);
   contentView->setWindow(shared_from_this());
}
#endif

//------------------------------------------------------//
// INTERNALS EVENT LISTENERS //
//------------------------------------------------------//

void MTWindow::setupInternal(ofEventArgs& args)
{
#ifndef TARGET_RASPBERRY_PI
   contentView->setSize(ofAppGLFWWindow::getWidth(), ofAppGLFWWindow::getHeight());
#else
   contentView->setSize(ofAppEGLWindow::getWidth(), ofAppEGLWindow::getHeight());
#endif
   //	isImGuiEnabled = true;
   //	gui = std::make_shared<ofxImGui::Gui>();
   //	gui->setup();
   //	imCtx = ImGui::GetCurrentContext();
   contentView->setFrameOrigin(glm::vec3(0, 0, 0));
   contentView->setup(args);
   auto size = contentView->getFrameSize();
   ofLogVerbose("MTWindow") << "setupInternal() contentView size:" << size.x << " " << size.y;
}

void MTWindow::update(ofEventArgs& args)
{
   while (!updateOpQueue.empty())
   {
      auto op = updateOpQueue.front();
      op();
      updateOpQueue.pop();
   }

   contentView->update(args);
}

void MTWindow::draw(ofEventArgs& args)
{
   ofClear(backgroundColor);
   ofDisableDepthTest();
#ifndef TARGET_RASPBERRY_PI
   ofSetupScreenPerspective(ofAppGLFWWindow::getWidth(), ofAppGLFWWindow::getHeight());
#else
   ofSetupScreenPerspective(ofAppEGLWindow::getWidth(), ofAppEGLWindow::getHeight());
#endif
   //ofBackground(backgroundColor);
   while (!drawOpQueue.empty())
   {
      auto op = drawOpQueue.front();
      op();
      drawOpQueue.pop();
   }

   contentView->backgroundColor = backgroundColor;
   contentView->draw(args);

   if (isImGuiEnabled)
   {
      if (ofGetLastFrameTime() != 0.0)
      {
         bindImGuiContext();
         getGui()->begin();

         drawImGuiForView(contentView.get());
         getGui()->end();
         getGui()->draw();
      }
   }
}

void MTWindow::drawImGuiForView(MTView* view)
{
   for (const auto& sv : view->getSubviews())
   {
      drawImGuiForView(sv.get());
   }

   view->drawGuiInternal();
}

void MTWindow::addSubview(std::shared_ptr<MTView> subview)
{
   contentView->addSubview(subview);
}

void MTWindow::removeAllSubviews()
{
   contentView->removeAllSubviews();
}

glm::vec2 MTWindow::getFrameSize()
{
   return contentView->getFrameSize();
}

const MTView* MTWindow::getRootView()
{
   return contentView.get();
}

//void MTWindow::exit(ofEventArgs& args)
//{
//	MTApp::Instance()->closeWindow(shared_from_this());
//}

void MTWindow::windowResized(ofResizeEventArgs& resize)
{
   auto size = this->getWindowSize();
   ofViewport(0, 0, size.x, size.y);
   contentView->setFrameSize(resize.width, resize.height);
   contentView->windowResized(resize);
   MTApp::Instance()->saveAppPreferences();
   //onWindowResized();
}

void MTWindow::keyPressed(ofKeyEventArgs& key)
{
   if (focusedView)
   {
      focusedView->keyPressedInternal(key);
   }
   //this->keyPressed(key.key);
   //onKeyPressed(key);
}

void MTWindow::keyReleased(ofKeyEventArgs& key)
{
   if (focusedView)
   {
      focusedView->keyReleasedInternal(key);
   }
   //this->keyReleased(key.key);
   //onKeyReleased(key);
}

void MTWindow::mouseMoved(ofMouseEventArgs& mouse)
{
   auto v = contentView->hitTest(mouse);
   if (mouseOverView)
   {
      if ((v != mouseOverView) && mouseOverView)
      {
         //			mo->mouseReleased(mouse);  // Why was this here??
         mouseOverView->mouseExited(mouse);
         mouseOverView = v;
         v->mouseEntered(mouse);
      }
      mouseOverView->mouseMoved(mouse);
   }
   else
   {
      mouseOverView = v;
      v->mouseEntered(mouse);
   }
}

void MTWindow::mouseDragged(ofMouseEventArgs& mouse)
{
   if (!isMouseDragging)
   {
      isMouseDragging = true;
      mouseButtonInUse = mouse.button;
      mouseDragStart = glm::vec2(mouse.x, mouse.y);
   }

   mouseOverView->mouseDragged(mouse);
}

void MTWindow::mousePressed(ofMouseEventArgs& mouse)
{
   isMouseDown = true;
   mouseButtonInUse = mouse.button;
   mouseDownPos = glm::vec2(mouse.x, mouse.y);

   mouseOverView->mousePressed(mouse);
   setFocusedView(mouseOverView);
}

void MTWindow::mouseReleased(ofMouseEventArgs& mouse)
{
   isMouseDown = false;
   isMouseDragging = false;
   mouseUpPos = glm::vec2(mouse.x, mouse.y);
   mouseButtonInUse = mouse.button;

   mouseOverView->mouseReleased(mouse);
}

/// TODO: Scrolling in MTWindow
void MTWindow::mouseScrolled(ofMouseEventArgs& mouse)
{
   mouseButtonInUse = mouse.button;

   mouseOverView->mouseScrolled(mouse);
}

void MTWindow::mouseEntered(ofMouseEventArgs& mouse)
{
   mouseButtonInUse = mouse.button;
}

void MTWindow::mouseExited(ofMouseEventArgs& mouse)
{
   mouseButtonInUse = mouse.button;
}

void MTWindow::dragged(ofDragInfo& drag)
{
}

void MTWindow::messageReceived(ofMessage& message)
{
}

void MTWindow::modelLoaded(ofEventArgs& args)
{
   //enqueueUpdateOperation(
   //    [this]()
   //    {
   //       modelLoaded();
   //       onModelLoaded();
   //    });

   contentView->modelLoaded(args);
}

// TODO: Touch
void MTWindow::touchDown(ofTouchEventArgs& touch)
{
}

void MTWindow::touchMoved(ofTouchEventArgs& touch)
{
}

void MTWindow::touchUp(ofTouchEventArgs& touch)
{
}

void MTWindow::touchDoubleTap(ofTouchEventArgs& touch)
{
}

void MTWindow::touchCancelled(ofTouchEventArgs& touch)
{
}

void MTWindow::setFullscreen(bool fullscreen)
{
	ofAppGLFWWindow::setFullscreen(fullscreen);
	fullScreenActive = fullscreen;
	ofLogVerbose("MTWindow") << "Full screen status changed to: " << fullScreenActive;
}

void MTWindow::setFocusedView(MTView* view)
{
   if (!view->wantsFocus) return;  // Exit if the view doesn't want focus

   auto fv = focusedView;
   if (fv)
   {
      if (fv != view)
      {
         focusedView = view;
         view->focusGained.notify(voidEventArgs);
         fv->focusLost.notify(voidEventArgs);
      }
   }
}

MTView* MTWindow::getFocusedView()
{
   auto fv = focusedView;
   if (fv)
   {
      return fv;
   }
   return nullptr;
}

int MTWindow::getWidth()
{
   // line 1715 in ofGLProgrammableRenderer.cpp needs to be addressed
   // before this approach works... perhaps I'll have to use my own renderer, yuk.

//	if (auto fv = focusedView.lock())
//	{
//		return fv->getFrameSize().y;
//	}
//	else
#ifndef TARGET_RASPBERRY_PI
   return ofAppGLFWWindow::getWidth();
#else
   return ofAppEGLWindow::getWidth();
#endif
}

int MTWindow::getHeight()
{
//	if (auto fv = focusedView.lock())
//	{
//		return fv->getFrameSize().y;
//	}
//	else
#ifndef TARGET_RASPBERRY_PI
   return ofAppGLFWWindow::getHeight();
#else
   return ofAppEGLWindow::getHeight();
#endif
}

void MTWindow::removeAllEvents()
{
}

void MTWindow::addAllEvents()
{
}

#pragma mark ImGui

std::shared_ptr<ofxImGui::Gui> MTWindow::getGui()
{
   return gui;
}

void MTWindow::setImGuiEnabled(bool doGui, ImGuiConfigFlags customFlags)
{
   if (isImGuiEnabled == doGui) return;

   if (doGui)
   {
      //enqueueUpdateOperation([this, doGui]()
      //					   {
      isImGuiEnabled = doGui;
      // Attempting a workaround for ImGUI to work on multiple windows but it just
      // doesn't work
      ImGui::SetCurrentContext(NULL);
      gui = std::make_shared<ofxImGui::Gui>();
      gui->setup(nullptr, false, customFlags, true, false);
      imCtx = ImGui::GetCurrentContext();
      //});
   }
   else
   {
      //enqueueUpdateOperation([this, doGui]()
      //{
      isImGuiEnabled = doGui;
      ImGui::SetCurrentContext(imCtx);
      gui.reset();
      imCtx = nullptr;
      //});
   }
}

//-------------------------------------------------------------
// HACKY THINGS TO GET ofGetWidth AND ofGetHeight TO WORK
// WITH THE FRAMEWORK
// There should be no need for you to call these
//-------------------------------------------------------------

/**
 * @brief mt_rotateMouseXY mimics rotateMouseXY in ofAppGLFWWindow, which is
 * inaccessible from this context. This is purely an internal function, there
 * should be no need to call it.
 */
static void mt_rotateMouseXY(ofOrientation orientation, int w, int h, double& x, double& y)
{
   int savedY;
   switch (orientation)
   {
   case OF_ORIENTATION_180:
      x = w - x;
      y = h - y;
      break;

   case OF_ORIENTATION_90_RIGHT:
      savedY = y;
      y = x;
      x = w - savedY;
      break;

   case OF_ORIENTATION_90_LEFT:
      savedY = y;
      y = h - x;
      x = savedY;
      break;

   case OF_ORIENTATION_DEFAULT:
   default: break;
   }
}

#ifndef TARGET_RASPBERRY_PI

void MTWindow::mt_motion_cb(GLFWwindow* windowP_, double x, double y)
{
   ofAppGLFWWindow* instance = static_cast<ofAppGLFWWindow*>(glfwGetWindowUserPointer(windowP_));

   MTWindow* mtWindow = static_cast<MTWindow*>(instance);

   std::shared_ptr<ofMainLoop> mainLoop = ofGetMainLoop();

   if (mainLoop)
   {
      mainLoop->setCurrentWindow(instance);
   }
   instance->makeCurrent();

   auto dims = instance->getWindowSize();
   mt_rotateMouseXY(instance->getOrientation(), dims.x, dims.y, x, y);

   if (!mtWindow->isMouseDown)
   {
      instance->events().notifyMouseMoved(x * instance->getPixelScreenCoordScale(), y * instance->getPixelScreenCoordScale());
   }
   else
   {
      instance->events().notifyMouseDragged(x * instance->getPixelScreenCoordScale(),
                                            y * instance->getPixelScreenCoordScale(),
                                            mtWindow->mouseButtonInUse);
   }
}

void MTWindow::mt_focus_callback(GLFWwindow* glfWwindow, int isFocused)
{
   ofAppGLFWWindow* instance = static_cast<ofAppGLFWWindow*>(glfwGetWindowUserPointer(glfWwindow));

   MTWindow* mtWindow = static_cast<MTWindow*>(instance);
   if (mtWindow)
   {
      MTWindowEventArgs focusArgs;
      focusArgs.window = mtWindow->shared_from_this();

      if (isFocused == GLFW_TRUE)
      {
         mtWindow->windowFocusGainedEvent.notify(focusArgs);
      }
      else
      {
         mtWindow->windowFocusLostEvent.notify(focusArgs);
      }
   }
}

#endif
