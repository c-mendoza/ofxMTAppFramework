#ifndef ofxMTApp_hpp
#define ofxMTApp_hpp

#include <utils/ofXml.h>
#include <unordered_map>
#include "GLFW/glfw3.h"
#include "MTAppFrameworkUtils.hpp"
#include "ofxImGui.h"
#include "ofBaseApp.h"
#include "ofAppRunner.h"
#include "ofAppGLFWWindow.h"
#include "ofWindowSettings.h"
#include "ofSystemUtils.h"
#include "ofPath.h"
#include "MTView.hpp"
#include "MTViewMode.hpp"
#include "MTOffscreenWindow.hpp"
#include "MTWindow.hpp"



typedef std::string MTAppModeName;

struct MTDisplay
{
	std::string name;
	ofRectangle frame;
	int id;
};

template<typename AppType, typename ModelType>
class MTApp : public ofBaseApp, public MTEventListenerStore
{

public:
	typedef MTApp<AppType, ModelType> MTBaseApp;
	MTApp();
	virtual ~MTApp();

	// TODO: Proper singleton

	static AppType& Instance()
	{
		static AppType instance;
		return instance;
	}

	static std::shared_ptr<ModelType> Model()
	{
		return MTApp::Instance().model;
	}

	static void RunApp()
	{
		MTApp::Instance().runApp();
	}

	friend AppType;

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
	 * @brief Starts the app's run loop.
	 */
	void runApp();


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
	virtual void appModeChanged(MTAppModeChangeArgs& changeArgs)
	{}

	virtual void exit();

	//// UI
	std::weak_ptr<ofAppBaseWindow> getMainWindow();

	std::shared_ptr<MTOffscreenWindow>
	createOffscreenWindow(std::string windowName, ofGLFWWindowSettings& settings, bool useTextureRectangle = true);

	/**
	 * @brief Creates a window.
	 * @param windowName
	 * @param settings
	 * @return A shared_ptr to the MTWindow
	 */
	std::shared_ptr<MTWindow> createWindow(std::string windowName, ofGLFWWindowSettings& settings);

//	/**
//	 * @brief Convenience method that creates a window and adds the passed view
//	 * to the contentView of the created window.
//	 * @param view
//	 * @param windowName
//	 * @param settings The ofWindowSettings for the window.
//	 * @return A shared_ptr to the MTWindow
//	 */
//	std::shared_ptr<MTWindow> createWindowForView(std::shared_ptr<MTView> view,
//												std::string windowName,
//												ofWindowSettings& settings);
	void removeWindow(std::shared_ptr<MTWindow> window);
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
	void registerAppPreference(ofAbstractParameter& preference);

	/////// UTILITY
	/**
	 * @brief Stringifies an ofPath.
	 */
	static std::string pathToString(ofPath& path);

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
	std::shared_ptr<ModelType> model;

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
	virtual void keyPressed(ofKeyEventArgs& key);
	/**
	 * @brief Event handler. You should not have to call this method.
 	 */
	virtual void keyReleased(ofKeyEventArgs& key);

	/**
	 * @brief Called whenever there is a key pressed anywhere in the app.
	 * Override this to detect key presses globally.
	 * Default implementation does nothing.
	 */
	virtual void appKeyPressed(ofKeyEventArgs& key)
	{}

	/**
	 * @brief Called whenever there is a key released anywhere in the app.
	 * Override this to detect key presses globally.
	 * Default implementation does nothing.
	 */
	virtual void appKeyReleased(ofKeyEventArgs& key)
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
	void addAllEvents(MTWindow* w);

	/**
 	 * 	Removes the standard event listeners to a window. You probably won't
 	 * 	need to call this method.
 	 */
	void removeAllEvents(MTWindow* w);

#pragma mark DISPLAY MANAGEMENT
public:

	/**
	 * @brief Gets the currently connected displays.
	 * @return A std::vector of MTDisplay structs.
	 * @sa MTDisplay
	 */
	static const std::vector<MTDisplay>& getDisplays()
	{ return displays; }

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

template<typename AppType, typename ModelType>
MTApp<AppType, ModelType>::MTApp()
{

	// TODO: Proper singleton
	// Set the basic preferences
	MTPrefLastFile.set("NSPrefLastFile", "");
	MTPrefAutoloadLastFile.set("MTPrefAutoloadLastFile", false);
	MTPrefLaunchInFullScreen.set("MTPrefLaunchInFullScreen", false);
	appPreferences.setName("App Preferences");
	appPreferences.add(MTPrefLaunchInFullScreen, MTPrefLastFile, MTPrefAutoloadLastFile);

	currentMode = defaultMode;
	registerAppMode(defaultMode);

	fileExtension = "";

	ofInit();
	glfwInit();
	MTApp::updateDisplays();
//		ofSetLogLevel(OF_LOG_NOTICE);

	model = std::make_shared<ModelType>();

	addEventListener(modelLoadedEvent.newListener([this](ofEventArgs& args)
												  { modelLoaded(); }, OF_EVENT_ORDER_BEFORE_APP));
}


template<typename AppType, typename ModelType>
MTApp<AppType, ModelType>::~MTApp()
{
	saveAppPreferences();
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::loadAppPreferences()
{
	if (!appPrefsXml.load(appPreferencesPath))
	{
		ofLog(OF_LOG_NOTICE, "App preferences not found, creating a new file.");
		saveAppPreferences();
	}
	else
	{
		ofDeserialize(appPrefsXml, appPreferences);

		// Load the saved view positions:
		auto windowsXml = appPrefsXml.find("//Windows/Window");
		//			auto windowsXml =
		// appPrefsXml.getChildren("windows");

		for (auto& winXml : windowsXml)
		{
			WindowParams wp;
			wp.name = winXml.getChild("Name").getValue();
			wp.position = winXml.getChild("Position").template getValue<glm::vec2>();
			wp.size = winXml.getChild("Size").template getValue<glm::vec2>();
			wpMap.insert({wp.name, wp});
		}
	}
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::registerAppPreference(ofAbstractParameter& preference)
{
	appPreferences.add(preference);
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::keyPressed(ofKeyEventArgs& key)
{
	appKeyPressed(key);
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::keyReleased(ofKeyEventArgs& key)
{
//	if (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))
//	{
//		auto k = key.key;
//		int bla = 'o';
//		if (k == 15)
//		{
//			open();
//		}
//		else if (k == 19)
//		{
//			if (ofGetKeyPressed(OF_KEY_SHIFT))
//			{
//				saveAs();
//			}
//			else
//			{
//				save();
//			}
//		}
//	}
//
//	appKeyReleased(key);

	if (key.hasModifier(OF_KEY_COMMAND))
	{
		if (key.codepoint == 'o')
		{
			open();
		}
		else if (key.key == 's')
		{
			if (key.hasModifier(OF_KEY_SHIFT))
			{
				saveAs();
			}
			else
			{
				save();
			}

		}
	}
	appKeyReleased(key);
}
/// Method is called in the MTApp constructor, right before the app is run.
/// Override this method and instantiate your model and main view classes, as
/// well as the main
/// window size and settings.

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::initialize()
{
	// These should be overriden sometime
	//	model = shared_ptr<ofxMTModel>(new ofxMTModel("default"));
	model = nullptr;
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::createAppViews()
{
	ofGLFWWindowSettings windowSettings;
	windowSettings.setSize(1280, 800);
	mainWindow = createWindow("Main Window", windowSettings);
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::runApp()
{
	initialize();
	createAppPreferencesFilePath();
	loadAppPreferences();
	createAppViews();
	appWillRun();
	ofRunApp(std::dynamic_pointer_cast<ofAppBaseWindow>(windows.front()), std::shared_ptr<ofBaseApp>(this));

//	MTApp::gui.setup();
	// Only the first window gets notified of setup when ofRunApp is called
	// so we need to do that ourselves:
	for (auto iter = windows.begin() + 1; iter < windows.end(); iter++)
	{
		(*iter)->events().notifySetup();
	}

//	ofAddListener(ofEvents().keyPressed, this, &MTApp::keyPressed);
	isInitialized = false;

	if (MTPrefAutoloadLastFile)
	{
		isInitialized = openImpl(MTPrefLastFile);

		if (!isInitialized)
		{
			std::string msg = "Tried to open " + MTPrefLastFile.get() + " but could not find it";
			ofLogNotice("MTApp") << msg;
			ofSystemAlertDialog(msg);
			isInitialized = true;
			newFile();
		}
	}
	else
	{
		isInitialized = true;
		newFile();
	}

	glfwSetMonitorCallback(&setMonitorCb);
	appWillRun();
	ofLogVerbose("MTApp") << "Running Main Loop";
	ofRunMainLoop();

//	ImGui::Shutdown();
}

/// APP MODES

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::setAppMode(MTAppModeName mode)
{
	//	if(!ofContains(appModes, mode)) return;

	if (mode == currentMode)
	{
		return;
	}
	else
	{
		static MTAppModeChangeArgs changeArgs;
		changeArgs.newModeName = mode;
		changeArgs.oldModeName = currentMode;
		currentMode = mode;
		appModeChanged(changeArgs);
		ofNotifyEvent(MTApp::appModeChangedEvent, changeArgs, this);
	}
}

template<typename AppType, typename ModelType>
MTAppModeName MTApp<AppType, ModelType>::getAppMode()
{
	return currentMode;
}

// TODO: Revisit MTApp::mainWindow. Think about either removing it or making it a weak_ptr

//std::shared_ptr<MTWindow> MTApp::createWindowForView(std::shared_ptr<MTView> view,
//												   std::string windowName,
//												   ofWindowSettings& settings)
//{
//	auto win = createWindow(windowName, settings);
//	view->setSize(win->getWidth(), win->getHeight());
//	win->contentView->addSubview(view);
//	return win;
//}

template<typename AppType, typename ModelType>
std::shared_ptr<MTOffscreenWindow> MTApp<AppType, ModelType>::createOffscreenWindow(std::string windowName,
																					ofGLFWWindowSettings& settings,
																					bool useTextureRectangle)
{
	auto offscreenWindow = std::make_shared<MTOffscreenWindow>(windowName, useTextureRectangle);
//	ofGLFWWindowSettings* glfwWS = dynamic_cast<ofGLFWWindowSettings*>(&settings);
	offscreenWindow->setup(settings);
	addAllEvents(offscreenWindow.get());
	ofGetMainLoop()->addWindow(offscreenWindow);
	windows.push_back(offscreenWindow);

	glfwHideWindow(offscreenWindow->getGLFWWindow());

	// The ofApp system only notifies setup for the first window it creates,
	// the rest are on their own apparently. So we check if we have initialized
	// the ofApp system, and if we have, then
	// that means that we need to notify setup for the window we are creating

	if (!ofAppInitialized)
	{
		ofAppInitialized = true;
	}
	else
	{
		// Note that MTView::setup is not called from this event, only the
		// MTWindow's setup
		offscreenWindow->events().notifySetup();
	}

	return offscreenWindow;
}

template<typename AppType, typename ModelType>
std::shared_ptr<MTWindow>
MTApp<AppType, ModelType>::createWindow(std::string windowName, ofGLFWWindowSettings& settings)
{

	auto window = std::make_shared<MTWindow>(windowName);
	ofGetMainLoop()->addWindow(window);
	windows.push_back(window);
	window->setup(settings);

	addAllEvents(window.get());

	// Who is adding these listeners?? It doesn't seem to be necessary
	// to add them here for some reason...

	// Add the "global" keyboard event listener:
//	ofAddListener(window->events().keyPressed,
//				  this,
//				  &MTApp::keyPressed,
//				  OF_EVENT_ORDER_BEFORE_APP);
//	ofAddListener(window->events().keyReleased,
//				  this,
//				  &MTApp::keyReleased,
//				  OF_EVENT_ORDER_BEFORE_APP);
	//    window->events().

	// Restore the window position and shape:
	auto wp = wpMap.find(windowName);
	if (wp != wpMap.end())
	{
		window->setWindowShape(wp->second.size.x, wp->second.size.y);
		window->setWindowPosition(wp->second.position.x, wp->second.position.y);
		window->setWindowTitle(windowName);
	}
	else
	{
		WindowParams wp;
		wp.name = windowName;
		wp.position = window->getWindowPosition();
		wp.size = window->getWindowSize();
		wpMap[wp.name] = wp;
	}

	// The ofApp system only notifies setup for the first window it creates,
	// the rest are on their own apparently. So we check if we have initialized
	// the ofApp system, and if we have, then
	// that means that we need to notify setup for the window we are creating

	if (!ofAppInitialized)
	{
		ofAppInitialized = true;
	}
	else
	{
		// Note that MTView::setup is not called from this event, only the
		// MTWindow's setup
		window->events().notifySetup();
	}

//	ofAddListener(window->events().keyPressed, this, &MTApp::keyPressed);
//	ofAddListener(window->events().keyReleased, this, &MTApp::keyReleasedInternal);

	return window;
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::removeWindow(std::shared_ptr<MTWindow> window)
{
	auto wIter = std::find(windows.begin(), windows.end(), window);
	windows.erase(wIter);
	window->setWindowShouldClose(); // Likely redundant, but probably harmless
	removeAllEvents(window.get());
//    ofRemoveListener(window->events().keyPressed,
//                  this,
//                  &MTApp::keyPressed,
//                  OF_EVENT_ORDER_BEFORE_APP);
//    ofRemoveListener(window->events().keyReleased,
//                  this,
//                  &MTApp::keyReleased,
//                  OF_EVENT_ORDER_BEFORE_APP);
//    wpMap.erase(window->name);
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::addAllEvents(MTWindow* w)
{
	// clang-format off
	w->events().enable();
	ofAddListener(w->events().setup, w, &MTWindow::setupInternal, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().update, w, &MTWindow::update, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().draw, w, &MTWindow::draw, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().exit, w, &MTWindow::exit, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(w->events().keyPressed, w, &MTWindow::keyPressed, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().keyReleased, w, &MTWindow::keyReleased, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseMoved, w, &MTWindow::mouseMoved, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseDragged, w, &MTWindow::mouseDragged, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mousePressed, w, &MTWindow::mousePressed, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseReleased, w, &MTWindow::mouseReleased, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseScrolled, w, &MTWindow::mouseScrolled, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseEntered, w, &MTWindow::mouseEntered, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseExited, w, &MTWindow::mouseExited, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().windowResized, w, &MTWindow::windowResized, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().messageEvent, w, &MTWindow::messageReceived, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().fileDragEvent, w, &MTWindow::dragged, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchCancelled, w, &MTWindow::touchCancelled, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchDoubleTap, w, &MTWindow::touchDoubleTap, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchDown, w, &MTWindow::touchDown, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchMoved, w, &MTWindow::touchMoved, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchUp, w, &MTWindow::touchUp, OF_EVENT_ORDER_APP);
	ofAddListener(appModeChangedEvent, w, &MTWindow::appModeChanged, OF_EVENT_ORDER_AFTER_APP + 1000);
	ofAddListener(modelLoadedEvent, w, &MTWindow::modelLoaded, OF_EVENT_ORDER_APP);

	// clang-format on
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::removeAllEvents(MTWindow* w)
{
	// clang-format off
	ofRemoveListener(w->events().setup, w, &MTWindow::setupInternal, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().update, w, &MTWindow::update, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().draw, w, &MTWindow::draw, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().exit, w, &MTWindow::exit, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(w->events().keyPressed, w, &MTWindow::keyPressed, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().keyReleased, w, &MTWindow::keyReleased, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseMoved, w, &MTWindow::mouseMoved, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseDragged, w, &MTWindow::mouseDragged, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mousePressed, w, &MTWindow::mousePressed, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseReleased, w, &MTWindow::mouseReleased, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseScrolled, w, &MTWindow::mouseScrolled, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseEntered, w, &MTWindow::mouseEntered, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseExited, w, &MTWindow::mouseExited, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().windowResized, w, &MTWindow::windowResized, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().messageEvent, w, &MTWindow::messageReceived, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().fileDragEvent, w, &MTWindow::dragged, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchCancelled, w, &MTWindow::touchCancelled, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchDoubleTap, w, &MTWindow::touchDoubleTap, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchDown, w, &MTWindow::touchDown, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchMoved, w, &MTWindow::touchMoved, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchUp, w, &MTWindow::touchUp, OF_EVENT_ORDER_APP);
	ofRemoveListener(appModeChangedEvent, w, &MTWindow::appModeChanged, OF_EVENT_ORDER_AFTER_APP + 1000);
	ofRemoveListener(modelLoadedEvent, w, &MTWindow::modelLoaded, OF_EVENT_ORDER_APP);
	// clang-format on
}

//// UI
template<typename AppType, typename ModelType>
std::weak_ptr<ofAppBaseWindow> MTApp<AppType, ModelType>::getMainWindow()
{
	return mainWindow;
}

//// FILE HANDLING
template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::open()
{
	ofFileDialogResult result = ofSystemLoadDialog();
	if (result.bSuccess)
	{
		openImpl(result.filePath);
	}
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::save()
{
	if (MTPrefLastFile.get() == std::string(""))
	{
		saveAs();
	}
	else
	{
		saveImpl();
	}
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::saveAs()
{
#ifdef TARGET_RASPBERRY_PI
	saveAsImpl("settings." + fileExtension);
#else
	ofFileDialogResult result = ofSystemSaveDialog(ofFilePath::getFileName(MTPrefLastFile.get()), "Save As...");
	if (!result.bSuccess)
	{
//		ofSystemAlertDialog("Could not save file!");
		return;
	}
	else
	{
		if (ofFilePath::getFileExt(result.filePath) != fileExtension)
		{
			saveAsImpl(result.filePath + "." + fileExtension);
		}
		else
		{
			saveAsImpl(result.filePath);
		}
	}
#endif
}

template<typename AppType, typename ModelType>
bool MTApp<AppType, ModelType>::saveAsImpl(std::string filePath)
{
	std::string temp = MTPrefLastFile;
	MTPrefLastFile.setWithoutEventNotifications(filePath);

	if (!saveImpl())
	{
		MTPrefLastFile.setWithoutEventNotifications(temp);
		return false;
	}

	fileName = ofFilePath::getFileName(filePath);
	isInitialized = true;
	mainWindow->setWindowTitle(fileName);
	// Fire the change value event?
	return true;
}

template<typename AppType, typename ModelType>
bool MTApp<AppType, ModelType>::saveImpl()
{
	serializer = ofXml();
	model->serialize(serializer);

	if (!serializer.save(MTPrefLastFile.get()))
	{
		ofLog(OF_LOG_ERROR, "Encountered an error while saving the file");
		ofSystemAlertDialog("Encountered an error while saving the file");
		return false;
	}

	saveAppPreferences();
	return true;
}

/// Open sesame.
template<typename AppType, typename ModelType>
bool MTApp<AppType, ModelType>::openImpl(std::string filePath)
{

	if (filePath == "")
	{
		newFile();
		return true;
	}

	if (!serializer.load(filePath))
	{
		ofLogError("MTApp::openImpl") << "Failed loading file " << filePath;
		return false;
	}
	else
	{
		ofLogVerbose("MTApp::openImpl") << "Opening file: " << filePath;
		model->deserialize(serializer);
		if (model == nullptr)
		{
			ofLogError("MTApp", "Failed Loading Model");
			ofSystemAlertDialog("Failed Loading Model");
		}
		else
		{
			MTPrefLastFile = filePath;
			fileName = ofFilePath::getFileName(filePath);
			isInitialized = true;
			mainWindow->setWindowTitle(fileName);
			//            saveAppPreferences();
			auto args = ofEventArgs();
			modelLoadedEvent.notify(args);
			ofLogVerbose("MTApp", "File loaded.");
			return true;
		}   // End load model
	}       // End loadLastFile
	return false;
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::newFile()
{
	// Call the user's newFile method:

	//    saveAppPreferences();

	newFileSetup();
	MTPrefLastFile = "";
	fileName = "";
	isInitialized = true;
//	mainWindow->setWindowTitle(fileName);

	setAppMode(defaultMode);

	auto args = ofEventArgs();
	modelLoadedEvent.notify(args);
}

/// Reload the last opened file.
template<typename AppType, typename ModelType>
bool MTApp<AppType, ModelType>::revert()
{
	return openImpl(MTPrefLastFile);
}

/// Saves!
template<typename AppType, typename ModelType>
bool MTApp<AppType, ModelType>::saveAppPreferences()
{
	ofSerialize(appPrefsXml, appPreferences);

	auto root = appPrefsXml.getChild("App_Preferences");
	root.removeChild("Windows");
	auto windowsXml = root.appendChild("Windows");

	for (auto& wp : wpMap)
	{
		auto winXml = windowsXml.appendChild("Window");
		winXml.appendChild("Name").set(wp.first);
		winXml.appendChild("Position").set(wp.second.position);
		winXml.appendChild("Size").set(wp.second.size);
	}
	ofDisableDataPath();
	auto success = appPrefsXml.save(appPreferencesPath);
	ofEnableDataPath();
	return success;
}

//// EVENTS
template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::windowClosing(std::shared_ptr<MTWindow> window)
{

	ofLogVerbose() << "Closing " << window->name;

	// Update the Window Parameters Map
	auto& wp = wpMap[window->name.get()];
	wp.position = window->getWindowPosition();
	wp.size = window->getWindowSize();

	saveAppPreferences();

	removeWindow(window);
//	ofRemoveListener(window->events().keyPressed,
//					 this,
//					 &MTApp::keyPressed,
//					 OF_EVENT_ORDER_BEFORE_APP);
//	ofRemoveListener(window->events().keyReleasedInternal,
//					 this,
//					 &MTApp::keyReleasedInternal,
//					 OF_EVENT_ORDER_BEFORE_APP);
//	//    ofRemoveListener(modelLoadedEvent, view,
//	//    &MTWindow::modelLoadedInternal, OF_EVENT_ORDER_AFTER_APP);
//
//	// This is another:
//	auto it =
//	  std::find_if(windows.begin(),
//				   windows.end(),
//				   [&](std::shared_ptr<ofAppBaseWindow> const& current) {
//					   return current.get() == window;
//				   });
//
//	if (it != windows.end())
//	{
//		(*it)->events().disable();
//		windows.erase(it);
//	}

}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::exit()
{
	// Last chance to store the size and position of windows:
	for (auto& w : windows)
	{
		auto iter = wpMap.find(w->name.get());
		if (iter != wpMap.end())
		{
			iter->second.position = w->getWindowPosition();
			iter->second.size = w->getWindowSize();
		}
	}

	saveAppPreferences();
}

/**
 * ~/.local/share/filename
 * ~/Library/Preferences/filename
 * %systemdrive%%homepath%\Roaming\filename
 *
 */
template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::createAppPreferencesFilePath()
{
	std::string prefix;
	std::string home = ofGetEnv("HOME");
	switch (ofGetTargetPlatform())
	{
		case OF_TARGET_LINUX:
		case OF_TARGET_LINUX64:
			prefix = home + "/.local/share/";
			break;
		case OF_TARGET_OSX:
			prefix = home + "/Library/Preferences/";
			break;
		case OF_TARGET_WINVS:
		case OF_TARGET_MINGW:
			prefix = home + "\\Roaming\\";
			break;
		default:
			ofLogError("MTApp", "Target platform is not supported");
	}

	appPreferencesPath = prefix + appPreferencesFilename;
}
//int MTApp::getLocalMouseX()
//{
//	/// TODO: Get local mouse
//	ofLogNotice() << "Not implemented yet!!";
//	return 0;
//	//    auto mtView = getMTViewForWindow(ofGetMainLoop()->getCurrentWindow());
//	//    if (mtView != nullptr)
//	//    {
//	//        return mtView->getContentMouse().x;
//	//    }
//	//    else
//	//    {
//	//        ofLogNotice("MTApp") << "getLocalMouseX: Could not find MTView for
//	//        window";
//	//        return -1;
//	//    }
//}
//
//int MTApp::getLocalMouseY()
//{
//	/// TODO: Get local mouse
//	ofLogNotice() << "Not implemented yet!!";
//	return 0;
//	//    auto mtView = getMTViewForWindow(ofGetMainLoop()->getCurrentWindow());
//	//    if (mtView != nullptr)
//	//    {
//	//        return mtView->getContentMouse().y;
//	//    }
//	//    else
//	//    {
//	//        ofLogNotice("MTApp") << "getLocalMouseY: Could not find MTView for
//	//        window";
//	//        return -1;
//	//    }
//}

int mtGetLocalMouseX()
{
//	return MTApp::Instance()->getLocalMouseX();
	return -1;
}

int mtGetLocalMouseY()
{
//	return MTApp::Instance()->getLocalMouseY();
	return -1;
}

template<typename AppType, typename ModelType>
ofPath MTApp<AppType, ModelType>::pathFromString(std::string s)
{
	std::vector<std::string> commandStrings = ofSplitString(s, "{", true, true);
	ofPath thePath;

	for (auto cs : commandStrings)
	{
		std::vector<std::string> commandStringElements = ofSplitString(cs, ";", true, true);

		ofPath::Command* thisCommand;
		int commandType = ofToInt(commandStringElements[0]);
		ofPoint p, cp1, cp2;
		switch (commandType)
		{
			case ofPath::Command::moveTo:
				p = ofFromString<ofPoint>(commandStringElements[1]);
				thePath.moveTo(p);
				break;
			case ofPath::Command::lineTo:
				p = ofFromString<ofPoint>(commandStringElements[1]);
				thePath.lineTo(p);
				break;
			case ofPath::Command::curveTo:
				p = ofFromString<ofPoint>(commandStringElements[1]);
				thePath.curveTo(p);
			case ofPath::Command::bezierTo:
				p = ofFromString<ofPoint>(commandStringElements[1]);
				cp1 = ofFromString<ofPoint>(commandStringElements[2]);
				cp2 = ofFromString<ofPoint>(commandStringElements[3]);
				thePath.bezierTo(cp1, cp2, p);
				break;
			case ofPath::Command::close:
				thePath.close();
				break;
			default:
				ofLog(OF_LOG_WARNING, "MTApp::pathFromString: A Path Command "
									  "supplied is not implemented");
				break;
		}
	}

	return thePath;
}

template<typename AppType, typename ModelType>
std::string MTApp<AppType, ModelType>::pathToString(ofPath& path)
{
	std::vector<ofPath::Command> commands = path.getCommands();

	std::string out = "";

	for (auto c : commands)
	{
		out += "{ " + ofToString(c.type) + "; " + ofToString(c.to) + "; " + ofToString(c.cp1) + "; " +
			   ofToString(c.cp2) + "; } ";
	}

	return out;
}

template<typename AppType, typename ModelType>
std::vector<MTDisplay> MTApp<AppType, ModelType>::displays;

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::updateDisplays()
{
	int num;
	auto glfwMonitors = glfwGetMonitors(&num);
	MTApp::displays.clear();

	for (int i = 0; i < num; i++)
	{
		MTDisplay mtDisplay;
		auto monitor = glfwMonitors[i];
		mtDisplay.name = std::string(glfwGetMonitorName(monitor));
		int x, y;
		glfwGetMonitorPos(monitor, &x, &y);
		auto mode = glfwGetVideoMode(monitor);
		mtDisplay.frame = ofRectangle(x, y, mode->width, mode->height);
		mtDisplay.id = i;
		MTApp::displays.push_back(mtDisplay);
	}
}

template<typename AppType, typename ModelType>
void MTApp<AppType, ModelType>::setMonitorCb(GLFWmonitor* monitor, int connected)
{
	MTApp::updateDisplays();
	ofEventArgs args;
	MTApp::Instance()->displaysChangedEvent.notify(args);
}

auto bla = MTApp::MTBaseApp::Instance();
#endif
