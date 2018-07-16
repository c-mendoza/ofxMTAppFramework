

//#include <ofMain.h>
#include "ofAppRunner.h"
#include "ofAppGLFWWindow.h"
#include "ofWindowSettings.h"
#include "ofSystemUtils.h"
#include "ofPath.h"
#include "MTApp.hpp"
#include "MTWindow.hpp"

#ifndef TARGET_RASPBERRY_PI
#include "MTOffscreenWindow.hpp"
#endif

const std::string MTApp::APP_PREFERENCES_FILE = "app_preferences.xml";
const std::string MTApp::MTPrefsWindowsGroupName = "//app_preferences/windows";
const std::string MTApp::MTPrefsWindowPositionName = "Position";
const std::string MTApp::MTPrefsWindowSizeName = "Size";

ofEvent<MTAppStateChangeArgs> MTApp::appStateChangedEvent;
ofEvent<ofEventArgs> MTApp::modelLoadedEvent;
MTApp* MTApp::sharedApp = 0;
//ofxImGui::Gui MTApp::gui;


MTApp::MTApp()
{

	// TODO: Proper singleton
	if (!MTApp::sharedApp)
	{
		// Set the basic preferences
		MTPrefLastFile.set("NSPrefLastFile", "");
		NSPrefAutoloadLastFile.set("NSPrefAutoloadLastFile", false);
		NSPrefLaunchInFullScreen.set("NSPrefLaunchInFullScreen", false);
		appPreferences.setName("App Preferences");
		appPreferences.add(
		  NSPrefLaunchInFullScreen, MTPrefLastFile, NSPrefAutoloadLastFile);

		MTApp::sharedApp = this;
		currentMode = defaultState;
		registerState(defaultState);

		fileExtension = "";

		ofInit();
        glfwInit();
        MTApp::updateDisplays();
//		ofSetLogLevel(OF_LOG_NOTICE);

		addEventListener(modelLoadedEvent.newListener(
		  [this](ofEventArgs& args) { modelLoaded(); },
		  OF_EVENT_ORDER_BEFORE_APP));
	}
}

MTApp::~MTApp()
{
	saveAppPreferences();
}

void MTApp::loadAppPreferences()
{
	if (!appPrefsXml.load(APP_PREFERENCES_FILE))
	{
		ofLog(OF_LOG_ERROR,
			  "App Preferences could not be loaded, creating a new file.");
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
			wp.position = winXml.getChild("Position").getValue<glm::vec2>();
			wp.size = winXml.getChild("Size").getValue<glm::vec2>();
			wpMap.insert({ wp.name, wp });
		}
	}
}

void MTApp::registerAppPreference(ofAbstractParameter& preference)
{
	appPreferences.add(preference);
}

void MTApp::keyPressed(ofKeyEventArgs& key)
{
	appKeyPressed(key);
}

void MTApp::keyReleased(ofKeyEventArgs& key)
{
	if (ofGetKeyPressed(OF_KEY_COMMAND) || ofGetKeyPressed(OF_KEY_CONTROL))
	{
		auto k = key.key;
		int bla = 'o';
		if (k == 15)
		{
			open();
		}
		else if (k == 19)
		{
			if (ofGetKeyPressed(OF_KEY_SHIFT))
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

void MTApp::initialize()
{
	// These should be overriden sometime
	//	model = shared_ptr<ofxMTModel>(new ofxMTModel("default"));
	model = nullptr;
}

void MTApp::createAppViews()
{
	#ifndef TARGET_RASPBERRY_PI
	ofGLFWWindowSettings windowSettings;
#else
	ofGLESWindowSettings windowSettings;
#endif
	windowSettings.setSize(1280, 800);
	mainWindow = createWindow("Main Window", windowSettings);
}

void MTApp::runApp()
{
	initialize();
	loadAppPreferences();
	createAppViews();
	ofRunApp(std::dynamic_pointer_cast<ofAppBaseWindow>(windows.front()),
			 std::shared_ptr<ofBaseApp>(this));

//	MTApp::gui.setup();
	// Only the first window gets notified of setup when ofRunApp is called
	// so we need to do that ourselves:
	for (auto iter = windows.begin()+1; iter < windows.end(); iter++)
	{
		(*iter)->events().notifySetup();
	}

//	ofAddListener(ofEvents().keyPressed, this, &MTApp::keyPressed);
	isInitialized = false;

	if (NSPrefAutoloadLastFile)
	{
		isInitialized = openImpl(MTPrefLastFile);

		if (!isInitialized)
		{
            std::string msg = "Tried to open " + MTPrefLastFile.get() +
						 " but could not find it";
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

#ifndef TARGET_RASPBERRY_PI
    glfwSetMonitorCallback(&setMonitorCb);
#endif
	appWillRun();
	ofLogVerbose("MTApp") << "Running Main Loop";
	ofRunMainLoop();

//	ImGui::Shutdown();
}

/// APP MODES

void MTApp::setAppState(MTAppStateName mode)
{
	//	if(!ofContains(appStates, mode)) return;

	if (mode == currentMode)
	{
		return;
	}
	else
	{
		static MTAppStateChangeArgs changeArgs;
		changeArgs.newStateName = mode;
		changeArgs.oldStateName = currentMode;
		currentMode = mode;
		appStateChanged(changeArgs);
		ofNotifyEvent(MTApp::appStateChangedEvent, changeArgs, this);
	}
}

MTAppStateName MTApp::getCurrentState()
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

#ifndef TARGET_RASPBERRY_PI
std::shared_ptr<MTOffscreenWindow> MTApp::createOffscreenWindow(std::string windowName,
														 ofGLFWWindowSettings& settings)
{
	auto offscreenWindow = std::make_shared<MTOffscreenWindow>(windowName);
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
#endif

#ifdef TARGET_OPENGLES
std::shared_ptr<MTWindow> MTApp::createWindow(std::string windowName,
											  ofGLESWindowSettings& settings)
{
	auto window = std::make_shared<MTWindow>(windowName);
	ofGetMainLoop()->addWindow(window);
	windows.push_back(window);
	window->setup(settings);

	addAllEvents(window.get());

	// Add the "global" keyboard event listener:
	ofAddListener(window->events().keyPressed,
				  this,
				  &MTApp::keyPressed,
				  OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(window->events().keyReleased,
				  this,
				  &MTApp::keyReleased,
				  OF_EVENT_ORDER_BEFORE_APP);

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
//	ofAddListener(window->events().keyReleased, this, &MTApp::keyReleased);

	return window;
}

#else

std::shared_ptr<MTWindow> MTApp::createWindow(std::string windowName,
											  ofGLFWWindowSettings& settings)
{

	auto window = std::make_shared<MTWindow>(windowName);
	ofGetMainLoop()->addWindow(window);
	windows.push_back(window);
	window->setup(settings);

	addAllEvents(window.get());

	// Add the "global" keyboard event listener:
	ofAddListener(window->events().keyPressed,
				  this,
				  &MTApp::keyPressed,
				  OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(window->events().keyReleased,
				  this,
				  &MTApp::keyReleased,
				  OF_EVENT_ORDER_BEFORE_APP);
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
//	ofAddListener(window->events().keyReleased, this, &MTApp::keyReleased);

	return window;
}

#endif

void MTApp::removeWindow(std::shared_ptr<MTWindow> window)
{
    auto wIter = std::find(windows.begin(), windows.end(), window);
    windows.erase(wIter);
    window->setWindowShouldClose(); // Likely redundant, but probably harmless
    removeAllEvents(window.get());
    ofRemoveListener(window->events().keyPressed,
                  this,
                  &MTApp::keyPressed,
                  OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(window->events().keyReleased,
                  this,
                  &MTApp::keyReleased,
                  OF_EVENT_ORDER_BEFORE_APP);
//    wpMap.erase(window->name);
}

void MTApp::addAllEvents(MTWindow* w)
{
	// clang-format off
	w->events().enable();
	ofAddListener(w->events().setup, w, &MTWindow::setupInternal, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().update, w, &MTWindow::update,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().draw, w, &MTWindow::draw,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().exit,w, &MTWindow::exit,OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(w->events().keyPressed,w, &MTWindow::keyPressed,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().keyReleased,w, &MTWindow::keyReleased,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseMoved,w, &MTWindow::mouseMoved,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseDragged,w, &MTWindow::mouseDragged,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mousePressed,w, &MTWindow::mousePressed,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseReleased,w, &MTWindow::mouseReleased,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseScrolled,w, &MTWindow::mouseScrolled,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseEntered,w, &MTWindow::mouseEntered,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseExited,w, &MTWindow::mouseExited,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().windowResized,w, &MTWindow::windowResized,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().messageEvent,w, &MTWindow::messageReceived,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().fileDragEvent,w, &MTWindow::dragged,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchCancelled,w, &MTWindow::touchCancelled,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchDoubleTap,w, &MTWindow::touchDoubleTap,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchDown,w, &MTWindow::touchDown,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchMoved,w, &MTWindow::touchMoved,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchUp,w, &MTWindow::touchUp,OF_EVENT_ORDER_APP);
	ofAddListener(MTApp::appStateChangedEvent, w, &MTWindow::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);
	ofAddListener(MTApp::modelLoadedEvent, w, &MTWindow::modelLoaded, OF_EVENT_ORDER_APP);

	// clang-format on
}

void MTApp::removeAllEvents(MTWindow* w)
{
	// clang-format off
	ofRemoveListener(w->events().setup, w, &MTWindow::setupInternal, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().update, w, &MTWindow::update,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().draw, w, &MTWindow::draw,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().exit,w, &MTWindow::exit,OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(w->events().keyPressed,w, &MTWindow::keyPressed,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().keyReleased,w, &MTWindow::keyReleased,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseMoved,w, &MTWindow::mouseMoved,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseDragged,w, &MTWindow::mouseDragged,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mousePressed,w, &MTWindow::mousePressed,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseReleased,w, &MTWindow::mouseReleased,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseScrolled,w, &MTWindow::mouseScrolled,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseEntered,w, &MTWindow::mouseEntered,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseExited,w, &MTWindow::mouseExited,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().windowResized,w, &MTWindow::windowResized,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().messageEvent,w, &MTWindow::messageReceived,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().fileDragEvent,w, &MTWindow::dragged,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchCancelled,w, &MTWindow::touchCancelled,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchDoubleTap,w, &MTWindow::touchDoubleTap,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchDown,w, &MTWindow::touchDown,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchMoved,w, &MTWindow::touchMoved,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchUp,w, &MTWindow::touchUp,OF_EVENT_ORDER_APP);
	ofRemoveListener(MTApp::appStateChangedEvent, w, &MTWindow::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);
	ofRemoveListener(MTApp::modelLoadedEvent, w, &MTWindow::modelLoaded, OF_EVENT_ORDER_APP);
	// clang-format on
}

//// UI
std::weak_ptr<ofAppBaseWindow> MTApp::getMainWindow()
{
	return mainWindow;
}

//// FILE HANDLING

void MTApp::open()
{
	ofFileDialogResult result = ofSystemLoadDialog();
	if (result.bSuccess)
	{
		openImpl(result.filePath);
	}
}

void MTApp::save()
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

void MTApp::saveAs()
{
#ifdef TARGET_RASPBERRY_PI
	saveAsImpl("settings." + fileExtension);
#else
	ofFileDialogResult result =
	  ofSystemSaveDialog(MTPrefLastFile, "Save As...");
	if (!result.bSuccess)
	{
		ofSystemAlertDialog("Could not save file!");
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

bool MTApp::saveAsImpl(std::string filePath)
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

bool MTApp::saveImpl()
{

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
bool MTApp::openImpl(std::string filePath)
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

void MTApp::newFile()
{
	// Call the user's newFile method:

	//    saveAppPreferences();

	newFileSetup();
	MTPrefLastFile = "";
	fileName = "";
	isInitialized = true;
//	mainWindow->setWindowTitle(fileName);

	setAppState(defaultState);

	auto args = ofEventArgs();
	modelLoadedEvent.notify(args);
}

/// Reload the last opened file.
bool MTApp::revert()
{
	return openImpl(MTPrefLastFile);
}

/// Saves!
bool MTApp::saveAppPreferences()
{
	ofSerialize(appPrefsXml, appPreferences);

	auto root = appPrefsXml.getChild("App_Preferences");
	bool success = root.removeChild("Windows");
	auto windowsXml = root.appendChild("Windows");

	for (auto& wp : wpMap)
	{
		auto winXml = windowsXml.appendChild("Window");
		winXml.appendChild("Name").set(wp.first);
		winXml.appendChild("Position").set(wp.second.position);
		winXml.appendChild("Size").set(wp.second.size);
	}

	return appPrefsXml.save(APP_PREFERENCES_FILE);
}

//// EVENTS
void MTApp::windowClosing(std::shared_ptr<MTWindow> window)
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
//	ofRemoveListener(window->events().keyReleased,
//					 this,
//					 &MTApp::keyReleased,
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

void MTApp::exit()
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

int MTApp::getLocalMouseX()
{
	/// TODO: Get local mouse
	ofLogNotice() << "Not implemented yet!!";
	return 0;
	//    auto mtView = getMTViewForWindow(ofGetMainLoop()->getCurrentWindow());
	//    if (mtView != nullptr)
	//    {
	//        return mtView->getContentMouse().x;
	//    }
	//    else
	//    {
	//        ofLogNotice("MTApp") << "getLocalMouseX: Could not find MTView for
	//        window";
	//        return -1;
	//    }
}

int MTApp::getLocalMouseY()
{
	/// TODO: Get local mouse
	ofLogNotice() << "Not implemented yet!!";
	return 0;
	//    auto mtView = getMTViewForWindow(ofGetMainLoop()->getCurrentWindow());
	//    if (mtView != nullptr)
	//    {
	//        return mtView->getContentMouse().y;
	//    }
	//    else
	//    {
	//        ofLogNotice("MTApp") << "getLocalMouseY: Could not find MTView for
	//        window";
	//        return -1;
	//    }
}

int mtGetLocalMouseX()
{
	return MTApp::sharedApp->getLocalMouseX();
}

int mtGetLocalMouseY()
{
	return MTApp::sharedApp->getLocalMouseY();
}

ofPath MTApp::pathFromString(std::string s)
{
    std::vector<std::string> commandStrings = ofSplitString(s, "{", true, true);
	ofPath thePath;

	for (auto cs : commandStrings)
	{
        std::vector<std::string> commandStringElements =
		  ofSplitString(cs, ";", true, true);

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
				ofLog(OF_LOG_WARNING,
					  "MTApp::pathFromString: A Path Command "
					  "supplied is not implemented");
				break;
		}
	}

	return thePath;
}

std::string MTApp::pathToString(ofPath& path)
{
    std::vector<ofPath::Command> commands = path.getCommands();

    std::string out = "";

	for (auto c : commands)
	{
		out += "{ " + ofToString(c.type) + "; " + ofToString(c.to) + "; " +
			   ofToString(c.cp1) + "; " + ofToString(c.cp2) + "; } ";
	}

	return out;
}

std::vector<MTDisplay> MTApp::displays;

void MTApp::updateDisplays()
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

 void MTApp::setMonitorCb(GLFWmonitor* monitor, int connected)
{
    MTApp::updateDisplays();
    ofEventArgs args;
    MTApp::sharedApp->displaysChangedEvent.notify(args);
}