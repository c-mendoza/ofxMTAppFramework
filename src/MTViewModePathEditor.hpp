#ifndef MTAPPMODEPATHEDITOR_HPP
#define MTAPPMODEPATHEDITOR_HPP

#include "ofxMTAppFramework.h"
#include "ofPath.h"

class MTUIPath;
class MTViewModePathEditor;

class PathEditorEventArgs : public ofEventArgs
{
private:
	MTViewModePathEditor* pathEditor;
public:
	/**
	 * @brief The path that triggered the event
	 */
	std::shared_ptr<ofPath> path;

	/**
	 * @return A reference to the path editor
	 */
	MTViewModePathEditor& getPathEditor()
	{
		return *pathEditor;
	}

	friend class MTViewModePathEditor;
};

/**
 * @brief Class to initialize and configure an MTViewModePathEditor
 */
class PathEditorSettings
{
public:
    std::shared_ptr<MTView> view;

	enum Options
	{
        CanAddPoints = 0,
        CanDeletePoints,
        CanConvertPoints,
        LimitToRegion,
		AllowsMultiplePaths,
		CanAddPaths,
		CanDeletePaths,
		PathsAreClosed,
        NotifyOnHandleDragged,
        LimitToView
	};

    std::bitset<10> options;
	/**
	 * @brief If @property allowMultiplePaths is true then this
	 * member must contain a valid vector of shared_ptr<ofPath>
	 */
    std::vector<std::shared_ptr<ofPath>> paths;
	/**
	 * @brief If @property allowMultiplePaths is false then this
	 * member must contain a valid shared_ptr<ofPath>
	 */
	std::shared_ptr<ofPath> path;
	/**
	 * @brief If true, a pathModifiedEvent will be continuously fired while a
	 * handle is being dragged.
	 * When it is false, pathModifiedEvents will fire only when the drag is
	 * complete (in other words, when the mouse is released).
	 * @default is false;
	 */

	int maxPaths = INT_MAX;
    std::string appModeName = "";

	ofColor pathColor = ofColor::yellow;
	float pathStrokeWidth = 2;

    /**
     * @brief A rectantgular region that denotes the maximum and minimum values for the
     * handles in this editor. Points outside of the validRegion will be clamped.
     * If PathEditorOptions::LimitToRegion is set, you must provide a validRegion.
     * If PathEditorOptions::LimitToView is set, the validRegion will be created automatically
     * based on the view's frame.
     */
    ofRectangle validRegion;

	/**
	 * @brief An unordered_map containing valid regions keyed to the ofPath they correspond to,
	 * allowing the usage of a valid region for a given ofPath. If using this option, each ofPath
	 * must have a validRegion ofRectangle assigned to it.
	 * Both PathEditionOptions::LimitToRegion and PathEditorOptions::AllowMultiplePaths must
	 * be set for this option to be used.
	 * If the validRegionsMap is not assigned, either PathEditorSettings::validRegion must be provided
	 * or PathEditorOptions::LimitToView must be set.
	 */
	std::unordered_map<ofPath*, ofRectangle> validRegionsMap;
};

/**
 * @brief MTViewModePathEditor is a View Mode that allows users to interactively create,
 * edit, or delete, one or multiple paths using GUI handles and keyboard commands.
 * Once the GUI representation of the path is modified, an event is fired which lets
 * you react to the changes the user makes to the paths.
 *
 *
 */
class MTViewModePathEditor : public MTViewMode {
  public:
	MTViewModePathEditor(PathEditorSettings& settings);
protected:
	virtual void setup();
public:
	virtual void exit();
	virtual void draw();
	virtual void keyReleased(int key);
	virtual void mouseReleased(int x, int y, int button);


	/**
	 * @brief Fires when a vertex is added or deleted, or when a handle is moved.
	 * To avoid sending out notifications while the user drags a handle, unset the
	 * NotifyOnHandleDragged option in the PathEditorSettings::options bitfield.
	 * If NotifyOnHandleDragged is off and the user is dragging a handle, this
	 * event fires once dragging is done and the mouse is released.
	 */
	ofEvent<PathEditorEventArgs> pathModifiedEvent;

	/**
	 * @brief Fires when a path is created interactively.
	 */
	ofEvent<PathEditorEventArgs> pathCreatedEvent;

	/**
	 * @brief Fires when the first path is created interactively.
	 */
	ofEvent<PathEditorEventArgs> firstPathCreatedEvent;

	/**
	 * @brief Fires when a path is deleted.
	 */
	ofEvent<PathEditorEventArgs> pathDeletedEvent;

	/**
	 * @brief Fires when a path is deleted and there are no more paths in the
	 * path collection.
	 */
	ofEvent<ofEventArgs> lastPathDeletedEvent;

	/**
	 * @brief Convenience lambda for event.
	 */
	std::function<void(PathEditorEventArgs)> onFirstPathCreated = [](PathEditorEventArgs args){};
	/**
	 * @brief Convenience lambda for event.
	 */
	std::function<void(PathEditorEventArgs)> onPathCreated = [](PathEditorEventArgs args){};
	/**
	 * @brief Convenience lambda for event.
	 */
	std::function<void(PathEditorEventArgs)> onPathModified = [](PathEditorEventArgs args){};
	/**
	 * @brief Convenience lambda for event.
	 */
	std::function<void(PathEditorEventArgs)> onPathDeleted = [](PathEditorEventArgs args){};
	/**
	 * @brief Convenience lambda for event.
	 */
	std::function<void()> onLastPathDeleted = []{};
	/**
	 * @brief Convenience lambda for event.
	 */
	std::function<void()> onExit =[]{};

	std::vector<std::shared_ptr<ofPath>> getPathCollection();


  private:
	PathEditorEventArgs pEventArgs;
    std::shared_ptr<MTUIPath> createUIPath(std::shared_ptr<ofPath> p);
	bool removeUIPath(std::shared_ptr<MTUIPath> p);
    std::vector<std::shared_ptr<MTUIPath>> uiPaths;
    std::vector<std::shared_ptr<ofPath>> pathCollection;
    std::shared_ptr<MTUIPath> activeUIPath = nullptr;
	void handleMoved(const void* handle, ofMouseEventArgs& args);
	bool handleWasPressed = false;

	std::bitset<10> options;
	std::shared_ptr<ofPath> path;
	int maxPaths = INT_MAX;
//	std::string appModeName = "";
	ofColor pathColor;
	float pathStrokeWidth = 2;
	ofRectangle validRegion;
	std::unordered_map<ofPath*, ofRectangle> validRegionsMap;
};

#endif   // MTAPPMODEPATHEDITOR_HPP
