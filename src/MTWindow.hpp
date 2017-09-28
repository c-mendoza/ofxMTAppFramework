//
//  ofxMTView.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#ifndef MTWindow_hpp
#define MTWindow_hpp

#include "ofxMTAppFramework.h"

#ifndef TARGET_RASPBERRY_PI
#include "ofAppGLFWWindow.h"
#else
#include "ofAppEGLWindow.h"
#endif

class MTModel;
class MTAppModeChangeArgs;
class MTView;

#ifndef TARGET_RASPBERRY_PI
class MTWindow : public ofAppGLFWWindow, public MTEventListenerStore {
#else
class MTWindow : public ofAppEGLWindow, public MTEventListenerStore {
#endif
  public:
	MTWindow(string name);
	~MTWindow();

	ofParameter<string> name;

	shared_ptr<MTView> contentView;

	//TODO: mouseX and mouseY in MTWindow
	int mouseX, mouseY;   // for processing heads

	//------------------------------------------------------//
	// INTERNALS EVENT LISTENERS
	// You should not need to call these methods
	//------------------------------------------------------//
#ifndef TARGET_RASPBERRY_PI
	virtual void setup(const ofGLFWWindowSettings& settings);
#else
	virtual void setup(const ofGLESWindowSettings& settings);
#endif
	void setupInternal(ofEventArgs& args);
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

	// TODO: Touch
	void touchDown(ofTouchEventArgs& touch);
	void touchMoved(ofTouchEventArgs& touch);
	void touchUp(ofTouchEventArgs& touch);
	void touchDoubleTap(ofTouchEventArgs& touch);
	void touchCancelled(ofTouchEventArgs& touch);

	//------------------------------------------------------//
	// EVENTS / OVERRIDABLE
	// //
	//------------------------------------------------------//

	/// \brief Called when the model is loaded by the app.
	/// The call occurs in the update() method of the MTWindow,
	/// so it is safe to call any GL functions from this method
	virtual void modelLoaded() {}
	virtual void windowResized(int w, int h) {}
	virtual void keyPressed(int key) {}
	virtual void keyReleased(int key) {}

	/// \brief Called when the app mode has been changed.
	/// Default implementation does nothing. Override this
	/// method if you want to respond to mode changes.
	virtual void appModeChanged(MTAppModeChangeArgs& modeChange) {}

	void setFocusedView(std::shared_ptr<MTView> view);

	//------------------------------------------------------//
	// OP QUEUES
	//------------------------------------------------------//

	/**
	 * @brief enqueueDrawOperation
	 * @param funct
	 */
	void enqueueDrawOperation(function<void()> funct)
	{
		drawOpQueue.push(funct);
	}

	/**
	 * @brief enqueueUpdateOperation
	 * @param f
	 */
	void enqueueUpdateOperation(function<void()> f) { updateOpQueue.push(f); }

	//------------------------------------------------------//
	// OFBASEWINDOW OVERRIDES
	//------------------------------------------------------//

	/**
	 * @brief getWidth Overrides ofBaseWindow::getWidth() and returns the width
	 * of the MTView that has focus in this MTWindow. This enables the user to
	 * call ofGetWidth() and get a more useful result that just the window's
	 * width. If you want to obtain the window's dimensions, use
	 * getWindowSize().
	 * @return The width of the MTView that is currently focused.
	 */
	int getWidth();

	/**
	 * @brief getHeight overrides ofBaseWindow::getHeight() and returns the height
	 * of the MTView that has focus in this MTWindow. This enables the user to
	 * call ofGetWidth() and get a more useful result that just the window's
	 * width. If you want to obtain the window's dimensions, use
	 * getWindowSize().
	 * @return The height of the MTView that is currently focused.
	 */
	int getHeight();

  protected:
	void removeAllEvents();
	void addAllEvents();

  private:
	/// This function is called internally by the framework to signal that a
	/// model
	/// has been loaded from a file. You don't need to call it.
	void modelLoadedInternal()
	{
		enqueueUpdateOperation([this]() { modelLoaded(); });
	}

	ofMatrix4x4 transMatrix;
	ofMatrix4x4 invTransMatrix;   // Just a cached value

	queue<function<void()>> updateOpQueue;
	queue<function<void()>> drawOpQueue;

	std::weak_ptr<MTView> focusedView;
	std::weak_ptr<MTView> mouseOverView;

	bool isMouseDown = false;
	bool isMouseDragging = false;

	// Hacky var to replace a private member in the superclass
	int mouseButtonInUse = 0;

	glm::vec2 mouseDownPos;
	glm::vec2 mouseUpPos;
	glm::vec2 mouseDragStart;


#ifndef TARGET_RASPBERRY_PI
	/**
	 * @brief "Override" mouse motion callback to avoid ofGetWidth and ofGetHeight
	 * getting called every time the mouse moves. This implementation changes
	 * window::getWidth() and window::getHeight() for getWindowSize().
	 */
	static void mt_motion_cb(GLFWwindow* windowP_, double x, double y);
#endif
};

#endif /* MTWindow_hpp */
