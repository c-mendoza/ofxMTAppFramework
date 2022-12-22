
#include <graphics/ofPolyline.h>
#include "MTAppFrameworkUtils.hpp"
#include "ofxImGui.h"
#include <imgui_internal.h>
#include <utils/ofXml.h>
#include "MTApp.hpp"

///////////////////////////////////////////
/// MTProcedure
///////////////////////////////////////////

bool MTAppFramework::fromBinding = false;

bool MTAppFramework::ofPathImGuiEditor(
    const char* id, const ofPath& originalPath, ofPath& resultPath, ImVec2& widgetSize, ImVec2& realSize, float handleRadius)
{
   bool didChange = false;
   static std::string selectedId = "";
   using namespace ImGui;
   const ImGuiStyle& Style = GetStyle();
   const ImGuiIO& IO = GetIO();
   ImDrawList* DrawList = GetWindowDrawList();
   ImGuiWindow* Window = GetCurrentWindow();
   if (Window->SkipItems) return false;

   // header and spacing

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

   int hovered = IsItemActive() || IsItemHovered();
   if (hovered)
   {
      selectedId = std::string(id);
   }

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
         break;
      }
      auto scaled = (glm::vec2(command.to) * invFactor) + bb.Min;
      scaledPoints.push_back(scaled);
   }

   if (strcmp(selectedId.c_str(), id) == 0)
   {
      for (int i = 0; i < originalPath.getCommands().size(); i++)
      {
         auto& command = originalPath.getCommands()[i];
         if (command.type == ofPath::Command::Type::close)
         {
            resultPath.close();
            break;
         }

         glm::vec2 nextPoint;
         if (selectedIndex < 0)
         {
            auto dist = glm::distance(glm::vec2(mouse.x, mouse.y), (glm::vec2) scaledPoints[i]);
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
               nextPoint = glm::clamp(nextPoint, glm::vec2(0, 0), glm::vec2(realSize.x, realSize.y));
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
   }

   for (int i = 0; i < resultPath.getCommands().size(); i++)
   {
      auto& command = resultPath.getCommands()[i];
      auto clamped = glm::clamp(command.to, glm::vec3(0, 0, 0), glm::vec3(realSize.x, realSize.y, 0));
      if (clamped != command.to)
      {
         command.to = clamped;
         didChange = true;
      }
   }

   DrawList->AddPolyline(scaledPoints.data(), scaledPoints.size(), IM_COL32(255, 255, 255, 255), true, 2);
   for (auto& p : scaledPoints)
   {
      DrawList->AddCircleFilled(p, handleRadius, IM_COL32(40, 40, 40, 255));
      DrawList->AddCircle(p, handleRadius, IM_COL32(255, 255, 255, 255), 12, 2);
   }

   return didChange;
}

//inline std::ostream& operator<<(std::ostream& os, const ofPath& path)
//{
//	os << MTAppFramework::PathToString2(path);
//	return os;
//}
//
//inline std::istream& operator>>(std::istream& is, ofPath& path)
//{
//	std::string line;
//	std::string result;
//	while (std::getline(is, line))
//	{
//		if (!line.empty())
//			result += line;
//	}
//
//	if (result[0] == '{') {
//		path = MTApp::pathFromString(result);
//	} else {
//		path = MTAppFramework::PathFromString2(result);
//	}
//	return is;
//}

void MTAppFramework::RemoveAllParameters(ofParameterGroup& group)
{
   while (group.size() > 0) group.remove(group.size() - 1);
}

std::string MTAppFramework::PathToString(ofPath& path)
{
   std::vector<ofPath::Command> commands = path.getCommands();

   std::string out = "";

   for (auto c : commands)
   {
      out += "{ " + ofToString(c.type) + "; " + ofToString(c.to) + "; " + ofToString(c.cp1) + "; " + ofToString(c.cp2) + "; } ";
   }

   return out;
}

std::string MTAppFramework::PathToString2(const ofPath& path)
{
   ofXml root;
   auto xml = root.appendChild("ofPath");
   auto points = xml.appendChild("points");

   std::vector<ofPath::Command> commands = path.getCommands();

   for (auto c : commands)
   {
      auto point = points.appendChild("point");
      point.appendAttribute("type").set(ofToString(c.type));
      point.appendAttribute("position").set(ofToString(c.to));
      point.appendAttribute("cp1").set(ofToString(c.cp1));
      point.appendAttribute("cp2").set(ofToString(c.cp2));
   }

   auto fill = xml.appendChild("fill");
   fill.appendAttribute("color").set(ofToString(path.getFillColor()));
   fill.appendAttribute("isFilled").set(ofToString(path.isFilled()));

   auto stroke = xml.appendChild("stroke");
   stroke.appendAttribute("color").set(ofToString(path.getStrokeColor()));
   stroke.appendAttribute("strokeWidth").set(ofToString(path.getStrokeWidth()));

   return root.toString();
}

ofPath MTAppFramework::PathFromString2(std::string s)
{
   ofXml rootXml;
   ofPath path;
   if (!rootXml.parse(s))
   {
      ofLogError("PathFromString2") << "Failed to parse the following string as xml:\n" << s;
      return path;
   }
   ofXml xml = rootXml.getFirstChild();
   auto iter = xml.getChild("points").getChildren();
   for (auto child : iter)
   {
      int commandType = child.getAttribute("type").getIntValue();
      ofPoint p, cp1, cp2;
      switch (commandType)
      {
      case ofPath::Command::moveTo:
         p = ofFromString<ofPoint>(child.getAttribute("position").getValue());
         path.moveTo(p);
         break;
      case ofPath::Command::lineTo:
         p = ofFromString<ofPoint>(child.getAttribute("position").getValue());
         path.lineTo(p);
         break;
      case ofPath::Command::curveTo:
         p = ofFromString<ofPoint>(child.getAttribute("position").getValue());
         path.curveTo(p);
         break;
      case ofPath::Command::bezierTo:
         p = ofFromString<ofPoint>(child.getAttribute("position").getValue());
         cp1 = ofFromString<ofPoint>(child.getAttribute("cp1").getValue());
         cp2 = ofFromString<ofPoint>(child.getAttribute("cp2").getValue());
         path.bezierTo(cp1, cp2, p);
         break;
      case ofPath::Command::quadBezierTo:
         p = ofFromString<ofPoint>(child.getAttribute("position").getValue());
         cp1 = ofFromString<ofPoint>(child.getAttribute("cp1").getValue());
         cp2 = ofFromString<ofPoint>(child.getAttribute("cp2").getValue());
         path.quadBezierTo(cp1, cp2, p);
         break;
      case ofPath::Command::close: path.close(); break;
      default:
         ofLog(OF_LOG_WARNING,
               "MTApp::pathFromString: A Path Command "
               "supplied is not implemented");
         break;
      }
   }

   auto fill = xml.getChild("fill");
   path.setFillColor(ofFromString<ofColor>(fill.getAttribute("color").getValue()));
   path.setFilled(fill.getAttribute("isFilled").getBoolValue());

   auto stroke = xml.getChild("stroke");
   path.setStrokeWidth(stroke.getAttribute("strokeWidth").getDoubleValue());
   path.setStrokeColor(ofFromString<ofColor>(stroke.getAttribute("color").getValue()));
   return path;
}

ofPath MTAppFramework::PathFromString(std::string s)
{
   std::vector<std::string> commandStrings = ofSplitString(s, "{", true, true);
   ofPath thePath;
   thePath.setFilled(false);
   thePath.setStrokeColor(ofColor::chocolate);
   thePath.setStrokeWidth(2);
   for (auto cs : commandStrings)
   {
      std::vector<std::string> commandStringElements = ofSplitString(cs, ";", true, true);

      int commandType = ofToInt(commandStringElements[0]);
      ofPoint p, cp1, cp2;
      switch (commandType)
      {
      case ofPath::Command::moveTo:
         p = ofFromString<ofPoint>(commandStringElements[1]);
         thePath.moveTo(p);
         break;
      case ofPath::Command::lineTo:
         p = ofFromString<ofPoint>(commandStringElements[1]);
         thePath.lineTo(p);
         break;
      case ofPath::Command::curveTo: p = ofFromString<ofPoint>(commandStringElements[1]); thePath.curveTo(p);
      case ofPath::Command::bezierTo:
         p = ofFromString<ofPoint>(commandStringElements[1]);
         cp1 = ofFromString<ofPoint>(commandStringElements[2]);
         cp2 = ofFromString<ofPoint>(commandStringElements[3]);
         thePath.bezierTo(cp1, cp2, p);
         break;
      case ofPath::Command::close: thePath.close(); break;
      default:
         ofLog(OF_LOG_WARNING,
               "MTApp::pathFromString: A Path Command "
               "supplied is not implemented");
         break;
      }
   }

   return thePath;
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
