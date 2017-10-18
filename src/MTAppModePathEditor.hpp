#ifndef MTAPPMODEPATHEDITOR_HPP
#define MTAPPMODEPATHEDITOR_HPP

#include "ofxMTAppFramework.h"

class MTUIPath;

class PathEditorEventArgs : public ofEventArgs
{
public:
	std::shared_ptr<ofPath> path;
};


class PathEditorSettings
{
public:
	shared_ptr<MTView> view;

	/**
	 * @brief allowMultiplePaths
	 * @default true
	 */
	std::bitset<8> options;
	
	enum PathEditorOptions
	{
		AllowsMultiplePaths = 0,
		CanAddPaths,
		CanDeletePaths,
		CanAddPoints,
		CanDeletePoints,
		CanConvertPoints,
		PathsAreClosed,
		DragEventsNotified
	};


	/**
	 * @brief If @property allowMultiplePaths is true then this
	 * member must contain a valid vector of shared_ptr<ofPath>
	 */
	vector<std::shared_ptr<ofPath>> *paths;
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
	string appModeName = "";

	ofColor pathColor = ofColor::yellow;
	float pathStrokeWidth = 3;

	//TODO style options for pathEditor

};


class MTAppModePathEditor : public MTAppMode {
  public:
	MTAppModePathEditor(const PathEditorSettings &settings);
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
	std::function<void(PathEditorEventArgs)> onFirstPathCreated;
	std::function<void(PathEditorEventArgs)> onPathCreated;
	std::function<void(PathEditorEventArgs)> onPathModified;
	std::function<void(PathEditorEventArgs)> onPathDeleted;
	std::function<void()> onLastPathDeleted;
	std::function<void()> onExit;



  protected:
	PathEditorEventArgs pEventArgs;
	PathEditorSettings settings;
	shared_ptr<MTUIPath> createUIPath(std::shared_ptr<ofPath> p);
	bool removeUIPath(shared_ptr<MTUIPath> p);
	typedef ofPath::Command ofPathCommand;
	vector<std::shared_ptr<MTUIPath>> uiPaths;
	vector<std::shared_ptr<ofPath>> pathCollection;
	shared_ptr<MTUIPath> activeUIPath = nullptr;
	void handleMoved(const void* handle, ofMouseEventArgs& args);
	bool handleWasPressed = false;
};

#endif   // MTAPPMODEPATHEDITOR_HPP
