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
#include "MTAppFrameworkUtils.hpp"

class ofXml;
class ofParameterGroup;

class MTModel : public MTEventListenerStore
{
public:
	MTModel(string _name);
	virtual ~MTModel(){}
	string getName();
	void setName(string n);
	ofParameterGroup& getParameters();

	virtual void serialize(ofXml& serializer);
	virtual void deserialize(ofXml& serializer);
//	virtual void loadFromSerializer(ofXml& serializer) = 0;
//	virtual void saveWithSerializer(ofXml& serializer) = 0;

protected:
	ofParameterGroup parameters;

private:
	string name;
};

#endif /* ofxMTModel_hpp */
