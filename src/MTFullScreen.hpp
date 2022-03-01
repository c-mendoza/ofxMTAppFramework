//
// Created by Cristobal Mendoza on 2/15/18.
//

#ifndef MTFULLSCREEN_HPP
#define MTFULLSCREEN_HPP

#include "ofMain.h"
#include "MTWindow.hpp"

struct MTFullScreenDisplayInfo
{
   /**
	 * @brief The physical display that will be used.
	 */
   std::shared_ptr<MTDisplay> display;
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
void setup(std::shared_ptr<MTWindow> windowWithOutput, ofTexture& outputTexture, int frameRate);
void updateFullscreenDisplays();
void toggleFullScreen();
void setFullScreen(bool fs);
bool isFullScreen();
void addFullScreenDisplay(std::shared_ptr<MTFullScreenDisplayInfo> fsDisplay);
void addFullScreenDisplay();
void removeFullScreenDisplay();
std::vector<std::shared_ptr<MTFullScreenDisplayInfo>>::iterator begin();
std::vector<std::shared_ptr<MTFullScreenDisplayInfo>>::iterator end();
int getDisplayCount();
std::vector<std::shared_ptr<MTFullScreenDisplayInfo>> getDisplayOutputs();
static ofEvent<void> displayCountChanged;
void updatePerspectiveTransforms();
}  // namespace MTFullScreen

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
   MTFullScreenView(std::string name, std::shared_ptr<MTFullScreenDisplayInfo> fullScreenDisplay, ofTexture& outputTexture);

   void setup() override;
   void update() override;
   void draw() override;
   void updatePerspectiveTransform();
};

/**
 * HomographyHelper
 * Created by Elliot Woods on 26/11/2010.
 * Edited by Krisjanis Rijnieks on 23/01/2016
 * Adapted from ofxPiMapper on 21 FEB 2018
 * Based entirely on arturo castro's homography implementation
 * Created 08/01/2010, arturo castro
*/
namespace MTHomographyHelper
{
static void gaussian_elimination(float* input, int n);
static glm::mat4 findHomography(float src[4][2], float dst[4][2]);
static glm::mat4 calculateHomography(ofRectangle source, std::vector<glm::vec3> vertices);
};  // namespace MTHomographyHelper

#endif  //MTFULLSCREEN_HPP
