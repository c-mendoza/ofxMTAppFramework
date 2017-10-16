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
#include "glm/glm.hpp"
#include "ofxImGui.h"

class MTModel;
class MTWindow;
class MTAppModeChangeArgs;
class MTAppMode;

class MTView : public MTEventListenerStore,
			   public std::enable_shared_from_this<MTView>
{

  public:
	MTView(string _name);
	virtual ~MTView();

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

	/// \brief Called on the active view when the mouse is moved.
	/// Position is given in frame coordinates.
	virtual void mouseMoved(int x, int y) {}

	/// \brief Called on the active view when the mouse is dragged, i.e.
	/// moved with a button pressed.
	///
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

	bool isMouseDown = false;
	bool isMouseDragging = false;

	bool hasFocus();

	/// Set this to false if you want this MTView to ignore
	/// keyboard focus
	bool wantsFocus = true;

	//------------------------------------------------------//
	// FRAME AND CONTENT                                    //
	//------------------------------------------------------//

	void setFrame(ofRectangle newFrame);

	/**
	 * @brief getFrame returns a copy of the frame
	 * @return ofRectangle
	 */
	ofRectangle getFrame() { return frame; }

	float getWidth() { return frame.width; }
	float getHeight() { return frame.height; }

	void setFrameOrigin(float x, float y);
	void setFrameOrigin(glm::vec3 pos);
	void shiftFrameOrigin(glm::vec3 shiftAmount);
	const glm::vec3& getFrameOrigin();

	void setFrameSize(glm::vec2 size);
	void setFrameSize(float width, float height);
	glm::vec2 getFrameSize();

	void setFrameFromCenter(glm::vec3 pos, glm::vec2 size);
	void setFrameCenter(glm::vec3 pos);
	glm::vec3 getFrameCenter();

	bool clipToFrame = false;
	void setContent(ofRectangle newContentRect);

	/**
	 * @brief getContent returns a copy of the content rect
	 * @return an ofRectangle
	 */
	ofRectangle getContent() { return content; }
	void setContentOrigin(glm::vec3 pos);
	const glm::vec3& getContentOrigin();

	void setContentSize(glm::vec2 size);
	void setContentSize(float width, float height);
	glm::vec2 getContentSize();

	/// \brief Sets the scale of the content matrix in a normalized scale
	/// (1 means no scaling, 0.5 means half scale, -1 means inverse scale).
	void setContentScale(float xs, float ys);
	float getContentScaleX() { return contentScaleX; }
	float getContentScaleY() { return contentScaleY; }
	

	/// TODO: Delete this method
	const ofRectangle& getScreenFrame() { return screenFrame; }

	/// \brief Sets the size of both the frame and the content
	void setSize(glm::vec2 size);

	/// \brief Sets the size of both the frame and the content
	void setSize(float width, float height);

	/// \brief Returns the deepest subview that occupies the specified
	/// window coordinate.
	virtual std::shared_ptr<MTView> hitTest(glm::vec2& windowCoord);

	/// \brief Gets the mouse position in frame coordinates.
	/// This will only report a useful number if the mouse
	/// is over the view instance. Other cases are undefined for the
	/// moment.
	const glm::vec2& getLocalMouse() { return contentMouse; }

	/// \brief Returns the mouse down position in frame coordinates.
	/// This will only report a useful number if the mouse
	/// is over the view instance. Other cases are undefined for the
	/// moment.
	const glm::vec2& getLocalMouseDown() { return contentMouseDown; }

	/// \brief Returns the last mouse up position in frame coordinates.
	/// This will only report a useful number if the mouse
	/// is over the view instance. Other cases are undefined for the
	/// moment.
	const glm::vec2& getLocalMouseUp() { return contentMouseUp; }

	/// \brief Transforms the passed point from its local (frame)
	/// coordinates to the frame coordinate system of a given MTView.
	glm::vec2 transformPoint(glm::vec2& coords, const MTView* toView);

	/// \brief Transforms the passed point from its local (frame)
	/// coordinates to the frame coordinate system of a given MTView.
	glm::vec2 transformPoint(glm::vec2& coords, std::shared_ptr<MTView> toView);

	/// \brief Transforms the passed point from frame
	/// coordinates to content coordinates.
	glm::vec2 frameToContent(glm::vec2& coords);

	/// \brief Returns a reference to the frame matrix. Modifying the reference
	/// might result in unexpected behavior!
	glm::mat4& getFrameMatrix() { return frameMatrix; }

	//------------------------------------------------------//
	// VIEW HEIRARCHY                                       //
	//------------------------------------------------------//

	/// \brief Moves this view and its hierarchy to the specified MTWindow/
	void setWindow(std::weak_ptr<MTWindow> window);

	/// \brief Gets this view's superview if there is one. Make sure to test the
	/// returned shared_ptr!
	std::shared_ptr<MTView> getSuperview();

	/// \brief Adds a subview.
	void addSubview(shared_ptr<MTView> subview);

	vector<shared_ptr<MTView>>& getSubviews();

	void setSuperview(shared_ptr<MTView> view);

	/// \returns True if successful.
	bool removeFromSuperview();

	/// \returns True if there was a view to be removed.
	bool removeLastSubview();
	bool removeSubview(std::shared_ptr<MTView> view);
	void removeAllSubviews();

	std::weak_ptr<MTWindow> getWindow();

	int getWindowWidth();
	int getWindowHeight();

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

	/// \brief Enables or disables the drawing of this view's background.
	/// The background is the extents of the view's frame.
	void setDrawBackground(bool drawIt)
	{
		if (drawIt != isDrawingBackground)
		{
			enqueueUpdateOperation(
			  [this, drawIt]() { isDrawingBackground = drawIt; });
		}
	}

	bool getDrawBackground() { return isDrawingBackground; }

	virtual void drawGui(){};
	ImGuiContext* imCtx;
	ofxImGui::Gui & getGui();

	//------------------------------------------------------//
	// APP MODES                                            //
	//------------------------------------------------------//

	virtual void appModeChanged(MTAppModeChangeArgs& modeChange) {}

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

	/// \brief Notified before this MTView is destroyed.
	ofEvent<ofEventArgs> exitEvent;

	//------------------------------------------------------//
	// OPERATION QUEUES
	//------------------------------------------------------//

	void enqueueDrawOperation(function<void()> funct)
	{
		drawOpQueue.push(funct);
	}

	void enqueueUpdateOperation(function<void()> f) { updateOpQueue.push(f); }

	friend class MTWindow;

  protected:
	std::weak_ptr<MTWindow> window;
	std::weak_ptr<MTView> superview;
	vector<shared_ptr<MTView>> subviews;

	std::shared_ptr<MTAppMode> currentAppMode;

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

	ofParameter<float> contentScaleX;
	ofParameter<float> contentScaleY;


	void updateMatrices();

  private:
	//------------------------------------------------------//
	// VIEW and MATRICES
	//------------------------------------------------------//

	glm::mat4 contentMatrix;
	glm::mat4 invContentMatrix;
	glm::mat4 frameMatrix;
	glm::mat4 invFrameMatrix;

	ofRectangle screenFrame;   // The Frame in screen coordinates and scale

	bool isDrawingBackground = true;

	//------------------------------------------------------//
	// MOUSE
	//------------------------------------------------------//
	// The mouse position in frame coordinates
	glm::vec2 contentMouse;

	// The mouse down position in frame coordinates
	glm::vec2 contentMouseDown;

	// The mouse up position in frame coordinates
	glm::vec2 contentMouseUp;

	glm::vec2 contentMouseDragStart;

	bool isDragging = false;

	//------------------------------------------------------//
	// QUEUES
	//------------------------------------------------------//

	queue<function<void()>> updateOpQueue;
	queue<function<void()>> drawOpQueue;

	//------------------------------------------------------//
	// INTERNALS / CONVENIENCE
	//------------------------------------------------------//

	void frameChangedInternal();
	void contentChangedInternal();
	//	void superviewFrameChangedInternal();
	void superviewContentChangedInternal();
	void layoutInternal();

	bool isSetUp = false;

	//------------------------------------------------------//
	// EVENTS
	//------------------------------------------------------//

	bool isFocused = false;
};

#endif /* MTView_hpp */
