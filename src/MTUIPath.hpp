//
//  MTUIPath.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 8/1/16.
//
//

#ifndef MTUIPath_h
#define MTUIPath_h

#include "ofMain.h"
#include "MTView.hpp"

class MTUIPathEventArgs;
class MTUIPathHandle;
/**
///
 Events:
 pathChangedEvent
    Triggered when a dragged handle (vertex or control point) is released

 pathVertexHandleMovedEvent
    Triggered when a vertex handle is moved

*/
class MTUIPath
{
//	friend class MTView;
public:
    ~MTUIPath();

    ///
    /// \brief Sets up the UIPath
    /// \param path is a pointer to the ofPath to be manipulated.
    /// It is not retained or stored in any way within this class,
    /// so make sure to store it somewhere!
    /// It is possible to modify the path outside and still have
    /// MTUIPath work as expected.
    /// \param view The MTView that the MTUIPath will be drawn on.
    /// The MTView is retained, so said view won't be destroyed
    /// until the MTUIPath is destroyed.
    /// The view also provides the reference coordinate system for
    /// the path and the MTUIPath.

    void setup(ofPath* path, shared_ptr<MTView> view);
    void draw();
    void setAutoDraw(bool autoDraw);
    void setVisibility(bool visible);
    void setClosed(bool closed);
    bool getClosed() { return isClosed; }
    void toggleVisibility();

    enum MTUIPathOptions
    {
        CanAddPoints,
        CanDeletePoints,
        CanConvertPoints
    };

    //DATA HANDLING
    /////////////////////////////////

    ///Returns true if the handle is found and deleted
    bool deleteHandle(shared_ptr<MTUIPathHandle> handle);

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
    void addToSelection(shared_ptr<MTUIPathHandle> handle);

    ///Removes a handle from the selection
    void removeFromSelection(shared_ptr<MTUIPathHandle> handle);

    ///Sets the selection to the specified handle, removing all others from the selection
    void setSelection(shared_ptr<MTUIPathHandle> handle);

    ///Removes all from selection
    void deselectAll();

    ///Take a guess...
    void selectAll();

    ///Gets the selected handles. It is not safe to modify this vector nor to change the Path commands directly.
    const vector<shared_ptr<MTView>> getSelection();


    ///These are called when the appropriate event occurs if useEventListeners = true.
    ///If you are handling events yourself make sure to call these functions when necessary.
    void mouseMoved(ofMouseEventArgs& args);
    void mousePressed(ofMouseEventArgs& args);
    void mouseReleased(ofMouseEventArgs& args);
    void mouseDragged(ofMouseEventArgs& args);
    void keyPressed(ofKeyEventArgs& args);
    void keyReleased(ofKeyEventArgs& args);

    void setUseAutoEventListeners(bool use);
    bool getUseAutoEventListeners() { return useAutoEventListeners; }

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

    ofPath* getPath() { return path; };

protected:
    typedef ofPath::Command ofPathCommand;
    ofPath* path = NULL;
    bool isClosed = false;
    vector<shared_ptr<MTUIPathHandle>> uiHandles;
    vector<shared_ptr<MTUIPathHandle>> selectedHandles;
    void handlePressed(MTUIPathHandle* pathHandle, ofMouseEventArgs &args);
    void handleReleased(MTUIPathHandle* pathHandle, ofMouseEventArgs &args);
    ofEventListener* drawListener;
    void drawEvent(ofEventArgs& args);

    bool isVisible = true;
    bool autoDraw = false;
    bool useAutoEventListeners = true;
    std::shared_ptr<MTView> view = nullptr;

    void arrangeClosedPath();
    void arrangeOpenPath();
    void addEventListeners();
    void removeEventListeners();

};

class MTUIHandle; // Forward declaration

/// \brief The MTUIPathHandle class wraps a set of handles that control
/// a vertex in a path.
class MTUIPathHandle : public MTEventListenerStore
{
    ofPath::Command* command;
    MTUIPath* uiPath;
    shared_ptr<MTUIHandle> toHandle;
    shared_ptr<MTUIHandle> cp1Handle;
    shared_ptr<MTUIHandle> cp2Handle;
    ofStyle currentStyle;

    bool mirroredControlPoints = false;
    bool useAutoEventListeners = true;

public:
    void setup(MTUIPath* uiPath, ofPath::Command* com);
    void setControlPoints();
    void setStyle(ofStyle newStyle);

    ///Sets whether the underlying ofMSAInteractiveObjects listen to events on their own. Defaults
    ///to true. If false, you are in charge of feeding event information to the MTView for
    ///proper functionality.
    void setAutoEventListeners(bool useEvents);
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
    shared_ptr<MTUIHandle> getPointHandle() { return toHandle; }
    shared_ptr<MTUIHandle> getCP1Handle() { return cp1Handle; }
    shared_ptr<MTUIHandle> getCP2Handle() { return cp2Handle; }
    const ofPath* getPath() { return uiPath->getPath(); }
    ofPath::Command* getCommand() { return command; }
    MTUIPath* getUIPath() { return uiPath; }

    ///Tests whether the point is inside the point handle or any of the control point handles
    bool hitTest(glm::vec2& point); //?

};


class MTUIHandle : public MTView
{
public:

    MTUIHandle(string _name);
    ~MTUIHandle();
    enum HandleType
    {
        SQUARE,
        ROUND
    };

    HandleType type;
    ofEvent<ofMouseEventArgs> handlePressedEvent;

//    std::shared_ptr<MTView> hitTest(glm::vec2 &windowCoord);
};


class MTUIPathEventArgs : public ofEventArgs
{
public:
    ofPath* path;
    MTUIPathHandle* pathHandle;
    void* userData;
//    glm::vec2* pointData;
};
#endif /* ofxUIPath_hpp */
