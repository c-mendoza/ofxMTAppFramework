  
## Ideas
*  Move ofxMTAppView to ofxMTWindow, which subclasses ofAppGLFWWindow or ofAppGLESWindow depending on build target.
  * Perhaps create an ofxMTDocumentWindow, which has scrollbars and a zoomable view. Of course we could further abstract this into views etc, much like Cocoa, but I don't think I want to go down that route.
