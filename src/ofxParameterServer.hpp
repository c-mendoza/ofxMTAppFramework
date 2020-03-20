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

class ServerMethod;

/**
 * @brief Types are deserialized by OF with ofFromString, which calls the << operator
 */
class ofxParameterServer : public ofThread
{
public:
	ofxParameterServer();
	~ofxParameterServer();
	void setup(ofParameterGroup& parameters,
			   std::string ipAddress,
			   int inPort = 12000,
			   int outPort = 12001);
	void threadedFunction();
	void sendMessage(ofxOscMessage& m);
	void sendReply(ServerMethod& method, ofxOscMessage& m);
	void addServerMethod(ServerMethod&& method);
	void setParameter(std::string path, std::string value);
	void addCustomDeserializer(std::string path,
							   std::function<void(std::string serializedString)> customDeserializer);

	template<typename Type>
	void addType(std::string friendlyName)
	{
//		auto hash = typeid(Type).hash_code();
		auto result = typeRegistry.insert({std::type_index(typeid(Type)), friendlyName});
		if (!result.second) ofLogWarning("ModelServer") << "Tried to add an existing type";
	}

	template<typename ParameterType>
	void addParameterType(std::string friendlyName)
	{
		auto hash = typeid(ParameterType).hash_code();
		auto result = typeRegistry.insert({std::type_index(typeid(ofParameter<ParameterType>)),
										   friendlyName});
		if (!result.second) ofLogWarning("ModelServer") << "Tried to add an existing type";
	}

	void update();

	struct ParameterData {
		std::string parameterPath;
		std::string parameterValue;
	};

private:
	ofXml createMetaModel();
	bool sendMetaModel();
	void parseMessage(ofxOscMessage& m);
	void serializeParameterGroup(std::shared_ptr<ofParameterGroup> params, ofXml& xml);
	void serializeParameter(std::shared_ptr<ofAbstractParameter> parameter, ofXml& xml);
	void serializeMethods(ofXml& xml);
	int inPort;
	int outPort;
	std::string ipAddress;
	ofxOscSender oscSender;
	ofxOscReceiver oscReceiver;
	std::shared_ptr<ofParameterGroup> group;

	/// {type hash code, type friendly name}
	std::unordered_map<std::type_index, std::string> typeRegistry = {};

	std::unordered_map<std::string, std::function<void(std::string)>> customDeserializers = {};

	std::unordered_map<std::string, ServerMethod> serverMethods;

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

	std::pair<bool, std::shared_ptr<ofAbstractParameter>>
	findParamWithEscapedName(ofParameterGroup& groupToSearch, std::string escapedName);

	const std::string ApiRoot = "mtafMethod";
	const std::string ApiResponse = "mtafResponse";

	bool useCustomDeserializer(const std::string& path, const std::string& value) const;

	ofThreadChannel<ParameterData> parameterThreadChannel;
	std::mutex serverMutex;
};

class ServerMethod
{
public:
	ServerMethod(std::string ident,
				 std::string uiName,
				 std::function<void(ServerMethod&, ofxOscMessage&, ofxParameterServer&)> action)
	{
		identifier = ident;
		this->action = action;
		this->uiName = uiName;
	}

	void execute(ofxOscMessage& message, ofxParameterServer& server)
	{
		action(*this, message, server);
	}

protected:
	/**
	 * @brief The name of the method, used for calling its action.
	 * This name needs to be legal for use as an OSC path.
	 */
	std::string identifier;

	/**
	 * @brief A friendlier name that is used in the mobile UI. For the moment this name
	 * is used by buttons. The convention is to start with a capital letter and
	 * use regular spaces for multiple words:
	 * yes:  Create mask
	 * no:   createMask
	 */
	std::string uiName;

	/**
	* @brief The action that the method performs once called. The function parameters
	* are:
	* ServerMethod&: A reference to this ServerMethod.
	* ofxOscMessage&: A reference to the OSC Message that called this method.
	* ModelServer&: A reference to the ofxParameterServer that received the message and
	* dispatched the method.
	**/
	 std::function<void(ServerMethod&, ofxOscMessage&, ofxParameterServer&)> action;

	ofxOscMessage oscMessage;

public:
	const std::string& getIdentifier() const
	{
		return identifier;
	}

	const std::string& getUiName() const
	{
		return uiName;
	}

protected:

};

//std::ostream& operator<<(std::ostream& os, const ofPath& path);
//std::istream& operator>>(std::istream& is, ofPath& path);

#endif //NERVOUSSTRUCTUREOF_MODELREMOTECONTROLLER_HPP
