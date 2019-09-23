
#include <graphics/ofPolyline.h>
#include "MTAppFrameworkUtils.hpp"
#include "ofxImGui.h"
#include <imgui_internal.h>

///////////////////////////////////////////
/// MTProcedure
///////////////////////////////////////////

bool MTAppFramework::fromBinding = false;

bool MTAppFramework::ofPathImGuiEditor(const char* id, ofPath& originalPath, ofPath& resultPath, ImVec2& widgetSize,
									   ImVec2& realSize, float handleRadius)
{
	bool didChange = false;
	using namespace ImGui;
	const ImGuiStyle& Style = GetStyle();
	const ImGuiIO& IO = GetIO();
	ImDrawList* DrawList = GetWindowDrawList();
	ImGuiWindow* Window = GetCurrentWindow();
	if (Window->SkipItems)
		return false;

	// header and spacing
	int hovered = IsItemActive() || IsItemHovered(); // IsItemDragged() ?

	ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + widgetSize);
	ImVec2 mouse = GetIO().MousePos;
	auto factor = realSize / widgetSize;
	auto invFactor = 1.0f / factor;
	DrawList->AddRect(bb.Min, bb.Max, IM_COL32(0, 0, 0, 255));
	InvisibleButton(id, widgetSize);
	static int selectedIndex = -1;
	static ImVec2 startPos(0, 0);
	float distance = 0;
	int i = 0;
//	ImGui::SetStateStorage();

	if (!IsMouseDown(0))
	{
		selectedIndex = -1;
	}

	std::vector<ImVec2> scaledPoints;
	scaledPoints.reserve(originalPath.getCommands().size());
	for (int i = 0; i < originalPath.getCommands().size(); i++)
	{
		auto& command = originalPath.getCommands()[i];
		if (command.type == ofPath::Command::Type::close)
		{
			resultPath.close();
			break;
		}

		auto scaled = (glm::vec2(command.to) * invFactor) + bb.Min;
		scaledPoints.push_back(scaled);
		glm::vec2 nextPoint;
		if (selectedIndex < 0)
		{
			auto dist = glm::distance(glm::vec2(mouse.x, mouse.y), scaled);
			if (dist <= handleRadius)
			{
				SetTooltip("%i", i);
				selectedIndex = i;
				startPos = mouse;
			}
			nextPoint = command.to;
		}
		else if (selectedIndex == i)
		{
			if (IsMouseDragging(0))
			{
				auto delta = glm::vec2(GetMouseDragDelta(0));
				auto newPos = startPos + GetMouseDragDelta(0);
				nextPoint = (newPos - bb.Min) * factor;
				nextPoint = glm::clamp(nextPoint,
									   glm::vec2(0, 0),
									   glm::vec2(realSize.x, realSize.y));
				didChange = true;
			}
		}
		else
		{
			nextPoint = command.to;
		}

		if (command.type == ofPath::Command::moveTo)
		{
			resultPath.moveTo(nextPoint);
		}
		else
		{
			resultPath.lineTo(nextPoint);
		}
	}

	DrawList->AddPolyline(scaledPoints.data(), scaledPoints.size(), IM_COL32(255, 255, 255, 255),
						  true, 2);
	for (auto& p : scaledPoints)
	{
		DrawList->AddCircleFilled(p, handleRadius, IM_COL32(40, 40, 40, 255));
		DrawList->AddCircle(p, handleRadius, IM_COL32(255, 255, 255, 255), 12, 2);
	}

	return didChange;
}


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