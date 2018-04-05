#ifndef ofxMTApp_hpp
#define ofxMTApp_hpp

#include <utils/ofXml.h>
#include <unordered_map>
#include <ofMain.h>
#include "GLFW/glfw3.h"
#include "ofxMTAppFramework.h"
#include "MTModel.hpp"
#include "ofxImGui.h"
#include "ofBaseApp.h"


class MTWindow;
class MTView;
class MTModel;
class MTViewMode;
class ofAppBaseWindow;
class ofWindowSettings;
class Gui;

typedef std::string MTAppStateName;


class MTAppStateChangeArgs;

struct MTDisplay
{
    std::string name;
    ofRectangle frame;
    int id;
};

class MTApp : public ofBaseApp, public MTEventListenerStore {

  public:
	MTApp();
	virtual ~MTApp();

	// TODO: Proper singleton
	static MTApp* sharedApp;

//	static ofxImGui::Gui gui;

	/**
	 * @brief initialize Extra "constructor" for the user. It is the last thing
	 * that is called in the default ofxMTApp contructor, and before the app's
	 * setup() function. This is where you want to instantiate your Model and
	 * your View. Default implementation creates a placeholder model and view.
	 */
	virtual void initialize();

	/**
	 * @brief createAppViews creates the app's views at initialization. This
	 * will set up your windows when the program launches or when a new MTModel
	 * (a document) is loaded. Override this to create your views.
	 */
	virtual void createAppViews();

	/**
	 * @brief appWillRun is called after all of the views and windows 
	 * have been created and immediately prior to the main loop running.
	 * Anything created in createAppViews should have had its setup() method
	 * called by now.
	 * Default implementation does nothing.
	 */
	virtual void appWillRun(){}
	virtual void runApp();

	template<class T>
	static std::shared_ptr<T> Model()
	{
		auto outModel = std::dynamic_pointer_cast<T>(MTApp::sharedApp->model);
		return outModel;
	}

	template<class T>
	static T* App()
	{
		return dynamic_cast<T>(MTApp::sharedApp);
	}

	//------ APP MODES
	const MTAppStateName defaultState = "MTAppStateDefault";
	void setAppState(MTAppStateName stateName);
	MTAppStateName getCurrentState();
	void registerState(MTAppStateName state) { appStates.push_back(state); }

	/**
	 * @brief Called whenever the App State changes. This function will get called
	 * before any other listener of the @code appModeChanged event.
	 * The default implementation does nothing.
	 */
	virtual void appStateChanged(MTAppStateChangeArgs& changeArgs){}

	static ofEvent<MTAppStateChangeArgs> appStateChangedEvent;
	static ofEvent<ofEventArgs> modelLoadedEvent;

	virtual void exit();

	//// UI
	std::weak_ptr<ofAppBaseWindow> getMainWindow();

	/**
	 * @brief Creates a window.
	 * @param windowName
	 * @param settings
	 * @return A shared_ptr to the MTWindow
	 */
	std::shared_ptr<MTWindow> createWindow(std::string windowName,
										 ofWindowSettings& settings);

	/**
	 * @brief Convenience method that creates a window and adds the passed view
	 * to the contentView of the created window.
	 * @param view
	 * @param windowName
	 * @param settings The ofWindowSettings for the window.
	 * @return A shared_ptr to the MTWindow
	 */
	std::shared_ptr<MTWindow> createWindowForView(std::shared_ptr<MTView> view,
												std::string windowName,
												ofWindowSettings& settings);

    void removeWindow(std::shared_ptr<MTWindow> window);
	/// Returns the mouse x-position in local coordinates of the current window
	int getLocalMouseX();
	/// Returns the mouse y-position in local coordinates of the current window
	int getLocalMouseY();

	void windowClosing(std::shared_ptr<MTWindow> window);

	/////// FILE HANDLING
	void saveAs();
	void open();
	void save();
	bool revert();
	bool saveAppPreferences();
	void newFile();

	/// Override this if you need to prep your app to create a new document.
	virtual void newFileSetup() {}

	void registerAppPreference(ofAbstractParameter& preference);

	/////// UTILITY
	/// Stringifies a path.
	static std::string pathToString(ofPath& path);

	/// Makes an ofPath from a stringified representation.
	static ofPath pathFromString(std::string s);
	ofParameter<std::string> MTPrefLastFile;
	ofParameter<bool> NSPrefAutoloadLastFile;
	ofParameter<bool> NSPrefLaunchInFullScreen;

	/** Sets the behavior for automatically updating and drawing
	 *  the App Modes. The default is set to true for both.
	 * Auto draw and auto update are called AFTER the user's
	 * update() and draw() calls.
	 **/
	void setAutoAppModeBehavior(bool autoDraw, bool autoUpdate)
	{
		autoDrawViewModes = autoDraw;
		autoUpdateAppModes = autoUpdate;
	}

	bool autoUpdateAppModes = true;
	bool autoDrawViewModes = true;

#pragma mark EVENTS
    /**
     * @brief Fires when displays are connected or disconnected.
     * When displays change, the MTApp::displays vector is recreated,
     * so if you are relying on MTDisplay references in your code
     * you will need to update them.
     */
    ofEvent<ofEventArgs> displaysChangedEvent;

  protected:
	ofXml serializer;
	ofXml appPrefsXml;

	/// The name of the current file.
	std::string fileName;

	/// Full path of the file
	std::string filePath;

	/// The file extension you want your documents to have. Defaults to ".xml",
	/// but it can be anything you want.
	std::string fileExtension = "xml";

	std::shared_ptr<MTWindow> mainWindow;
    std::shared_ptr<MTModel> model;
	const static std::string APP_PREFERENCES_FILE;
	bool isInitialized;
	ofParameterGroup appPreferences;
	ofParameterGroup MTPrefsWindowsGroup;

	std::vector<std::shared_ptr<MTWindow>> windows;

	virtual void keyPressed(ofKeyEventArgs& key);
	virtual void keyReleased(ofKeyEventArgs& key);

	/// Called whenever there is a key pressed anywhere in the app. Default
	/// implementation does nothing.
	virtual void appKeyPressed(ofKeyEventArgs& key){}

	/// Called whenever there is a key released anywhere in the app. Default
	/// implementation does nothing.
	virtual void appKeyReleased(ofKeyEventArgs& key){}

	/// Called once the model is loaded
	virtual void modelLoaded(){}

	// APP MODES
	MTAppStateName currentMode;
    std::vector<MTAppStateName> appStates;

	//////////////////////////////
	//	CONVENIENCE
	//////////////////////////////

	/// \brief Adds the standard event listeners to a window
	void addAllEvents(MTWindow* w);
	void removeAllEvents(MTWindow* w);

#pragma mark DISPLAY MANAGEMENT
public:

    /**
     * @brief Gets the currently connected displays.
     * @return A std::vector of MTDisplay structs.
     * @sa MTDisplay
     */
    static const std::vector<MTDisplay>& getDisplays() { return displays; }

    /**
     * @brief Updates the internal count of available displays.
     * You should not have to call this function.
     */
    static void updateDisplays();
    static void setMonitorCb(GLFWmonitor* monitor, int connected);
protected:
    static std::vector<MTDisplay> displays;

#pragma mark INTERNALS

  private:
	bool ofAppInitialized = false;

	bool saveAsImpl(std::string newName);
	bool saveImpl();
	bool openImpl(std::string file);

	void loadAppPreferences();
	//	void newFileImpl();

	// UI / Convenience
	//	void storeViewParameters(ofxMTView* view);

	const static std::string MTPrefsWindowsGroupName;
	const static std::string MTPrefsWindowPositionName;
	const static std::string MTPrefsWindowSizeName;

	ofEventListener exitHandler;

	struct WindowParams
	{
		std::string name;
		glm::vec2 position;
		glm::vec2 size;
	};

	std::unordered_map<std::string, WindowParams> wpMap;



};

class MTAppStateChangeArgs : public ofEventArgs {
  public:
	MTAppStateName newStateName;
	MTAppStateName oldStateName;
	MTAppStateChangeArgs() {}
};

int mtGetLocalMouseX();
int mtGetLocalMouseY();

#endif
