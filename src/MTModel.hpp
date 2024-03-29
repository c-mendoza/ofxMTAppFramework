//
//  ofxMTModel.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/18/15.
//
//

#ifndef ofxMTModel_hpp
#define ofxMTModel_hpp

#include "MTAppFrameworkUtils.hpp"
#include "ofJson.h"

class ofXml;
class ofParameterGroup;

class MTModel : public MTEventListenerStore
{
 public:
   MTModel(std::string _name);
   MTModel() : MTModel("Model")
   {
   }
   virtual ~MTModel()
   {
   }
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
   virtual void serialize(ofJson& serializer);
   /**
	 * @brief Deserializes the ofParameterGroup of the Model. Override this method
	 * if you need to deserialize data that the ofParameter system can't handle on
	 * its own.
	 * If you are using nested ofParameterGroups you'll need to use this method.
	 * ofDeserialize does not handle nested groups.
	 * @param serializer. This is provided by the Framework.
	 */
   virtual void deserialize(ofXml& serializer);
   virtual void deserialize(ofJson& json);

   void addChildModel(std::shared_ptr<MTModel> childModel);

   /**
	 * @brief Adds one or many parameters to the model.
	 */
   template<typename... Args>
   void addParameters(ofAbstractParameter& p, Args&... parameters)
   {
      this->parameters.add(p);
      this->parameters.add(parameters...);
   }

   /**
	 * @brief Adds a parameter to the model.
	 */
   void addParameters(ofAbstractParameter& param)
   {
      this->parameters.add(param);
   }

   virtual void newFile()
   {
   }

 protected:
   ofParameterGroup parameters;

 private:
   std::string name;
   std::vector<std::shared_ptr<MTModel>> children;
};

#endif /* ofxMTModel_hpp */
