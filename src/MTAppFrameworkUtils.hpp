#pragma once
#include "ofMain.h"
//#include "ofxMTAppFramework.h"

//------------------------------------------------------//
// MT-PROCEDURE     									//
//------------------------------------------------------//

/// A single step in the overall procedure.
/// Still TODO: Fire events when appropriate.
class MTProcedureStep
{
public:
    int index;
    string name = "Step";
    string information = "Descriptive text goes here";
    ///If you test for success, and it is successful, you get sweet success action
    std::function<bool()> successTest = []{ return false; };
    ///When tests mean success, Action is your reward.
    std::function<void()> successAction = []{};
    bool complete = false;
};

/// MTProcedure is a queue of steps that are executed sequentially upon a
/// success condition being met for each step. Useful for implementing actions
/// that are sequential in nature and that require stepwise completion.
/// Still TODO: Fire events when appropriate.

class MTProcedure
{
public:

    MTProcedureStep getCurrentStep();
    void update();
    void addStep(MTProcedureStep step);
    bool isProcedureComplete() { return procedureComplete; }
    std::function<void()> procedureCompleteAction = [] {};

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

/////////////////////////////////////////////////
/// \brief The MTEventListenerStore class adds
/// ofEventListener storage to any class that
/// inherits from it.
/////////////////////////////////////////////////

class MTEventListenerStore
{
public:
    //    MTEventListenerStore(){}
    //    ~MTEventListenerStore()
    //    {
    //        eventListeners.clear(); //Check to see if this is necessary
    //    }

    /// \brief Adds event listeners that should be destroyed when this
    /// object is destroyed.
    void addEventListener(ofEventListener &&e)
    {
        eventListeners.push_back(std::move(e));
    }

    void clearEventListeners()
    {
        eventListeners.clear();
    }

protected:
    vector<ofEventListener> eventListeners;
};