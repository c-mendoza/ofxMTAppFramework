//
//  MTModel.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/18/15.
//
//

#include "MTModel.hpp"
#include "ofXml.h"

MTModel::MTModel(std::string _name)
{
    name = _name;
    parameters.setName(name);
}

//MTModel::~MTModel()
//{
//    for (auto &el : eventListeners)
//    {
//        el.unsubscribe();
//    }

//    eventListeners.clear();
//}

std::string MTModel::getName()
{
    return name;
}

void MTModel::setName(std::string n)
{
    name = n;
    parameters.setName(n);
}

ofParameterGroup& MTModel::getParameters()
{
    return parameters;
}

void MTModel::serialize(ofXml& serializer)
{
    ofSerialize(serializer, parameters);
}

void MTModel::deserialize(ofXml &serializer)
{
    ofDeserialize(serializer, parameters);
	
//	for (auto &parameter : parameters)
//	{
//        std::string escapedName = parameter->getEscapedName();
//		auto child = serializer.getChild(escapedName);
//		if(parameter->type() == typeid(ofParameter <int> ).name()){
//			parameter->cast <int>() = child.getIntValue();
//		}else if(parameter->type() == typeid(ofParameter <float> ).name()){
//			parameter->cast <float>() = child.getFloatValue();
//		}else if(parameter->type() == typeid(ofParameter <bool> ).name()){
//			parameter->cast <bool>() = child.getBoolValue();
//		}else if(parameter->type() == typeid(ofParameter <std::string> ).name()){
//			parameter->cast <std::string>() = child.getValue();
//		} else if(parameter->type() == typeid(ofParameterGroup).name()) {
//			//Ignore groups
//
//		} else {
//			parameter->fromString(child.getValue());
//		}
//
//	}


}

void MTModel::addChildModel(std::shared_ptr<MTModel> childModel)
{
	children.push_back(childModel);
	parameters.add(childModel->getParameters());
}
