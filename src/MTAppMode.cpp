//
//  ofxMTAppMode.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 6/7/16.
//
//

#include "MTAppMode.hpp"

MTAppMode::~MTAppMode()
{
    eventListeners.clear();
}

void MTAppMode::addAllEventListeners()
{
    addEventListener(view->keyPressedEvent.newListener(
        [this](ofKeyEventArgs& args) { keyPressed(args.key); }));

    addEventListener(view->keyReleasedEvent.newListener(
        [this](ofKeyEventArgs& args) { keyReleased(args.key); }));

    addEventListener(view->mousePressedEvent.newListener(
        [this](ofMouseEventArgs& args) {
            mousePressed(args.x, args.y, args.button);
        }));

    addEventListener(view->mouseReleasedEvent.newListener(
        [this](ofMouseEventArgs& args) {
            mouseReleased(args.x, args.y, args.button);
        }));

    addEventListener(view->mouseMovedEvent.newListener(
        [this](ofMouseEventArgs& args) {
            mouseMoved(args.x, args.y);
        }));

    addEventListener(view->mouseDraggedEvent.newListener(
        [this](ofMouseEventArgs& args) {
            mouseDragged(args.x, args.y, args.button);
        }));

    addEventListener(view->mouseDraggedEndEvent.newListener(
        [this](ofMouseEventArgs& args) {
            mouseDraggedEnd(args.x, args.y, args.button);
        }));

    addEventListener(view->mouseEnteredEvent.newListener(
        [this](ofMouseEventArgs& args) {
            mouseEntered(args.x, args.y);
        }));

    addEventListener(view->mouseExitedEvent.newListener(
        [this](ofMouseEventArgs& args) {
            mouseExited(args.x, args.y);
        }));

    addEventListener(view->windowResizedEvent.newListener(
        [this](ofResizeEventArgs& args) {
            windowResized(args.width, args.height);
        }));

//    addEventListener(view->drawEvent.newListener(
//        [this](ofEventArgs& args) {
//                         draw();
//        }));

//    addEventListener(view->updateEvent.newListener(
//        [this](ofEventArgs& args) {
//                         update();
//        }));


}
