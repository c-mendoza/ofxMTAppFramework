//
//  MTModel.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/18/15.
//
//

#include "MTModel.hpp"

MTModel::MTModel(string _name)
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

string MTModel::getName()
{
    return name;
}

void MTModel::setName(string n)
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
