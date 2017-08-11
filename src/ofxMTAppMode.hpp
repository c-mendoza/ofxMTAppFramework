//
//  ofxMTAppMode.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 6/7/16.
//
//

#ifndef ofxMTAppMode_hpp
#define ofxMTAppMode_hpp

#include "ofxMTAppFramework.h"

class ofxMTView;

class ofxMTAppMode : public MTEventListenerStore
{
public:
    ofxMTAppMode(string name) { this->name = name; }
	~ofxMTAppMode(){}
    std::string getName() { return name; }
    void setName(string name) { this->name = name; }
    virtual void setup(ofxMTView* view) = 0;
    virtual void exit() = 0;
    virtual void update(){};
    virtual void draw(){};
    virtual void keyPressed(int key){};
    virtual void keyReleased(int key){};
    virtual void mouseMoved(int x, int y ){};
    virtual void mouseDragged(int x, int y, int button){};
    virtual void mousePressed(int x, int y, int button){};
    virtual void mouseReleased(int x, int y, int button){};
    virtual void mouseEntered(int x, int y){};
    virtual void mouseExited(int x, int y){};
    virtual void windowResized(int w, int h){};

    //	virtual void dragEvent(ofDragInfo dragInfo);
    //	virtual void gotMessage(ofMessage msg);
protected:
    ofxMTView* view;
    std::string name;
};

class MTAppModeVoid : public ofxMTAppMode
{
public:
    MTAppModeVoid() : ofxMTAppMode("App Mode Void"){}
    virtual void setup(ofxMTView* view) {}
    virtual void exit() {}
};

#endif /* ofxMTAppMode_hpp */
