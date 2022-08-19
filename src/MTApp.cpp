

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

#ifndef TARGET_RASPBERRY_PI

#include "MTWindow.hpp"
#include "MTModel.hpp"

#endif

std::vector<std::shared_ptr<MTDisplay>> MTApp::displays;
std::weak_ptr<MTApp> MTApp::AppPtr;

MTApp::MTApp()
{
   // Set the basic preferences
   MTPrefLastFile.set("MTPrefLastFile", "");
   MTPrefAutoloadLastFile.set("Automatically Load Last File", true);
   MTPrefLaunchInFullScreen.set("Launch in Full Screen", false);
   appPreferences.setName("App Preferences");
   appPreferences.add(MTPrefLaunchInFullScreen, MTPrefLastFile, MTPrefAutoloadLastFile);

   currentMode = defaultMode;
   registerAppMode(defaultMode);

   ofInit();
#ifndef TARGET_OPENGLES
   glfwInit();
#endif
   MTApp::updateDisplays();
   //		ofSetLogLevel(OF_LOG_NOTICE);

   addEventListener(modelLoadedEvent.newListener([this](ofEventArgs& args) { modelLoaded(); }, OF_EVENT_ORDER_BEFORE_APP));
   addEventListener(ofGetMainLoop()->loopEvent.newListener(
       [this]()
       {
          // We only iterate until size so that if a
          // loopFunction adds another entry into the deque
          // it won't get executed until the next loop
          auto size = loopFunctions.size();
          if (size == 0) return;
          for (int i = 0; i < size; i++)
          {
             auto f = loopFunctions.front();
             loopFunctions.pop_front();
             f();
          }
       }));

   addEventListener(ofGetMainLoop()->exitEvent.newListener(
       [this]()
       {
          saveAppPreferences();
          exit();
       }));


   runOncePostLoop(
       [this]()
       {
          initialize();
          loadAppPreferences();
          // Only the first window gets notified of setup when ofRunApp is called
          // so we need to do that ourselves:
          for (auto iter = windows.begin() + 1; iter < windows.end(); iter++)
          {
             (*iter)->events().notifySetup();
          }

          //	ofAddListener(ofEvents().keyPressed, this, &MTApp::keyPressed);
          isInitialized = false;
#ifndef TARGET_RASPBERRY_PI
          glfwSetMonitorCallback(&setMonitorCb);
#endif
          createAppViews();

          // Is this for loop still necessary?
          for (auto& win : windows)
          {
             auto wp = wpMap.find(win->name);
             if (wp != wpMap.end())
             {
                win->setWindowShape(wp->second.size.x, wp->second.size.y);
                win->setWindowPosition(wp->second.position.x, wp->second.position.y);
                win->setWindowTitle(win->name);
             }
          }
          runOncePostLoop([this]() { appWillRun(); });

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


          ofLogVerbose("MTApp") << "Running Main Loop";
       });
}

void MTApp::RunApp(std::shared_ptr<MTApp>&& app, ofGLFWWindowSettings mainWindowSettings)
{
   app->createAppPreferencesFilePath();
   app->loadAppPreferences();
   app->mainWindow = app->createWindow("Main Window", mainWindowSettings);
   AppPtr = app;
   ofRunApp(std::move(app));
}

MTApp::~MTApp()
{
   releasePointers();
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
   case OF_TARGET_MINGW: commandModifier = OF_KEY_CONTROL; break;
   case OF_TARGET_OSX: commandModifier = OF_KEY_COMMAND; break;
   default: ofLogError("MTApp", "Target platform is not supported"); commandModifier = OF_KEY_CONTROL;
   }

   if (key.hasModifier(commandModifier))
   {
      if (key.keycode == GLFW_KEY_O)
      {
         open();
      }
      else if (key.keycode == GLFW_KEY_S)
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
   exit();
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

   eventListeners.unsubscribeAll();
}

void MTApp::releasePointers()
{
   windows.clear();
   mainWindow.reset();
   eventListeners.unsubscribeAll();
   AppPtr.reset();
}
/// Method is called in the MTApp constructor, right before the app is run.
/// Override this method and instantiate your model and main view classes, as
/// well as the main
/// window size and settings.

void MTApp::initialize()
{
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

//std::shared_ptr<MTOffscreenWindow>
//MTApp::createOffscreenWindow(std::string windowName, ofGLFWWindowSettings& settings, bool useTextureRectangle)
//{
//	if (ofGetTargetPlatform() == OF_TARGET_ANDROID ||
//		ofGetTargetPlatform() == OF_TARGET_IOS ||
//		ofGetTargetPlatform() == OF_TARGET_LINUXARMV6L ||
//		ofGetTargetPlatform() == OF_TARGET_LINUXARMV7L ||
//		ofGetTargetPlatform() == OF_TARGET_EMSCRIPTEN)
//	{
//		ofLogError("MTApp") << "createOffscreenWindow() is not compatible with the current platform";
//		return nullptr;
//	}
//
//	if (!ofAppInitialized)
//	{
//		glVersionMajor = settings.glVersionMajor;
//		glVersionMinor = settings.glVersionMinor;
//	}
//	else
//	{
//		settings.glVersionMajor = glVersionMajor;
//		settings.glVersionMinor = glVersionMinor;
//	}
//
//	auto offscreenWindow = std::make_shared<MTOffscreenWindow>(windowName, useTextureRectangle);
////	ofGLFWWindowSettings* glfwWS = dynamic_cast<ofGLFWWindowSettings*>(&settings);
//	offscreenWindow->setup(settings);
//	addAllEvents(offscreenWindow.get());
//	ofGetMainLoop()->addWindow(offscreenWindow);
//	windows.push_back(offscreenWindow);
//
//	glfwHideWindow(offscreenWindow->getGLFWWindow());
//
//	// The ofApp system only notifies setup for the first window it creates,
//	// the rest are on their own apparently. So we check if we have initialized
//	// the ofApp system, and if we have, then
//	// that means that we need to notify setup for the window we are creating
//
//	if (!ofAppInitialized)
//	{
//		ofAppInitialized = true;
//	}
//	else
//	{
//		// Note that MTView::setup is not called from this event, only the
//		// MTWindow's setup
//		offscreenWindow->events().notifySetup();
//	}
//
//	return offscreenWindow;
//}

#endif

#ifdef TARGET_RASPBERRY_PI
std::shared_ptr<MTWindow> MTApp::createWindow(std::string windowName, ofAppEGLWindowSettings& settings)
{
   auto window = std::make_shared<MTWindow>(windowName);
   ofGetMainLoop()->addWindow(window);
   windows.push_back(window);
   window->setup(settings);

   addAllEvents(window.get());

   // Add the "global" keyboard event listener:
   ofAddListener(window->events().keyPressed, this, &MTApp::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
   ofAddListener(window->events().keyReleased, this, &MTApp::keyReleased, OF_EVENT_ORDER_BEFORE_APP);

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

std::shared_ptr<MTWindow> MTApp::createWindow(std::string windowName)
{
   ofGLFWWindowSettings settings;
   settings.shareContextWith = mainWindow;
   return createWindow(windowName, settings);
}

std::shared_ptr<MTWindow> MTApp::createWindow(std::string windowName, ofGLFWWindowSettings settings)
{
   for (auto& w : windows)
   {
      if (w->name.get() == windowName)
      {
         return nullptr;
      }
   }

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
      // Need to make a copy here because calling setWindowShape will change the value
      // of the stored windowParam.
      WindowParams paramsCopy = wp->second;
      window->setWindowShape(paramsCopy.size.x, paramsCopy.size.y);
      window->setWindowPosition(paramsCopy.position.x, paramsCopy.position.y);
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
//	auto offscreenWindow = std::dynamic_pointer_cast<MTOffscreenWindow>(window);
#else
   std::shared_ptr<MTWindow> offscreenWindow = nullptr;
#endif
   // 	Update the Window Parameters Map if the window is not an offscreen
   // 	window
   //	if (!offscreenWindow)
   //	{
   auto it = wpMap.find(window->name.get());
   if (it != wpMap.end())
   {
      auto& wp = it->second;
      wp.position = window->getWindowPosition();
      wp.size = window->getWindowSize();
      saveAppPreferences();
   }
   //	}

   auto wIter = std::find(windows.begin(), windows.end(), window);
   if (wIter != windows.end())
   {
      windows.erase(wIter);
   }
   else
   {
      ofLogWarning(__FUNCTION__) << "Could not find window to delete: " << window->name;
   }
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

	w->getGLFWWindow();

	glfwSetWindowPosCallback(w->getGLFWWindow(), &MTApp::windowPosCb);

	addEventListener(w->events().windowMoved.newListener([this, w](ofWindowPosEventArgs& args)
	{
		if (wpMap.count(w->name.get()) > 0)
		{
			wpMap[w->name].size = w->getWindowSize();
			wpMap[w->name].position = w->getWindowPosition();
			saveAppPreferences();
		}
	}));
	addEventListener(w->events().windowResized.newListener([this, w](ofResizeEventArgs& args)
	{
		if (wpMap.count(w->name.get()) > 0)
		{
			wpMap[w->name].size = w->getWindowSize();
			wpMap[w->name].position = w->getWindowPosition();
			saveAppPreferences();
		}
	}));
	// We only add these for the windows that are created after the mainWindow, so we test
	// whether mainWindow is set:
	if (mainWindow)
	{
		ofAddListener(w->events().keyPressed, this, &MTApp::keyPressed, OF_EVENT_ORDER_APP);
		ofAddListener(w->events().keyReleased, this, &MTApp::keyReleased, OF_EVENT_ORDER_APP);
	}
   // clang-format on
}

void MTApp::windowPosCb(GLFWwindow* w, int x, int y)
{
   if (auto app = MTApp::Instance())
   {
      if (app->isInitialized)
      {
         auto ofw = dynamic_pointer_cast<MTWindow>(ofGetCurrentWindow());
         if (ofw) ofw->events().notifyWindowMoved(x, y);
      }
   }
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
	if (mainWindow)
	{
		ofRemoveListener(w->events().keyPressed, this, &MTApp::keyPressed, OF_EVENT_ORDER_APP);
		ofRemoveListener(w->events().keyReleased, this, &MTApp::keyReleased, OF_EVENT_ORDER_APP);
	}
   // clang-format on
}

//// UI
std::weak_ptr<MTWindow> MTApp::getMainWindow()
{
   return mainWindow;
}

//// FILE HANDLING

bool MTApp::open(std::string filepath)
{
   if (ofGetTargetPlatform() == OF_TARGET_ANDROID || ofGetTargetPlatform() == OF_TARGET_IOS ||
       ofGetTargetPlatform() == OF_TARGET_LINUXARMV6L || ofGetTargetPlatform() == OF_TARGET_LINUXARMV7L ||
       ofGetTargetPlatform() == OF_TARGET_EMSCRIPTEN)
   {
      ofLogError("MTApp") << "open() is not compatible with the current platform";
      return false;
   }

   if (filepath.empty())
   {
      ofFileDialogResult result = ofSystemLoadDialog();
      if (result.bSuccess)
      {
         return openImpl(result.filePath);
      }
      else
      {
         ofLogError("MTApp::open") << "Could not open file: " << filepath;
         return false;
      }
   }
   else
   {
      if (!std::filesystem::exists(filepath))
      {
         ofLogError("MTApp::open") << "Tried opening file that does not exist: " << filepath;
         return false;
      }
      else
      {
         return openImpl(filepath);
      }
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

void MTApp::saveCopy(std::string filepath)
{
   std::string temp = MTPrefLastFile.get();
   MTPrefLastFile.setWithoutEventNotifications(filepath);
   saveImpl();
   MTPrefLastFile.setWithoutEventNotifications(temp);
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
   if (serializerType == XML)
   {
      auto serializer = ofXml();
      model->serialize(serializer);

      if (!serializer.save(MTPrefLastFile.get()))
      {
         ofLog(OF_LOG_ERROR, "Encountered an error while saving the file");
         ofSystemAlertDialog("Encountered an error while saving the file");
         return false;
      }
   }
   else
   {
      ofJson serializer;
      model->serialize(serializer);

      if (!ofSavePrettyJson(MTPrefLastFile.get(), serializer))
      {
         ofLog(OF_LOG_ERROR, "Encountered an error while saving the file");
         ofSystemAlertDialog("Encountered an error while saving the file");
         return false;
      }
   }
   saveAppPreferences();
   return true;
}

/// Open sesame.
bool MTApp::openImpl(std::string path)
{
   if (path.empty())
   {
      newFile();
      return true;
   }
   std::string filepath = path;
   if (ofFilePath::isAbsolute(path))
   {
      auto exePath = ofFilePath::getEnclosingDirectory(ofFilePath::getCurrentExePath(), false);
      auto rPath = ofFilePath::makeRelative(exePath, path);
      ofLogNotice("MTApp::openImpl") << "Document path: " << rPath;
      if (!rPath.empty())
      {
         filepath = rPath;
      }
   }

   bool success = false;
   ofJson json;
   ofXml xml;

   if (serializerType == XML)
   {
      success = xml.load(filepath);
   }
   else
   {
      try
      {
         json = ofLoadJson(filepath);
         success = json.empty() ? false : true;
      }
      catch (std::exception& e)
      {
         success = false;
      }
   }

   if (!success)
   {
      ofLogError("MTApp::openImpl") << "Failed loading file " << filepath;
      return false;
   }
   ofLogVerbose("MTApp::openImpl") << "Opening file: " << filepath;
   if (serializerType == XML)
   {
      model->deserialize(xml);
   }
   else
   {
      model->deserialize(json);
   }

   if (model == nullptr)
   {
      ofLogError("MTApp::openImpl") << "Failed to load Model";
      //ofSystemAlertDialog("Failed Loading Model");
      return false;
   }

   MTPrefLastFile = filepath;
   fileName = ofFilePath::getFileName(filepath);
   isInitialized = true;
   mainWindow->setWindowTitle(fileName);
   //            saveAppPreferences();
   auto args = ofEventArgs();
   modelLoadedEvent.notify(args);
   saveAppPreferences();
   ofLogVerbose("MTApp") << "File loaded.";
   return true;
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

   //setAppMode(defaultMode);

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
   // Return if the app has not yet initialized
   // Prevents bogus window prefs from being saved
   if (!isInitialized) return false;

   appPrefsXml.removeChild("App_Preferences");
   ofSerialize(appPrefsXml, appPreferences);

   auto root = appPrefsXml.getChild("App_Preferences");
   root.removeChild("Windows");
   auto windowsXml = root.appendChild("Windows");

   for (auto& wp : wpMap)
   {
      auto foundFsPos = wp.first.find("FS");
      if (foundFsPos == 0)
      {
         continue;  // Don't store FS windows
      }
      // TODO: Avoid saving prefs of ofWindowMode::OF_FULLSCREEN windows
      // Constrain window pos and size to something reasonable to avoid NaNs and weird values

      wp.second.position = glm::max(wp.second.position, {-10000, -10000});
      auto sizeNan = glm::isnan(wp.second.size);
      if (sizeNan.x || sizeNan.y)
      {
         ofLogNotice("MTApp::saveAppPreferences") << "Window size was invalid, setting default values instead";
         wp.second.size = {1024, 768};
      }
      else
      {
         wp.second.size = glm::max(wp.second.size, {200, 200});
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
 * ~/Library/Preferences/filename`
 * %systemdrive%%homepath%\Roaming\filename
 *
 */
void MTApp::createAppPreferencesFilePath()
{
   if (!saveAppPreferencesInHomeDir)
   {
      appPreferencesPath = ofToDataPath(appPreferencesFilename);
   }
   else
   {
      std::string prefix;
      std::string home = ofFilePath::getUserHomeDir();

      switch (ofGetTargetPlatform())
      {
      case OF_TARGET_LINUX:
      case OF_TARGET_LINUX64:
      case OF_TARGET_LINUXARMV7L:
      case OF_TARGET_LINUXARMV6L: prefix = home + "/.local/share/"; break;
      case OF_TARGET_OSX: prefix = home + "/Library/Preferences/"; break;
      case OF_TARGET_WINVS:
      case OF_TARGET_MINGW: prefix = home + "\\AppData\\Roaming\\"; break;
      default: ofLogError("MTApp") << "Target platform is not supported";
      }

      appPreferencesPath = prefix + appPreferencesFilename;
   }
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
      case ofPath::Command::curveTo: p = ofFromString<ofPoint>(commandStringElements[1]); thePath.curveTo(p);
      case ofPath::Command::bezierTo:
         p = ofFromString<ofPoint>(commandStringElements[1]);
         cp1 = ofFromString<ofPoint>(commandStringElements[2]);
         cp2 = ofFromString<ofPoint>(commandStringElements[3]);
         thePath.bezierTo(cp1, cp2, p);
         break;
      case ofPath::Command::close: thePath.close(); break;
      default:
         ofLog(OF_LOG_WARNING,
               "MTApp::pathFromString: A Path Command "
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
      out += "{ " + ofToString(c.type) + "; " + ofToString(c.to) + "; " + ofToString(c.cp1) + "; " + ofToString(c.cp2) + "; } ";
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
   MTApp::Instance()->displaysChangedEvent.notify(args);
}

#endif
