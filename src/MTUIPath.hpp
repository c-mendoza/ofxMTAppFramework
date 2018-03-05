//
//  MTUIPath.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 8/1/16.
//
//

#ifndef MTUIPath_h
#define MTUIPath_h

#include "MTView.hpp"
#include "ofPath.h"
#include "ofGraphics.h"
#include <bitset>

class MTUIPathEventArgs;
class MTUIPathVertex;
class MTUIPathHandle;
/**
///
 Events:
 pathChangedEvent
	Triggered when a dragged handle (vertex or control point) is released

 pathVertexHandleMovedEvent
	Triggered when a vertex handle is moved

*/

//const unsigned char MTUIPathOptionCanAddPoints		= 1 << 0;
//const unsigned char MTUIPathOptionCanDeletePoints	= 1 << 1;
//const unsigned char MTUIPathOptionCanConvertPoints  = 1 << 2;

class MTUIPath :
		public std::enable_shared_from_this<MTUIPath>
{
	friend class MTUIPathVertex;
public:
	~MTUIPath();

	/**
	* @brief Sets up the UIPath
	* @param path is a shared_ptr pointer to the ofPath to be manipulated.
	* It is possible to modify the path outside and still have
	* MTUIPath work as expected, but it is not thread-safe.
	* @param view The MTView that the MTUIPath will be drawn on.
	* The MTView is retained, so it won't be destroyed
	* until the MTUIPath is destroyed.
	* The view also provides the reference coordinate system for
	* the path and the MTUIPath.
	* @param options Or'd option flags. See MTUIPathOption* in this
	* header file.
	*/
	void setup(std::shared_ptr<ofPath> path,
			   std::shared_ptr<MTView> view,
			   unsigned int options);

	 /**
	 * @brief Sets up the UIPath with all of the options enabled.
	 * @param path is a shared_ptr pointer to the ofPath to be manipulated.
	 * It is possible to modify the path outside and still have
	 * MTUIPath work as expected, but it is not thread-safe.
	 * @param view The MTView that the MTUIPath will be drawn on.
	 * The MTView is retained, so it won't be destroyed
	 * until the MTUIPath is destroyed.
	 * The view also provides the reference coordinate system for
	 * the path and the MTUIPath.
	 */

	void setup(std::shared_ptr<ofPath> path,
			   std::shared_ptr<MTView> view);
	void draw();
//    void setAutoDraw(bool autoDraw);
	void setVisibility(bool visible);
	void setClosed(bool closed);
	bool getClosed() { return isClosed; }
	void toggleVisibility();

    void setRegion(ofRectangle region) { this->region = region; }

    /**
     * @brief
     * NotifyOnHandleDragged: Notifies listeners of pathHandleMoved while a handle is being dragged.
     * If you want to only be notified when the handle is done moving, set this option to false.
     */
    enum MTUIPathOptions
	{
		CanAddPoints = 0,
		CanDeletePoints,
		CanConvertPoints,
        LimitToRegion,
        NotifyOnHandleDragged
	};

	std::bitset<5> pathOptionFlags;

	//DATA HANDLING
	/////////////////////////////////

	///Returns true if the handle is found and deleted
	bool deleteHandle(std::shared_ptr<MTUIPathVertex> handle);

	///Deletes all selected handles
	void deleteSelected();

	///Adds a command and its corresponding handle after the last command in the path.
	void addCommand(ofPath::Command &command);

	///Inserts a command in the path at the specified index.
	void insertCommand(ofPath::Command &command, int index);

	/// Adds a user data pointer, which gets returned via the MTUIPath events.
	/// Useful to attach data to the UIPath that needs to be referenced when the UIPath changes.
	void* userData = NULL;
	void addUserData(void* data) { userData = data; }

	//SELECTION
	/////////////////////////////////

	///Adds a handle to the selection
	void addToSelection(std::shared_ptr<MTUIPathVertex> handle);

	///Removes a handle from the selection
	void removeFromSelection(std::shared_ptr<MTUIPathVertex> handle);

	///Sets the selection to the specified handle, removing all others from the selection
	void setSelection(std::shared_ptr<MTUIPathVertex> handle);

	///Removes all from selection
	void deselectAll();

	///Take a guess...
	void selectAll();

	///Gets the selected handles. It is not safe to modify this vector nor to change the Path commands directly.
	const std::vector<std::shared_ptr<MTView>> getSelection();

//    ///
//    /// \brief addEventListeners adds mouse and keyboard
//    /// listeners for events occuring in the MTView associated
//    /// with this MTUIPath. You normally won't have to call this
//    /// method.
//    void addEventListeners();

//    ///
//    /// \brief addEventListeners removes mouse and keyboard
//    /// listeners for events occuring in the MTView associated
//    /// with this MTUIPath. You normally won't have to call this
//    /// method.
//    void removeEventListeners();


	void mousePressed(ofMouseEventArgs& args);
	void mouseMoved(ofMouseEventArgs& args);
	void keyReleased(ofKeyEventArgs& args);

//    void setUseAutoEventListeners(bool use);
//    bool getUseAutoEventListeners() { return useAutoEventListeners; }

	//Options

	bool selectsLastInsertion = true;

	//Events
	/////////////////////////////////

	/// Triggered whenever a command is added or deleted from the path.
	/// Sender: MTUIPath that changed.
	ofEvent<void> pathChangedEvent;

	/// Triggered whenever a vertex handle is moved.
	/// Sender: MTView that moved.
	ofEvent<ofMouseEventArgs> pathHandleMovedEvent;

	/// Triggered when the last handle of the path is deleted. Use this event to delete
	/// the UIPath if you don't need it anymore.
	/// Sender: MTUIPath
	ofEvent<void> lastHandleDeletedEvent;

	/// Triggered whenever the mouse is pressed on a path handle.
	/// Sender: MTView
	ofEvent<ofMouseEventArgs> pathHandlePressedEvent;

	/// Triggered whenever the mouse is released on a path handle.
	/// Sender: MTView
	ofEvent<ofMouseEventArgs> pathHandleReleasedEvent;

	static int vertexHandleSize;
	static int cpHandleSize;
	static ofStyle vertexHandleStyle;
	static ofStyle selectedVextexHandleStyle;
	static ofStyle cpHandleStyle;
	static MTUIPathEventArgs pathEventArgs;

	std::shared_ptr<ofPath> getPath() { return path; }

protected:
	typedef ofPath::Command ofPathCommand;
	std::shared_ptr<ofPath> path = NULL;
	bool isClosed = false;
    std::vector<std::shared_ptr<MTUIPathVertex>> pathVertices;
    std::vector<std::shared_ptr<MTUIPathVertex>> selectedVertices;
	void handlePressed(MTUIPathVertex* vertex, ofMouseEventArgs &args);
	void handleReleased(MTUIPathVertex* vertex, ofMouseEventArgs &args);
//    ofEventListener* drawListener;
//    void drawEvent(ofEventArgs& args);

	bool isVisible = true;
	bool autoDraw = false;
//    bool useAutoEventListeners = true;
	std::shared_ptr<MTView> view = nullptr;

	void updatePath();

	void arrangeClosedPath();
	void arrangeOpenPath();
	void addEventListeners();
	void removeEventListeners();

	bool handleWasPressed = false;

	struct Midpoint
	{
		unsigned int index1;
		unsigned int index2;
		glm::vec3 pos;
	};

    std::vector<Midpoint> midpoints;
	Midpoint closestMidpoint;
	Midpoint& getClosestMidpoint(const glm::vec3 & point);

    ofRectangle region;

};

class MTUIHandle; // Forward declaration

/// \brief The MTUIPathVertex class wraps a set of handles that control
/// a vertex in a path.
class MTUIPathVertex : public MTEventListenerStore
{
	ofPath::Command* command;
	MTUIPath* uiPath;
	unsigned int index;
	std::weak_ptr<MTUIPathVertex> nextVertex;
	std::weak_ptr<MTUIPathVertex> prevVertex;
    std::shared_ptr<MTUIHandle> toHandle;
    std::shared_ptr<MTUIHandle> cp1Handle;
    std::shared_ptr<MTUIHandle> cp2Handle;
	ofStyle currentStyle;

	bool mirroredControlPoints = false;
	bool useAutoEventListeners = true;

public:
	~MTUIPathVertex();
	void setup(MTUIPath* uiPath, ofPath::Command* com);
	void setControlPoints();
	void setStyle(ofStyle newStyle);

	///Sets whether the underlying ofMSAInteractiveObjects listen to events on their own. Defaults
	///to true. If false, you are in charge of feeding event information to the MTView for
	///proper functionality.
//    void setAutoEventListeners(bool useEvents);
	bool getUseAutoEventListeners() { return useAutoEventListeners; }

	///If you are handling events yourself make sure to call these functions when necessary.
	void mouseMoved(ofMouseEventArgs& args);

	///Call this function to simulate a mousePressed event
	void mousePressed(ofMouseEventArgs& args);
	///
	void mouseReleased(ofMouseEventArgs& args);
	void mouseDragged(ofMouseEventArgs& args);
	void keyPressed(ofKeyEventArgs& args);
	void keyReleased(ofKeyEventArgs& args);

	void draw();
	std::shared_ptr<MTUIHandle> getPointHandle() { return toHandle; }
	std::shared_ptr<MTUIHandle> getCP1Handle() { return cp1Handle; }
	std::shared_ptr<MTUIHandle> getCP2Handle() { return cp2Handle; }
	std::shared_ptr<ofPath> getPath() { return uiPath->getPath(); }
	ofPath::Command* getCommand() { return command; }
	MTUIPath* getUIPath() { return uiPath; }

	///
	/// \brief Updates the coordinates of the command to match that
	/// of the handles. Called automatically whenever a handle is moved with
	/// the mouse.
	void updateCommand();

	///Tests whether the point is inside the point handle or any of the control point handles
//    bool hitTest(glm::vec2& point); //?

};


class MTUIHandle : public MTView
{
public:

	MTUIHandle(std::string _name);

	void draw() override;
    void mouseDragged(int x, int y, int button) override;
    void mousePressed(int x, int y, int button) override;
    void mouseReleased(int x, int y, int button) override;
    void mouseEntered(int x, int y) override;
    void mouseExited(int x, int y) override;
    void superviewContentChanged() override;

    enum HandleState
    {
        NORMAL = 0,
        SELECTED,
        PRESSED,
        INACTIVE
    };

    struct HandleStyle
    {
        float size;
        ofColor strokeColor;
        float strokeWeight;
        ofColor fillColor;
        bool useFill;
        bool useStroke;
    };

    void setHandleStyleForState(HandleStyle style, HandleState state);
    HandleState getState();
    void setState(HandleState newState);

protected:
	HandleState state;
    std::unordered_map<int, HandleStyle> stylesMap;
    float originalWidth;
    float originalHeight;

};


class MTUIPathEventArgs : public ofEventArgs
{
public:
	std::shared_ptr<ofPath> path;
	MTUIPathVertex* pathHandle;
	void* userData;
//    glm::vec2* pointData;
};
#endif /* ofxUIPath_hpp */
