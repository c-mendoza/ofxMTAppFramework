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
	ofxMTModel(string _name){
		name = _name;
		parameters.setName(name);
	};
	virtual ~ofxMTModel(){};
	
	string getName() { return name; };
	void setName(string n) {
		name = n;
		parameters.setName(n);
	}
	
	ofParameterGroup& getParameters() { return parameters; };
	
	virtual void loadFromSerializer(ofXml& serializer) = 0;
	virtual void saveWithSerializer(ofXml& serializer) = 0;
	
//	virtual void setup(){};
	
protected:
	ofParameterGroup parameters;
private:
	string name;
};

#endif /* ofxMTModel_hpp */
