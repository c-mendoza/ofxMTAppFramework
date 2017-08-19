//
//  ofxMTAppFramework.h
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 3/11/16.
//
//

#ifndef ofxMTWindow_h
#define ofxMTWindow_h

#include "ofxMTWindow.hpp"
#include "MTAppMode.hpp"
#include "MTApp.hpp"
#include "MTModel.hpp"
#include "MTView.hpp"

static ofEventArgs voidEventArgs;

ofxMTWindow::ofxMTWindow(string name)
{
    contentView = std::make_shared<MTView>("root");
    focusedView = contentView;
    mouseOverView = contentView;
    this->name.set("Window Name", name);
}

ofxMTWindow::~ofxMTWindow()
{

}

//void ofxMTWindow::setup(ofEventArgs & args)
//{
//    contentView->setup(args);
//}

#ifndef TARGET_OPENGLES
void ofxMTWindow::setup(const ofGLFWWindowSettings & settings)
{
    ofAppGLFWWindow::setup(settings);
}
#else
void ofxMTWindow::setup(const ofGLESWindowSettings & settings)
{
    ofAppGLESWindow::setup(settings);
}
#endif

//void ofxMTWindow::update()
//{
//	ofAppGLFWWindow::update();
//}
//
//void ofxMTWindow::draw()
//{
//	ofAppGLFWWindow::draw();
//}

void ofxMTWindow::setupInternal(ofEventArgs & args)
{
    contentView->setSize(ofAppGLFWWindow::getWidth(),
                         ofAppGLFWWindow::getHeight());
    contentView->setFrameOrigin(glm::vec3(0,0,0));
    contentView->setup(args);
}

void ofxMTWindow::update(ofEventArgs & args)
{
    contentView->update(args);
}

void ofxMTWindow::draw(ofEventArgs & args)
{
    ofSetupScreenPerspective(ofAppGLFWWindow::getWidth(),
                             ofAppGLFWWindow::getHeight());
    ofBackground(0);
    contentView->draw(args);
}

void ofxMTWindow::exit(ofEventArgs & args)
{
    contentView->exit(args);
    contentView = nullptr;
}

void ofxMTWindow::windowResized(ofResizeEventArgs & resize)
{
    contentView->setFrameSize(resize.width, resize.height);
    contentView->windowResized(resize);
}

void ofxMTWindow::modelDidLoad()
{
    contentView->modelDidLoad();
}

void ofxMTWindow::keyPressed( ofKeyEventArgs & key )
{
    auto fv = focusedView.lock();
    if (fv)
    {
        fv->keyPressed(key);
    }
}

void ofxMTWindow::keyReleased( ofKeyEventArgs & key )
{
    auto fv = focusedView.lock();
    if (fv)
    {
        fv->keyReleased(key);
    }
    this->keyReleased(key.key);
}

void ofxMTWindow::mouseMoved( ofMouseEventArgs & mouse )
{
    auto v = contentView->hitTest(mouse);
    auto mo = mouseOverView.lock();
    if (v != mo)
    {
        mo->mouseReleased(mouse);
        mo->mouseExited(mouse);
        mouseOverView = v;
        v->mouseEntered(mouse);
    }

    mouseOverView.lock()->mouseMoved(mouse);
}

void ofxMTWindow::mouseDragged( ofMouseEventArgs & mouse )
{
    if (!isMouseDragging)
    {
        isMouseDragging = true;
        mouseDragStart = mouse.xy();
    }

    auto mo = mouseOverView.lock();
    mo->mouseDragged(mouse);
}

void ofxMTWindow::mousePressed( ofMouseEventArgs & mouse )
{
    isMouseDown = true;
    mouseDownPos = mouse.xy();

    auto mo = mouseOverView.lock();
    mo->mousePressed(mouse);
    setFocusedView(mo);
}

void ofxMTWindow::mouseReleased(ofMouseEventArgs & mouse)
{
    isMouseDown = false;
    isMouseDragging = false;
    mouseUpPos = mouse.xy();
    auto mo = mouseOverView.lock();
    mo->mouseReleased(mouse);
}

void ofxMTWindow::mouseScrolled( ofMouseEventArgs & mouse ){}
void ofxMTWindow::mouseEntered( ofMouseEventArgs & mouse ){}
void ofxMTWindow::mouseExited( ofMouseEventArgs & mouse ){}
void ofxMTWindow::dragged(ofDragInfo & drag){}
void ofxMTWindow::messageReceived(ofMessage & message){}

//TODO: Touch
void ofxMTWindow::touchDown(ofTouchEventArgs & touch){}
void ofxMTWindow::touchMoved(ofTouchEventArgs & touch){}
void ofxMTWindow::touchUp(ofTouchEventArgs & touch){}
void ofxMTWindow::touchDoubleTap(ofTouchEventArgs & touch){}
void ofxMTWindow::touchCancelled(ofTouchEventArgs & touch){}

void ofxMTWindow::setFocusedView(std::shared_ptr<MTView> view)
{
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

int ofxMTWindow::getWidth()
{
    return ofAppGLFWWindow::getWidth();
}

int ofxMTWindow::getHeight()
{
    return ofAppGLFWWindow::getHeight();
}

void ofxMTWindow::removeAllEvents()
{

}

void ofxMTWindow::addAllEvents()
{

}

#endif /* ofxMTWindow_h */
