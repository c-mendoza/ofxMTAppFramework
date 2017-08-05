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
    ofxMTWindow(string name);
    ~ofxMTWindow();

   ofParameter<string> name;

   shared_ptr<ofxMTView> contentView;

    int mouseX, mouseY;			// for processing heads

    bool isMouseDown = false;
    bool isMouseDragging = true;

    ////////////////////////////////
    /// Events to override if needed
    ////////////////////////////////

    /// \brief Default implementation calls modelDidLoad() of
    ///  the content view
    virtual void modelDidLoad();


    virtual void windowResized(int w, int h){}
    virtual void keyPressed( int key ){}
    virtual void keyReleased( int key ){}

    /// \brief Called when the app mode has been changed.
    /// Default implementation does nothing. Override this
    /// method if you want to respond to mode changes.
    virtual void appModeChanged(MTAppModeChangeArgs & modeChange){}


    ////// INTERNALS
    //  DO NOT OVERRIDE:
    // TODO: Address TARGET_OPENGLES
#ifndef TARGET_OPENGLES
    virtual void setup(const ofGLFWWindowSettings & settings);
#else
    virtual void setup(const ofGLESWindowSettings & settings);
#endif
//	void setup();
//	void update();
//	void draw();
//	void exit();
    void update(ofEventArgs & args);
    void draw(ofEventArgs & args);
    void exit(ofEventArgs & args);

    void windowResized(ofResizeEventArgs & resize);
    void keyPressed( ofKeyEventArgs & key );
    void keyReleased( ofKeyEventArgs & key );
    void mouseMoved( ofMouseEventArgs & mouse );
    void mouseDragged( ofMouseEventArgs & mouse );
    void mousePressed( ofMouseEventArgs & mouse );
    void mouseReleased(ofMouseEventArgs & mouse);
    void mouseScrolled( ofMouseEventArgs & mouse );
    void mouseEntered( ofMouseEventArgs & mouse );
    void mouseExited( ofMouseEventArgs & mouse );
    void dragged(ofDragInfo & drag);
    void messageReceived(ofMessage & message);

    //TODO: Touch
    virtual void touchDown(ofTouchEventArgs & touch);
    virtual void touchMoved(ofTouchEventArgs & touch);
    virtual void touchUp(ofTouchEventArgs & touch);
    virtual void touchDoubleTap(ofTouchEventArgs & touch);
    virtual void touchCancelled(ofTouchEventArgs & touch);

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

//    void updateMatrix();


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

    std::weak_ptr<ofxMTView> focusedView;
};

#endif /* ofxMTWindow_hpp */
