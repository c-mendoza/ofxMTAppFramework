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

class MTViewMode;

class MTView : public MTEventListenerStore
{

 protected:
   MTView(std::string _name);
 public:
   virtual ~MTView();

   template<typename T = MTView, typename std::enable_if<std::is_base_of<MTView, T>::value>::type* = nullptr, typename... Args>
   static std::shared_ptr<T> CreateView(Args&... args)
   {
      return std::shared_ptr<T>(new T(args...));
   }

   template<typename T = MTView, typename std::enable_if<std::is_base_of<MTView, T>::value>::type* = nullptr, typename... Args>
   static std::shared_ptr<T> CreateView(T* pointerRef, Args&... args)
   {
      auto ptr = std::shared_ptr<T>(new T(args...));
      pointerRef = ptr.get();
      return ptr;
   }

   //-----------------------------//
   // PARAMETERS                  //
   //-----------------------------//

   ofParameter<ofFloatColor> backgroundColor;

   /**
	 * @brief A friendly name for the view
	 */
   ofParameter<std::string> name;

   friend class MTWindow;

 protected:
   std::weak_ptr<MTWindow> window;
   MTView* superview = nullptr;
   std::vector<std::shared_ptr<MTView>> subviews;

 public:
   /**
	 * @brief Sets the current view mode for this view. This method will call
	 * MTViewMode::exit() on currentViewMode, assign 'mode' as the
	 * currentViewMode, and call MTViewMode::setup() on the now currentViewMode.
	 * @param mode
	 */
   void setViewMode(std::shared_ptr<MTViewMode> mode);

   /**
	 * @brief Gets the current view mode assigned to this view. This may or may not
	 * correspond to the app's mode, depending on whether you've assigned this
	 * directly.
	 * @return  The current MTViewMode assigned to this view.
	 */
   const std::shared_ptr<MTViewMode> getViewMode()
   {
      return currentViewMode;
   }

 private:
   std::shared_ptr<MTViewMode> currentViewMode;

#pragma mark EVENT METHODS TO OVERRIDE
   //-----------------------------//
   // EVENTS: METHODS TO OVERRIDE //
   //-----------------------------//

 public:
   /**
 	 * @brief Called once the model is successfully loaded from file.
 	 * Default implementation does nothing.
 	*/
   virtual void modelLoaded()
   {
   }

   virtual void setup()
   {
   }

   virtual void update()
   {
   }

   virtual void draw()
   {
   }

   virtual void exit()
   {
   }

   virtual void windowResized(int w, int h)
   {
   }

   virtual void superviewFrameChanged()
   {
   }

   virtual void frameChanged()
   {
   }

   virtual void contentChanged()
   {
   }

   virtual void superviewContentChanged()
   {
   }

   virtual void keyPressed(int key)
   {
   }

   virtual void keyPressed(ofKeyEventArgs& keyArgs)
   {
   }

   virtual void keyReleased(int key)
   {
   }

   virtual void keyReleased(ofKeyEventArgs& keyArgs)
   {
   }


   /**
	 * @brief Called when this view's window property is set.
	 */
   virtual void addedToWindow()
   {
   }

   /**
	 * @brief Called when this view's window property is invalidated.
	 */
   virtual void removedFromWindow()
   {
   }

   /// \brief Called on the active view when the mouse is moved.
   /// Position is given in frame coordinates.
   virtual void mouseMoved(int x, int y)
   {
   }

   /// \brief Called on the active view when the mouse is dragged.
   /// Position is given in frame coordinates.
   virtual void mouseDragged(int x, int y, int button)
   {
   }

   /// \brief Called on the active view when a mouse button is pressed.
   ///
   /// Position is given in frame coordinates.
   virtual void mousePressed(int x, int y, int button)
   {
   }

   /// \brief Called on the active view when a mouse button is released.
   ///
   /// Position is given in frame coordinates.
   virtual void mouseReleased(int x, int y, int button)
   {
   }

   /// \brief Called on the active view when the mouse wheel is scrolled.
   ///
   /// Position is given in frame coordinates.
   virtual void mouseScrolled(int x, int y, float scrollX, float scrollY)
   {
   }

   /// \brief Called on the active view when the mouse cursor enters the
   /// window area
   ///
   /// Note that the mouse coordinates are the last known x/y before the
   /// event occurred, i.e. from the previous frame.
   ///
   /// Position is given in frame coordinates.
   virtual void mouseEntered(int x, int y)
   {
   }

   /// \brief Called on the active view when the mouse cursor leaves the
   /// window area
   ///
   /// Note that the mouse coordinates are the last known x/y before the
   /// event occurred, i.e. from the previous frame.
   ///
   /// Position is given in frame coordinates.
   virtual void mouseExited(int x, int y)
   {
   }

   virtual void dragEvent(ofDragInfo dragInfo)
   {
   }

   virtual void gotMessage(ofMessage msg)
   {
   }

   // TODO: Touch Events
   virtual void touchDown(int x, int y, int id)
   {
   }

   virtual void touchMoved(int x, int y, int id)
   {
   }

   virtual void touchUp(int x, int y, int id)
   {
   }

   virtual void touchDoubleTap(int x, int y, int id)
   {
   }

   virtual void touchCancelled(int x, int y, int id)
   {
   }

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

   std::function<void(MTView*)> onModelLoaded = [](MTView* view) {};
   std::function<void(MTView*)> onSetup = [](MTView* view) {};
   std::function<void(MTView*)> onUpdate = [](MTView* view) {};
   std::function<void(MTView*)> onDraw = [](MTView* view) {};
   std::function<void(MTView*)> onExit = [](MTView* view) {};
   std::function<void(MTView*, int, int)> onWindowResized = [](MTView* view, int w, int h) {};
   std::function<void(MTView*)> onSuperviewFrameChanged = [](MTView* view) {};
   std::function<void(MTView*)> onFrameChanged = [](MTView* view) {};
   std::function<void(MTView*)> onSuperviewContentChanged = [](MTView* view) {};
   std::function<void(MTView*, int)> onKeyPressed = [](MTView* view, int key) {};
   std::function<void(MTView*, int)> onKeyReleased = [](MTView* view, int key) {};
   std::function<void(MTView*, int, int)> onMouseMoved = [](MTView* view, int x, int y) {};
   std::function<void(MTView*, int, int, int)> onMouseDragged = [](MTView* view, int x, int y, int button) {};
   std::function<void(MTView*, int, int, int)> onMousePressed = [](MTView* view, int x, int y, int button) {};
   std::function<void(MTView*, int, int, int)> onMouseReleased = [](MTView*, int x, int y, int button) {};
   std::function<void(MTView*, int, int, float, float)> onMouseScrolled =
       [](MTView* view, int x, int y, float scrollX, float scrollY) {};
   std::function<void(MTView*, int, int)> onMouseEntered = [](MTView* view, int x, int y) {};
   std::function<void(MTView*, int, int)> onMouseExited = [](MTView* view, int x, int y) {};


#pragma mark FRAME AND CONTENT
   //------------------------------------------------------//
   // FRAME AND CONTENT                                    //
   //------------------------------------------------------//

   void setFrame(ofRectangle newFrame);

   /**
	 * @brief getFrame returns a copy of the frame
	 * @return ofRectangle
	 */
   ofRectangle getFrame()
   {
      return frame;
   }

   /**
    * @return Returns the width of the view's frame
    */
   float getWidth()
   {
      return frame.width;
   }

   /**
	 * @return Returns the height of the view's frame
	 */
   float getHeight()
   {
      return frame.height;
   }

   void setFrameOrigin(float x, float y);
   void setFrameOrigin(const glm::vec2& pos);
   void shiftFrameOrigin(const glm::vec2& shiftAmount);
   const glm::vec3& getFrameOrigin();

   void setFrameSize(const glm::vec2& size);
   void setFrameSize(float width, float height);
   glm::vec2 getFrameSize();

   void setFrameFromCenter(const glm::vec2& pos, const glm::vec2& size);
   void setFrameCenter(const glm::vec2& pos);
   glm::vec3 getFrameCenter();

   /**
	 * @brief If true, the frame will clip the content. Otherwise, if the
	 * content is larger than the frame the overflow contents will be visible.
	 */
   bool clipToFrame = false;

   /**
	 * @brief Sets the content rect for this View.
	 * @param newContentRect
	 */
   void setContent(ofRectangle newContentRect);

   /**
	 * @brief getContent returns a copy of the content rect
	 * @return an ofRectangle
	 */
   ofRectangle getContent()
   {
      return content;
   }

   void setContentOrigin(const glm::vec2& pos);
   const glm::vec3& getContentOrigin();
   void shiftContentOrigin(const glm::vec2& shiftAmount);
   void setContentSize(const glm::vec2& size);
   void setContentSize(float width, float height);
   glm::vec2 getContentSize();

   /// \brief Sets the scale of the content matrix in a normalized scale
   /// (1 means no scaling, 0.5 means half scale, -1 means inverse scale).
   void setContentScale(float xs, float ys);

   float getContentScaleX()
   {
      return contentScaleX;
   }

   float getContentScaleY()
   {
      return contentScaleY;
   }


   /// TODO: Delete this method
   const ofRectangle& getScreenFrame()
   {
      return screenFrame;
   }

   /// \brief Sets the size of both the frame and the content
   void setSize(const glm::vec2& size);

   /// \brief Sets the size of both the frame and the content
   void setSize(float width, float height);
   void setPosition(float x, float y);

   /// \brief Returns the deepest subview that occupies the specified
   /// window coordinate.
   virtual MTView* hitTest(glm::vec2& windowCoord);


#pragma mark MOUSE

   /// \brief Gets the mouse position in content coordinates.
   /// This will only report a useful number if the mouse
   /// is over the view instance. Other cases are undefined for the
   /// moment.
   const glm::vec2& getContentMouse()
   {
      return contentMouse;
   }

   const glm::vec2& getPrevContentMouse()
   {
      return prevContentMouse;
   }

   /// \brief Returns the mouse down position in content coordinates.
   /// This will only report a useful number if the mouse
   /// is over the view instance. Other cases are undefined for the
   /// moment.
   const glm::vec2& getContentMouseDown()
   {
      return contentMouseDown;
   }

   /// \brief Returns the last mouse up position in content coordinates.
   /// This will only report a useful number if the mouse
   /// is over the view instance. Other cases are undefined for the
   /// moment.
   const glm::vec2& getContentMouseUp()
   {
      return contentMouseUp;
   }

   const glm::vec2& getContentMouseDragStart()
   {
      return contentMouseDragStart;
   }

   const glm::vec2& getScreenMouseDown()
   {
      return windowMouseDown;
   }

   const glm::vec2& getScreenMouseDragStart()
   {
      return windowMouseDragStart;
   }

   const glm::vec2& getScreenMouse()
   {
      return windowMouse;
   }

   const glm::vec2& getPrevScreenMouse()
   {
      return prevWindowMouse;
   }


   /**
 * @brief Transforms the passed point from its local (frame)
 * coordinates to the frame coordinate system of a given MTView.
 */
   glm::vec2 transformPoint(glm::vec2& coords, const MTView* toView);

   /// \brief Transforms the passed point from its local (frame)
   /// coordinates to the frame coordinate system of a given MTView.
   //glm::vec2 transformPoint(glm::vec2& coords, MTView* toView);

   /// \brief Transforms the passed point from frame
   /// coordinates to content coordinates.
   glm::vec2 transformFramePointToContent(glm::vec2& coords);

   /// \brief Transforms the passed point from frame
   /// coordinates to screen coordinates.
   glm::vec2 transformFramePointToScreen(glm::vec2& coords);

   /// \brief Returns a reference to the frame matrix. Modifying the reference
   /// might result in unexpected behavior!
   glm::mat4& getFrameMatrix()
   {
      return frameMatrix;
   }


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
   MTView* getSuperview();

   /// \brief Adds a subview.
   void addSubview(std::shared_ptr<MTView> subview);

   const std::vector<std::shared_ptr<MTView>>& getSubviews() const;

   /**
    * @brief Returns the named view, or nullptr if none is found
    */
   std::weak_ptr<MTView> getSubviewWithName(std::string viewName) const;

 private:
   void setSuperview(MTView* view);

 public:
   /// \returns True if successful.
   std::shared_ptr<MTView> removeFromSuperview();

   /// \returns True if there was a view to be removed.
   std::shared_ptr<MTView> removeLastSubview();
   std::shared_ptr<MTView> removeSubview(MTView* view);

   void removeAllSubviews(bool recursive = true);

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
   virtual void layout()
   {
   }

   /**
	  * @brief lambda flavor of layout()
	  */
   std::function<void()> onLayout = []() {};

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
   {
      return isDrawingBackground;
   }

 private:
   void drawGuiInternal();
 public:
   virtual void drawGui(){};
   std::function<void()> onDrawGui = [] {};


   bool isRenderingEnabled = true;

   //------------------------------------------------------//
   // APP MODES                                            //
   //------------------------------------------------------//

   virtual void appModeChanged(MTAppModeChangeArgs& modeChange)
   {
   }

   //#pragma mark FULL SCREEN
   //private:
   //	bool isFullScreenActive = false;
   //
   //public:
   //	bool isFullScreen() { return isFullScreenActive; }
   //	void setFullScreen(bool fs);


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

   void keyPressedInternal(ofKeyEventArgs& key);
   void keyReleasedInternal(ofKeyEventArgs& key);
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
   void appModeChangedInternal(MTAppModeChangeArgs& args);

 private:
   void updateMousePositionsWithWindowCoordinate(glm::vec2 windowCoord);
   void updateMouseDownPositionsWithWindowCoordinate(glm::vec2 windowCoord);
   void updateMouseUpPositionsWithWindowCoordinate(glm::vec2 windowCoord);
   ofEventListener appModeChangedListener;
   ofEventListener exitEventListener;

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
   ofEvent<ofResizeEventArgs> windowResizedEvent;  //?
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

   // These should probably be private!
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

   /**
	 * @brief Adds a function that gets executed only once in the View's
	 * draw method. The function is then discarded.
	 * Very useful in multithreaded scenarios and a bit more
	 * flexible than an ofThreadChannel. If you need to queue one-off slow
	 * graphics code (i.e. allocating an FBO during runtime), this method
	 * can make your life a lot less complicated.
	 * @param funct
	 */
   void enqueueDrawOperation(std::function<void()> funct)
   {
      drawOpQueue.push(funct);
   }

   /**
	 * @brief Adds a function that gets executed only once in the View's
	 * draw update. The function is then discarded.
	 * In views where rendering has been disabled, update() still will be
	 * called on every frame.
	 * @param funct
	 */
   void enqueueUpdateOperation(std::function<void()> f)
   {
      updateOpQueue.push(f);
   }


#pragma mark VIEW AND MATRICES

 protected:
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
   ofRectangle screenFrame;  // The Frame in screen coordinates and scale

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
   glm::vec2 windowMouseDown;
   ;
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

#endif /* MTView_hpp */
