
## ~~New Approach~~ IMPLEMENTED
* ofxMTWindow is derived from the appropriate ofWindow given the platform
* This class is in charge of:
  * Providing a graphics context (done via the superclass)
  * Keeping track of views
  * Sending events to views
  * Creating and deleting views
  * Handling full screen in a more sophisticated way than ofToggleFullScreen. ofxScreenSetup can be the base, or can be used outright
* ofxMTView is a lightweight graphics view abstraction, in which we can draw. Windows may have multiple views, even overlapping views.  

## Further Ideas
* "Override" the ofAppGLFWWindow mouse and keyboard callbacks to produce ofCoreEvent behavior that is consistent with the MTView hierarchy.
