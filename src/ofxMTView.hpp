//
//  ofxMTView.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#ifndef ofxMTView_hpp
#define ofxMTView_hpp

#include "ofxMTAppFramework.h"

class ofxMTModel;
class MTAppModeChangeArgs;

class ofxMTView {

public:
    ofxMTView(string _name);
    ~ofxMTView();
//	void setModel(shared_ptr<ofxMTModel> model);
//	shared_ptr<ofxMTModel> getModel() { return model; }
    void setName(string viewName);
    string getName() { return name; }
    void setWindow(shared_ptr<ofAppBaseWindow> window);

    /// Use this to add event listeners that should be destroyed when this view is destroyed.
    void addEventListener(ofEventListener&& el) { eventListeners.push_back(move(el)); }


    /// Called once the model is successfully loaded from file.
    /// Default implementation does nothing.
    virtual void modelDidLoad(){}
    virtual void setup(){}
    virtual void update(){}
    virtual void draw(){}
    virtual void exit(){}
    virtual void windowResized(int w, int h){}
    virtual void keyPressed( int key ){}
    virtual void keyReleased( int key ){}

    ///Transform a point from view-coordinate to content-coordinate
    ofVec3f viewToContent(ofVec3f viewCoord)
    {
        return viewCoord * invTransMatrix;
    }

    ///Transform a point from view-coordinate to content-coordinate
    ofVec3f viewToContent(float x, float y)
    {
        return viewToContent(ofVec3f(x, y, 0));
    }

    ofVec3f contentToView(ofVec3f contentCoord)
    {
        return contentCoord * transMatrix;
    }

    ofVec3f contentToView(float x, float y)
    {
        return contentToView(ofVec3f(x, y, 0));
    }

    /// Moves the content within the view's frame by dx and dy.
    void scrollBy(float dx, float dy);

    /// Moves the content to the specified view coordinates.
    void scrollTo(float x, float y);

    /// \brief Positive values zoom in, negative zoom out. Zoom=1 is the default.
    void zoomTo(float scale);

    /// \brief Change the view's zoom (scale) by a relative value.
    void zoomBy(float zoomChange);

    void setContentFrame(ofRectangle frame)
    {
        contentFrame = frame;
        updateScrollbars();
    }

    ///Returns a reference to the content frame.
    const ofRectangle& getContentFrame()
    {
        return contentFrame;
    }

    void setScrollbarsVisible(bool visible) { bScrollbarsVisible = visible; }
    void updateScrollbars() {}


    /// \brief Called on the active window when the mouse is moved
    virtual void mouseMoved( int x, int y ){}

    /// \brief Called on the active window when the mouse is dragged, i.e.
    /// moved with a button pressed
    virtual void mouseDragged( int x, int y, int button ) {
//		int dx = x - ofGetPreviousMouseX();
//		int dy = y - ofGetPreviousMouseY();
//
//
//		contentPosition.x += dx;
//		contentPosition.y += dy;
//		ofVec3f mouse;
//		mouse.x = x;
//		mouse.y = y;
//
//		updateMatrix();
//		ofLogNotice("mouseDragged") << mouse * transMatrix.getInverse();
        //

    }

    /// \brief Called on the active window when a mouse button is pressed
    virtual void mousePressed( int x, int y, int button ){}

    /// \brief Called on the active window when a mouse button is released
    virtual void mouseReleased(int x, int y, int button ){}

    /// \brief Called on the active window when the mouse wheel is scrolled
    virtual void mouseScrolled(int x, int y, float scrollX, float scrollY ){}

    /// \brief Called on the active window when the mouse cursor enters the
    /// window area
    ///
    /// Note that the mouse coordinates are the last known x/y before the
    /// event occurred, i.e. from the previous frame
    virtual void mouseEntered( int x, int y ){}

    /// \brief Called on the active window when the mouse cursor leaves the
    /// window area
    ///
    /// Note that the mouse coordinates are the last known x/y before the
    /// event occurred, i.e. from the previous frame
    virtual void mouseExited( int x, int y){}

    virtual void dragEvent(ofDragInfo dragInfo) { }
    virtual void gotMessage(ofMessage msg){ }

    int mouseX, mouseY;			// for processing heads

    bool isMouseDown = false;
    bool isMouseDragging = true;

    ofParameter<ofColor> scrollbarColor;

    virtual void touchDown(int x, int y, int id) {};
    virtual void touchMoved(int x, int y, int id) {};
    virtual void touchUp(int x, int y, int id) {};
    virtual void touchDoubleTap(int x, int y, int id) {};
    virtual void touchCancelled(int x, int y, int id) {};


    //DO NOT OVERRIDE:
    virtual void setup(ofEventArgs & args){
        setup();
    }
    virtual void update(ofEventArgs & args);
    virtual void draw(ofEventArgs & args);
    virtual void exit(ofEventArgs & args);

    virtual void windowResized(ofResizeEventArgs & resize){
        windowResized(resize.width,resize.height);
    }

    virtual void keyPressed( ofKeyEventArgs & key ){
        keyPressed(key.key);
    }
    virtual void keyReleased( ofKeyEventArgs & key ){
        keyReleased(key.key);
    }

    virtual void mouseMoved( ofMouseEventArgs & mouse ){
        mouseX=mouse.x;
        mouseY=mouse.y;
        contentMouse = viewToContent(mouse);
        mouseMoved(mouse.x,mouse.y);
    }

    virtual void mouseDragged( ofMouseEventArgs & mouse ){
        mouseX=mouse.x;
        mouseY=mouse.y;
        contentMouse = viewToContent(mouse);
        mouseDragged(mouse.x,mouse.y,mouse.button);
        isMouseDragging = true;
    }
    virtual void mousePressed( ofMouseEventArgs & mouse ){
        mouseX=mouse.x;
        mouseY=mouse.y;
        contentMouse = viewToContent(mouse);
        isMouseDown = true;
        mousePressed(mouse.x,mouse.y,mouse.button);
    }
    virtual void mouseReleased(ofMouseEventArgs & mouse){
        mouseX=mouse.x;
        mouseY=mouse.y;
        contentMouse = viewToContent(mouse);
        isMouseDown = false;
        mouseReleased(mouse.x,mouse.y,mouse.button);
        isMouseDragging = false;
    }
    virtual void mouseScrolled( ofMouseEventArgs & mouse ){
        mouseScrolled(mouse.x, mouse.y, mouse.scrollX, mouse.scrollY);
    }
    virtual void mouseEntered( ofMouseEventArgs & mouse ){
        mouseEntered(mouse.x,mouse.y);
        mouseX=mouse.x;
        mouseY=mouse.y;
        contentMouse = viewToContent(mouse);
    }
    virtual void mouseExited( ofMouseEventArgs & mouse ){
        mouseX=mouse.x;
        mouseY=mouse.y;
        contentMouse = viewToContent(mouse);
        mouseExited(mouse.x,mouse.y);
    }
    virtual void dragged(ofDragInfo & drag){
        dragEvent(drag);
    }
    virtual void messageReceived(ofMessage & message){
        gotMessage(message);
    }

    //TODO: Touch
    virtual void touchDown(ofTouchEventArgs & touch) {
        touchDown(touch.x, touch.y, touch.id);
    }
    virtual void touchMoved(ofTouchEventArgs & touch) {
        touchMoved(touch.x, touch.y, touch.id);
    }
    virtual void touchUp(ofTouchEventArgs & touch) {
        touchUp(touch.x, touch.y, touch.id);
    }
    virtual void touchDoubleTap(ofTouchEventArgs & touch) {
        touchDoubleTap(touch.x, touch.y, touch.id);
    }
    virtual void touchCancelled(ofTouchEventArgs & touch){
        touchCancelled(touch.x, touch.y, touch.id);
    }


    virtual void appModeChanged(MTAppModeChangeArgs & modeChange){}

    shared_ptr<ofAppBaseWindow> getWindow();
    const ofMatrix4x4 & getTransformationMatrix() { return transMatrix; }
    const ofMatrix4x4 & getInverseTransformationMatrix() { return invTransMatrix; }

    ///Returns the mouse position in content (local) coordinates
    const ofVec3f & getContentMouse() { return contentMouse; }
    shared_ptr<ofxMTAppMode> currentAppMode; //?

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
    shared_ptr<ofAppBaseWindow> window;
    string name;

    vector<ofEventListener> eventListeners;

    bool bScrollbarsVisible = false;

    //Transform
    ofRectangle contentFrame;
    ofVec3f contentPosition;  //TODO: Rename to localOrigin?//Figure out how to make this a param?
    ofVec3f contentMouse; //localMousePos?
    ofParameter<float>	contentScale;

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

#endif /* ofxMTView_hpp */

