//
// Created by Cristobal Mendoza on 2/15/18.
//

#ifndef NERVOUSSTRUCTUREOF_MTFULLSCREEN_HPP
#define NERVOUSSTRUCTUREOF_MTFULLSCREEN_HPP

#include "ofMain.h"
#include "MTWindow.hpp"

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
	std::shared_ptr<ofPath> perspectiveQuad;
};


namespace MTFullScreen
{
	void setup(std::shared_ptr<MTWindow> windowWithOutput,
			   ofTexture& outputTexture);
	void updateFullscreenDisplays();
	void toggleFullScreen();
	void setFullScreen(bool fs);
	void addFullScreenDisplay(std::shared_ptr<MTFullScreenDisplayInfo> fsDisplay);
	void addFullScreenDisplay();
	void removeFullScreenDisplay();
	std::vector<std::shared_ptr<MTFullScreenDisplayInfo> >::iterator begin();
	std::vector<std::shared_ptr<MTFullScreenDisplayInfo> >::iterator end();
	int getDisplayCount();
	std::vector<std::shared_ptr<MTFullScreenDisplayInfo>> getDisplayOutputs();


}

#include "ofMesh.h"

class MTFullScreenView : public MTView
{
protected:
	ofFbo outputFbo;
	ofTexture outputTexture;
	ofTexture testTexture;
	std::shared_ptr<ofPath> perspectiveQuad;
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
