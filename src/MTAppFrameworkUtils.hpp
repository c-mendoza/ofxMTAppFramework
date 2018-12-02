#pragma once
//#include "ofMain.h"
//#include "ofxMTAppFramework.h"
#include <cstdint>
#include <functional>
#include <string>
#include <queue>
#include "ofConstants.h"
#include <events/ofEvent.h>
#include <types/ofRectangle.h>
#include <glm/mat4x4.hpp>
#include <ofParameter.h>
#include <graphics/ofPath.h>
//------------------------------------------------------//
// MT-PROCEDURE     									//
//------------------------------------------------------//

/// A single step in the overall procedure.
/// Still TODO: Fire events when appropriate.
class MTProcedureStep
{
public:
	int index;
	std::string name = "Step";
	std::string information = "Descriptive text goes here";
	///If you test for success, and it is successful, you get sweet success action
	std::function<bool()> successTest = []
	{ return false; };
	///When tests mean success, Action is your reward.
	std::function<void()> successAction = []
	{};
	bool complete = false;
};

/**
* MTProcedure is a queue of steps that are executed sequentially upon a
* success condition being met for each step. Useful for implementing actions
* that are sequential in nature and that require stepwise completion.
* Still TODO: Fire events when appropriate.
*/

class MTProcedure
{
public:

	MTProcedureStep getCurrentStep();
	void update();
	void addStep(MTProcedureStep step);

	bool isProcedureComplete()
	{ return procedureComplete; }

	std::function<void()> procedureCompleteAction = []
	{};

private:
	std::queue<MTProcedureStep> stepQueue;
	int totalSteps = -1;
	MTProcedureStep current;
	bool procedureComplete = false;
};

//template <class T>
//class MTStorageController
//{

//    void insertAtIndex(T&& item, int index);
//    void push_back(T&& item);
//    T& removeItem(T& item);
//    void removeAllItems();

//    void addToSelection(T& item);
//    void addToSelection(std::vector<T>& items);
//    bool removeFromSelection(T& item);
//    void clearSelection();

//private:
//    std::vector<T> dataStore;
//    std::vector<T> selection;
//};

//------------------------------------------------------//
// MT-EVENT-LISTENER  									//
//------------------------------------------------------//

/**
 * @brief The MTEventListenerStore class adds ofEventListener storage to
 * any class that inherits from it. This class was created before OF rolled
 * out its own eventListener storage (which the class uses), but it remains
 * here for both convenience and compatibility.
 */
class MTEventListenerStore
{
public:
	/// \brief Adds event listeners that should be destroyed when this
	/// object is destroyed.
	void addEventListener(std::unique_ptr<of::priv::AbstractEventToken> listener)
	{
		eventListeners.push(std::move(listener));
	}

	void clearEventListeners()
	{
		eventListeners.unsubscribeAll();
	}

protected:
//    std::vector<ofEventListener> eventListeners;
	ofEventListeners eventListeners;
};


/**
 * @brief Class containing static utility methods.
 */
class MTAppFramework
{
private:
	static bool fromBinding;
	MTAppFramework(){};
	~MTAppFramework(){};

public:
	/**
	 * @brief Synchronizes the values of two distinct parameters.
	 * @tparam ParamTypeA Should be assignable to ParamTypeB
	 * @tparam ParamTypeB Should be assignable to ParamTypeA
	 * @param aParam
	 * @param bParam
	 */
	template<typename ParamTypeA, typename ParamTypeB>
	static void BindParameters(ofParameter<ParamTypeA>& aParam, ofParameter<ParamTypeB>& bParam)
	{

		aParam.newListener([&](ParamTypeA& aValue)
						   {
							   if (!fromBinding)
							   {
								   // Flag that the changed came from the binding so that the bound eventListener
								   // doesn't attempt to change aParam in an infinite loop:
								   fromBinding = true;

								   // Set bParam to aValue:
								   bParam = aValue;
							   }
							   else
							   {
								   // "Consume" the fromBinding flag
								   fromBinding = false;
							   }
						   });

		bParam.newListener([&](ParamTypeB& bValue)
						   {
							   if (!fromBinding)
							   {
								   fromBinding = true;
								   aParam = bValue;
							   }
							   else
							   {
								   fromBinding = false;
							   }
						   });
	}

	/**
	 * @brief Stringifies an ofPath.
	 */
	static std::string PathToString(ofPath& path)
	{
		std::vector<ofPath::Command> commands = path.getCommands();

		std::string out = "";

		for (auto c : commands)
		{
			out += "{ "+ofToString(c.type)+"; "+ofToString(c.to)+"; "+ofToString(c.cp1)+"; "+ofToString(c.cp2)+"; } ";
		}

		return out;
	}

	/**
	 * @brief Makes an ofPath from a stringified representation.
	 * @param s The string to parse
	 * @return an ofPath. If the string parsing fails the path will
	 * be empty. TODO: Catching parsing errors.
	 */
	static ofPath PathFromString(std::string s)
	{
		std::vector<std::string> commandStrings = ofSplitString(s, "{", true, true);
		ofPath thePath;

		for (auto cs : commandStrings)
		{
			std::vector<std::string> commandStringElements = ofSplitString(cs, ";", true, true);

			ofPath::Command *thisCommand;
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
				case ofPath::Command::curveTo:
					p = ofFromString<ofPoint>(commandStringElements[1]);
					thePath.curveTo(p);
				case ofPath::Command::bezierTo:
					p = ofFromString<ofPoint>(commandStringElements[1]);
					cp1 = ofFromString<ofPoint>(commandStringElements[2]);
					cp2 = ofFromString<ofPoint>(commandStringElements[3]);
					thePath.bezierTo(cp1, cp2, p);
					break;
				case ofPath::Command::close:
					thePath.close();
					break;
				default:
					ofLog(OF_LOG_WARNING, "MTApp::pathFromString: A Path Command "
										  "supplied is not implemented");
					break;
			}
		}

		return thePath;
	}

};

namespace ofxImGui
{
	void AddParameter(std::shared_ptr<ofAbstractParameter> parameter);
};