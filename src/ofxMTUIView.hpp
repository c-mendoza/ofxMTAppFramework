//
//  ofxMTUIView.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 9/19/16.
//
//

#ifndef ofxMTUIView_hpp
#define ofxMTUIView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "OrthoCamera.h"

class ofxMTUIView {

	ofFbo drawingSurface;
	float scale;
	ofRectangle viewport;
	ofRectangle contentFrame;
	orthoCamera camera;

public:
	ofxMTUIView(ofRectangle viewport, ofRectangle contentFrame);
	~ofxMTUIView();
	
	virtual void draw();
	virtual void setup(){}
	
	void setViewport(ofRectangle newViewport);
	ofRectangle getViewFrame();
	void setContentFrame(ofRectangle newContentFrame);
	ofRectangle getContentFrame();
	
};
#endif /* ofxMTUIView_hpp */
