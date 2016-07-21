//
//  ofxMTView.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/20/15.
//
//

#ifndef ofxMTView_hpp
#define ofxMTView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxMTApp.hpp"

class ofxMTModel;

class ofxMTView {
	
public:
	ofxMTView(string _name);
	~ofxMTView();
	void setModel(ofPtr<ofxMTModel> model);
	void setName(string viewName);
	string getName() { return name; }
	void setWindow(shared_ptr<ofAppBaseWindow> window);
	
	//Called once the model is successfully loaded from file
	//Default implementation does nothing
	virtual void modelDidLoad(){}
	virtual void setup(){}
	virtual void update(){}
	virtual void draw();
	virtual void exit(){}
	
	virtual void windowResized(int w, int h){}
	
	virtual void keyPressed( int key ){}
	virtual void keyReleased( int key ){}
	
	/// \brief Called on the active window when the mouse is moved
	virtual void mouseMoved( int x, int y ){}
	
	/// \brief Called on the active window when the mouse is dragged, i.e.
	/// moved with a button pressed
	virtual void mouseDragged( int x, int y, int button ){}
	
	/// \brief Called on the active window when a mouse button is pressed
	virtual void mousePressed( int x, int y, int button ){}
	
	/// \brief Called on the active window when a mouse button is released
	virtual void mouseReleased(int x, int y, int button ){}
	
	/// \brief Called on the active window when the mouse wheel is scrolled
	virtual void mouseScrolled(int x, int y, float scrollX, float scrollY ){}
	
	/// \brief Called on the active window when the mouse cursor enters the
	/// window area
	///
	/// Note that the mouse coordinates are the last known x/y before the
	/// event occurred, i.e. from the previous frame
	virtual void mouseEntered( int x, int y ){}
	
	/// \brief Called on the active window when the mouse cursor leaves the
	/// window area
	///
	/// Note that the mouse coordinates are the last known x/y before the
	/// event occurred, i.e. from the previous frame
	virtual void mouseExited( int x, int y){}
	
	virtual void dragEvent(ofDragInfo dragInfo) { }
	virtual void gotMessage(ofMessage msg){ }
	
	int mouseX, mouseY;			// for processing heads
	
	bool isMouseDown = false;
	
	virtual void setup(ofEventArgs & args){
		setup();
	}
	virtual void update(ofEventArgs & args){
		update();
	}
	virtual void draw(ofEventArgs & args){
		draw();
	}
	virtual void exit(ofEventArgs & args){
//		removeAllEvents();
		ofxMTApp::sharedApp->viewClosing(this);
		exit();
	}
	
	virtual void windowResized(ofResizeEventArgs & resize){
		windowResized(resize.width,resize.height);
	}
	
	virtual void keyPressed( ofKeyEventArgs & key ){
		keyPressed(key.key);
	}
	virtual void keyReleased( ofKeyEventArgs & key ){
		keyReleased(key.key);
	}
	
	virtual void mouseMoved( ofMouseEventArgs & mouse ){
		mouseX=mouse.x;
		mouseY=mouse.y;
		mouseMoved(mouse.x,mouse.y);
	}
	virtual void mouseDragged( ofMouseEventArgs & mouse ){
		mouseX=mouse.x;
		mouseY=mouse.y;
		mouseDragged(mouse.x,mouse.y,mouse.button);
	}
	virtual void mousePressed( ofMouseEventArgs & mouse ){
		mouseX=mouse.x;
		mouseY=mouse.y;
		isMouseDown = true;
		mousePressed(mouse.x,mouse.y,mouse.button);
	}
	virtual void mouseReleased(ofMouseEventArgs & mouse){
		mouseX=mouse.x;
		mouseY=mouse.y;
		isMouseDown = false;
		mouseReleased(mouse.x,mouse.y,mouse.button);
	}
	virtual void mouseScrolled( ofMouseEventArgs & mouse ){
		mouseScrolled(mouse.x, mouse.y, mouse.scrollX, mouse.scrollY);
	}
	virtual void mouseEntered( ofMouseEventArgs & mouse ){
		mouseEntered(mouse.x,mouse.y);
	}
	virtual void mouseExited( ofMouseEventArgs & mouse ){
		mouseExited(mouse.x,mouse.y);
	}
	virtual void dragged(ofDragInfo & drag){
		dragEvent(drag);
	}
	virtual void messageReceived(ofMessage & message){
		gotMessage(message);
	}
	
	virtual void touchDown(int x, int y, int id) {};
	virtual void touchMoved(int x, int y, int id) {};
	virtual void touchUp(int x, int y, int id) {};
	virtual void touchDoubleTap(int x, int y, int id) {};
	virtual void touchCancelled(int x, int y, int id) {};
	
	virtual void touchDown(ofTouchEventArgs & touch) {
		touchDown(touch.x, touch.y, touch.id);
	};
	virtual void touchMoved(ofTouchEventArgs & touch) {
		touchMoved(touch.x, touch.y, touch.id);
	};
	virtual void touchUp(ofTouchEventArgs & touch) {
		touchUp(touch.x, touch.y, touch.id);
	};
	virtual void touchDoubleTap(ofTouchEventArgs & touch) {
		touchDoubleTap(touch.x, touch.y, touch.id);
	};
	virtual void touchCancelled(ofTouchEventArgs & touch){
		touchCancelled(touch.x, touch.y, touch.id);
	}

	virtual void appModeChanged(MTAppModeChangeArgs & modeChange){}
	
	ofPtr<ofAppBaseWindow> getWindow();
	
	
protected:
	shared_ptr<ofxMTModel> model;
	shared_ptr<ofAppBaseWindow> window;
	string name;
	
	void removeAllEvents();
	void addAllEvents();
};

#endif /* ofxMTView_hpp */

