
## Ideas
  *  Move ofxMTAppView to ofxMTWindow, which subclasses ofAppGLFWWindow or ofAppGLESWindow depending on build target.
  * Perhaps create an ofxMTDocumentWindow, which has scrollbars and a zoomable view. Of course we could further abstract this into views etc, much like Cocoa, but I don't think I want to go down that route.

## New Approach
* ofxMTWindow is derived from the appropriate ofWindow given the platform
* This class is in charge of:
  * Providing a graphics context (done via the superclass)
  * Keeping track of views
  * Sending events to views
  * Creating and deleting views
  * Handling full screen in a more sophisticated way than ofToggleFullScreen. ofxScreenSetup can be the base, or can be used outright
* ofxMTView is a lightweight graphics view abstraction, in which we can draw. Windows may have multiple views, even overlapping views.  
