//
//  ofxMTAppFramework.h
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 3/11/16.
//
//

#ifndef ofxMTWindow_h
#define ofxMTWindow_h

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

//void ofxMTWindow::setup(ofEventArgs & args)
//{
//    contentView->setup(args);
//}

#ifndef TARGET_OPENGLES
void MTWindow::setup(const ofGLFWWindowSettings & settings)
{
    ofAppGLFWWindow::setup(settings);
}
#else
void ofxMTWindow::setup(const ofGLESWindowSettings & settings)
{
    ofAppGLESWindow::setup(settings);
}
#endif

//------------------------------------------------------//
// INTERNALS EVENT LISTENERS 							//
//------------------------------------------------------//

void MTWindow::setupInternal(ofEventArgs & args)
{
    contentView->setSize(ofAppGLFWWindow::getWidth(),
                         ofAppGLFWWindow::getHeight());
    contentView->setFrameOrigin(glm::vec3(0,0,0));
    contentView->setup(args);
}

void MTWindow::update(ofEventArgs & args)
{
    contentView->update(args);
}

void MTWindow::draw(ofEventArgs & args)
{
    ofSetupScreenPerspective(ofAppGLFWWindow::getWidth(),
                             ofAppGLFWWindow::getHeight());
    ofBackground(0);
    contentView->draw(args);
}

void MTWindow::exit(ofEventArgs & args)
{
    contentView->exit(args);
    contentView = nullptr;
}

void MTWindow::windowResized(ofResizeEventArgs & resize)
{
    contentView->setFrameSize(resize.width, resize.height);
    contentView->windowResized(resize);
}

void MTWindow::keyPressed( ofKeyEventArgs & key )
{
    auto fv = focusedView.lock();
    if (fv)
    {
        fv->keyPressed(key);
    }
	this->keyPressed(key.key);
}

void MTWindow::keyReleased( ofKeyEventArgs & key )
{
    auto fv = focusedView.lock();
    if (fv)
    {
        fv->keyReleased(key);
    }
    this->keyReleased(key.key);
}

void MTWindow::mouseMoved( ofMouseEventArgs & mouse )
{
    auto v = contentView->hitTest(mouse);
	if(auto mo = mouseOverView.lock())
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

void MTWindow::mouseDragged( ofMouseEventArgs & mouse )
{
    if (!isMouseDragging)
    {
        isMouseDragging = true;
        mouseDragStart = mouse.xy();
    }
	
	if(auto mo = mouseOverView.lock())
	{
		mo->mouseDragged(mouse);
	}
}

void MTWindow::mousePressed( ofMouseEventArgs & mouse )
{
    isMouseDown = true;
    mouseDownPos = mouse.xy();

    if(auto mo = mouseOverView.lock())
	{
		mo->mousePressed(mouse);
		setFocusedView(mo);
	}
}

void MTWindow::mouseReleased(ofMouseEventArgs & mouse)
{
    isMouseDown = false;
    isMouseDragging = false;
    mouseUpPos = mouse.xy();
	if(auto mo = mouseOverView.lock())
	{
		mo->mouseReleased(mouse);
	}
}

void MTWindow::mouseScrolled( ofMouseEventArgs & mouse ){}
void MTWindow::mouseEntered( ofMouseEventArgs & mouse ){}
void MTWindow::mouseExited( ofMouseEventArgs & mouse ){}
void MTWindow::dragged(ofDragInfo & drag){}
void MTWindow::messageReceived(ofMessage & message){}

void MTWindow::modelLoaded(ofEventArgs &args)
{
    enqueueUpdateOperation([this]()
    {
        modelLoaded();
    });

    contentView->modelLoaded(args);
}

//TODO: Touch
void MTWindow::touchDown(ofTouchEventArgs & touch){}
void MTWindow::touchMoved(ofTouchEventArgs & touch){}
void MTWindow::touchUp(ofTouchEventArgs & touch){}
void MTWindow::touchDoubleTap(ofTouchEventArgs & touch){}
void MTWindow::touchCancelled(ofTouchEventArgs & touch){}

void MTWindow::setFocusedView(std::shared_ptr<MTView> view)
{
	if (!view->wantsFocus) return; // Exit if the view doesn't want focus
	
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
    return ofAppGLFWWindow::getWidth();
}

int MTWindow::getHeight()
{
    return ofAppGLFWWindow::getHeight();
}

void MTWindow::removeAllEvents()
{

}

void MTWindow::addAllEvents()
{

}

#endif /* ofxMTWindow_h */
