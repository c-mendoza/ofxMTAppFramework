//
// Created by Cristobal Mendoza on 2019-07-26.
//

#ifndef MTMASK_HPP
#define MTMASK_HPP

#include "ofShader.h"
#include "ofFbo.h"

class MTMask
{
public:
	MTMask();

	void setup(int width, int height);
	void update(ofTexture& mask, ofTexture& image);
	ofTexture& getTexture();


protected:
	ofFbo buffer;
	ofShader maskShader;
	int width;
	int height;

	std::string generateFragmentShader();
	std::string generateVertexShader();

};


#endif //MTMASK_HPP
