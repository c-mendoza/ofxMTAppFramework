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

	/**
	 * @brief Serializes the ofParameterGroup of the Model. Override this method
	 * if you need to serialize data that the ofParameter system can't handle on
	 * its own.
	 * @param serializer. This is provided by the Framework.
	 */
	virtual void serialize(ofXml& serializer);

	/**
	 * @brief Deserializes the ofParameterGroup of the Model. Override this method
	 * if you need to deserialize data that the ofParameter system can't handle on
	 * its own.
	 * If you are using nested ofParameterGroups you'll need to use this method.
	 * ofDeserialize does not handle nested groups.
	 * @param serializer. This is provided by the Framework.
	 */
	virtual void deserialize(ofXml& serializer);

	void addChildModel(std::shared_ptr<MTModel> childModel);

protected:
	ofParameterGroup parameters;

private:
    std::string name;
    std::vector<std::shared_ptr<MTModel>> children;
};

#endif /* ofxMTModel_hpp */
