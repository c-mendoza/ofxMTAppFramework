//
//  ofxMTView.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#ifndef MTView_hpp
#define MTView_hpp

#include "ofxMTAppFramework.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <gl/ofFbo.h>
#include "ofxImGui.h"

enum MTViewResizePolicy
{
	// Default. Does not do any resizing when superview's size changes.
	ResizePolicyNone = 0,

	// Resizes to superview's size.
	ResizePolicySuperview,

	// Maintains exact pixel distance between respective edges of subview and superview.
	ResizePolicyKeepExact,

	// Maintains a proportional distance between respective edges of subview and superview.
	ResizePolicyKeepProportional,

	// Resizes maintaining subview's aspect ratio
	ResizePolicyAspectRatio
};


class MTModel;
class MTWindow;
class MTAppModeChangeArgs;
class MTAppMode;

class MTView : public MTEventListenerStore,
			   public std::enable_shared_from_this<MTView>
{

public:
	MTView(std::string _name);
	virtual ~MTView();

	//-----------------------------//
	// PARAMETERS                  //
	//-----------------------------//

	ofParameter<ofFloatColor> backgroundColor;

	/**
	 * @brief A friendly name for the view
	 */
	ofParameter<std::string> name;

#pragma mark EVENT METHODS TO OVERRIDE
	//-----------------------------//
	// EVENTS: METHODS TO OVERRIDE //
	//-----------------------------//

	/**
	 * @brief 	Called once the model is successfully loaded from file.
	 * Default implementation does nothing.
	 */
	virtual void modelLoaded() {}
	virtual void setup() {}
	virtual void update() {}
	virtual void draw() {}
	virtual void exit() {}
	virtual void windowResized(int w, int h) {}
	virtual void superviewFrameChanged() {}
	virtual void frameChanged() {}
	virtual void contentChanged() {}
	virtual void superviewContentChanged() {}
	virtual void keyPressed(int key) {}
	virtual void keyReleased(int key) {}

	/**
	 * @brief Called when this view's window property is set.
	 */
	virtual void addedToWindow(){}

	/**
	 * @brief Called when this view's window property is invalidated.
	 */
	virtual void removedFromWindow(){}

	/// \brief Called on the active view when the mouse is moved.
	/// Position is given in frame coordinates.
	virtual void mouseMoved(int x, int y) {}

	/// \brief Called on the active view when the mouse is dragged.
	/// Position is given in frame coordinates.
	virtual void mouseDragged(int x, int y, int button) {}

	/// \brief Called on the active view when a mouse button is pressed.
	///
	/// Position is given in frame coordinates.
	virtual void mousePressed(int x, int y, int button) {}

	/// \brief Called on the active view when a mouse button is released.
	///
	/// Position is given in frame coordinates.
	virtual void mouseReleased(int x, int y, int button) {}

	/// \brief Called on the active view when the mouse wheel is scrolled.
	///
	/// Position is given in frame coordinates.
	virtual void mouseScrolled(int x, int y, float scrollX, float scrollY) {}

	/// \brief Called on the active view when the mouse cursor enters the
	/// window area
	///
	/// Note that the mouse coordinates are the last known x/y before the
	/// event occurred, i.e. from the previous frame.
	///
	/// Position is given in frame coordinates.
	virtual void mouseEntered(int x, int y) {}

	/// \brief Called on the active view when the mouse cursor leaves the
	/// window area
	///
	/// Note that the mouse coordinates are the last known x/y before the
	/// event occurred, i.e. from the previous frame.
	///
	/// Position is given in frame coordinates.
	virtual void mouseExited(int x, int y) {}
	virtual void dragEvent(ofDragInfo dragInfo) {}
	virtual void gotMessage(ofMessage msg) {}

	// TODO: Touch Events
	virtual void touchDown(int x, int y, int id) {}
	virtual void touchMoved(int x, int y, int id) {}
	virtual void touchUp(int x, int y, int id) {}
	virtual void touchDoubleTap(int x, int y, int id) {}
	virtual void touchCancelled(int x, int y, int id) {}

	//--------------------------------------------------//
	// EVENTS: LAMBDAS
	//
	// If you don't want to subclass an MTView,
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

	std::function<void()> onModelLoaded = [] {};
	std::function<void()> onSetup = [] {};
	std::function<void()> onUpdate = [] {};
	std::function<void()> onDraw = [] {};
	std::function<void()> onExit = [] {};
	std::function<void(int, int)> onWindowResized = [](int w, int h) {};
	std::function<void()> onSuperviewFrameChanged = [] {};
	std::function<void()> onFrameChanged = [] {};
	std::function<void()> onSuperviewContentChanged = [] {};
	std::function<void(int)> onKeyPressed = [](int key) {};
	std::function<void(int)> onKeyReleased = [](int key) {};
	std::function<void(int, int)> onMouseMoved = [](int x, int y) {};
	std::function<void(int, int, int)> onMouseDragged =
	  [](int x, int y, int button) {};
	std::function<void(int, int, int)> onMousePressed =
	  [](int x, int y, int button) {};
	std::function<void(int, int, int)> onMouseReleased =
	  [](int x, int y, int button) {};
	std::function<void(int, int, float, float)> onMouseScrolled =
	  [](int x, int y, float scrollX, float scrollY) {};
	std::function<void(int, int)> onMouseEntered = [](int x, int y) {};
	std::function<void(int, int)> onMouseExited = [](int x, int y) {};


#pragma mark FRAME AND CONTENT
	//------------------------------------------------------//
	// FRAME AND CONTENT                                    //
	//------------------------------------------------------//

	void setFrame(ofRectangle newFrame);

	/**
	 * @brief getFrame returns a copy of the frame
	 * @return ofRectangle
	 */
	ofRectangle getFrame() { return frame; }

    /**
     * @return Returns the width of the view's frame
     */
	float getWidth() { return frame.width; }

    /**
     * @return Returns the height of the view's frame
     */
    float getHeight() { return frame.height; }

	void setFrameOrigin(float x, float y);
	void setFrameOrigin(glm::vec2 pos);
	void shiftFrameOrigin(glm::vec2 shiftAmount);
	const glm::vec3& getFrameOrigin();

	void setFrameSize(glm::vec2 size);
	void setFrameSize(float width, float height);
	glm::vec2 getFrameSize();

	void setFrameFromCenter(glm::vec2 pos, glm::vec2 size);
	void setFrameCenter(glm::vec2 pos);
	glm::vec3 getFrameCenter();

	bool clipToFrame = false;
	void setContent(ofRectangle newContentRect);

	/**
	 * @brief getContent returns a copy of the content rect
	 * @return an ofRectangle
	 */
	ofRectangle getContent()
	{ return content; }

	void setContentOrigin(glm::vec2 pos);
	const glm::vec3& getContentOrigin();
	void shiftContentOrigin(glm::vec2 shiftAmount);
	void setContentSize(glm::vec2 size);
	void setContentSize(float width, float height);
	glm::vec2 getContentSize();

	/// \brief Sets the scale of the content matrix in a normalized scale
	/// (1 means no scaling, 0.5 means half scale, -1 means inverse scale).
	void setContentScale(float xs, float ys);

	float getContentScaleX()
	{ return contentScaleX; }

	float getContentScaleY()
	{ return contentScaleY; }


	/// TODO: Delete this method
	const ofRectangle& getScreenFrame()
	{ return screenFrame; }

	/// \brief Sets the size of both the frame and the content
	void setSize(glm::vec2 size);

	/// \brief Sets the size of both the frame and the content
	void setSize(float width, float height);

	/// \brief Returns the deepest subview that occupies the specified
	/// window coordinate.
	virtual std::shared_ptr<MTView> hitTest(glm::vec2& windowCoord);


#pragma mark MOUSE

	/// \brief Gets the mouse position in content coordinates.
	/// This will only report a useful number if the mouse
	/// is over the view instance. Other cases are undefined for the
	/// moment.
	const glm::vec2& getContentMouse()
	{ return contentMouse; }

	const glm::vec2& getPrevContentMouse()
	{ return prevContentMouse; }

	/// \brief Returns the mouse down position in content coordinates.
	/// This will only report a useful number if the mouse
	/// is over the view instance. Other cases are undefined for the
	/// moment.
	const glm::vec2& getContentMouseDown()
	{ return contentMouseDown; }

	/// \brief Returns the last mouse up position in content coordinates.
	/// This will only report a useful number if the mouse
	/// is over the view instance. Other cases are undefined for the
	/// moment.
	const glm::vec2& getContentMouseUp()
	{ return contentMouseUp; }

	const glm::vec2& getContentMouseDragStart()
	{ return contentMouseDragStart; }

	const glm::vec2& getScreenMouseDown()
	{ return windowMouseDown; }

	const glm::vec2& getScreenMouseDragStart()
	{ return windowMouseDragStart; }

	const glm::vec2& getScreenMouse()
	{ return windowMouse; }

	const glm::vec2& getPrevScreenMouse()
	{ return prevWindowMouse; }


/**
 * @brief Transforms the passed point from its local (frame)
 * coordinates to the frame coordinate system of a given MTView.
 */
	glm::vec2 transformPoint(glm::vec2& coords, const MTView* toView);

	/// \brief Transforms the passed point from its local (frame)
	/// coordinates to the frame coordinate system of a given MTView.
	glm::vec2 transformPoint(glm::vec2& coords, std::shared_ptr<MTView> toView);

	/// \brief Transforms the passed point from frame
	/// coordinates to content coordinates.
	glm::vec2 transformFramePointToContent(glm::vec2& coords);

	/// \brief Transforms the passed point from frame
	/// coordinates to screen coordinates.
	glm::vec2 transformFramePointToScreen(glm::vec2& coords);

	/// \brief Returns a reference to the frame matrix. Modifying the reference
	/// might result in unexpected behavior!
	glm::mat4& getFrameMatrix()
	{ return frameMatrix; }


#pragma mark VIEW HEIRARCHY
	//------------------------------------------------------//
	// VIEW HEIRARCHY                                       //
	//------------------------------------------------------//

protected:
	/**
	 * @brief Sets the window for this view and its subviews.
	 * This method is not meant to move a view to a window; it just sets the window
	 * pointer of the view and its subviews.
	 */
	void setWindow(std::weak_ptr<MTWindow> window);

public:
	/**
	 * @brief
	 * @return Returns this view's superview, or @code nullptr @endcode if there isn't one.
	 */
	std::shared_ptr<MTView> getSuperview();

	/// \brief Adds a subview.
	void addSubview(std::shared_ptr<MTView> subview);

	std::vector<std::shared_ptr<MTView>>& getSubviews();

	void setSuperview(std::shared_ptr<MTView> view);

	/// \returns True if successful.
	bool removeFromSuperview();

	/// \returns True if there was a view to be removed.
	bool removeLastSubview();
	bool removeSubview(std::shared_ptr<MTView> view);
	void removeAllSubviews();

	std::weak_ptr<MTWindow> getWindow();

	int getWindowWidth();
	int getWindowHeight();


#pragma mark VIEW RELATED
	//------------------------------------------------------//
	// VIEW Releated                                        //
	//------------------------------------------------------//

	/**
	 * @brief Called whenever a view:
	 * 1. Is added to a superview
	 * 2. It changes size or position
	 * 3. Its superview changes size or position
	 * 4. Its parent window changes size
	 *
	 * The default implementation does nothing.
	 */
	virtual void layout(){}

	/**
	  * @brief lambda flavor of layout()
	  */
	std::function<void()> onLayout = [](){};

	MTViewResizePolicy resizePolicy = ResizePolicyNone;

	//This should be private:
	void performResizePolicy();

	/** @brief Enables or disables the drawing of this view's background.
	 *  The background is the extents of the view's frame.
	 */
	void setDrawBackground(bool drawIt)
	{
//		if (drawIt != isDrawingBackground)
//		{
//			enqueueUpdateOperation(
//			  [this, drawIt]() { isDrawingBackground = drawIt; });
//		}
		isDrawingBackground = drawIt;
	}

	bool getDrawBackground()
	{ return isDrawingBackground; }

	virtual void drawGui()
	{};


	bool isRenderingEnabled = true;

	//------------------------------------------------------//
	// APP MODES                                            //
	//------------------------------------------------------//

	virtual void appModeChanged(MTAppModeChangeArgs& modeChange) {}


#pragma mark INTERNAL EVENT LISTENERS
	//------------------------------------------------------//
	// INTERNAL EVENT LISTENERS
	//
	// You do not need to call these methods
	//------------------------------------------------------//
	void setup(ofEventArgs& args);
	void update(ofEventArgs& args);
	void draw(ofEventArgs& args);
	void exit(ofEventArgs& args);

	void windowResized(ofResizeEventArgs& resize);

	void keyPressed(ofKeyEventArgs& key);
	void keyReleased(ofKeyEventArgs& key);
	void mouseMoved(ofMouseEventArgs& mouse);
	void mouseDragged(ofMouseEventArgs& mouse);
	void mousePressed(ofMouseEventArgs& mouse);
	void mouseReleased(ofMouseEventArgs& mouse);
	void mouseScrolled(ofMouseEventArgs& mouse);
	void mouseEntered(ofMouseEventArgs& mouse);
	void mouseExited(ofMouseEventArgs& mouse);
	void dragged(ofDragInfo& drag);
	void messageReceived(ofMessage& message);
	void modelLoaded(ofEventArgs& args);

private:
	void updateMousePositionsWithWindowCoordinate(glm::vec2 windowCoord);
	void updateMouseDownPositionsWithWindowCoordinate(glm::vec2 windowCoord);
	void updateMouseUpPositionsWithWindowCoordinate(glm::vec2 windowCoord);

public:

#pragma mark EVENTS
	//------------------------------------------------------//
	// EVENTS
	//
	//------------------------------------------------------//

	ofEvent<ofEventArgs> focusGained;
	ofEvent<ofEventArgs> focusLost;
	ofEvent<ofMouseEventArgs> mouseMovedEvent;
	ofEvent<ofMouseEventArgs> mouseDraggedEvent;
	ofEvent<ofMouseEventArgs> mouseDraggedEndEvent;
	ofEvent<ofMouseEventArgs> mousePressedEvent;
	ofEvent<ofMouseEventArgs> mouseReleasedEvent;
	ofEvent<ofMouseEventArgs> mouseScrolledEvent;
	ofEvent<ofMouseEventArgs> mouseEnteredEvent;
	ofEvent<ofMouseEventArgs> mouseExitedEvent;
	ofEvent<ofKeyEventArgs> keyPressedEvent;
	ofEvent<ofKeyEventArgs> keyReleasedEvent;
	ofEvent<ofDragInfo> draggedEvent;
	ofEvent<ofMessage> messageEvent;
	ofEvent<ofEventArgs> frameChangedEvent;
	ofEvent<ofResizeEventArgs> windowResizedEvent;   //?
	ofEvent<ofEventArgs> addedToWindowEvent;
	ofEvent<ofEventArgs> removedFromWindowEvent;
	/**
	 * @brief Notifies when this view is added to a view hierarchy.
	 */
	ofEvent<ofEventArgs> addedToSuperviewEvent;
	ofEvent<ofEventArgs> removedFromSuperviewEvent;
	/**
	 * @brief Notifies before this MTView is destroyed.
	 */
	ofEvent<ofEventArgs> exitEvent;

	bool isMouseDown = false;
	bool isMouseDragging = false;

	bool hasFocus();

	/**
	 * @brief Set this to false if you want this MTView to ignore keyboard focus
	 */
	bool wantsFocus = true;


#pragma mark OPERATION QUEUES
	//------------------------------------------------------//
	// OPERATION QUEUES
	//------------------------------------------------------//

	void enqueueDrawOperation(std::function<void()> funct)
	{
		drawOpQueue.push(funct);
	}

	void enqueueUpdateOperation(std::function<void()> f)
	{ updateOpQueue.push(f); }

	friend class MTWindow;

protected:
	std::weak_ptr<MTWindow> window;
	std::weak_ptr<MTView> superview;
	std::vector<std::shared_ptr<MTView>> subviews;

public:
	/**
	 * @brief Directly sets this view's app mode. This does not change
	 * the app mode that is managed by MTApp.
	 * @param mode
	 */
	void setViewAppMode(std::shared_ptr<MTAppMode> mode)
	{ currentAppMode = mode; }

	/**
	 * @brief Gets the current app mode assigned to this view. This may or may not
	 * be the same as the app's MTAppMode, depending on whether you've assigned this
	 * directly.
	 * @return  The current MTAppMode assigned to this view.
	 */
	std::shared_ptr<MTAppMode> getViewAppMode()
	{ return currentAppMode; }

protected:
	std::shared_ptr<MTAppMode> currentAppMode;

	/**
	 * @brief The rectangle that specifies the size and position of
	 * the actual content of the view.
	 * The location of the content is specified in the view's coordinate system.
	 */
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

	ofParameter<float> contentScaleX;
	ofParameter<float> contentScaleY;


	void updateMatrices();

	/**
	 * @brief An FPS Counter for debugging or any other purposes. Call counter.newFrame()
	 * to make use of it, and counter.getFPS() to obtain the measured framerate.
	 */
	ofFpsCounter counter;


#pragma mark VIEW AND MATRICES
private:
	//------------------------------------------------------//
	// VIEW and MATRICES
	//------------------------------------------------------//

	glm::mat4 contentMatrix;
	glm::mat4 invContentMatrix;
	glm::mat4 frameMatrix;
	glm::mat4 invFrameMatrix;
public:
	const ofParameter<ofFloatColor>& getBackgroundColor() const;

	const glm::mat4& getContentMatrix() const;

	const glm::mat4& getInvContentMatrix() const;

	const glm::mat4& getInvFrameMatrix() const;

private:

	ofRectangle screenFrame;   // The Frame in screen coordinates and scale

	void updateScreenFrame();

	bool isDrawingBackground = true;
	bool isImGuiEnabled = false;

	//------------------------------------------------------//
	// MOUSE
	//------------------------------------------------------//
	// The mouse position in content coordinates
	glm::vec2 contentMouse;

	glm::vec2 prevContentMouse;

	// The mouse down position in content coordinates
	glm::vec2 contentMouseDown;

	// The mouse up position in content coordinates
	glm::vec2 contentMouseUp;

	glm::vec2 contentMouseDragStart;

	/**
	 * @brief Mouse in screen coordinates
	 */
	glm::vec2 windowMouse;

	glm::vec2 prevWindowMouse;

	/**
	 * @brief The mouse down position in window coordinates
	 */
	glm::vec2 windowMouseDown;;
	/**
	 * @brief The mouse drag start in screen coordinates
	 */
	glm::vec2 windowMouseDragStart;

	bool isDragging = false;

	//------------------------------------------------------//
	// QUEUES
	//------------------------------------------------------//

	std::queue<std::function<void()>> updateOpQueue;
	std::queue<std::function<void()>> drawOpQueue;

	//------------------------------------------------------//
	// INTERNALS / CONVENIENCE
	//------------------------------------------------------//

	void frameChangedInternal();
	void contentChangedInternal();
	void superviewFrameChangedInternal();
	void superviewContentChangedInternal();
	void layoutInternal();
	void resetWindowPointer();

	bool isSetUp = false;

	float mouseWheel = 0;

	//------------------------------------------------------//
	// EVENTS
	//------------------------------------------------------//

	bool isFocused = false;

	const glm::mat4& getFrameMatrix() const;

};

class ofTexture;

class MTOffscreenView : public MTView
{

private:
	ofFbo viewFbo;
public:
	MTOffscreenView(std::string name);
	void setup() override;
	void drawOffscreen();
	void frameChanged() override;
	ofTexture& getViewTexture();
};


#endif /* MTView_hpp */
