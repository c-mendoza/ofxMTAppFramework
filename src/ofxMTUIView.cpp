//
//  ofxMTUIView.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 9/19/16.
//
//

#include "ofxMTUIView.hpp"

ofxMTUIView::ofxMTUIView(ofRectangle viewport, ofRectangle contentFrame)
{
	this->contentFrame = contentFrame;
	this->viewport = viewport;
}

void ofxMTUIView::draw()
{
	camera.begin(viewport);
	
	ofSetColor(ofColor::red);
	ofFill();
	ofDrawRectangle(0, 0, 1, viewport.width / 2.0, viewport.height/2.0);
	camera.end();
}