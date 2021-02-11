

//#include <ofMain.h>
#include "ofAppRunner.h"
#ifndef TARGET_OPENGLES
#include "ofAppGLFWWindow.h"
#else
#include "ofAppEGLWindow.h"
#endif
#include "ofWindowSettings.h"
#include "ofSystemUtils.h"
#include "ofPath.h"
#include "MTApp.hpp"
#include "MTWindow.hpp"

#ifndef TARGET_RASPBERRY_PI
#include "MTOffscreenWindow.hpp"
#endif

MTApp* MTApp::sharedApp = 0;
std::vector<std::shared_ptr<MTDisplay>> MTApp::displays;

MTApp::MTApp()
{

	// TODO: Proper singleton
	if (!MTApp::sharedApp)
	{
		// Set the basic preferences
		MTPrefLastFile.set("MTPrefLastFile", "");
		MTPrefAutoloadLastFile.set("MTPrefAutoloadLastFile", true);
		MTPrefLaunchInFullScreen.set("MTPrefLaunchInFullScreen", false);
		appPreferences.setName("App Preferences");
		appPreferences.add(MTPrefLaunchInFullScreen, MTPrefLastFile, MTPrefAutoloadLastFile);

		MTApp::sharedApp = this;
		currentMode = defaultMode;
		registerAppMode(defaultMode);

		fileExtension = "";

		ofInit();
#ifndef TARGET_OPENGLES
		glfwInit();
#endif
		MTApp::updateDisplays();
//		ofSetLogLevel(OF_LOG_NOTICE);

		addEventListener(modelLoadedEvent.newListener([this](ofEventArgs& args)
													  { modelLoaded(); }, OF_EVENT_ORDER_BEFORE_APP));
	}
}

MTApp::~MTApp()
{
	saveAppPreferences();
}

void MTApp::loadAppPreferences()
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
			wp.position = winXml.getChild("Position").getValue<glm::vec2>();
			wp.size = winXml.getChild("Size").getValue<glm::vec2>();
			wpMap.insert({wp.name, wp});
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
	int commandModifier;
	switch (ofGetTargetPlatform())
	{
		case OF_TARGET_LINUX:
		case OF_TARGET_LINUX64:
		case OF_TARGET_WINVS:
		case OF_TARGET_MINGW:
			commandModifier = OF_KEY_CONTROL;
			break;
		case OF_TARGET_OSX:
			commandModifier = OF_KEY_COMMAND;
			break;
		default:
			ofLogError("MTApp", "Target platform is not supported");
			commandModifier = OF_KEY_CONTROL;
	}

	if (key.hasModifier(commandModifier))
	{
		if (key.codepoint == 'o')
		{
			open();
		}
		else if (key.codepoint == 's')
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

void MTApp::exit(ofEventArgs& args)
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
	exit();
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
	ofAppEGLWindowSettings windowSettings;
#endif
	windowSettings.setSize(1280, 800);
	mainWindow = createWindow("Main Window", windowSettings);
}

void MTApp::runApp()
{
	initialize();
	createAppPreferencesFilePath();
	loadAppPreferences();
	createAppViews();
	appWillRun();
	ofRunApp(std::dynamic_pointer_cast<ofAppBaseWindow>(windows.front()), std::shared_ptr<ofBaseApp>(this));

	addEventListener(ofGetMainLoop()->exitEvent.newListener([this]()
															{
																saveAppPreferences();
																exit();
																return true;
															}));
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

#ifndef TARGET_RASPBERRY_PI
	glfwSetMonitorCallback(&setMonitorCb);
#endif
	appWillRun();
	ofLogVerbose("MTApp") << "Running Main Loop";
	ofRunMainLoop();

//	ImGui::Shutdown();
}

/// APP MODES

void MTApp::setAppMode(MTAppModeName mode)
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

MTAppModeName MTApp::getAppMode()
{
	return currentMode;
}

#ifndef TARGET_RASPBERRY_PI

std::shared_ptr<MTOffscreenWindow>
MTApp::createOffscreenWindow(std::string windowName, ofGLFWWindowSettings& settings, bool useTextureRectangle)
{
	if (ofGetTargetPlatform() == OF_TARGET_ANDROID ||
		ofGetTargetPlatform() == OF_TARGET_IOS ||
		ofGetTargetPlatform() == OF_TARGET_LINUXARMV6L ||
		ofGetTargetPlatform() == OF_TARGET_LINUXARMV7L ||
		ofGetTargetPlatform() == OF_TARGET_EMSCRIPTEN)
	{
		ofLogError("MTApp") << "createOffscreenWindow() is not compatible with the current platform";
		return nullptr;
	}

	if (!ofAppInitialized)
	{
		glVersionMajor = settings.glVersionMajor;
		glVersionMinor = settings.glVersionMinor;
	}
	else
	{
		settings.glVersionMajor = glVersionMajor;
		settings.glVersionMinor = glVersionMinor;
	}

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

#endif

#ifdef TARGET_RASPBERRY_PI
std::shared_ptr<MTWindow> MTApp::createWindow(std::string windowName,
											  ofAppEGLWindowSettings& settings)
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
//	ofAddListener(window->events().keyReleased, this, &MTApp::keyReleasedInternal);

	return window;
}

#else

std::shared_ptr<MTWindow> MTApp::createWindow(std::string windowName, ofGLFWWindowSettings& settings)
{

	auto window = std::make_shared<MTWindow>(windowName);
	ofGetMainLoop()->addWindow(window);
	windows.push_back(window);

	if (!ofAppInitialized)
	{
		glVersionMajor = settings.glVersionMajor;
		glVersionMinor = settings.glVersionMinor;
	}
	else
	{
		settings.glVersionMajor = glVersionMajor;
		settings.glVersionMinor = glVersionMinor;
	}

	window->setup(settings);

	addAllEvents(window.get());

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

#endif

void MTApp::closeWindow(std::shared_ptr<MTWindow> window)
{

#ifndef TARGET_RASPBERRY_PI
	auto offscreenWindow = std::dynamic_pointer_cast<MTOffscreenWindow>(window);
#else
	std::shared_ptr<MTWindow> offscreenWindow = nullptr;
#endif
	// 	Update the Window Parameters Map if the window is not an offscreen
	// 	window
	if (!offscreenWindow)
	{
		auto it = wpMap.find(window->name.get());
		if (it != wpMap.end())
		{
			auto& wp = it->second;
			wp.position = window->getWindowPosition();
			wp.size = window->getWindowSize();
			saveAppPreferences();
		}
	}

	auto wIter = std::find(windows.begin(), windows.end(), window);
	windows.erase(wIter);
	removeAllEvents(window.get());
	window->setWindowShouldClose();
}

void MTApp::addAllEvents(MTWindow* w)
{
	// clang-format off
	w->events().enable();
	ofAddListener(w->events().setup, w, &MTWindow::setupInternal, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().update, w, &MTWindow::update, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().draw, w, &MTWindow::draw, OF_EVENT_ORDER_APP);
//	ofAddListener(w->events().exit, w, &MTWindow::exit, OF_EVENT_ORDER_BEFORE_APP);
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
	ofAddListener(MTApp::appModeChangedEvent, w, &MTWindow::appModeChanged, OF_EVENT_ORDER_AFTER_APP + 1000);
	ofAddListener(MTApp::modelLoadedEvent, w, &MTWindow::modelLoaded, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().keyPressed, this, &MTApp::keyPressed, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().keyReleased, this, &MTApp::keyReleased, OF_EVENT_ORDER_APP);

	// clang-format on
}

void MTApp::removeAllEvents(MTWindow* w)
{
	// clang-format off
	ofRemoveListener(w->events().setup, w, &MTWindow::setupInternal, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().update, w, &MTWindow::update, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().draw, w, &MTWindow::draw, OF_EVENT_ORDER_APP);
//	ofRemoveListener(w->events().exit, w, &MTWindow::exit, OF_EVENT_ORDER_BEFORE_APP);
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
	ofRemoveListener(MTApp::appModeChangedEvent, w, &MTWindow::appModeChanged, OF_EVENT_ORDER_AFTER_APP + 1000);
	ofRemoveListener(MTApp::modelLoadedEvent, w, &MTWindow::modelLoaded, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().keyPressed, this, &MTApp::keyPressed, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().keyReleased, this, &MTApp::keyReleased, OF_EVENT_ORDER_APP);
	// clang-format on
}

//// UI
std::weak_ptr<MTWindow> MTApp::getMainWindow()
{
	return mainWindow;
}

//// FILE HANDLING

void MTApp::open()
{
	if (ofGetTargetPlatform() == OF_TARGET_ANDROID ||
		ofGetTargetPlatform() == OF_TARGET_IOS ||
		ofGetTargetPlatform() == OF_TARGET_LINUXARMV6L ||
		ofGetTargetPlatform() == OF_TARGET_LINUXARMV7L ||
		ofGetTargetPlatform() == OF_TARGET_EMSCRIPTEN)
	{
		ofLogError("MTApp") << "open() is not compatible with the current platform";
		return;
	}

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

	setAppMode(defaultMode);

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
	appPrefsXml.removeChild("App_Preferences");
	ofSerialize(appPrefsXml, appPreferences);

	auto root = appPrefsXml.getChild("App_Preferences");
	root.removeChild("Windows");
	auto windowsXml = root.appendChild("Windows");

	for (auto& wp : wpMap)
	{
		auto foundFsPos = wp.first.find("FS");
		if (foundFsPos == 0) {
			continue; // Don't store FS windows
		}
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

/**
 * ~/.local/share/filename
 * ~/Library/Preferences/filename
 * %systemdrive%%homepath%\Roaming\filename
 *
 */
void MTApp::createAppPreferencesFilePath()
{
	std::string prefix;
	std::string home = ofFilePath::getUserHomeDir();

	switch (ofGetTargetPlatform())
	{
		case OF_TARGET_LINUX:
		case OF_TARGET_LINUX64:
		case OF_TARGET_LINUXARMV7L:
		case OF_TARGET_LINUXARMV6L:
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
//	return MTApp::sharedApp->getLocalMouseX();
	return -1;
}

int mtGetLocalMouseY()
{
//	return MTApp::sharedApp->getLocalMouseY();
	return -1;
}

ofPath MTApp::pathFromString(std::string s)
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

std::string MTApp::pathToString(const ofPath& path)
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


void MTApp::updateDisplays()
{
#ifndef TARGET_RASPBERRY_PI
	int num;
	auto glfwMonitors = glfwGetMonitors(&num);
	MTApp::displays.clear();

	for (int i = 0; i < num; i++)
	{
		auto monitor = glfwMonitors[i];
		auto name = std::string(glfwGetMonitorName(monitor));
		int x, y;
		glfwGetMonitorPos(monitor, &x, &y);
		auto mode = glfwGetVideoMode(monitor);
		auto frame = ofRectangle(x, y, mode->width, mode->height);
		auto display = std::make_shared<MTDisplay>(name, frame, i);
		MTApp::displays.push_back(display);
	}
#endif
}

#ifndef TARGET_RASPBERRY_PI

void MTApp::setMonitorCb(GLFWmonitor* monitor, int connected)
{
	MTApp::updateDisplays();
	ofEventArgs args;
	MTApp::sharedApp->displaysChangedEvent.notify(args);
}

#endif