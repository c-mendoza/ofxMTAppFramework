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
}

void MTModel::addChildModel(std::shared_ptr<MTModel> childModel)
{
	children.push_back(childModel);
	parameters.add(childModel->getParameters());
}
