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
#include "ofxImGui.h"

class ofXml;
class ofParameterGroup;

class MTModel : public MTEventListenerStore
{
public:
	MTModel(std::string _name);
	virtual ~MTModel(){}
    std::string getName();
	void setName(std::string n);
	ofParameterGroup& getParameters();

	virtual void serialize(ofXml& serializer);
	virtual void deserialize(ofXml& serializer);

    /**
     * @brief Override this to allow the model to draw its own gui
     * via ImGui
     */
    virtual void drawGui(ofxImGui::Settings& settings){}
//	virtual void loadFromSerializer(ofXml& serializer) = 0;
//	virtual void saveWithSerializer(ofXml& serializer) = 0;

protected:
	ofParameterGroup parameters;

private:
    std::string name;
};

#endif /* ofxMTModel_hpp */
