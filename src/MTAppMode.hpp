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

class MTView;

class MTAppMode : public MTEventListenerStore {
public:
    MTAppMode(string name, std::shared_ptr<MTView> view)
    {
        this->name = name;
        this->view = view;
//        addAllEventListeners();
    }
    ~MTAppMode();
    void addAllEventListeners();
    //    void removeAllEventListeners();
    std::string getName() { return name; }
    void setName(string name) { this->name = name; }

    virtual void setup() = 0;
    virtual void exit(){};
    virtual void update(){};
    virtual void draw(){};
    virtual void keyPressed(int key){};
    virtual void keyReleased(int key){};
    virtual void mouseMoved(int x, int y){};
    virtual void mouseDragged(int x, int y, int button){};
    virtual void mouseDraggedEnd(int x, int y, int button){};
    virtual void mousePressed(int x, int y, int button){};
    virtual void mouseReleased(int x, int y, int button){};
    virtual void mouseEntered(int x, int y){};
    virtual void mouseExited(int x, int y){};
    virtual void windowResized(int w, int h){};

    //	virtual void dragEvent(ofDragInfo dragInfo);
    //	virtual void gotMessage(ofMessage msg);
protected:
    shared_ptr<MTView> view = nullptr;
    std::string name;
};

class MTAppModeVoid : public MTAppMode {
public:
    MTAppModeVoid(std::shared_ptr<MTView> view)
        : MTAppMode("App Mode Void", view)
    {
    }
    virtual void setup(){}
    virtual void exit() {}
};

#endif /* ofxMTAppMode_hpp */
