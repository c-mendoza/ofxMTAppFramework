//
//  ofxMTModel.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/18/15.
//
//

#include "ofxMTModel.hpp"

ofxMTModel::ofxMTModel(string _name)
{
    name = _name;
    parameters.setName(name);
}

//ofxMTModel::~ofxMTModel()
//{
//    for (auto &el : eventListeners)
//    {
//        el.unsubscribe();
//    }

//    eventListeners.clear();
//}

string ofxMTModel::getName()
{
    return name;
}

void ofxMTModel::setName(string n)
{
    name = n;
    parameters.setName(n);
}

ofParameterGroup& ofxMTModel::getParameters()
{
    return parameters;
}

void ofxMTModel::serialize(ofXml& serializer)
{
    ofSerialize(serializer, parameters);
}

void ofxMTModel::deserialize(ofXml &serializer)
{
    ofDeserialize(serializer, parameters);
}
