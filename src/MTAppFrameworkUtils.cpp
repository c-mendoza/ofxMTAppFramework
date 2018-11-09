
#include <graphics/ofPolyline.h>
#include "MTAppFrameworkUtils.hpp"

///////////////////////////////////////////
/// MTProcedure
///////////////////////////////////////////


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
