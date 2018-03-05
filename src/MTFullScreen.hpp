//
// Created by Cristobal Mendoza on 2/15/18.
//

#ifndef NERVOUSSTRUCTUREOF_MTFULLSCREEN_HPP
#define NERVOUSSTRUCTUREOF_MTFULLSCREEN_HPP

#include "ofxMTAppFramework.h"

struct MTFullScreenDisplayInfo
{
	MTDisplay display;
	std::shared_ptr<ofPath> outputQuad;
};

class MTFullScreen
{
public:

	MTFullScreen()
	{}

	void setup(std::vector<MTFullScreenDisplayInfo> displayOutputs,
			   std::shared_ptr<MTWindow> windowWithOutput,
			   ofTexture& outputTexture);
	void toggleFullScreen();
	void setFullScreen(bool fs);

protected:
	std::vector<std::shared_ptr<MTWindow>> fullScreenWindows;
	bool isFullScreen;
	std::vector<MTFullScreenDisplayInfo> displayOutputs;
	std::shared_ptr<MTWindow> windowWithOutput;

private:
	void enterFullScreen();
	void exitFullScreen();
	ofTexture outputTexture;
	glm::vec2 windowPos;
};

#include "ofMesh.h"

class MTFullScreenView : public MTView
{
protected:
	ofFbo outputFbo;
	ofTexture outputTexture;
	ofTexture testTexture;
	std::shared_ptr<ofPath> outputSurface;
	ofMesh outputMesh;
	glm::mat4 perspectiveMatrix;

public:
	MTFullScreenView(std::string name,
					 ofTexture& outputTexture,
					 ofRectangle textureSubsection,
					 std::shared_ptr<ofPath> outputSurface);

	void setup() override;
	void update() override;
	void draw() override;
};

#endif //NERVOUSSTRUCTUREOF_MTFULLSCREEN_HPP
