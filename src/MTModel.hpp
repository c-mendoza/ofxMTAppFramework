//
//  ofxMTModel.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/18/15.
//
//

#ifndef ofxMTModel_hpp
#define ofxMTModel_hpp

#include <Helpers.h>
#include "MTAppFrameworkUtils.hpp"

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

protected:
	ofParameterGroup parameters;

private:
    std::string name;
};

#endif /* ofxMTModel_hpp */
