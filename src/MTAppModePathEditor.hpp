#ifndef MTAPPMODEPATHEDITOR_HPP
#define MTAPPMODEPATHEDITOR_HPP

#include "ofxMTAppFramework.h"
#include "ofPath.h"

class MTUIPath;

class PathEditorEventArgs : public ofEventArgs
{
public:
	std::shared_ptr<ofPath> path;
};


class PathEditorSettings
{
public:
    std::shared_ptr<MTView> view;

	
	enum PathEditorOptions
	{
        CanAddPoints = 0,
        CanDeletePoints,
        CanConvertPoints,
        LimitToRegion,
		AllowsMultiplePaths,
		CanAddPaths,
		CanDeletePaths,
		PathsAreClosed,
		DragEventsNotified,
        LimitToView
	};

    std::bitset<10> options;
	/**
	 * @brief If @property allowMultiplePaths is true then this
	 * member must contain a valid vector of shared_ptr<ofPath>
	 */
    std::vector<std::shared_ptr<ofPath>> *paths;
	/**
	 * @brief If @property allowMultiplePaths is false then this
	 * member must contain a valid shared_ptr<ofPath>
	 */
	std::shared_ptr<ofPath> path;
	/**
	 * @brief If true, a pathModifiedEvent will be continuosly fired while a
	 * handle is being dragged.
	 * When it is false, pathModifiedEvents will fire only when the drag is
	 * complete (in other words, when the mouse is released).
	 * @default is false;
	 */

	int maxPaths = INT_MAX;
    std::string appModeName = "";

	ofColor pathColor = ofColor::yellow;
	float pathStrokeWidth = 3;

    /**
     * @brief A rectantgular region that denotes the maximum and minimum values for the
     * handles in this editor. Points outside of the validRegion will be clamped.
     * If PathEditorOptions::LimitToRegion is set, you must provide a validRegion.
     * If PathEditorOptions::LimitToView is set, the validRegion will be created automatically
     * based on the view.
     */
    ofRectangle validRegion;

	//TODO style options for pathEditor

};


class MTAppModePathEditor : public MTAppMode {
  public:
	MTAppModePathEditor(PathEditorSettings& settings);
	virtual void setup();
	virtual void exit();
	virtual void draw();
	virtual void keyReleased(int key);
	virtual void mouseReleased(int x, int y, int button);


	/**
	 * @brief Fires when a vertex is added or deleted, or when a vertex is moved
	 */
	ofEvent<PathEditorEventArgs> pathModifiedEvent;
	ofEvent<PathEditorEventArgs> pathCreatedEvent;
	ofEvent<PathEditorEventArgs> firstPathCreatedEvent;
	ofEvent<PathEditorEventArgs> pathDeletedEvent;
	ofEvent<ofEventArgs> lastPathDeletedEvent;

	/**
	 * @brief onFirstPathCreated
	 */
	std::function<void(PathEditorEventArgs)> onFirstPathCreated = [](PathEditorEventArgs args){};
	std::function<void(PathEditorEventArgs)> onPathCreated = [](PathEditorEventArgs args){};
	std::function<void(PathEditorEventArgs)> onPathModified = [](PathEditorEventArgs args){};
	std::function<void(PathEditorEventArgs)> onPathDeleted = [](PathEditorEventArgs args){};
	std::function<void()> onLastPathDeleted = []{};
	std::function<void()> onExit =[]{};



  protected:
	PathEditorEventArgs pEventArgs;
	PathEditorSettings settings;
    std::shared_ptr<MTUIPath> createUIPath(std::shared_ptr<ofPath> p);
	bool removeUIPath(std::shared_ptr<MTUIPath> p);
	typedef ofPath::Command ofPathCommand;
    std::vector<std::shared_ptr<MTUIPath>> uiPaths;
    std::vector<std::shared_ptr<ofPath>> pathCollection;
    std::shared_ptr<MTUIPath> activeUIPath = nullptr;
	void handleMoved(const void* handle, ofMouseEventArgs& args);
	bool handleWasPressed = false;
};

#endif   // MTAPPMODEPATHEDITOR_HPP
