//
//  ofxMTAppFramework.h
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 3/11/16.
//
//

#ifndef MTWindow_h
#define MTWindow_h

#include "MTWindow.hpp"
#include "MTAppMode.hpp"
#include "MTApp.hpp"
#include "MTModel.hpp"
#include "MTView.hpp"

static ofEventArgs voidEventArgs;

MTWindow::MTWindow(string name)
{
	contentView = std::make_shared<MTView>("root");
	focusedView = contentView;
	mouseOverView = contentView;
	this->name.set("Window Name", name);
}

MTWindow::~MTWindow()
{
}

// void ofxMTWindow::setup(ofEventArgs & args)
//{
//    contentView->setup(args);
//}

#ifndef TARGET_OPENGLES
void MTWindow::setup(const ofGLFWWindowSettings& settings)
{
	ofAppGLFWWindow::setup(settings);
	glfwSetCursorPosCallback(getGLFWWindow(), nullptr);
	glfwSetCursorPosCallback(getGLFWWindow(), &MTWindow::mt_motion_cb);
}
#else
void ofxMTWindow::setup(const ofGLESWindowSettings& settings)
{
	ofAppGLESWindow::setup(settings);
}
#endif

//------------------------------------------------------//
// INTERNALS EVENT LISTENERS //
//------------------------------------------------------//

void MTWindow::setupInternal(ofEventArgs& args)
{
	contentView->setSize(ofAppGLFWWindow::getWidth(),
						 ofAppGLFWWindow::getHeight());
	contentView->setFrameOrigin(glm::vec3(0, 0, 0));
	contentView->setup(args);
}

void MTWindow::update(ofEventArgs& args)
{
	contentView->update(args);
}

void MTWindow::draw(ofEventArgs& args)
{
	ofSetupScreenPerspective(ofAppGLFWWindow::getWidth(),
							 ofAppGLFWWindow::getHeight());
	ofBackground(0);
	contentView->draw(args);
}

void MTWindow::exit(ofEventArgs& args)
{
	MTApp::sharedApp->windowClosing(this);
	contentView->exit(args);
	contentView = nullptr;
}

void MTWindow::windowResized(ofResizeEventArgs& resize)
{
	auto size = this->getWindowSize();
	ofViewport(0, 0, size.x, size.y);
	contentView->setFrameSize(resize.width, resize.height);
	contentView->windowResized(resize);
}

void MTWindow::keyPressed(ofKeyEventArgs& key)
{
	auto fv = focusedView.lock();
	if (fv)
	{
		fv->keyPressed(key);
	}
	this->keyPressed(key.key);
}

void MTWindow::keyReleased(ofKeyEventArgs& key)
{
	auto fv = focusedView.lock();
	if (fv)
	{
		fv->keyReleased(key);
	}
	this->keyReleased(key.key);
}

void MTWindow::mouseMoved(ofMouseEventArgs& mouse)
{
	auto v = contentView->hitTest(mouse);
	if (auto mo = mouseOverView.lock())
	{
		if ((v != mo) && mo)
		{
			mo->mouseReleased(mouse);
			mo->mouseExited(mouse);
			mouseOverView = v;
			v->mouseEntered(mouse);
		}
		mo->mouseMoved(mouse);
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
		mouseDragStart = mouse.xy();
	}

	if (auto mo = mouseOverView.lock())
	{
		mo->mouseDragged(mouse);
	}
}

void MTWindow::mousePressed(ofMouseEventArgs& mouse)
{
	isMouseDown = true;
	mouseButtonInUse = mouse.button;
	mouseDownPos = mouse.xy();

	if (auto mo = mouseOverView.lock())
	{
		mo->mousePressed(mouse);
		setFocusedView(mo);
	}
}

void MTWindow::mouseReleased(ofMouseEventArgs& mouse)
{
	isMouseDown = false;
	isMouseDragging = false;
	mouseUpPos = mouse.xy();
	mouseButtonInUse = mouse.button;
	if (auto mo = mouseOverView.lock())
	{
		mo->mouseReleased(mouse);
	}
}

/// TODO: Scrolling in MTWindow
void MTWindow::mouseScrolled(ofMouseEventArgs& mouse)
{
	mouseButtonInUse = mouse.button;
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
	enqueueUpdateOperation([this]() { modelLoaded(); });

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

void MTWindow::setFocusedView(std::shared_ptr<MTView> view)
{
	if (!view->wantsFocus)
		return;   // Exit if the view doesn't want focus

	auto fv = focusedView.lock();
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

int MTWindow::getWidth()
{
	// line 1715 in ofGLProgrammableRenderer.cpp needs to be addressed
	// before this approach works... perhaps I'll have to use my own renderer, yuk.

//	if (auto fv = focusedView.lock())
//	{
//		return fv->getFrameSize().y;
//	}
//	else
	{
		return ofAppGLFWWindow::getWidth();
	}
}

int MTWindow::getHeight()
{
//	if (auto fv = focusedView.lock())
//	{
//		return fv->getFrameSize().y;
//	}
//	else
	{
		return ofAppGLFWWindow::getHeight();
	}
}

void MTWindow::removeAllEvents()
{
}

void MTWindow::addAllEvents()
{
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
static void mt_rotateMouseXY(ofOrientation orientation,
							 int w,
							 int h,
							 double& x,
							 double& y)
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
		default:
			break;
	}
}

void MTWindow::mt_motion_cb(GLFWwindow* windowP_, double x, double y)
{
	ofAppGLFWWindow* instance =
	  static_cast<ofAppGLFWWindow*>(glfwGetWindowUserPointer(windowP_));

	MTWindow* mtWindow = static_cast<MTWindow*>(instance);

	shared_ptr<ofMainLoop> mainLoop = ofGetMainLoop();

	if (mainLoop)
	{
		mainLoop->setCurrentWindow(instance);
	}
	instance->makeCurrent();

	auto dims = instance->getWindowSize();
	mt_rotateMouseXY(instance->getOrientation(), dims.x, dims.y, x, y);

	if (!mtWindow->isMouseDown)
	{
		instance->events().notifyMouseMoved(
		  x * instance->getPixelScreenCoordScale(),
		  y * instance->getPixelScreenCoordScale());
	}
	else
	{
		instance->events().notifyMouseDragged(
		  x * instance->getPixelScreenCoordScale(),
		  y * instance->getPixelScreenCoordScale(),
		  mtWindow->mouseButtonInUse);
	}
}

#endif /* MTWindow_h */
