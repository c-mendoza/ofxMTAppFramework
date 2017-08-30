#ifndef ofxMTApp_hpp
#define ofxMTApp_hpp

#include "ofxMTAppFramework.h"

class MTWindow;
class MTView;
class MTModel;
class MTAppMode;

typedef string MTAppModeName;

class MTAppModeChangeArgs;

class MTApp : public ofBaseApp, public MTEventListenerStore
{

public:
    MTApp();
	virtual ~MTApp();

    //TODO: Proper singleton
    static MTApp* sharedApp;

    /// Extra "constructor" for the user. It is the last thing that is called in the default ofxMTApp contructor,
    /// and before the app's setup() function. This is where you want to instantiate your Model and your View.
    /// Default implementation creates a placeholder model and view.
    virtual void initialize();


    /// Creates the app's views at initialization. This will set up your windows when the program launches or when
    /// a new MTModel (a document) is loaded. Override this to create your views.
    virtual void createAppViews();

    virtual void run();

    //------ APP MODES
    const MTAppModeName defaultMode = "MTAppModeDefault";
    void setAppMode(MTAppModeName mode);
    MTAppModeName getCurrentMode();
    void registerMode(MTAppModeName mode)
    {
        appModes.push_back(mode);
    }

    static ofEvent<MTAppModeChangeArgs> appChangeModeEvent;
    static ofEvent<ofEventArgs> modelLoadedEvent;

    virtual void exit();

    //// UI
    weak_ptr<ofAppBaseWindow> getMainWindow();
    shared_ptr<MTWindow> createWindow(string windowName, ofWindowSettings& settings);

    ///Returns the mouse x-position in local coordinates of the current window
    int getLocalMouseX();
    ///Returns the mouse y-position in local coordinates of the current window
    int getLocalMouseY();

    void windowClosing(MTWindow* window);

    /////// FILE HANDLING
    void saveAs();
    void open();
    void save();
    bool revert();
    bool saveAppPreferences();
    void newFile();

    /// Override this if you need to prep your app to create a new document.
    virtual void newFileSetup(){}

    void registerAppPreference(ofAbstractParameter& preference);

    virtual shared_ptr<MTModel> getModel() { return model; }

    /////// UTILITY
    /// Stringifies a path.
    static string pathToString(ofPath& path);

    /// Makes an ofPath from a stringified representation.
    static ofPath pathFromString(string s);
    ofParameter<string> MTPrefLastFile;
    ofParameter<bool> NSPrefAutoloadLastFile;
    ofParameter<bool> NSPrefLaunchInFullScreen;

    /** Sets the behavior for automatically updating and drawing
     *  the App Modes. The default is set to true for both.
     * Auto draw and auto update are called AFTER the user's
     * update() and draw() calls.
     **/
    void setAutoAppModeBehavior(bool autoDraw, bool autoUpdate)
    {
        autoDrawAppModes = autoDraw;
        autoUpdateAppModes = autoUpdate;
    }

    bool autoUpdateAppModes = true;
    bool autoDrawAppModes = true;

protected:

    ofXml serializer;
    ofXml appPrefsXml;

    /// The name of the current file.
    string fileName;

    /// Full path of the file
    string filePath;

    /// The file extension you want your documents to have. Defaults to ".xml", but it can be anything you want.
    string fileExtension = "xml";

    shared_ptr<MTWindow> mainWindow;
    shared_ptr<MTModel> model;
    const static string APP_PREFERENCES_FILE;
    bool isInitialized;
    ofParameterGroup appPreferences;
    ofParameterGroup MTPrefsWindowsGroup;

    //TODO: make these private?
    std::vector<std::shared_ptr<MTWindow>> windows;

    virtual void keyPressed(ofKeyEventArgs &key);
    virtual void keyReleased(ofKeyEventArgs &key);

    /// Called whenever there is a key pressed anywhere in the app. Other than the built-in behavior, default
    /// implementation does nothing.
    virtual void appKeyPressed(int key){};

    /// Called whenever there is a key released anywhere in the app. Other than the built-in behavior, default
    /// implementation does nothing.
    virtual void appKeyReleased(int key){};

    /// Called once the model is loaded
    virtual void modelLoaded(){};

    //APP MODES
    MTAppModeName currentMode;
    vector<MTAppModeName> appModes;

    //////////////////////////////
    //	CONVENIENCE
    //////////////////////////////

    /// \brief Adds the standard event listeners to a window
    void addAllEvents(MTWindow* w);
    void removeAllEvents(MTWindow* w);

private:
    bool ofAppInitialized = false;

    bool saveAsImpl(string newName);
    bool saveImpl();
    bool openImpl(string file);

    void loadAppPreferences();
//	void newFileImpl();

    //UI / Convenience
//	void storeViewParameters(ofxMTView* view);

    const static string MTPrefsWindowsGroupName;
    const static string MTPrefsWindowPositionName;
    const static string MTPrefsWindowSizeName;

    ofEventListener exitHandler;

    struct WindowParams
    {
        string name;
        glm::vec3 position;
        glm::vec3 size;
    };

    std::unordered_map<std::string, WindowParams> wpMap;
};

class MTAppModeChangeArgs : public ofEventArgs
{
public:
    MTAppModeName newMode;
    MTAppModeName oldMode;
    MTAppModeChangeArgs(){}

};

int mtGetLocalMouseX();
int mtGetLocalMouseY();


#endif
