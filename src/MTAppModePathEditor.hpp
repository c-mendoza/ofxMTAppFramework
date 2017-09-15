#ifndef MTAPPMODEPATHEDITOR_HPP
#define MTAPPMODEPATHEDITOR_HPP

#include "ofxMTAppFramework.h"

class MTUIPath;

class PathEditorEventArgs : public ofEventArgs
{
public:
    std::shared_ptr<ofPath> path;
};

class MTAppModePathEditor : public MTAppMode {
  public:
    MTAppModePathEditor(PathEditorSettings& settings);
    virtual void setup();
    virtual void exit();
    virtual void draw();
    virtual void keyPressed(int key);
    virtual void keyReleased(int key);
    virtual void mouseMoved(int x, int y);
    virtual void mouseDragged(int x, int y, int button);
    virtual void mousePressed(int x, int y, int button);
    virtual void mouseReleased(int x, int y, int button);

    std::function<void()> onPathCreated = [] {};

    ofEvent<PathEditorEventArgs> pathCreatedEvent;
    ofEvent<PathEditorEventArgs> firstPathCreatedEvent;
    ofEvent<ofEventArgs> pathDeletedEvent;
    ofEvent<ofEventArgs> lastPathDeletedEvent;

    /**
     * @brief Fires when a vertex is added or deleted, or when a vertex is moved
     */
    ofEvent<PathEditorEventArgs> pathModifiedEvent;
    ofEvent<PathEditorEventArgs> pathCreatedEvent;

    class PathEditorSettings {
      public:
        shared_ptr<MTView> view;

        /**
         * @brief allowMultiplePaths
         * @default true
         */
        bool allowMultiplePaths = true;

        /**
         * @brief allowOpenPaths
         * @default true
         */
        bool allowOpenPaths = true;
        /**
         * @brief If @property allowMultiplePaths is true then this
         * member must contain a valid vector of shared_ptr<ofPath>
         */
        vector<std::shared_ptr<ofPath>>& paths;
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
        bool dragEventsNotified = false;
        int maxPaths = INT_MAX;
        string appModeName = "";

        //TODO style options for pathEditor

    };


  protected:
    PathEditorEventArgs pEventArgs;
    PathEditorSettings settings;
    shared_ptr<MTUIPath> createUIPath(std::shared_ptr<ofPath> p);
    bool removeUIPath(shared_ptr<MTUIPath> p);
    typedef ofPath::Command ofPathCommand;
    vector<shared_ptr<MTUIPath>> uiPaths;
    vector<std::shared_ptr<vector>> pathCollection;
    shared_ptr<MTUIPath> activeUIPath = nullptr;
    void handleMoved(const void* handle, ofMouseEventArgs& args);
    bool handleWasPressed = false;
};

#endif   // MTAPPMODEPATHEDITOR_HPP
