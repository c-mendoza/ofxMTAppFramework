//
//  ofxMTModel.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/18/15.
//
//

#ifndef ofxMTModel_hpp
#define ofxMTModel_hpp

#include <stdio.h>
#include "ofMain.h"


class ofxMTModel {
	
public:
	ofxMTModel(string _name);
	virtual ~ofxMTModel();
	string getName();
	void setName(string n);
	ofParameterGroup& getParameters();
	
	virtual void loadFromSerializer(ofXml& serializer) = 0;
	virtual void saveWithSerializer(ofXml& serializer) = 0;
	
	/// If you need event listeners that should die when this object gets destroyed,
	/// add them with this method.
	void addEventListener(ofEventListener&& el);

protected:
	ofParameterGroup parameters;
	vector<ofEventListener> eventListeners;
private:
	string name;
};

#endif /* ofxMTModel_hpp */
