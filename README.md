# ofxMTAppFramework

The ofxMTAppFramework is a set of classes that facilitate the creation of applications that require a more complex set of UI behaviors than those provided by the standard ofApp application. With the ofxMTAppFramework you can:

* Easily create nested graphical views, each of which have their own set of mouse and keyboard events. View focus, local drawing coordinates, local mouse coordinates, hit testing, etc, is all handled for you behind the scenes. Think UIView/NSView or Flash Component-like behavior.
* Get "save" and "save as" behaviors for your applications, with dialog boxes, accessible via keyboard shortcuts.
* Use a persistent app preferences system. By default it stores app window size and position, but it can very easily be extended to remember custom app properties of any type. Should your app start on full screen, or should it be windowed? Should it run on debug mode, or presentation mode? Should it automatically load the last file that was saved, or should it create a new one on start? Easily manage and store these states with app preferences.

The goal of the framework is to provide useful app behavior needed in any non-trivial OF application without writing much boilerplate code, and yet still be lightweight enough to be useful as a prototyping framework.

## Architecture
* Model-View (no Controller) design. The View controls itself, the Model may control data-related logic.
* Document-based application, where only a single document is open at a given time.
* Main classes:
  - **MTApp**: Represents the application itself. Manages the current app state, keeps track of Windows, loads files (deserializes data), saves files (serializes data), manages preferences.
  - **MTWindow**: A class derived from ofBaseAppWindow. Manages views (represented by the MTView class) and administers events to the views. Each MTWindow has a root MTView.
  - **MTView**: An MTView represents a space in which you can draw. It is also a container that holds additional MTViews (*subviews*), which have a single "parent" (a *superview*). Each MTView has a position and bounds (the *frame*), which is always expressed in the coordinate system of the superview. Each MTView also has a *content* space, which has its own position and coordinate system, and which may differ from the *frame's*. MTViews can receive mouse events, which are automatically transformed into local (frame) coordinates. They can also receive keyboard input, with the MTWindow handling which MTView has keyboard focus. They behave a bit like Cocoa's NSView/UIView, or like Flash's MovieClips objects (but without a timeline).
  - **MTModel**: The data store of the app. It relies significantly on ofParameter.

## Design
The framework is inspired by the Cocoa approach to windows, views, and documents. Unlike Cocoa, which uses a Model-View-Controller paradigm, ofxMTAppFramework uses a "Model-View" (no Controller) design pattern, where the logic for controlling the view is in the view class itself.
This is more amenable to the type of applications made with OpenFrameworks, which typically have a single view for the app data, and reduces the number of classes where the application is distributed to.


The workflow in ofxMTAppFramework is a bit different than in the traditional OF app, where a single class represents the app, the data (model), and the output (view).
Given that most OF applications are output-oriented, most of the programming is done on Views.

is to transform the OF workflow from a single-app-class
