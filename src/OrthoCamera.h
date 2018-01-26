#pragma once

//#include "ofMain.h"
#include <3d/ofCamera.h>
#include "ofAppRunner.h"

// Ortho camera is a custom
//	camera we've created in
//  this example
//
// We inherit from ofCamera

class orthoCamera : public ofCamera {
	public:
		orthoCamera();
		void begin(ofRectangle rect = ofGetWindowRect());
		float scale;
};