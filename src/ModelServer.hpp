//
// Created by Cristobal Mendoza on 2/18/20.
//

#ifndef NERVOUSSTRUCTUREOF_MODELREMOTECONTROLLER_HPP
#define NERVOUSSTRUCTUREOF_MODELREMOTECONTROLLER_HPP

#include <utils/ofXml.h>
#include "ofxOsc.h"
#include "ofPath.h"
#include "ofThread.h"
#include <unordered_map>
#include <typeindex>

class ApiMethod;

class ModelServer : public ofThread
{
public:
	ModelServer();
	~ModelServer();
	void setup(std::string ipAddress, int inPort = 12000, int outPort = 12001);
	void threadedFunction();
	void sendMessage(ofxOscMessage& m);
	void sendReply(ApiMethod& method, ofxOscMessage& m);
	void addApiMethod(ApiMethod&& method);

	template<typename ParameterType>
	void addType(std::string friendlyName)
	{
		auto hash = typeid(ParameterType).hash_code();
		auto result = typeRegistry.insert({std::type_index(typeid(ParameterType)), friendlyName});
		if (!result.second) ofLogWarning("ModelServer") << "Tried to add an existing type";
	}

private:
	ofXml createMetaModel();
	bool sendMetaModel();
	void parseMessage(ofxOscMessage& m);
	void serializeParameter(ofXml& xml, std::shared_ptr<ofAbstractParameter> parameter);
	int inPort;
	int outPort;
	std::string ipAddress;
	ofxOscSender oscSender;
	ofxOscReceiver oscReceiver;

//	enum DataType
//	{
//		unknown,
//		integer,
//		floating,
//		boolean,
//		color,
//		string,
//		group,
//		vec2
//	};
	/// {type hash code, type friendly name}
	std::unordered_map<std::type_index, std::string> typeRegistry = {};

//	std::map<DataType, std::string> dataTypeStrings = {
//			{DataType::unknown,  "unknown"},
//			{DataType::integer,  "int"},
//			{DataType::floating, "float"},
//			{DataType::color,    "floatColor"},
//			{DataType::group,    "group"},
//			{DataType::string,   "string"},
//			{DataType::boolean,  "boolean"},
//			{DataType::vec2,     "vec2"}
//	};

	std::unordered_map<std::string, ApiMethod> ApiMethods;

	void serializeParameterGroup(const ofParameterGroup& params, ofXml& xml);

	template<typename T>
	bool isParameterType(std::shared_ptr<ofAbstractParameter> parameter)
	{
		auto castParameter = std::dynamic_pointer_cast<ofParameter<T>>(parameter);
		if (castParameter)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	const std::string ApiRoot = "mtafMethod";
	const std::string ApiResponse = "mtafResponse";

};

class ApiMethod
{
public:
	ApiMethod(std::string ident,
			  std::function<void(ApiMethod&, ofxOscMessage&, ModelServer&)> action)
	{
		identifier = ident;
		this->action = action;
	}

	void execute(ofxOscMessage& message, ModelServer& server)
	{
		action(*this, message, server);
	}

protected:
	std::string identifier;
public:
	const std::string& getIdentifier() const
	{
		return identifier;
	}

protected:
	std::function<void(ApiMethod&, ofxOscMessage&, ModelServer&)> action;
	ofxOscMessage payload;
};

//std::ostream& operator<<(std::ostream& os, const ofPath& path);
//std::istream& operator>>(std::istream& is, ofPath& path);

#endif //NERVOUSSTRUCTUREOF_MODELREMOTECONTROLLER_HPP
