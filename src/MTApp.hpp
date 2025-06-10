#ifndef ofxMTApp_hpp
#define ofxMTApp_hpp

#include <utils/ofXml.h>
#include <unordered_map>
#include "GLFW/glfw3.h"
#include "MTAppFrameworkUtils.hpp"
#include "ofxImGui.h"
#include "ofAppGLFWWindow.h"
#include "ofBaseApp.h"
#include "ofJson.h"

class MTView;
class MTModel;
class MTViewMode;
class MTWindow;
class MTOffscreenWindow;
class ofAppEGLWindowSettings;
class MTAppModeChangeArgs;

typedef std::string MTAppModeName;

class MTDisplay
{
   private:
    std::string name;
    ofRectangle frame;
    int id;

   public:
    MTDisplay(std::string name, ofRectangle frame, int id)
    {
        this->name = name;
        this->frame = frame;
        this->id = id;
    }

    const std::string& getName() const { return name; }
    const ofRectangle& getFrame() const { return frame; }
    int getId() const { return id; }
};

class MTApp : public ofBaseApp, public MTEventListenerStore
{
   public:
    MTApp(MTApp const&) = delete;
    void operator=(MTApp const&) = delete;
    enum SerializerType
    {
        XML,
        JSON
    };

    struct MTAppSettings
    {
        std::string fileExtension = "xml";
        std::string appPreferencesFileName = "com.yourName.yourApp";
        std::string modelName = "yourModel";
        ofGLFWWindowSettings mainWindowSettings;
        SerializerType serializerType = XML;
        bool appPreferencesFileInHomeDir = false;
        /**
         * @brief Specifies a file to open, typically used from the command
         * line. If this string is set, it will override MTPrefAutoloadLastFile
         */
        std::string fileToOpen;
        std::tuple<int, char**> commandLineArgs;
    };

    template <class AppType = MTApp, class ModelType = MTModel>
    static void CreateApp(MTAppSettings settings)
    {
        ofLogVerbose("MTApp") << "Creating App...";
        auto model = std::make_shared<ModelType>(settings.modelName);
        auto app = std::make_shared<AppType>();
        app->model = model;
        AppPtr = app;
        ofLogVerbose("MTApp") << "Creating Model " << settings.modelName;
        app->model->newFile();
        app->appPreferencesFilename = settings.appPreferencesFileName;
        app->fileExtension = settings.fileExtension;
        app->serializerType = settings.serializerType;
        app->saveAppPreferencesInHomeDir = settings.appPreferencesFileInHomeDir;
        app->filePathFromAppSettings = settings.fileToOpen;
        app->commandLineArgs = settings.commandLineArgs;
        // RunApp(std::move(app), settings.mainWindowSettings);
        ofLogVerbose("MTApp") << "Running app...";
        app->createAppPreferencesFilePath();
        app->loadAppPreferences();
        app->mainWindow =
            app->createWindow("Main Window", settings.mainWindowSettings);
        ofRunApp(std::move(app));
    }

    template <class AppType = MTApp, class ModelType = MTModel>
    static void CreateApp()
    {
        MTAppSettings settings;
        CreateApp<AppType, ModelType>(settings);
    }

   private:
    void releasePointers();
    //	static MTApp* instance;
    //	static std::shared_ptr<MTApp> instance;

   public:
    static MTApp* Instance() { return GetApp(); }

    MTApp();
    ~MTApp() override;

    /**
     * @brief initialize  Override this method to register custom preferences
     * and perform other general app setup. Called before createAppViews, so
     * there will be no Windows created yet. Default implementation does
     * nothing.
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
     * @brief Called by the framework after all of the views and windows
     * have been created, and after any model file loaded automatically.
     * Anything created in createAppViews should have had its setup() method
     * called by now.
     * Useful for any general setup that requires an existing OpenGL context.
     * Default implementation does nothing.
     */
    virtual void appWillRun() {}

    // I'm sure that there is a better way than this, but right now...
    template <class T = MTModel>
    static std::shared_ptr<T> GetModel()
    {
        //		auto p = MTApp::Instance()->model.get();
        //		return *dynamic_cast<T*>(p);
        return std::dynamic_pointer_cast<T>(Instance()->model);
    }

    // I'm sure that there is a better way than this, but right now...
    template <class T = MTApp>
    static T* GetApp()
    {
        return dynamic_cast<T*>(AppPtr.lock().get());
    }

   private:
    static std::weak_ptr<MTApp> AppPtr;

   public:
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
    {
        appModes.push_back(modeName);
    }

    /**
     * @brief Called whenever the App Mode changes. This conveninece method
     * will be called before any other listener of the @code appModeChanged
     * event.
     * The default implementation does nothing.
     */
    virtual void appModeChanged(MTAppModeChangeArgs& changeArgs) {}

    virtual void exit()
    {
        //		MTApp::instance.reset();
    }

    //// UI
    std::weak_ptr<MTWindow> getMainWindow();

#ifndef TARGET_RASPBERRY_PI
//	std::shared_ptr<MTOffscreenWindow>
//	createOffscreenWindow(std::string windowName, ofGLFWWindowSettings&
// settings, bool useTextureRectangle = true);
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
     * @brief Creates a window. If a Main Window exists, the OpenGL major and
     * minor versions will be copied from that window.
     * @param windowName is used to provide a title, and recall size and
     * position. It must not be in use by any other window, otherwise
     * createWindow returns nullptr.
     * @param settings
     * @return A shared_ptr to the MTWindow, or nullptr if you used a windowName
     * already in use by another window.
     */
    std::shared_ptr<MTWindow> createWindow(std::string windowName,
                                           ofGLFWWindowSettings settings);

    /**
     * @brief Creates a window with default settings. The OpenGL context will be
     * shared with the Main Window.
     * @param windowName is used to provide a title, and recall size and
     * position. It must not be in use by any other window, otherwise
     * createWindow returns nullptr.
     * @return A shared_ptr to the MTWindow, or nullptr if you used a windowName
     * already in use by another window.
     */
    std::shared_ptr<MTWindow> createWindow(std::string windowName);

#endif

    void closeWindow(std::shared_ptr<MTWindow> window);
    //	void removeWindow(std::shared_ptr<MTWindow> window);
    //	/// Returns the mouse x-position in local coordinates of the current
    // window 	int getLocalMouseX();
    //	/// Returns the mouse y-position in local coordinates of the current
    // window 	int getLocalMouseY();

    std::shared_ptr<MTWindow> getWindowWithName(std::string windowName);
    /**
     * @brief You should not have to call this method.
     * Event handler for the windowClosing event.
     * @param window
     */
    void windowClosing(std::shared_ptr<MTWindow> window);

    /////// FILE HANDLING
    void saveAs();
    /**
     * @brief Opens a file. If the optional filepath is not passed, or if the
     * filepath is empty, a file system dialog box is shown to select a file to
     * open.
     * @param filepath The optional path to open.
     * @return True if the open operation succeeds.
     */
    bool open(std::string filepath = "");
    void save();
    bool revert();
    bool saveAppPreferences();
    void newFile();
    void saveCopy(std::string filename);

    /**
     * @brief Override this if you need to do anything when creating a new,
     * empty document.
     */
    virtual void newFileSetup() {}

    /**
     * @brief Gets the current file name, without the path.
     * @return A copy of the the file name.
     */
    std::string getFileName() { return fileName; }

    /**
     * @brief Gets the full path of the current file. This should be the same as
     * the value stored in MTPrefLastFile.
     * @return A string copy of the full file path.
     */
    std::string getFilePath() { return filePath; }
    /**
     * @brief Registers a new app preference. App preferences are saved
     * automatically prior to the app closing.
     * @param preference An ofParameter
     */
    // void registerAppPreference(ofAbstractParameter& preference);

    template <typename T>
    void registerAppPreference(ofParameter<T>& preference)
    {
        appPreferences.add(preference);
        prefEventListeners.push(preference.newListener(
            [this](T&)
            {
                saveAppPreferences();
            }));
    }

    /////// UTILITY
    /**
     * @brief Stringifies an ofPath.
     */
    static std::string pathToString(const ofPath& path);

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

    /**
     * @brief Runs a lambda a single time after the mainLoop iterates over the
     * windows.
     * @param f
     */
    void runOncePostLoop(std::function<void()> f)
    {
        loopFunctions.push_back(f);
    }

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
     * Fires after a model is loaded from the file system and deserialized. At
     * this point, all parameters in the Model's parameterGroup should be set.
     */
    ofEvent<ofEventArgs> modelLoadedEvent;

    /**
     * @brief Fires when a new file is created
     */
    ofEvent<void> newFileEvent;

    /**
     * Notifies after a document file has been opened and parsed, but prior to
     * MTModel::deserialize being called.
     */
    ofEvent<ofJson> preDeserializeJsonEvent;

    /**
     * @brief Notifies after a document file has been opened and parsed, but
     * prior to MTModel::deserialize being called. Use this version of the event
     * if your app's serializer type is XML.
     */
    ofEvent<ofXml> preDeserializeXMLEvent;

    /**
     * Notifies prior to the model being serialized and saved to disk.
     */
    ofEvent<void> modelWillSaveEvent;

   protected:
    SerializerType serializerType = XML;
    /// The name of the current file.
    std::string fileName;

    /// Full path of the file
    std::string filePath;

    /// The file extension you want your documents to have. Defaults to
    /// ".settings", but it can be anything you want.
    std::string fileExtension = "settings";
    std::tuple<int, char**> commandLineArgs;

   private:
    std::string filePathFromAppSettings;

   private:
    std::shared_ptr<MTWindow> mainWindow;
    std::shared_ptr<MTModel> model;

   protected:
    /**
     * The app preferences file name. It can be anything you want, but to avoid
     * naming conflicts it is recommended that you use reverse DNS notation.
     * Make sure to change the value of this field in your subclass, ideally
     * in the @code initialize() method.
     * Where this file is written to depends on the target OS.
     */
    std::string appPreferencesFilename = "com.yourGroup.yourApp.prefs";
    bool saveAppPreferencesInHomeDir = false;
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
    virtual void appKeyPressed(ofKeyEventArgs& key) {}

    /**
     * @brief Called whenever there is a key released anywhere in the app.
     * Override this to detect key presses globally.
     * Default implementation does nothing.
     */
    virtual void appKeyReleased(ofKeyEventArgs& key) {}

    /**
     * @brief Called once the model is loaded from disk. This is mostly a
     * convenience method so that you don't have to listen for the
     * modelLoadedEvent yourself. Default implementation does nothing.
     */
    virtual void modelLoaded() {}

    virtual void exit(ofEventArgs& args);
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

   private:
    static void windowPosCb(GLFWwindow* w, int x, int y);
    ofEventListeners internalEventListeners;

   public:
    /**
     * @brief Gets the currently connected displays.
     * @return A std::vector of shared_ptr<MTDisplay>.
     * @sa MTDisplay
     */
    static const std::vector<std::shared_ptr<MTDisplay>>& getDisplays()
    {
        return displays;
    }

    /**
     * @brief Updates the internal count of available displays.
     * You should not have to call this function.
     */
    static void updateDisplays();

#ifndef TARGET_OPENGLES
    static void setMonitorCb(GLFWmonitor* monitor, int connected);
#endif

   protected:
    static std::vector<std::shared_ptr<MTDisplay>> displays;

   private:
    bool ofAppInitialized = false;

    int glVersionMajor;
    int glVersionMinor;

    bool saveAsImpl(std::string newName);
    bool saveImpl();
    bool openImpl(std::string file);

    void loadAppPreferences();
    ofEventListeners prefEventListeners;

    struct WindowParams
    {
        std::string name;
        glm::vec2 position;
        glm::vec2 size;
    };

    std::unordered_map<std::string, WindowParams> wpMap;

    GLFWmonitor* getMonitorForWindow(MTWindow* w);

    /**
     * ~/.local/share/filename
     * ~/Library/Preferences/filename
     * %systemdrive%%homepath%\Roaming\filename
     *
     */
    std::filesystem::path appPreferencesPath = "";
    void createAppPreferencesFilePath();

    //	std::queue<std::function<void()>> loopFunctionQueue;

    std::deque<std::function<void()>> loopFunctions;

    bool inLoop = false;

   private:
    bool fileHandlingShortcutsEnabled = true;

   public:
    void enableFileHandlingShortcuts() { fileHandlingShortcutsEnabled = true; }

    void disableFileHandlingShortcuts()
    {
        fileHandlingShortcutsEnabled = false;
    }
};

class MTAppModeChangeArgs : public ofEventArgs
{
   public:
    MTAppModeName newModeName;
    MTAppModeName oldModeName;

    MTAppModeChangeArgs() {}
};

int mtGetLocalMouseX();
int mtGetLocalMouseY();

#endif
