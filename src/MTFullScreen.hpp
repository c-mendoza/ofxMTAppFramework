//
// Created by Cristobal Mendoza on 2/15/18.
//

#ifndef NERVOUSSTRUCTUREOF_MTFULLSCREEN_HPP
#define NERVOUSSTRUCTUREOF_MTFULLSCREEN_HPP

#include "ofxMTAppFramework.h"

struct MTFullScreenDisplayInfo
{
	/**
	 * @brief The physical display that will be used.
	 */
	MTDisplay display;
	/**
	 * @brief The rectangular area of the overall output that will be shown by this display.
	 */
	std::shared_ptr<ofRectangle> outputArea;
	/**
	 * @brief The perspective transformation points for this display.
	 */
	std::shared_ptr<ofPath> outputQuad;
};

class MTFullScreen
{
public:

//	MTFullScreen()
//	{}

	static void setup(std::vector<std::shared_ptr<MTFullScreenDisplayInfo>> displayOutputs,
			   std::shared_ptr<MTWindow> windowWithOutput,
			   ofTexture& outputTexture);
	static void updateFullscreenDisplays();
	static void toggleFullScreen();
	static void setFullScreen(bool fs);
	static void addFullScreenDisplay(std::shared_ptr<MTFullScreenDisplayInfo> fsDisplay);
	static void addFullScreenDisplay();
	static void removeFullScreenDisplay();
	static std::vector<std::shared_ptr<MTFullScreenDisplayInfo> >::iterator begin();
	static std::vector<std::shared_ptr<MTFullScreenDisplayInfo> >::iterator end();
protected:
	static std::vector<std::shared_ptr<MTWindow>> fullScreenWindows;
	static bool isFullScreen;
	static std::vector<std::shared_ptr<MTFullScreenDisplayInfo>> displayOutputs;
	static std::shared_ptr<MTWindow> windowWithOutput;
	static float outputWidth;
	static float outputHeight;

private:
	static void enterFullScreen();
	static void exitFullScreen();
	static ofTexture outputTexture;
	static glm::vec2 windowPos;
};

#include "ofMesh.h"

class MTFullScreenView : public MTView
{
protected:
	ofFbo outputFbo;
	ofTexture outputTexture;
	ofTexture testTexture;
	std::shared_ptr<ofPath> outputQuad;
	ofMesh outputMesh;
	glm::mat4 perspectiveMatrix;
	std::weak_ptr<MTFullScreenDisplayInfo> fullScreenDisplay;

public:
	MTFullScreenView(std::string name,
					 std::shared_ptr<MTFullScreenDisplayInfo> fullScreenDisplay,
					 ofTexture& outputTexture);

	void setup() override;
	void update() override;
	void draw() override;
};

#endif //NERVOUSSTRUCTUREOF_MTFULLSCREEN_HPP
