//
//  ofxMTAppMode.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 6/7/16.
//
//

#ifndef ofxMTAppMode_hpp
#define ofxMTAppMode_hpp

#include <stdio.h>
#include <string>

class ofxMTView;
class ofxMTModel;


class ofxMTAppMode  {
	ofxMTModel* model;
	ofxMTView* view;
	std::string name;
	
public:
//	ofxMTAppMode(){};
	
	void setModel(ofxMTModel* model)
	{
		this->model = model;
	};
	
	void setView(ofxMTView* view)
	{
		this->view = view;
	};
	
	void setName(std::string n) { name = n; };
	std::string getName() { return name; }
	
	virtual void activate(){};
	virtual void deactivate(){};
	virtual void setup(){};
	virtual void update(){};
	virtual void draw(){};
	virtual void exit(){};
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
	
};

#endif /* ofxMTAppMode_hpp */
