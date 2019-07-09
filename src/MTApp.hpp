#ifndef ofxMTApp_hpp
#define ofxMTApp_hpp

#include <ofMain.h>
#include <utils/ofXml.h>
#include <unordered_map>
#include "GLFW/glfw3.h"
#include "MTAppFrameworkUtils.hpp"
#include "ofxImGui.h"
#include "ofBaseApp.h"

class MTWindow;
class MTView;
class MTModel;
class MTViewMode;
class ofAppBaseWindow;
class ofWindowSettings;
class MTOffscreenWindow;
class ofAppEGLWindowSettings;

typedef std::string MTAppModeName;

class MTAppModeChangeArgs;

class MTDisplay
{
private:
	std::string name;
	ofRectangle frame;
	int id;
public:
	MTDisplay(std::string name, ofRectangle frame, int id) {
		this->name = name;
		this->frame = frame;
		this->id = id;
	}

	const string& getName() const
	{
		return name;
	}

	const ofRectangle& getFrame() const
	{
		return frame;
	}

	int getId() const
	{
		return id;
	}
};

//template<typename T>
//class MTModelStore
//{
//	std::shared_ptr<T> model;
//
//	void setModel(std::shared_ptr<T> _model)
//	{
//		model = _model;
//	}
//
//	std::shared_ptr<T> getModel()
//	{
//		return model;
//	}
//};

class MTApp : public ofBaseApp, public MTEventListenerStore
{

public:
	MTApp();
	virtual ~MTApp();

	// TODO: Proper singleton
	static MTApp *sharedApp;

	/**
	 * @brief initialize  Override this method to instantiate your model,
	 * register custom preferences and other general app setup. Called before
	 * createAppViews, so unless you create a Window in this method there will
	 * be no available GPU Contexts. Default implementation does nothing.
	 */
	virtual void initialize();

	/**
	 * @brief createAppViews Override this method to create your app's windows
	 * and views. Called before the app's run loop begins, so it is
	 * called only once for the life of the app. Default implementation creates
	 * a window.
	 */
	virtual void createAppViews();

	/**
	 * @brief appWillRun is called after all of the views and windows 
	 * have been created and immediately prior to the main loop running.
	 * Anything created in createAppViews should have had its setup() method
	 * called by now.
	 * Default implementation does nothing.
	 */
	virtual void appWillRun()
	{}

	/**
	 * @brief Starts the app's run loop. You shouldn't have to call this
	 * method yourself.
	 */
	void runApp();

	// I'm sure that there is a better way than this, but right now...
	template<class T>
	static std::shared_ptr<T> Model()
	{
		auto outModel = std::dynamic_pointer_cast<T>(MTApp::sharedApp->model);
		return outModel;
	}

	// I'm sure that there is a better way than this, but right now...
	template<class T>
	static T *App()
	{
		return dynamic_cast<T>(MTApp::sharedApp);
	}

	//------ APP MODES
	const MTAppModeName defaultMode = "MTAppModeDefault";

	/**
	 * @brief Changes the current App Mode, which triggers an
	 * appModeChangedEvent.
	 * @param modeName The new App Mode name.
	 */
	void setAppMode(MTAppModeName modeName);
	MTAppModeName getAppMode();

	/**
	 * @brief Usage of this method is currently not required, and has no real
	 * effect on the app.
	 * In the future, this method will register App Modes
	 * and give setAppMode a bit of input validation.
	 * @param modeName
	 */
	void registerAppMode(MTAppModeName modeName)
	{ appModes.push_back(modeName); }

	/**
	 * @brief Called whenever the App Mode changes. This conveninece methdd
	 * will be called before any other listener of the @code appModeChanged
	 * event.
	 * The default implementation does nothing.
	 */
	virtual void appModeChanged(MTAppModeChangeArgs &changeArgs)
	{}

	virtual void exit();

	//// UI
	std::weak_ptr<ofAppBaseWindow> getMainWindow();

#ifndef TARGET_RASPBERRY_PI
	std::shared_ptr<MTOffscreenWindow>
	createOffscreenWindow(std::string windowName, ofGLFWWindowSettings &settings, bool useTextureRectangle = true);
#endif

#ifdef TARGET_OPENGLES
	/**
	 * @brief Convenience method that creates a window and adds the passed view
	 * to the contentView of the created window.
	 * @param view
	 * @param windowName
	 * @param settings The ofWindowSettings for the window.
	 * @return A shared_ptr to the MTWindow
	 */
std::shared_ptr<MTWindow> createWindow(std::string windowName,
									   ofAppEGLWindowSettings& settings);
#else

	/**
	 * @brief Creates a window.
	 * @param windowName
	 * @param settings
	 * @return A shared_ptr to the MTWindow
	 */
	std::shared_ptr<MTWindow> createWindow(std::string windowName, ofGLFWWindowSettings &settings);
#endif


	void closeWindow(std::shared_ptr<MTWindow> window);
//	void removeWindow(std::shared_ptr<MTWindow> window);
//	/// Returns the mouse x-position in local coordinates of the current window
//	int getLocalMouseX();
//	/// Returns the mouse y-position in local coordinates of the current window
//	int getLocalMouseY();

	/**
	 * @brief You should not have to call this method.
	 * Event handler for the windowClosing event.
	 * @param window
	 */
	void windowClosing(std::shared_ptr<MTWindow> window);

	/////// FILE HANDLING
	void saveAs();
	void open();
	void save();
	bool revert();
	bool saveAppPreferences();
	void newFile();

	/**
	 * @brief Override this if you need to do anything when creating a new, empty
	 * document.
	 */
	virtual void newFileSetup()
	{}

	/**
	 * @brief Registers a new app preference. App preferences are saved
	 * automatically prior to the app closing.
	 * @param preference An ofParameter
	 */
	void registerAppPreference(ofAbstractParameter &preference);

	/////// UTILITY
	/**
	 * @brief Stringifies an ofPath.
	 */
	static std::string pathToString(ofPath &path);

	/**
	 * @brief Makes an ofPath from a stringified representation.
	 * @param s The string to parse
	 * @return an ofPath. If the string parsing fails the path will
	 * be empty. TODO: Catching parsing errors.
	 */
	static ofPath pathFromString(std::string s);

	ofParameter<std::string> MTPrefLastFile;
	ofParameter<bool> MTPrefAutoloadLastFile;
	ofParameter<bool> MTPrefLaunchInFullScreen;

	/**
	 * @brief Sets the behavior for automatically updating and drawing
	 * the App Modes. The default is set to true for both.
	 * Auto draw and auto update are called AFTER the user's
	 * update() and draw() calls.
	 * @param autoDraw
	 * @param autoUpdate
	 */
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

	/**
	 * Fires after MTApp::setAppMode is called
	 */
	ofEvent<MTAppModeChangeArgs> appModeChangedEvent;

	/**
	 * Fires after a model is loaded from the file system.
	 */
	ofEvent<ofEventArgs> modelLoadedEvent;

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

	/**
	 * The app preferences file name. It can be anything you want, but to avoid
	 * naming conflicts it is recommended that you use reverse DNS notation.
	 * Make sure to change the value of this field in your subclass, ideally
	 * in the @code initialize() method.
	 * Where this file is written to depends on the target OS.
	 */
	std::string appPreferencesFilename = "com.yourGroup.yourApp.prefs";
	bool isInitialized;

	/**
	 * @brief ofParameterGroup for the App Preferences. You shouldn't have to
	 * mess with this directly. This should probably be private in the future.
	 */
	ofParameterGroup appPreferences;

	std::vector<std::shared_ptr<MTWindow>> windows;

	/**
	 * @brief Event handler. You should not have to call this method.
	 */
	virtual void keyPressed(ofKeyEventArgs &key);
	/**
	 * @brief Event handler. You should not have to call this method.
 	 */
	virtual void keyReleased(ofKeyEventArgs &key);

	/**
	 * @brief Called whenever there is a key pressed anywhere in the app.
	 * Override this to detect key presses globally.
	 * Default implementation does nothing.
	 */
	virtual void appKeyPressed(ofKeyEventArgs &key)
	{}

	/**
	 * @brief Called whenever there is a key released anywhere in the app.
	 * Override this to detect key presses globally.
	 * Default implementation does nothing.
	 */
	virtual void appKeyReleased(ofKeyEventArgs &key)
	{}

	/**
	 * @brief Called once the model is loaded from disk. This is mostly a
	 * convenience method so that you don't have to listen for the
	 * modelLoadedEvent yourself. Default implementation does nothing.
	 */
	virtual void modelLoaded()
	{}

	// APP MODES
	MTAppModeName currentMode;
	std::vector<MTAppModeName> appModes;

	//////////////////////////////
	//	CONVENIENCE
	//////////////////////////////

	/**
	 * 	Adds the standard event listeners to a window. You probably won't
	 * 	need to call this method.
	 */
	void addAllEvents(MTWindow *w);

	/**
 	 * 	Removes the standard event listeners to a window. You probably won't
 	 * 	need to call this method.
 	 */
	void removeAllEvents(MTWindow *w);

#pragma mark DISPLAY MANAGEMENT
public:

	/**
	 * @brief Gets the currently connected displays.
	 * @return A std::vector of shared_ptr<MTDisplay>.
	 * @sa MTDisplay
	 */
	static const std::vector<std::shared_ptr<MTDisplay>> &getDisplays()
	{ return displays; }

	/**
	 * @brief Updates the internal count of available displays.
	 * You should not have to call this function.
	 */
	static void updateDisplays();

#ifndef TARGET_OPENGLES
	static void setMonitorCb(GLFWmonitor *monitor, int connected);
#endif

protected:
	static std::vector<std::shared_ptr<MTDisplay>> displays;

#pragma mark INTERNALS

private:
	bool ofAppInitialized = false;

	bool saveAsImpl(std::string newName);
	bool saveImpl();
	bool openImpl(std::string file);

	void loadAppPreferences();

	struct WindowParams
	{
		std::string name;
		glm::vec2 position;
		glm::vec2 size;
	};

	std::unordered_map<std::string, WindowParams> wpMap;

	/**
	 * ~/.local/share/filename
	 * ~/Library/Preferences/filename
	 * %systemdrive%%homepath%\Roaming\filename
	 *
	 */
	std::filesystem::path appPreferencesPath = "";
	void createAppPreferencesFilePath();
};

class MTAppModeChangeArgs : public ofEventArgs
{
public:
	MTAppModeName newModeName;
	MTAppModeName oldModeName;

	MTAppModeChangeArgs()
	{}
};

int mtGetLocalMouseX();
int mtGetLocalMouseY();

#endif
