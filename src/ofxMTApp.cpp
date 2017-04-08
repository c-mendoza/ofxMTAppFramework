#include "ofxMTApp.hpp"

const string ofxMTApp::APP_PREFERENCES_FILE = "appSettings.xml";
const string ofxMTApp::NSPrefsViewsGroupName = "Views";
const string ofxMTApp::NSPrefsViewPositionName = "Position";
const string ofxMTApp::NSPrefsViewSizeName = "Size";

ofEvent<MTAppModeChangeArgs> ofxMTApp::appChangeModeEvent;
ofEvent<void> ofxMTApp::modelLoadedEvent;
ofxMTApp* ofxMTApp::sharedApp = 0;


ofxMTApp::ofxMTApp()
{
	
	//TODO: Proper singleton
	if(!ofxMTApp::sharedApp)
	{
		//Set the basic preferences
		NSPrefLastFile.set("NSPrefLastFile", "");
		NSPrefAutoloadLastFile.set("NSPrefAutoloadLastFile", false);
		NSPrefLaunchInFullScreen.set("NSPrefLaunchInFullScreen", false);
		NSPrefsViewsGroup.setName(NSPrefsViewsGroupName);
		NSPrefsViewsGroup.setSerializable(true);
		appPreferences.setName("App Preferences");
		appPreferences.add(NSPrefLaunchInFullScreen,
						   NSPrefLastFile,
						   NSPrefAutoloadLastFile);
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
			auto viewsXml = appPrefsXml.findFirst("//App_Preferences/Views").getChildren();

			
			for (auto & view : viewsXml)
			{
				ofParameterGroup thisView;
				thisView.setName(view.getName());
				ofLogVerbose("View in XML: " + view.getName());
				ofParameter<ofPoint> pos, size;
				pos.set(NSPrefsViewPositionName, view.getChild("Position").getValue<ofVec3f>());
				size.set(NSPrefsViewSizeName, view.getChild("Size").getValue<ofVec3f>());
				thisView.add(pos, size);
				NSPrefsViewsGroup.add(thisView);
			}
		}
		appPreferences.add(NSPrefsViewsGroup);
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
	createAppViews();
}

void ofxMTApp::createAppViews()
{
	mainView = shared_ptr<ofxMTView>(new ofxMTView("Main_View"));
	ofGLFWWindowSettings windowSettings;
	windowSettings.setGLVersion(2, 1);
	windowSettings.width = 1280;
	windowSettings.height = 800;
	createWindowForView(mainView, windowSettings);
}

void ofxMTApp::run()
{
	ofRunApp(mainView->getWindow(), shared_ptr<ofBaseApp>(this));
//	ofGetMainLoop()->run(std::shared_ptr<ofBaseApp> (this));
	ofAddListener(ofEvents().keyPressed, this, &ofxMTApp::keyPressed);
	isInitialized = false;
	
	for (auto view : views)
	{
		//		view->setup();
	}
	
	if (NSPrefAutoloadLastFile)
	{
		isInitialized = openImpl(NSPrefLastFile);
		
		if (!isInitialized)
		{
			string msg = "Tried to open " + NSPrefLastFile.get() + " but could not find it";
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

void ofxMTApp::setMode(MTAppMode mode)
{
//	if(!ofContains(appModes, mode)) return;
	
	if (mode == currentMode)
	{
		return;
	}
	else
	{
		static MTAppModeChangeArgs changeArgs;
		changeArgs.newMode = mode;
		changeArgs.oldMode = currentMode;
		currentMode = mode;
		ofNotifyEvent(ofxMTApp::appChangeModeEvent, changeArgs, this);
	}
}

MTAppMode ofxMTApp::getCurrentMode()
{
	return currentMode;
}

void ofxMTApp::createWindowForView(shared_ptr<ofxMTView> view, ofGLFWWindowSettings settings)
{
	
	shared_ptr<ofAppBaseWindow> window = ofCreateWindow(settings);
	view->setWindow(window);
	
	ofAddListener(ofxMTApp::modelLoadedEvent, view.get(), &ofxMTView::modelDidLoadInternal, OF_EVENT_ORDER_AFTER_APP);
	
	// Add the "global" keyboard event listener:
	ofAddListener(window->events().keyPressed, this, &ofxMTApp::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
	ofAddListener(window->events().keyReleased, this, &ofxMTApp::keyReleased, OF_EVENT_ORDER_BEFORE_APP);
	
	
	ofParameterGroup* thisView;
	
	
	if(NSPrefsViewsGroup.contains(view->getName()))
	{
		thisView = &NSPrefsViewsGroup.getGroup(view->getName());
		auto pos = thisView->getVec3f(NSPrefsViewPositionName);
		auto size = thisView->getVec3f(NSPrefsViewSizeName);
		window->setWindowShape(size->x, size->y);
		window->setWindowPosition(pos->x, pos->y);
	}
	else
	{
		thisView = new ofParameterGroup();
		thisView->setName(view->getName());
		ofParameter<ofPoint>* pos = new ofParameter<ofPoint>();
		ofParameter<ofPoint>* size = new ofParameter<ofPoint>();
		pos->set(NSPrefsViewPositionName, view->getWindow()->getWindowPosition());
		size->set(NSPrefsViewSizeName, view->getWindow()->getWindowSize());
		thisView->add(*pos, *size);
		NSPrefsViewsGroup.add(*thisView);
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
	
	if (view == mainView)
	{
		windows.insert(windows.begin(), move(window));
		views.insert(views.begin(), view);
	}
	else
	{
		windows.push_back(move(window));
		views.push_back(view);
	}
}

//// UI
shared_ptr<ofAppBaseWindow> ofxMTApp::getMainWindow()
{
	return mainView->getWindow();
}

shared_ptr<ofxMTView> ofxMTApp::getMainView()
{
	return mainView;
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
	if (NSPrefLastFile.get() == string(""))
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
	ofFileDialogResult result = ofSystemSaveDialog(NSPrefLastFile, "Save As...");
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
	string temp = NSPrefLastFile;
	NSPrefLastFile.setWithoutEventNotifications(filePath);

	if(!saveImpl())
	{
		NSPrefLastFile.setWithoutEventNotifications(temp);
		return false;
	}
	
	fileName = ofFilePath::getFileName(filePath);
	isInitialized = true;
	mainView->getWindow()->setWindowTitle(fileName);
	//Fire the change value event?
	return true;
}

bool ofxMTApp::saveImpl()
{

	model->serialize(serializer);
	
	if(!serializer.save(NSPrefLastFile.get()))
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
			ofLog(OF_LOG_ERROR, "Failed Loading Model");
			ofSystemAlertDialog("Failed Loading Model");
		}
		else
		{
			NSPrefLastFile = filePath;
			fileName = ofFilePath::getFileName(filePath);
			isInitialized = true;
			mainView->getWindow()->setWindowTitle(fileName);
			saveAppPreferences();
			modelLoadedEvent.notify(this);
//			mainView->modelDidLoad();
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
	NSPrefLastFile = "";
	fileName = "";
	isInitialized = true;
	mainView->getWindow()->setWindowTitle(fileName);

	setMode(defaultMode);
	
	modelLoadedEvent.notify(this);
}

/// Reload the last opened file.
bool ofxMTApp::revert()
{
	return openImpl(NSPrefLastFile);
}

/// Saves!
bool ofxMTApp::saveAppPreferences()
{
	auto viewsGroup = appPreferences.getGroup(NSPrefsViewsGroupName);
	
	for (int i = 0; i < viewsGroup.size(); i++)
	{
		auto thisView = viewsGroup.getGroup(i);
		
		auto it = std::find_if(views.begin(), views.end(), [&](std::shared_ptr<ofxMTView> const& current) {
			return current->getName() == thisView.getName();
		});
		
		if (it != views.end())
		{
			thisView.getVec3f(NSPrefsViewPositionName).set(glm::vec3((*it)->getWindow()->getWindowPosition(), 0));
			thisView.getVec3f(NSPrefsViewSizeName).set(glm::vec3((*it)->getWindow()->getWindowSize(), 0));
		}
	}
//	NSPrefsViewsGroup.setParent(appPreferences);
//	appPreferences.add(NSPrefsViewsGroup);
	ofSerialize(appPrefsXml, appPreferences);
	return appPrefsXml.save(APP_PREFERENCES_FILE);
}

//// EVENTS

void ofxMTApp::viewClosing(ofxMTView* view)
{
	cout << "Closing " << view->getName() << "\n";
	auto window = view->getWindow();
	
	ofRemoveListener(window->events().keyPressed, this, &ofxMTApp::keyPressed, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(window->events().keyReleased, this, &ofxMTApp::keyReleased, OF_EVENT_ORDER_BEFORE_APP);
	ofRemoveListener(modelLoadedEvent, view, &ofxMTView::modelDidLoadInternal, OF_EVENT_ORDER_AFTER_APP);
	
	//One way to find for stuff:
	for (auto i = views.begin(); i < views.end(); ++i)
	{
		if (i->get() == view)
		{
			saveAppPreferences();
			views.erase(i);
			break;
		}
	}
	
	//This is another:
	auto it = std::find_if(windows.begin(), windows.end(), [&](std::shared_ptr<ofAppBaseWindow> const& current) {
		return current.get() == window.get();
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

	views.clear();
	windows.clear();
}

shared_ptr<ofxMTView> ofxMTApp::getMTViewForWindow(shared_ptr<ofAppBaseWindow> window)
{
	for (auto mtView : views)
	{
		if (mtView->getWindow().get() == window.get())
		{
			return mtView;
		}
	}
	
	return nullptr;
}

int ofxMTApp::getLocalMouseX()
{
	auto mtView = getMTViewForWindow(ofGetMainLoop()->getCurrentWindow());
	if (mtView != nullptr)
	{
		return mtView->getContentMouse().x;
	}
	else
	{
		ofLogNotice("ofxMTApp") << "getLocalMouseX: Could not find MTView for window";
		return -1;
	}
}


int ofxMTApp::getLocalMouseY()
{
	auto mtView = getMTViewForWindow(ofGetMainLoop()->getCurrentWindow());
	if (mtView != nullptr)
	{
		return mtView->getContentMouse().y;
	}
	else
	{
		ofLogNotice("ofxMTApp") << "getLocalMouseY: Could not find MTView for window";
		return -1;
	}
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
