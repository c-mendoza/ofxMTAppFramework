#ifndef ofxMTApp_hpp
#define ofxMTApp_hpp

#include "ofxMTAppFramework.h"

class ofxMTView;
class ofxMTModel;
class ofxMTAppMode;

typedef string MTAppMode;

class MTAppModeChangeArgs;

class ofxMTApp : public ofBaseApp
{
	
public:
	ofxMTApp();
	
	//TODO: Proper singleton
	static ofxMTApp* sharedApp;
	
	/// Extra "constructor" for the user. It is the last thing that is called in the default ofxMTApp contructor,
	/// and before the app's setup() function. This is where you want to instantiate your Model and your View.
	/// Default implementation creates a placeholder model and view.
	virtual void initialize();
	void run();
	
	//------ APP MODES
	const MTAppMode defaultMode = "MTAppModeDefault";
	void setMode(MTAppMode mode);
	void registerMode(MTAppMode mode)
	{
		appModes.push_back(mode);
	}
	
	//------ EVENTS
	static ofEvent<MTAppModeChangeArgs> appChangeModeEvent;
	static ofEvent<void> modelLoadedEvent;
	
	virtual void exit();

	
	//// UI
	shared_ptr<ofAppBaseWindow> getMainWindow();
	shared_ptr<ofxMTView> getMainView();
	void createWindowForView(shared_ptr<ofxMTView> view, ofGLFWWindowSettings settings);
	
	///Returns the ofxMTView associated with the passed ofBaseAppWindow, or nullptr if the window does not
	///have any ofxMTView partner.
	shared_ptr<ofxMTView> getMTViewForWindow(shared_ptr<ofAppBaseWindow> window);
	
	///Returns the mouse x-position in local coordinates of the current window
	int getLocalMouseX();
	///Returns the mouse y-position in local coordinates of the current window
	int getLocalMouseY();
	
	void viewClosing(ofxMTView* view);
	
	//// FILE HANDLING
	void saveAs();
	void open();
	void save();
	bool revert();
	bool saveAppPreferences();
	
	void registerAppPreference(ofAbstractParameter& preference);
	
	shared_ptr<ofxMTModel> getModel() { return model; };
	
	//// UTILITY
	/// Stringifies a path.
	static string pathToString(ofPath& path);
	
	/// Makes an ofPath from a stringified representation.
	static ofPath pathFromString(string s);


protected:
	
	ofPtr<ofXml> serializer;
	ofPtr<ofXml> appPrefsSerializer;

	///The name of the current file.
	string fileName;
	
	///Full path of the file
	string filePath;
	
	///The file extension you want your documents to have. Defaults to ".xml", but it can be anything you want.
	string fileExtension = "xml";

	shared_ptr<ofxMTView> mainView;
	shared_ptr<ofxMTModel> model;
	const static string APP_PREFERENCES_FILE;
	bool isInitialized;
	ofParameterGroup appPreferences;
	ofParameter<string> NSPrefLastFile;
	ofParameter<bool> NSPrefAutoloadLastFile;
	ofParameter<bool> NSPrefLaunchInFullScreen;
	ofParameterGroup NSPrefsViewsGroup;

	vector<shared_ptr<ofAppBaseWindow>> windows;
	vector<shared_ptr<ofxMTView>> views;
	
	bool saveAsImpl(string newName);
	bool saveImpl();
	bool openImpl(string file);
	void keyPressed(ofKeyEventArgs &key);
	
	//APP MODES
	MTAppMode currentMode;
	vector<MTAppMode> appModes;
	//UI

	
private:
	bool ofAppInitialized = false;
	
	//UI / Convenience
	void storeViewParameters(ofxMTView* view);
	
	const static string NSPrefsViewsGroupName;
	const static string NSPrefsViewPositionName;
	const static string NSPrefsViewSizeName;
};

class MTAppModeChangeArgs : public ofEventArgs
{
public:
	MTAppMode newMode;
	MTAppMode oldMode;
	MTAppModeChangeArgs(){}
	
};

int mtGetLocalMouseX();
int mtGetLocalMouseY();



#endif
