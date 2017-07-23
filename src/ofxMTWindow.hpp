//
//  ofxMTView.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#ifndef ofxMTWindow_hpp
#define ofxMTWindow_hpp

#include "ofxMTAppFramework.h"

class ofxMTModel;
class MTAppModeChangeArgs;
class ofxMTView;

#ifndef TARGET_OPENGLES
class ofxMTWindow :
        public ofAppGLFWWindow,
        public MTEventListenerStore
{
#else
class ofxMTWindow :
        public ofAppBaseGLESWindow,
        public MTEventListenerStore
{
#endif
public:
    ofxMTWindow();
    ~ofxMTWindow();

   ofParameter<string> name;

   shared_ptr<ofxMTView> contentView;

    virtual void dragEvent(ofDragInfo dragInfo) { }
    virtual void gotMessage(ofMessage msg){ }

    int mouseX, mouseY;			// for processing heads

    bool isMouseDown = false;
    bool isMouseDragging = true;

    //DO NOT OVERRIDE:
    virtual void setup(ofEventArgs & args);
    virtual void update(ofEventArgs & args);
    virtual void draw(ofEventArgs & args);
    virtual void exit(ofEventArgs & args);

    virtual void windowResized(ofResizeEventArgs & resize);
    virtual void keyPressed( ofKeyEventArgs & key );
    virtual void keyReleased( ofKeyEventArgs & key );
    virtual void mouseMoved( ofMouseEventArgs & mouse );
    virtual void mouseDragged( ofMouseEventArgs & mouse );
    virtual void mousePressed( ofMouseEventArgs & mouse );
    virtual void mouseReleased(ofMouseEventArgs & mouse);
    virtual void mouseScrolled( ofMouseEventArgs & mouse );
    virtual void mouseEntered( ofMouseEventArgs & mouse );
    virtual void mouseExited( ofMouseEventArgs & mouse );
    virtual void dragged(ofDragInfo & drag);
    virtual void messageReceived(ofMessage & message);

    //TODO: Touch
    virtual void touchDown(ofTouchEventArgs & touch);
    virtual void touchMoved(ofTouchEventArgs & touch);
    virtual void touchUp(ofTouchEventArgs & touch);
    virtual void touchDoubleTap(ofTouchEventArgs & touch);
    virtual void touchCancelled(ofTouchEventArgs & touch);

    /// \brief Called when the app mode has been changed.
    /// Default implementation does nothing. Override this
    /// method if you want to respond to mode changes.
    virtual void appModeChanged(MTAppModeChangeArgs & modeChange){}


    ////// INTERNALS

    void enqueueDrawOperation(function<void()> funct)
    {
        drawOpQueue.push(funct);
    }

    void enqueueUpdateOperation(function<void()> f)
    {
        updateOpQueue.push(f);
    }


protected:
//	shared_ptr<ofxMTModel> model;

    vector<ofEventListener> eventListeners;

    ofParameter<bool> scrollbarsVisible;

    std::unique_ptr<ofxMTView> rootView;
    //Transform

    void removeAllEvents();
    void addAllEvents();

    void updateMatrix();


private:
    /// This function is called internally by the framework to signal that a model
    /// has been loaded from a file. You don't need to call it.
    void modelDidLoadInternal()
    {
        enqueueUpdateOperation([this]()
        {
            modelDidLoad();
        });
    }

    ofMatrix4x4 transMatrix;
    ofMatrix4x4 invTransMatrix; //Just a cached value

    queue<function<void()>> updateOpQueue;
    queue<function<void()>> drawOpQueue;
};

#endif /* ofxMTWindow_hpp */
