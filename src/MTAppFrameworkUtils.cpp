
#include <graphics/ofPolyline.h>
#include "MTAppFrameworkUtils.hpp"
#include "ofxImGui.h"

///////////////////////////////////////////
/// MTProcedure
///////////////////////////////////////////

bool MTAppFramework::fromBinding = false;

MTProcedureStep MTProcedure::getCurrentStep()
{
	return current;
}

void MTProcedure::update()
{
	if (!procedureComplete)
	{
		if (current.successTest())
		{
			current.successAction();
			stepQueue.pop();
			//TODO: Send a "stepComplete" event
			if (stepQueue.size() == 0)
			{
				procedureComplete = true;
				procedureCompleteAction();
				//TODO: Send a "procedureComplete" event?
			}
			else
			{
				current = stepQueue.front();
			}
		}
	}
}

void MTProcedure::addStep(MTProcedureStep step)
{
	stepQueue.push(step);
	step.index = stepQueue.size();
	totalSteps = stepQueue.size();
	current = stepQueue.front();
}

void ofxImGui::AddParameter(std::shared_ptr<ofAbstractParameter> parameter)
{
//	auto parameter = param.newReference();
	// Parameter, try everything we know how to handle.
#if OF_VERSION_MINOR >= 10
	auto parameterVec2f = std::dynamic_pointer_cast<ofParameter<glm::vec2>>(parameter);
	if (parameterVec2f)
	{
		ofxImGui::AddParameter(*parameterVec2f);
		return;
	}
	auto parameterVec3f = std::dynamic_pointer_cast<ofParameter<glm::vec3>>(parameter);
	if (parameterVec3f)
	{
		ofxImGui::AddParameter(*parameterVec3f);
		return;
	}
	auto parameterVec4f = std::dynamic_pointer_cast<ofParameter<glm::vec4>>(parameter);
	if (parameterVec4f)
	{
		ofxImGui::AddParameter(*parameterVec4f);
		return;
	}
#endif
	auto parameterOfVec2f = std::dynamic_pointer_cast<ofParameter<ofVec2f>>(parameter);
	if (parameterOfVec2f)
	{
		ofxImGui::AddParameter(*parameterOfVec2f);
		return;
	}
	auto parameterOfVec3f = std::dynamic_pointer_cast<ofParameter<ofVec3f>>(parameter);
	if (parameterOfVec3f)
	{
		ofxImGui::AddParameter(*parameterOfVec3f);
		return;
	}
	auto parameterOfVec4f = std::dynamic_pointer_cast<ofParameter<ofVec4f>>(parameter);
	if (parameterOfVec4f)
	{
		ofxImGui::AddParameter(*parameterOfVec4f);
		return;
	}
	auto parameterFloatColor = std::dynamic_pointer_cast<ofParameter<ofFloatColor>>(parameter);
	if (parameterFloatColor)
	{
		ofxImGui::AddParameter(*parameterFloatColor);
		return;
	}
	auto parameterFloat = std::dynamic_pointer_cast<ofParameter<float>>(parameter);
	if (parameterFloat)
	{
		ofxImGui::AddParameter(*parameterFloat);
		return;
	}
	auto parameterInt = std::dynamic_pointer_cast<ofParameter<int>>(parameter);
	if (parameterInt)
	{
		ofxImGui::AddParameter(*parameterInt);
		return;
	}
	auto parameterBool = std::dynamic_pointer_cast<ofParameter<bool>>(parameter);
	if (parameterBool)
	{
		ofxImGui::AddParameter(*parameterBool);
		return;
	}

//	ofLogWarning(__FUNCTION__) << "Could not create GUI element for parameter " << parameter->getName();

}