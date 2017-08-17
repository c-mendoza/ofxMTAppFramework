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
#include "glm/glm.hpp"

class ofxMTModel;
class ofxMTWindow;
class MTAppModeChangeArgs;
class ofxMTAppMode;

class ofxMTView : public MTEventListenerStore
{

public:
    static std::shared_ptr<ofxMTView> createView(string name);
    virtual ~ofxMTView();
//	void setModel(shared_ptr<ofxMTModel> model);
//	shared_ptr<ofxMTModel> getModel() { return model; }
//    void setWindow(shared_ptr<ofxMTWindow> window);

    //-----------------------------//
    // PARAMETERS                  //
    //-----------------------------//
    ofParameter<ofFloatColor> backgroundColor;
    ofParameter<string> name;

    //-----------------------------//
    // EVENTS: METHODS TO OVERRIDE //
    //-----------------------------//


    /// Called once the model is successfully loaded from file.
    /// Default implementation does nothing.
    virtual void modelDidLoad(){}
    virtual void setup(){}
    virtual void update(){}
    virtual void draw(){}
    virtual void exit(){}
    virtual void windowResized(int w, int h){}
    virtual void superviewFrameChanged(){}
    virtual void frameChanged(){}
    virtual void contentChanged(){}
    virtual void superviewContentChanged(){}
    virtual void keyPressed( int key ){}
    virtual void keyReleased( int key ){}

    /// \brief Called on the active view when the mouse is moved.
    /// Position is given in local coordinates.
    virtual void mouseMoved( int x, int y ){}

    /// \brief Called on the active view when the mouse is dragged, i.e.
    /// moved with a button pressed.
    ///
    /// Position is given in local coordinates.
    virtual void mouseDragged( int x, int y, int button ) {}
    /// \brief Called on the active view when a mouse button is pressed.
    ///
    /// Position is given in local coordinates.
    virtual void mousePressed( int x, int y, int button ){}

    /// \brief Called on the active view when a mouse button is released.
    ///
    /// Position is given in local coordinates.
    virtual void mouseReleased(int x, int y, int button ){}

    /// \brief Called on the active view when the mouse wheel is scrolled.
    ///
    /// Position is given in local coordinates.
    virtual void mouseScrolled(int x, int y, float scrollX, float scrollY ){}

    /// \brief Called on the active view when the mouse cursor enters the
    /// window area
    ///
    /// Note that the mouse coordinates are the last known x/y before the
    /// event occurred, i.e. from the previous frame.
    ///
    /// Position is given in local coordinates.
    virtual void mouseEntered( int x, int y ){}

    /// \brief Called on the active view when the mouse cursor leaves the
    /// window area
    ///
    /// Note that the mouse coordinates are the last known x/y before the
    /// event occurred, i.e. from the previous frame.
    ///
    /// Position is given in local coordinates.
    virtual void mouseExited( int x, int y){}
    virtual void dragEvent(ofDragInfo dragInfo) { }
    virtual void gotMessage(ofMessage msg){ }

    //TODO: Touch Events
    virtual void touchDown(int x, int y, int id) {}
    virtual void touchMoved(int x, int y, int id) {}
    virtual void touchUp(int x, int y, int id) {}
    virtual void touchDoubleTap(int x, int y, int id) {}
    virtual void touchCancelled(int x, int y, int id) {}

    //--------------------------------------------------//
    // EVENTS: LAMBDAS
    //
    // If you don't want to override a whole class,
    // you can instead use these lambdas. They work
    // just like their equivalent event methods.
    // Make sure to assign the right signature to the
    // event lambda you want to use.
    //
    // These lambdas are executed after their corresponding
    // override, so if you override an event method and
    // provide a lambda, the override will be called first.
    //
    //--------------------------------------------------//

    std::function<void()> onModelDidLoad = []{};
    std::function<void()> onSetup = []{};
    std::function<void()> onUpdate = []{};
    std::function<void()> onDraw = []{};
    std::function<void()> onExit = []{};
    std::function<void(int, int)> onWindowResized = [](int w, int h){};
    std::function<void()> onSuperviewFrameChanged = []{};
    std::function<void()> onFrameChanged = []{};
    std::function<void()> onSuperviewContentChanged = []{};
    std::function<void(int)> onKeyPressed = [](int key){};
    std::function<void(int)> onKeyReleased = [](int key){};
    std::function<void(int, int)> onMouseMoved = [](int x, int y){};
    std::function<void(int, int, int)> onMouseDragged = [](int x, int y, int button){};
    std::function<void(int, int, int)> onMousePressed = [](int x, int y, int button){};
    std::function<void(int, int, int)> onMouseReleased = [](int x, int y, int button){};
    std::function<void(int, int, float, float)> onMouseScrolled = [](int x, int y,
                                                               float scrollX,
                                                               float scrollY){};
    std::function<void(int, int)> onMouseEntered = [](int x, int y){};
    std::function<void(int, int)> onMouseExited = [](int x, int y){};




    int mouseX, mouseY;			// for processing heads

    bool isMouseDown = false;
    bool isMouseDragging = false;

    bool hasFocus();

//    ///Transform a point from view-coordinate to content-coordinate
//    glm::vec3 viewToContent(glm::vec3 viewCoord)
//    {
//        return viewCoord * invTransMatrix;
//    }

//    ///Transform a point from view-coordinate to content-coordinate
//    glm::vec3 viewToContent(float x, float y)
//    {
//        return viewToContent(ofVec3f(x, y, 0));
//    }

//    glm::vec3 contentToView(ofVec3f contentCoord)
//    {
//        return contentCoord * transMatrix;
//    }

//    glm::vec3 contentToView(float x, float y)
//    {
//        return contentToView(ofVec3f(x, y, 0));
//    }

//    /// Moves the content within the view's frame by dx and dy.
//    void scrollBy(float dx, float dy);

//    /// Moves the content to the specified view coordinates.
//    void scrollTo(float x, float y);

//    /// \brief Positive values zoom in, negative zoom out. Zoom=1 is the default.
//    void zoomTo(float scale);

//    /// \brief Change the view's zoom (scale) by a relative value.
//    void zoomBy(float zoomChange);

    //------------------------------------------------------//
    // FRAME AND CONTENT                                    //
    //------------------------------------------------------//

    void setFrame(ofRectangle newFrame);

    void setFrameOrigin(glm::vec3 pos);
    const glm::vec3& getFrameOrigin();

    void setFrameSize(glm::vec2 size);
    void setFrameSize(float width, float height);
    glm::vec2 getFrameSize();

    void setContent(ofRectangle newContentRect);
    void setContentOrigin(glm::vec3 pos);
    const glm::vec3& getContentOrigin();

    void setContentSize(glm::vec2 size);
    void setContentSize(float width, float height);
    glm::vec2 getContentSize();

    /// \brief Sets the size of both the frame and the content
    void setSize(glm::vec2 size);

    /// \brief Sets the size of both the frame and the content
    void setSize(float width, float height);

    /// \brief Returns the deepest subview that occupies the specified
    /// window coordinate.
    std::shared_ptr<ofxMTView> hitTest(glm::vec2 &windowCoord);

    /// \brief Gets the mouse in local coordinates.
    /// This will only report a useful number if the mouse
    /// is over the view instance. Other cases are undefined for the
    /// moment.
    const glm::vec2 & getLocalMouse() { return localMouse; }

    /// \brief Returns the mouse down position in local coordinates.
    /// This will only report a useful number if the mouse
    /// is over the view instance. Other cases are undefined for the
    /// moment.
    const glm::vec2 & getLocalMouseDown() { return localMouseDown; }

    /// \brief Returns the last mouse up position in local coordinates.
    /// This will only report a useful number if the mouse
    /// is over the view instance. Other cases are undefined for the
    /// moment.
    const glm::vec2 & getLocalMouseUp() { return localMouseUp; }

    /// \brief Transforms the passed point from its local
    /// coordinates to the coordinate system of a given MTView
    glm::vec2 transformPoint(glm::vec2& coords,
                             const ofxMTView* toView);

    //------------------------------------------------------//
    // VIEW HEIRARCHY                                       //
    //------------------------------------------------------//

    void setWindow(std::weak_ptr<ofxMTWindow> window);

    /// \brief Gets this view's superview if there is one.
    std::weak_ptr<ofxMTView> getSuperview();

    /// \brief Adds a subview.
    /// \return A reference to the added view.
    /// \note Uses move semantics, object takes ownership of the passed value
    void addSubview(shared_ptr<ofxMTView> subview);

    vector<shared_ptr<ofxMTView>>& getSubviews();

    void setSuperview(shared_ptr<ofxMTView> view);

    /// \returns True if successful.
    bool removeFromSuperview();

    /// \returns True if there was a view to be removed.
    bool removeLastSubview();

    void removeAllSubviews();

    std::weak_ptr<ofxMTWindow> getWindow();


    //------------------------------------------------------//
    // APP MODES                                            //
    //------------------------------------------------------//

    virtual void appModeChanged(MTAppModeChangeArgs & modeChange){}

    //------------------------------------------------------//
    // INTERNAL EVENT LISTENERS
    //
    // You do not need to call these methods
    //------------------------------------------------------//
    virtual void setup(ofEventArgs & args) final;
    virtual void update(ofEventArgs & args) final;
    virtual void draw(ofEventArgs & args) final;
    virtual void exit(ofEventArgs & args) final;

    virtual void windowResized(ofResizeEventArgs & resize) final;

    virtual void keyPressed( ofKeyEventArgs & key ) final;
    virtual void keyReleased( ofKeyEventArgs & key ) final;
    virtual void mouseMoved( ofMouseEventArgs & mouse ) final;
    virtual void mouseDragged( ofMouseEventArgs & mouse ) final;
    virtual void mousePressed( ofMouseEventArgs & mouse ) final;
    virtual void mouseReleased(ofMouseEventArgs & mouse) final;// {
//        mouseX=mouse.x;
//        mouseY=mouse.y;
//        contentMouse = viewToContent(mouse);
//        isMouseDown = false;
//        mouseReleased(mouse.x,mouse.y,mouse.button);
//        isMouseDragging = false;
//    }
    virtual void mouseScrolled( ofMouseEventArgs & mouse ) final;
    virtual void mouseEntered( ofMouseEventArgs & mouse ) final;
    virtual void mouseExited( ofMouseEventArgs & mouse ) final;
    virtual void dragged(ofDragInfo & drag) final;
    virtual void messageReceived(ofMessage & message) final;

    //TODO: Touch
//    virtual void touchDown(ofTouchEventArgs & touch) {
//        touchDown(touch.x, touch.y, touch.id);
//    }
//    virtual void touchMoved(ofTouchEventArgs & touch) {
//        touchMoved(touch.x, touch.y, touch.id);
//    }
//    virtual void touchUp(ofTouchEventArgs & touch) {
//        touchUp(touch.x, touch.y, touch.id);
//    }
//    virtual void touchDoubleTap(ofTouchEventArgs & touch) {
//        touchDoubleTap(touch.x, touch.y, touch.id);
//    }
//    virtual void touchCancelled(ofTouchEventArgs & touch){
//        touchCancelled(touch.x, touch.y, touch.id);
//    }

    //------------------------------------------------------//
    // EVENTS												//
    //------------------------------------------------------//

    ofEvent<ofEventArgs> focusGained;
    ofEvent<ofEventArgs> focusLost;
    ofEvent<ofMouseEventArgs> mouseMovedEvent;
    ofEvent<ofMouseEventArgs> mouseDraggedEvent;
    ofEvent<ofMouseEventArgs> mousePressedEvent;
    ofEvent<ofMouseEventArgs> mouseReleasedEvent;
    ofEvent<ofMouseEventArgs> mouseScrolledEvent;
    ofEvent<ofMouseEventArgs> mouseEnteredEvent;
    ofEvent<ofMouseEventArgs> mouseExitedEvent;
    ofEvent<ofDragInfo> draggedEvent;
    ofEvent<ofMessage> messageEvent;


    //------------------------------------------------------//
    // OPERATION QUEUES
    //------------------------------------------------------//

    void enqueueDrawOperation(function<void()> funct)
    {
        drawOpQueue.push(funct);
    }

    void enqueueUpdateOperation(function<void()> f)
    {
        updateOpQueue.push(f);
    }

    friend class ofxMTWindow;

protected:
    std::weak_ptr<ofxMTWindow> window;
    std::weak_ptr<ofxMTView> superview;
    vector<shared_ptr<ofxMTView>> subviews;

    std::shared_ptr<ofxMTAppMode> currentAppMode;

    /// \brief The rectangle that specifies the size and position of
    /// the actual content of the view.
    ///
    /// The location of the content is specified in the view's coordinate
    /// system.
    ofRectangle content;

    /// \brief The rectangle that determines the visible part of the content.
    /// The frame is essentially the "window" through which you can see the
    /// content. If the content is larger than the frame, it is clipped.
    /// If it is smaller, then you'll see the content plus the view's
    /// background color (if specified).
    ///
    /// The location of the frame is specified in its superview's coordinate
    /// system.
    ofRectangle frame;

    ofVec3f contentMouse; //localMousePos?
    ofParameter<float>	contentScale;

    void removeAllEvents();
    void addAllEvents();

    void updateMatrices();


private:
    ofxMTView(string _name);

    /// This function is called internally by the framework to signal that a model
    /// has been loaded from a file. You don't need to call it.
    void modelDidLoadInternal()
    {
        enqueueUpdateOperation([this]()
        {
            modelDidLoad();
        });
    }

    //------------------------------------------------------//
    // VIEW and MATRICES									//
    //------------------------------------------------------//

    glm::mat4 contentMatrix;
    glm::mat4 invContentMatrix; //Just a cached value
    glm::mat4 frameMatrix;
    glm::mat4 invFrameMatrix;

    ofRectangle screenFrame; //The Frame in screen coordinates and scale


    //------------------------------------------------------//
    // MOUSE            									//
    //------------------------------------------------------//
    //The mouse position in local coordinates
    glm::vec2 localMouse;

    //The mouse down position in local coordinates
    glm::vec2 localMouseDown;

    //The mouse up position in local coordinates
    glm::vec2 localMouseUp;

    glm::vec2 localMouseDragStart;

    bool isDragging = false;

    //------------------------------------------------------//
    // QUEUES												//
    //------------------------------------------------------//

    queue<function<void()>> updateOpQueue;
    queue<function<void()>> drawOpQueue;

    //------------------------------------------------------//
    // INTERNALS / CONVENIENCE								//
    //------------------------------------------------------//

    void frameChangedInternal();
    void contentChangedInternal();

    //------------------------------------------------------//
    // EVENTS												//
    //------------------------------------------------------//

    bool isFocused = false;

    /// \brief Internal shared pointer
    std::weak_ptr<ofxMTView> thisView;
};

#endif /* ofxMTView_hpp */

