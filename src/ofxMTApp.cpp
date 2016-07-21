#include "ofxMTApp.hpp"
#include "ofxMTView.hpp"
#include "ofxMTModel.hpp"

const string ofxMTApp::APP_SETTINGS_FILE = "appSettings.xml";
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
		appPreferences.setName("App Preferences");
		appPreferences.add(NSPrefLaunchInFullScreen,
						   NSPrefLastFile,
						   NSPrefAutoloadLastFile,
						   NSPrefsViewsGroup);
		

		
		ofxMTApp::sharedApp = this;
		currentMode = defaultMode;
		registerMode(defaultMode);
		
		fileExtension = "";
		
		ofInit();
		
		ofSetLogLevel(OF_LOG_VERBOSE);
		
		serializer = ofPtr<ofXml>(new ofXml);
		appPrefsSerializer = ofPtr<ofXml>(new ofXml);
		
		if(!appPrefsSerializer->load(APP_SETTINGS_FILE))
		{
			ofLog(OF_LOG_ERROR, "App Preferences could not be loaded, creating a new file.");
			ofSystemAlertDialog("App Preferences could not be loaded, creating a new file.");
			saveAppSettings();
		}
		else
		{
			appPrefsSerializer->setTo("//" + appPreferences.getEscapedName());
			appPrefsSerializer->setTo(NSPrefsViewsGroupName);
			
			bool success = appPrefsSerializer->setToChild(0);
			
			if (success)
			{
				do
				{
					string name = appPrefsSerializer->getName();
					ofParameterGroup thisView;
					thisView.setName(name);
					ofParameter<ofPoint> pos, size;
					pos.set(NSPrefsViewPositionName, ofVec3f(0, 0, 0));
					size.set(NSPrefsViewSizeName, ofVec3f(600, 400, 0));
					thisView.add(pos, size);
					NSPrefsViewsGroup.add(thisView);
				} while (appPrefsSerializer->setToSibling());
			}
			
			appPrefsSerializer->setTo("//" + appPreferences.getEscapedName());
			appPrefsSerializer->deserialize(appPreferences);
			appPrefsSerializer->clear();
		}
	}
}

void ofxMTApp::keyPressed(ofKeyEventArgs &key)
{
	
	if (ofGetKeyPressed(OF_KEY_COMMAND))
	{
		switch (key.key)
		{
			case 'o':
				open();
			break;
			case 's':
				if (ofGetKeyPressed(OF_KEY_SHIFT))
				{
					saveAs();
				}
				else
				{
					save();
				}
			default:
			break;
		}
	}
}

/// Method is called in the ofxMTApp contructor, right before the app is run.
/// Override this method and instantiate your model and main view classes, as well as the main
/// window size and settings.

void ofxMTApp::initialize()
{
	// These should be overriden sometime
//	model = shared_ptr<ofxMTModel>(new ofxMTModel("default"));
	model = nullptr;
	mainView = shared_ptr<ofxMTView>(new ofxMTView("Main View"));
	
	ofGLFWWindowSettings windowSettings;
	windowSettings.setGLVersion(2, 1);
	windowSettings.width = 1280;
	windowSettings.height = 800;
	createWindowForView(mainView, windowSettings);
}

void ofxMTApp::run()
{
//	ofRunApp(mainView->getWindow(), shared_ptr<ofBaseApp>(this));
	
	ofGetMainLoop()->run(std::shared_ptr<ofBaseApp> (this));
	ofAddListener(ofEvents().keyPressed, this, &ofxMTApp::keyPressed);

	ofRunMainLoop();
	
	//	ofGetMainLoop()->run(mainView->getWindow(), shared_ptr<ofBaseApp>(this));
	//	ofRunMainLoop();
}

/// APP MODES

void ofxMTApp::setMode(MTAppMode mode)
{
	if(!ofContains(appModes, mode)) return;
	
	if (mode == currentMode)
	{
		return;
	}
	else
	{
		static MTAppModeChangeArgs changeArgs;
		changeArgs.newMode = mode;
		changeArgs.oldMode = currentMode;
		ofNotifyEvent(ofxMTApp::appChangeModeEvent, changeArgs, this);
	}
}

void ofxMTApp::createWindowForView(shared_ptr<ofxMTView>& view, ofGLFWWindowSettings settings)
{
	
	shared_ptr<ofAppBaseWindow> window = ofCreateWindow(settings);
	view->setWindow(window);
	ofAddListener(ofxMTApp::modelLoadedEvent, view.get(), &ofxMTView::modelDidLoad, OF_EVENT_ORDER_AFTER_APP);

	if (view == mainView)
	{
		windows.insert(windows.begin(), window);
		views.insert(views.begin(), view);
	}
	else
	{
		windows.push_back(window);
		views.push_back(view);
	}
	
	ofParameterGroup* thisView;
	
	if(NSPrefsViewsGroup.contains(view->getName()))
	{
		thisView = &NSPrefsViewsGroup.getGroup(view->getName());
		ofPoint pos = thisView->getVec3f(NSPrefsViewPositionName);
		ofPoint size = thisView->getVec3f(NSPrefsViewSizeName);
		window->setWindowShape(size.x, size.y);
		window->setWindowPosition(pos.x, pos.y);
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
	
	
	
	
	///////NO:
//	window->events().notifySetup();
	
	//The ofApp system only notifies setup for the first window it creates, the rest are on their own aparently.
	//So we check if we have initilized the ofApp system, and if we have, then that means that we need
	//to notify setup for the window we are creating
//	if(!ofAppInitialized)
//	{
//		ofAppInitialized = true;
//	}
//	else
//	{
//		window->events().notifySetup();
//	}
}

void ofxMTApp::setup()
{
	isInitialized = false;
	
	for (auto view : views)
	{
		view->setup();
	}
	
	if (NSPrefAutoloadLastFile)
	{
		isInitialized = openImpl(NSPrefLastFile);
	}
	else
	{
		isInitialized = true;
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
	serializer->clear();
	model->saveWithSerializer(*serializer);
	
	if(!serializer->save(NSPrefLastFile))
	{
		ofLog(OF_LOG_ERROR, "Encountered an error while saving the file");
		ofSystemAlertDialog("Encountered an error while saving the file");
		return false;
	}
	
	saveAppSettings();
	return true;

}

/// Open sesame.
bool ofxMTApp::openImpl(string filePath)
{
	serializer->clear();
	
	if(!serializer->load(filePath))
	{
		ofLog(OF_LOG_ERROR, "Failed loading file");
		return false;
	}
	else
	{
		model->loadFromSerializer(*serializer);
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
			saveAppSettings();
			modelLoadedEvent.notify();
//			mainView->modelDidLoad();
			return true;
		} //End load model
	} //End loadLastFile
	return false;
}

/// Reload the last opened file.
bool ofxMTApp::revert()
{
	return openImpl(NSPrefLastFile);
}

/// Saves!
bool ofxMTApp::saveAppSettings()
{
	serializer->clear();
	appPrefsSerializer->serialize(appPreferences);
	return appPrefsSerializer->save(APP_SETTINGS_FILE);
}

void ofxMTApp::storeViewParameters(ofxMTView* v)
{
	ofParameterGroup thisView = NSPrefsViewsGroup.getGroup(v->getName());

	thisView.getVec3f(NSPrefsViewPositionName).set(v->getWindow()->getWindowPosition());
	thisView.getVec3f(NSPrefsViewSizeName).set(v->getWindow()->getWindowSize());

}

//// EVENTS

void ofxMTApp::viewClosing(ofxMTView* view)
{
	cout << "Closing " << view->getName() << "\n";
	auto window = view->getWindow();
	
	//One way to find for stuff:
	for (auto i = views.begin(); i < views.end(); ++i)
	{
		if (i->get() == view)
		{
			storeViewParameters(view);
			saveAppSettings();
			views.erase(i);
			return;
		}
	}
	
	//This is another:
	auto it = std::find_if(windows.begin(), windows.end(), [&](std::shared_ptr<ofAppBaseWindow> const& current) {
		return current.get() == window.get();
	});
	
	if (it != windows.end())
	{
		windows.erase(it);
	}
}


void ofxMTApp::exit()
{
	for (auto i : windows)
	{
		i->setWindowShouldClose();
		i->events().disable();
	}
}

//--------------------------------------------------------------
void ofxMTApp::keyPressed(int key)
{
	
}

//--------------------------------------------------------------
void ofxMTApp::keyReleased(int key)
{
	
}
//
////--------------------------------------------------------------
//void ofxMTApp::mouseMoved(int x, int y ){
//	
//}
//
////--------------------------------------------------------------
//void ofxMTApp::mouseDragged(int x, int y, int button){
//	
//}
//
////--------------------------------------------------------------
//void ofxMTApp::mousePressed(int x, int y, int button){
//	
//}
//
////--------------------------------------------------------------
//void ofxMTApp::mouseReleased(int x, int y, int button){
//	
//}
//
////--------------------------------------------------------------
//void ofxMTApp::mouseEntered(int x, int y){
//	
//}
//
////--------------------------------------------------------------
//void ofxMTApp::mouseExited(int x, int y){
//	
//}
//
////--------------------------------------------------------------
//void ofxMTApp::windowResized(int w, int h){
//	
//}
//
////--------------------------------------------------------------
//void ofxMTApp::gotMessage(ofMessage msg){
//	
//}
//
////--------------------------------------------------------------
//void ofxMTApp::dragEvent(ofDragInfo dragInfo){
//	
//}
