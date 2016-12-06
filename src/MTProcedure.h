//
//  ofxMTAppProcedure.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 12/2/16.
//
//

#ifndef MTAppProcedure_h
#define MTAppProcedure_h

#include "ofxMTAppFramework.h"


class MTProcedureStep
{
	public:
		MTProcedureStep() {}
		int index;
		string name;
		string information;
		std::function successTest;
		bool complete = false;
};

class MTProcedure
{
	public:
		MTProcedure(){}

		const MTProcedureStep& getCurrentStep();
		void update();
		void addStep(MTProcedureStep step);
		void setTotalSteps() { totalSteps = stepStack.size(); }

	private:
		std::stack<MTProcedureStep> stepStack;
		int totalSteps;
};


#endif /* MTAppProcedure_h */
