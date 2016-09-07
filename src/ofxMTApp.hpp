#ifndef ofxMTApp_hpp
#define ofxMTApp_hpp

#include "ofMain.h"

//#include "ofxMTModel.hpp"
//#include "ofxMTView.hpp"
#include "ofxMTAppMode.hpp"

class ofxMTView;
class ofxMTModel;
//class ofxMTAppMode;

typedef string MTAppMode;

class MTAppModeChangeArgs;

class ofxMTApp : public ofBaseApp
{
	
public:
//	static ofxMTApp& sharedApp() {
//		static ofxMTApp instance;
//		return instance;
//	};
//	
//	ofxMTApp(ofxMTApp const&) = delete;
//	void operator=(ofxMTApp const&) = delete;

	ofxMTApp();
	
	//TODO: Proper singleton
	static ofxMTApp* sharedApp;
	
	/// Extra "constructor" for the user. It is the last thing that is called in the default ofxMTApp contructor,
	/// and before the app's setup() function. This is where you want to instantiate your Model and your View.
	/// Default implementation creates a placeholder model and view.
	virtual void initialize();
	void run();
	
	/// APP MODES
	const MTAppMode defaultMode = "MTAppModeDefault";

	
	void setMode(MTAppMode mode);
	void registerMode(MTAppMode mode)
	{
		appModes.push_back(mode);
	}
	
	//------------------- EVENTS
	/// Called after the window is created and once the OpenGL context is up.
	virtual void setup();
//	virtual void update();
//	virtual void draw();
	virtual void exit();
	virtual void keyPressed(int key);
	virtual void keyReleased(int key);
//	virtual void mouseMoved(int x, int y );
//	virtual void mouseDragged(int x, int y, int button);
//	virtual void mousePressed(int x, int y, int button);
//	virtual void mouseReleased(int x, int y, int button);
//	virtual void mouseEntered(int x, int y);
//	virtual void mouseExited(int x, int y);
//	virtual void windowResized(int w, int h);
//	virtual void dragEvent(ofDragInfo dragInfo);
//	virtual void gotMessage(ofMessage msg);
	
	//// UI
	shared_ptr<ofAppBaseWindow> getMainWindow();
	shared_ptr<ofxMTView> getMainView();
	void createWindowForView(shared_ptr<ofxMTView> view, ofGLFWWindowSettings settings);
	void viewClosing(ofxMTView* view);
	
	//// FILE HANDLING
	void saveAs();
	void open();
	void save();
	bool revert();
	bool saveAppPreferences();
	
	void registerAppPreference(ofAbstractParameter& preference);
	
	shared_ptr<ofxMTModel> getModel() { return model; };
	
	static ofEvent<MTAppModeChangeArgs> appChangeModeEvent;
	static ofEvent<void> modelLoadedEvent;


protected:
	
	ofPtr<ofXml> serializer;
	ofPtr<ofXml> appPrefsSerializer;

	///The current file that is open
	string fileName;
	
	///Full path of the file
	string filePath;
	
	///The file extension you want your documents to have
	string fileExtension;
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



#endif