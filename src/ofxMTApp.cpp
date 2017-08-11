#include "ofxMTApp.hpp"
#include "ofxMTWindow.hpp"
#include "ofxMTView.hpp"
#include "ofxMTAppMode.hpp"
#include "ofxMTModel.hpp"

const string ofxMTApp::APP_PREFERENCES_FILE = "app_preferences.xml";
const string ofxMTApp::MTPrefsWindowsGroupName = "Views";
const string ofxMTApp::MTPrefsWindowPositionName = "Position";
const string ofxMTApp::MTPrefsWindowSizeName = "Size";

ofEvent<MTAppModeChangeArgs> ofxMTApp::appChangeModeEvent;
ofEvent<void> ofxMTApp::modelLoadedEvent;
ofxMTApp* ofxMTApp::sharedApp = 0;


ofxMTApp::ofxMTApp()
{

    //TODO: Proper singleton
    if(!ofxMTApp::sharedApp)
    {
        //Set the basic preferences
        MTPrefLastFile.set("NSPrefLastFile", "");
        NSPrefAutoloadLastFile.set("NSPrefAutoloadLastFile", false);
        NSPrefLaunchInFullScreen.set("NSPrefLaunchInFullScreen", false);
        MTPrefsWindowsGroup.setName(MTPrefsWindowsGroupName);
        MTPrefsWindowsGroup.setSerializable(true);
        appPreferences.setName("App Preferences");
        appPreferences.add(NSPrefLaunchInFullScreen,
                           MTPrefLastFile,
                           NSPrefAutoloadLastFile,
                           MTPrefsWindowsGroup);
        ofxMTApp::sharedApp = this;
        currentMode = defaultMode;
        registerMode(defaultMode);

        fileExtension = "";

        ofInit();

        ofSetLogLevel(OF_LOG_VERBOSE);


        if(!appPrefsXml.load(APP_PREFERENCES_FILE))
        {
            ofLog(OF_LOG_ERROR, "App Preferences could not be loaded, creating a new file.");
//			ofSystemAlertDialog("App Preferences could not be loaded, creating a new file.");
            saveAppPreferences();
        }
		else
		{
			ofDeserialize(appPrefsXml, appPreferences);
			
			// Load the saved view positions:
			auto viewsXml = appPrefsXml.find("//App_Preferences/Views/View");
			
				for (auto & view : viewsXml)
				{
					ofParameterGroup thisView;
					thisView.setName(view.getChild("Name").getValue());
					ofLogVerbose("View in XML: " + view.getName());
					ofParameter<ofPoint> pos, size;
					pos.set(MTPrefsWindowPositionName, view.getChild("Position").getValue<ofVec3f>());
					size.set(MTPrefsWindowSizeName, view.getChild("Size").getValue<ofVec3f>());
					thisView.add(pos, size);
					MTPrefsWindowsGroup.add(thisView);
				}
		}
		
        appPreferences.add(MTPrefsWindowsGroup);

        addEventListener(modelLoadedEvent.newListener([this]()
        {
            modelDidLoad();
#ifndef TARGET_RASPBERRY_PI
            for (auto window : windows)
            {
                window->enqueueUpdateOperation([window]()
                {
                    window->modelDidLoad();
                });
            }
#else
            mainWindow->enqueueUpdateOperation([this]()
            {
                mainWindow->modelDidLoad();
            });
#endif
        }));
    }
}

void ofxMTApp::registerAppPreference(ofAbstractParameter &preference)
{
    if (appPrefsXml.load(APP_PREFERENCES_FILE))
    {

        appPreferences.add(preference);
        auto prefXml = appPrefsXml.findFirst(preference.getEscapedName());
        if (prefXml)
        {
            ofDeserialize(prefXml, preference);
        }
        else
        {
            saveAppPreferences();
        }
    }
}

void ofxMTApp::keyPressed(ofKeyEventArgs &key)
{
//	ofLogNotice("ofxMTApp") << "Key Pressed " << getMTViewForWindow(ofGetMainLoop()->getCurrentWindow())->getName();
    appKeyPressed(key.key);
}

void ofxMTApp::keyReleased(ofKeyEventArgs &key)
{
    if (ofGetKeyPressed(OF_KEY_COMMAND))
    {
        auto k = key.key;
        int bla = 'o';
        if ( k == 15 )
        {
                open();
        }
        else if ( k == 19 )
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

    appKeyReleased(key.key);

}
/// Method is called in the ofxMTApp contructor, right before the app is run.
/// Override this method and instantiate your model and main view classes, as well as the main
/// window size and settings.

void ofxMTApp::initialize()
{
    // These should be overriden sometime
//	model = shared_ptr<ofxMTModel>(new ofxMTModel("default"));
    model = nullptr;
}

void ofxMTApp::createAppViews()
{
    ofGLFWWindowSettings windowSettings;
    windowSettings.width = 1280;
    windowSettings.height = 800;
	mainWindow = createWindow("Main Window", windowSettings);
}

void ofxMTApp::run()
{
    initialize();
    createAppViews();
    ofRunApp(std::dynamic_pointer_cast<ofAppBaseWindow>(mainWindow), shared_ptr<ofBaseApp>(this));
    ofAddListener(ofEvents().keyPressed, this, &ofxMTApp::keyPressed);
    isInitialized = false;


    if (NSPrefAutoloadLastFile)
    {
        isInitialized = openImpl(MTPrefLastFile);

        if (!isInitialized)
        {
            string msg = "Tried to open " + MTPrefLastFile.get() + " but could not find it";
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

    ofRunMainLoop();
}

/// APP MODES

void ofxMTApp::setAppMode(MTAppModeName mode)
{
//	if(!ofContains(appModes, mode)) return;

    if (mode == currentMode)
    {
        return;
    }
    else
    {

        // TODO: Make mode changes instantiate themselves, listen for their own events
        static MTAppModeChangeArgs changeArgs;
        changeArgs.newMode = mode;
        changeArgs.oldMode = currentMode;
        currentMode = mode;
        ofNotifyEvent(ofxMTApp::appChangeModeEvent, changeArgs, this);
    }
}

MTAppModeName ofxMTApp::getCurrentMode()
{
    return currentMode;
}

shared_ptr<ofxMTWindow> ofxMTApp::createWindow(string windowName, ofWindowSettings& settings)
{

    auto window = shared_ptr<ofxMTWindow>(new ofxMTWindow(windowName));
    window->contentView->setWindow(window);
#ifndef TARGET_OPENGLES
    ofGLFWWindowSettings glfwWS = (ofGLFWWindowSettings) settings;
    window->setup(glfwWS);
#else
    ofGLESWindowSettings esWS = (ofGLESWindowSettings) settings;
    window->setup(esWs);
#endif

	addAllEvents(window.get());

    // Add the "global" keyboard event listener:
    ofAddListener(window->events().keyPressed, this, &ofxMTApp::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(window->events().keyReleased, this, &ofxMTApp::keyReleased, OF_EVENT_ORDER_BEFORE_APP);
//    window->events().
    ofGetMainLoop()->addWindow(window);
    windows.push_back(window);
    ofParameterGroup* thisWindow;

    if(MTPrefsWindowsGroup.contains(window->name))
    {
        thisWindow = &MTPrefsWindowsGroup.getGroup(window->name);
        auto pos = thisWindow->getVec3f(MTPrefsWindowPositionName);
        auto size = thisWindow->getVec3f(MTPrefsWindowSizeName);
        window->setWindowShape(size->x, size->y);
        window->setWindowPosition(pos->x, pos->y);
    }
    else
    {
        thisWindow = new ofParameterGroup();
        thisWindow->setName(window->name);
        ofParameter<ofPoint>* pos = new ofParameter<ofPoint>();
        ofParameter<ofPoint>* size = new ofParameter<ofPoint>();
        pos->set(MTPrefsWindowPositionName, window->getWindowPosition());
        size->set(MTPrefsWindowSizeName, window->getWindowSize());
        thisWindow->add(*pos, *size);
        MTPrefsWindowsGroup.add(*thisWindow);
    }


//	The ofApp system only notifies setup for the first window it creates, the rest are on their own aparently.
//	So we check if we have initilized the ofApp system, and if we have, then that means that we need
//	to notify setup for the window we are creating
    if(!ofAppInitialized)
    {
        ofAppInitialized = true;
    }
    else
    {
        window->events().notifySetup();
    }



    return window;
}


void ofxMTApp::addAllEvents(ofxMTWindow* w)
{
	w->events().enable();
	ofAddListener(w->events().setup, w, &ofxMTWindow::setupInternal, OF_EVENT_ORDER_APP);
	ofAddListener(w->events().update, w, &ofxMTWindow::update,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().draw, w, &ofxMTWindow::draw,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().exit,w, &ofxMTWindow::exit,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().keyPressed,w, &ofxMTWindow::keyPressed,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().keyReleased,w, &ofxMTWindow::keyReleased,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseMoved,w, &ofxMTWindow::mouseMoved,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseDragged,w, &ofxMTWindow::mouseDragged,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mousePressed,w, &ofxMTWindow::mousePressed,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseReleased,w, &ofxMTWindow::mouseReleased,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseScrolled,w, &ofxMTWindow::mouseScrolled,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseEntered,w, &ofxMTWindow::mouseEntered,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().mouseExited,w, &ofxMTWindow::mouseExited,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().windowResized,w, &ofxMTWindow::windowResized,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().messageEvent,w, &ofxMTWindow::messageReceived,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().fileDragEvent,w, &ofxMTWindow::dragged,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchCancelled,w, &ofxMTWindow::touchCancelled,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchDoubleTap,w, &ofxMTWindow::touchDoubleTap,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchDown,w, &ofxMTWindow::touchDown,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchMoved,w, &ofxMTWindow::touchMoved,OF_EVENT_ORDER_APP);
	ofAddListener(w->events().touchUp,w, &ofxMTWindow::touchUp,OF_EVENT_ORDER_APP);
	ofAddListener(ofxMTApp::appChangeModeEvent, w, &ofxMTWindow::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);
}

void ofxMTApp::removeAllEvents(ofxMTWindow* w)
{
	
	ofRemoveListener(w->events().setup, w, &ofxMTWindow::setupInternal, OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().update, w, &ofxMTWindow::update,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().draw, w, &ofxMTWindow::draw,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().exit,w, &ofxMTWindow::exit,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().keyPressed,w, &ofxMTWindow::keyPressed,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().keyReleased,w, &ofxMTWindow::keyReleased,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseMoved,w, &ofxMTWindow::mouseMoved,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseDragged,w, &ofxMTWindow::mouseDragged,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mousePressed,w, &ofxMTWindow::mousePressed,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseReleased,w, &ofxMTWindow::mouseReleased,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseScrolled,w, &ofxMTWindow::mouseScrolled,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseEntered,w, &ofxMTWindow::mouseEntered,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().mouseExited,w, &ofxMTWindow::mouseExited,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().windowResized,w, &ofxMTWindow::windowResized,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().messageEvent,w, &ofxMTWindow::messageReceived,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().fileDragEvent,w, &ofxMTWindow::dragged,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchCancelled,w, &ofxMTWindow::touchCancelled,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchDoubleTap,w, &ofxMTWindow::touchDoubleTap,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchDown,w, &ofxMTWindow::touchDown,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchMoved,w, &ofxMTWindow::touchMoved,OF_EVENT_ORDER_APP);
	ofRemoveListener(w->events().touchUp,w, &ofxMTWindow::touchUp,OF_EVENT_ORDER_APP);
	ofRemoveListener(ofxMTApp::appChangeModeEvent, w, &ofxMTWindow::appModeChanged,OF_EVENT_ORDER_AFTER_APP + 1000);
}

//// UI
weak_ptr<ofAppBaseWindow> ofxMTApp::getMainWindow()
{
    return mainWindow;
}

//// FILE HANDLING


void ofxMTApp::open()
{
    ofFileDialogResult result = ofSystemLoadDialog();
    if (result.bSuccess)
    {
        openImpl(result.filePath);
    }
}


void ofxMTApp::save() {
    if (MTPrefLastFile.get() == string(""))
    {
        saveAs();
    }
    else
    {
        saveImpl();
    }
}

void ofxMTApp::saveAs()
{
    ofFileDialogResult result = ofSystemSaveDialog(MTPrefLastFile, "Save As...");
    if(!result.bSuccess)
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
}

bool ofxMTApp::saveAsImpl(string filePath)
{
    string temp = MTPrefLastFile;
    MTPrefLastFile.setWithoutEventNotifications(filePath);

    if(!saveImpl())
    {
        MTPrefLastFile.setWithoutEventNotifications(temp);
        return false;
    }

    fileName = ofFilePath::getFileName(filePath);
    isInitialized = true;
    mainWindow->setWindowTitle(fileName);
    //Fire the change value event?
    return true;
}

bool ofxMTApp::saveImpl()
{

    model->serialize(serializer);

    if(!serializer.save(MTPrefLastFile.get()))
    {
        ofLog(OF_LOG_ERROR, "Encountered an error while saving the file");
        ofSystemAlertDialog("Encountered an error while saving the file");
        return false;
    }

    saveAppPreferences();
    return true;

}

/// Open sesame.
bool ofxMTApp::openImpl(string filePath)
{

    if (filePath == "")
    {
        newFile();
        return true;
    }

    if(!serializer.load(filePath))
    {
        ofLog(OF_LOG_ERROR, "Failed loading file");
        return false;
    }
    else
    {
        model->deserialize(serializer);
        if (model == nullptr)
        {
            ofLogError("ofxMTApp", "Failed Loading Model");
            ofSystemAlertDialog("Failed Loading Model");
        }
        else
        {
            MTPrefLastFile = filePath;
            fileName = ofFilePath::getFileName(filePath);
            isInitialized = true;
            mainWindow->setWindowTitle(fileName);
            saveAppPreferences();
            modelLoadedEvent.notify(this);
            ofLogVerbose("ofxMTApp", "File loaded.");
            return true;
        } //End load model
    } //End loadLastFile
    return false;
}

void ofxMTApp::newFile()
{
    // Call the user's newFile method:

    saveAppPreferences();

    newFileSetup();
    MTPrefLastFile = "";
    fileName = "";
    isInitialized = true;
    mainWindow->setWindowTitle(fileName);

    setAppMode(defaultMode);

    modelLoadedEvent.notify(this);
}

/// Reload the last opened file.
bool ofxMTApp::revert()
{
    return openImpl(MTPrefLastFile);
}

/// Saves!
bool ofxMTApp::saveAppPreferences()
{
    auto windowsGroup = appPreferences.getGroup(MTPrefsWindowsGroupName);

    for (int i = 0; i < windowsGroup.size(); i++)
    {
        auto thisWindow = windowsGroup.getGroup(i);

        auto it = std::find_if(windows.begin(), windows.end(),
                               [&thisWindow](std::shared_ptr<ofxMTWindow> current)
        { return current->name.get() == thisWindow.getName(); });

        if (it != windows.end())
        {
            thisWindow.getVec3f(MTPrefsWindowPositionName).set(glm::vec3((*it)->getWindowPosition(), 0));
            thisWindow.getVec3f(MTPrefsWindowSizeName).set(glm::vec3((*it)->getWindowSize(), 0));
        }
    }
//	NSPrefsViewsGroup.setParent(appPreferences);
//	appPreferences.add(NSPrefsViewsGroup);
    ofSerialize(appPrefsXml, appPreferences);
    return appPrefsXml.save(APP_PREFERENCES_FILE);
}

//// EVENTS

void ofxMTApp::windowClosing(ofxMTWindow* window)
{
    ofLogVerbose() << "Closing " << window->name;

    ofRemoveListener(window->events().keyPressed, this, &ofxMTApp::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
    ofRemoveListener(window->events().keyReleased, this, &ofxMTApp::keyReleased, OF_EVENT_ORDER_BEFORE_APP);
//    ofRemoveListener(modelLoadedEvent, view, &ofxMTWindow::modelDidLoadInternal, OF_EVENT_ORDER_AFTER_APP);

    //This is another:
    auto it = std::find_if(windows.begin(), windows.end(), [&](std::shared_ptr<ofAppBaseWindow> const& current) {
        return current.get() == window;
    });

    if (it != windows.end())
    {
        (*it)->events().disable();
        windows.erase(it);
    }
}

void ofxMTApp::exit()
{
    // Shouldn't be necessary, but just in case:
//	if (windows.size() > 0)
//	{
//		for (auto window : windows)
//		{
//
//			window->setWindowShouldClose();
//			window->events().disable();
//		}
//	}

//	views.clear();
//	windows.clear();
}


int ofxMTApp::getLocalMouseX()
{
    ///TODO: Get local mouse
    ofLogNotice() << "Not implemented yet!!";
    return 0;
//    auto mtView = getMTViewForWindow(ofGetMainLoop()->getCurrentWindow());
//    if (mtView != nullptr)
//    {
//        return mtView->getContentMouse().x;
//    }
//    else
//    {
//        ofLogNotice("ofxMTApp") << "getLocalMouseX: Could not find MTView for window";
//        return -1;
//    }
}


int ofxMTApp::getLocalMouseY()
{
    ///TODO: Get local mouse
    ofLogNotice() << "Not implemented yet!!";
    return 0;
//    auto mtView = getMTViewForWindow(ofGetMainLoop()->getCurrentWindow());
//    if (mtView != nullptr)
//    {
//        return mtView->getContentMouse().y;
//    }
//    else
//    {
//        ofLogNotice("ofxMTApp") << "getLocalMouseY: Could not find MTView for window";
//        return -1;
//    }
}

int mtGetLocalMouseX()
{
    return ofxMTApp::sharedApp->getLocalMouseX();
}

int mtGetLocalMouseY()
{
    return ofxMTApp::sharedApp->getLocalMouseY();

}

ofPath ofxMTApp::pathFromString(string s)
{
    vector<string> commandStrings = ofSplitString(s, "{", true, true);
    ofPath thePath;

    for (auto cs : commandStrings) {
        vector<string> commandStringElements = ofSplitString(cs, ";", true, true);

        ofPath::Command* thisCommand;
        int commandType = ofToInt(commandStringElements[0]);
        ofPoint p, cp1, cp2;
        switch (commandType) {
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
                ofLog(OF_LOG_WARNING, "ofxMTApp::pathFromString: A Path Command supplied is not implemented");
                break;
        }

    }

    return thePath;
}

string ofxMTApp::pathToString(ofPath &path)
{
    vector<ofPath::Command> commands = path.getCommands();

    string out = "";

    for (auto c : commands)
    {
        out += "{ " + ofToString(c.type) + "; " +
        ofToString(c.to) + "; " +
        ofToString(c.cp1) + "; " +
        ofToString(c.cp2) + "; } ";

    }

    return out;
}
