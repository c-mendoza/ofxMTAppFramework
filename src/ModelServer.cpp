//
// Created by Cristobal Mendoza on 2/18/20.
//

#include "ModelServer.hpp"

#include "MTApp.hpp"
#include "MTModel.hpp"

//const std::string ModelRemoteController::ApiRoot = "mtafMethod";

ModelServer::ModelServer() : ofThread()
{
	ipAddress = "";

	addApiMethod(ApiMethod("getModel",
						   [this](ApiMethod& method, ofxOscMessage& m, ModelServer& server)
						   {
							   ofxOscMessage outMessage;
							   auto xml = createMetaModel();
							   outMessage.addStringArg(xml.toString());
							   outMessage.setRemoteEndpoint(m.getRemoteHost(), outPort);
							   server.sendReply(method, outMessage);
						   }));

	addApiMethod(ApiMethod("connect",
						   [this](ApiMethod& method, ofxOscMessage& m, ModelServer& server)
						   {
							   oscSender.setup(m.getArgAsString(0), outPort);
						   }));

	addType<ofParameter<int>>("int");
	addType<ofParameter<float>>("float");
	addType<ofParameter<ofFloatColor>>("floatColor");
	addType<ofParameterGroup>("group");
	addType<ofParameter<std::string>>("string");
	addType<ofParameter<bool>>("boolean");
	addType<ofParameter<glm::vec2>>("vec2");
	addType<ofParameter<glm::vec3>>("vec3");
	addType<ofParameter<glm::vec4>>("vec4");
	addType<ofParameter<ofPath>>("ofPath");
}

ModelServer::~ModelServer()
{
	waitForThread();
	oscReceiver.stop();
	oscSender.clear();
}

void ModelServer::setup(std::string remoteHost, int inPort, int outPort)
{
	auto listener = MTApp::sharedApp->modelLoadedEvent.newListener([this](ofEventArgs& args)
																   {
																	   auto xml = createMetaModel();
																	   xml.save("metamodel.xml");
																   });
	MTApp::sharedApp->addEventListener(std::move(listener));
	oscReceiver.setup(inPort);
	this->inPort = inPort;
	this->outPort = outPort;
//	oscSender.setup(remoteHost, outPort);
	startThread();
}

ofXml ModelServer::createMetaModel()
{
	ofXml xml;
	auto model = MTApp::Model<MTModel>();
	auto params = model->getParameters();
	serializeParameterGroup(params, xml);

	return xml;
}

void ModelServer::serializeParameterGroup(const ofParameterGroup& params, ofXml& xml)
{
	auto groupXml = xml.appendChild(params.getEscapedName());
	groupXml.appendAttribute("type").set("group"); // TODO: type string constants?
	groupXml.appendAttribute("name").set(params.getName());
//	auto childXml = groupXml.appendChild("children");
	for (auto& param : params)
	{
		serializeParameter(groupXml, param);
	}
}

void ModelServer::serializeParameter(ofXml& xml, std::shared_ptr<ofAbstractParameter> parameter)
{
	std::string type;
	float min = 0;
	float max = 0;

	try
	{
//		auto code = typeid(*(parameter.get())).hash_code();
//		auto hey = typeid(*(parameter.get())).name();
//		auto yuk = (*parameter).valueType();
//		if (hey == NULL)
//			return;
//		auto value = hey->valueType();
//		ofLogError() << hey << " " << yuk <<  " " << typeid(*parameter).name();
//		ofLogError() << "TYPE: " << typeid(*bla).name();
		type = typeRegistry.at(std::type_index(typeid(*parameter)));
	}
	catch (std::out_of_range e)
	{
		ofLogError("ModelServer") << "Tried adding parameter of unknown type: "
								  << typeid(*parameter).name()
								  << " Register the type with addType before adding such a parameter.";
		return;
	}

//	ofLogError() << "Type: " << parameter->type() << " " << parameter->valueType() << " " << typeid(*parameter).hash_code();

	auto parameterGroup = std::dynamic_pointer_cast<ofParameterGroup>(parameter);
	if (parameterGroup)
	{
		serializeParameterGroup(*parameterGroup.get(), xml);
		return;
	}

	ofXml paramXml = xml.appendChild(parameter->getEscapedName());

	if (isParameterType<float>(parameter))
	{
		auto castParameter = std::dynamic_pointer_cast<ofParameter<float>>(parameter);
		min = castParameter->getMin();
		max = castParameter->getMax();
	}
	else if (isParameterType<int>(parameter))
	{
		auto parameterInt = std::dynamic_pointer_cast<ofParameter<int>>(parameter);
		min = parameterInt->getMin();
		max = parameterInt->getMax();
	}
//	else if (isParameterType<glm::vec2>(parameter))
//	{
//		type = stringForType(DataType::vec2);
//		//TODO: Min and max for vec
//	}

	paramXml.appendAttribute("type").set(type);
	paramXml.appendAttribute("name").set(parameter->getName());
	paramXml.appendChild("value").set(parameter->toString());
	if (min != max)
	{
		paramXml.appendChild("min").set(min);
		paramXml.appendChild("max").set(max);
	}

	glm::vec2 bla;
}

template<typename T>
bool isParameterType(ofAbstractParameter& parameter)
{
	auto castParameter = std::dynamic_pointer_cast<ofParameter<T>>
			(parameter);
	if (castParameter)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ModelServer::sendMetaModel()
{
	return false;
}

void ModelServer::parseMessage(ofxOscMessage& m)
{
	cout << "address = " << m.getAddress() << "\n";
	cout << "arg = " << m.getArgAsString(0) << "\n";
	auto components = ofSplitString(m.getAddress(),
									ofToString((char) std::filesystem::path::preferred_separator),
									false,
									true);
	for (int j = 0; j < components.size(); ++j)
	{
		cout << j << " " << components[j] << "\n";
	}

	if (components.size() < 3) return;

	if (components[1] == ApiRoot)
	{
		auto result = ApiMethods.find(components[2]);
		if (result != ApiMethods.end())
		{
			result->second.execute(m, *this);
		}
	}
}

void ModelServer::threadedFunction()
{
	while (isThreadRunning())
	{
		while (oscReceiver.hasWaitingMessages())
		{
			ofxOscMessage inMsg;
			oscReceiver.getNextMessage(inMsg);
			parseMessage(inMsg);
		}
		sleep(500);
//		yield();
	}
}

void ModelServer::sendMessage(ofxOscMessage& m)
{
	oscSender.sendMessage(m, false);
}

void ModelServer::sendReply(ApiMethod& method, ofxOscMessage& m)
{
	m.setAddress("/"+ApiResponse+"/"+method.getIdentifier());
	oscSender.sendMessage(m, false);
}

void ModelServer::addApiMethod(ApiMethod&& method)
{
	ApiMethods.insert({method.getIdentifier(), method});
}


//std::ostream& operator<<(std::ostream& os, const ofPath& path)
//{
//	os << MTApp::pathToString(path);
//	return os;
//}
//
//std::istream& operator>>(std::istream& is, ofPath& path)
//{
//	std::string inString;
//	std::getline(is, inString);
//	path = MTApp::pathFromString(inString);
//	return is;
//}
