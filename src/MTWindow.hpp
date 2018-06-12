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
#include "glm/fwd.hpp"

#ifndef TARGET_RASPBERRY_PI
#include "ofAppGLFWWindow.h"
#else
#include "ofAppEGLWindow.h"
#endif

#include "ofxImGui.h"

class MTModel;
class MTAppStateChangeArgs;
class MTView;
class MTWindowEventArgs;

#ifndef TARGET_RASPBERRY_PI
class MTWindow : public ofAppGLFWWindow,
                 public MTEventListenerStore,
                 public std::enable_shared_from_this<MTWindow>
{

#else
class MTWindow : public ofAppEGLWindow, public MTEventListenerStore, public std::enable_shared_from_this<MTWindow>
{
#endif
  public:
	MTWindow(std::string name);
	~MTWindow();

	ofParameter<std::string> name;

    std::shared_ptr<MTView> contentView;

	//TODO: mouseX and mouseY in MTWindow
	int mouseX, mouseY;   // for processing heads

	//------------------------------------------------------//
	// INTERNALS EVENT LISTENERS
	// You should not need to call these methods
	//------------------------------------------------------//
#ifndef TARGET_RASPBERRY_PI
	virtual void setup(ofGLFWWindowSettings& settings);
#else
	virtual void setup(ofGLESWindowSettings& settings);
#endif
	virtual void setupInternal(ofEventArgs& args);
	virtual void update(ofEventArgs& args);
	virtual void draw(ofEventArgs& args);
	virtual void exit(ofEventArgs& args);

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

    ofEvent<MTWindowEventArgs> windowDidBecomeActiveEvent;
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
	virtual void appModeChanged(MTAppStateChangeArgs& modeChange) {}

	void setFocusedView(std::shared_ptr<MTView> view);

    /**
     * @return A shared pointer to the focused view if there is one,
     * or nullptr if there is no focused view. Given that this pointer is derived
     * from a weak_ptr, it is not recommended to retain this shared_ptr.
     */
    std::shared_ptr<MTView> getFocusedView();

	//------------------------------------------------------//
	// OP QUEUES
	//------------------------------------------------------//

	/**
	 * @brief enqueueDrawOperation
	 * @param funct
	 */
	void enqueueDrawOperation(std::function<void()> funct)
	{
		drawOpQueue.push(funct);
	}

	/**
	 * @brief enqueueUpdateOperation
	 * @param f
	 */
	void enqueueUpdateOperation(std::function<void()> f) { updateOpQueue.push(f); }

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


#pragma mark ImGui

	/**
	 * @brief Enables or disables ofxImGui for this window. Because the ImGui context is destroyed
	 * immediately after disabling, make sure that this window is not in the process of drawing any views,
	 * otherwise you will likely get a crash. It is safe to call this method after creating your window
	 * however.
	 * @param doGui
	 */
	void setImGuiEnabled(bool doGui);
	ofxImGui::Gui & getGui();
protected:
	/**
	 * @brief Recursively calls drawGui() on this view and all of its subviews
	 * @param view
	 */
	void drawImGuiForView(std::shared_ptr<MTView> view);
	bool isImGuiEnabled = false;
	ImGuiContext* imCtx;
	ofxImGui::Gui gui;

#pragma mark Internals

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

    std::queue<std::function<void()>> updateOpQueue;
    std::queue<std::function<void()>> drawOpQueue;

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
    static void mt_focus_callback(GLFWwindow* glfWwindow, int isFocused);
#endif
};


class MTWindowEventArgs : public ofEventArgs
{
public:
    std::weak_ptr<MTWindow> window;
};
#endif /* MTWindow_hpp */
