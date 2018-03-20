//
// Created by Cristobal Mendoza on 2/15/18.
//

#include "MTFullScreen.hpp"
#include "ofGraphics.h"
#include "ofPath.h"

void MTFullScreen::setup(std::vector<std::shared_ptr<MTFullScreenDisplayInfo>> displayOutputs,
						 std::shared_ptr<MTWindow> windowWithOutput,
						 ofTexture& outputTexture)
{
	this->displayOutputs = displayOutputs;
	this->windowWithOutput = windowWithOutput;
	this->outputTexture = outputTexture;
	outputWidth = outputTexture.getWidth();
	outputHeight = outputTexture.getHeight();
}

void MTFullScreen::updateFullscreenDisplays()
{
	int count = 0;
	float oneWidth = outputWidth / (float) displayOutputs.size();
	for (auto fsDisplay : displayOutputs)
	{
		ofRectangle displayArea;

		if (count == 0)
		{
			fsDisplay->outputArea.setPosition(0, 0);
		}
		else
		{
			fsDisplay->outputArea.setPosition(oneWidth * count, 0);
		}
		fsDisplay->outputArea.setSize(oneWidth,
									  outputHeight);
		if (fsDisplay->outputQuad == nullptr)
		{
			fsDisplay->outputQuad = std::make_shared<ofPath>();
			fsDisplay->outputQuad->rectangle(fsDisplay->outputArea);
		}

		count++;
	}
}

void MTFullScreen::toggleFullScreen()
{
	setFullScreen(!isFullScreen);
}

void MTFullScreen::setFullScreen(bool fs)
{
	isFullScreen = fs;
	if (fs)
	{
		enterFullScreen();
	}
	else
	{
		exitFullScreen();
	}
}

void MTFullScreen::enterFullScreen()
{
	int count = 0;
	glm::vec3 lastPosition;
	glm::vec2 lastSize;
	float oneWidth = outputWidth / (float) displayOutputs.size();
	for (auto fsDisplay : displayOutputs)
	{
		ofGLFWWindowSettings glfwWindowSettings;
		glfwWindowSettings.shareContextWith = windowWithOutput;
		glfwWindowSettings.monitor = fsDisplay->display.id;
		glfwWindowSettings.windowMode = OF_FULLSCREEN;
		glfwWindowSettings.visible = true;
		ofRectangle displayArea;

		if (count == 0)
		{
			fsDisplay->outputArea.setPosition(0, 0);
		}
		else
		{
			fsDisplay->outputArea.setPosition(oneWidth * count, 0);
		}
		fsDisplay->outputArea.setSize(oneWidth,
									  outputHeight);

		auto window = MTApp::sharedApp->createWindow("FS " + ofToString(count), glfwWindowSettings);
		auto fsView = std::make_shared<MTFullScreenView>("FS View " + ofToString(count),
														 fsDisplay,
														 outputTexture,

		window->contentView->addSubview(fsView);
		fullScreenWindows.push_back(window);
		count++;
	}

	windowPos = windowWithOutput->getWindowPosition();
	glfwHideWindow(windowWithOutput->getGLFWWindow());
//	windowWithOutput->setWindowPosition(-windowWithOutput->getWidth(), -windowWithOutput->getHeight());
}

void MTFullScreen::exitFullScreen()
{
	fullScreenWindows.at(0)->setFullscreen(false);
	for (auto window : fullScreenWindows)
	{
		MTApp::sharedApp->windowClosing(window);
		window->setWindowShouldClose();
	}
	windowWithOutput->setWindowPosition(windowPos.x, windowPos.y);
	glfwShowWindow(windowWithOutput->getGLFWWindow());
	fullScreenWindows.clear();
}

void MTFullScreen::addFullScreenDisplay(std::shared_ptr<MTFullScreenDisplayInfo> fsDisplay)
{
	displayOutputs.push_back(fsDisplay);
	updateFullscreenDisplays();
	auto bla = displayOutputs.begin();
}

void MTFullScreen::addFullScreenDisplay()
{
	auto fsDisplay = std::make_shared<MTFullScreenDisplayInfo>();
	displayOutputs.push_back(fsDisplay);
	updateFullscreenDisplays();
}

void MTFullScreen::removeFullScreenDisplay()
{
	displayOutputs.pop_back();
	updateFullscreenDisplays();
}

std::vector<std::shared_ptr<MTFullScreenDisplayInfo> >::iterator MTFullScreen::begin()
{
	return displayOutputs.begin();
}

std::vector<std::shared_ptr<MTFullScreenDisplayInfo> >::iterator MTFullScreen::end()
{
	return displayOutputs.end();
}

#pragma mark MTFullScreenView

MTFullScreenView::MTFullScreenView(std::string name,
								   std::shared_ptr<MTFullScreenDisplayInfo> fullScreenDisplay,
								   ofTexture& outputTexture) : MTView(name)
{
	this->fullScreenDisplay = fullScreenDisplay;
	this->outputTexture = outputTexture;
	this->outputQuad = fullScreenDisplay->outputQuad;
	bool wasHackEnabled = ofIsTextureEdgeHackEnabled();
	ofEnableTextureEdgeHack();
	outputMesh = outputTexture.getMeshForSubsection(0, 0, 0,
													fullScreenDisplay->outputArea.getWidth(),
													fullScreenDisplay->outputArea.getHeight(),
													fullScreenDisplay->outputArea.x, fullScreenDisplay->outputArea.y,
													fullScreenDisplay->outputArea.getWidth(),
													fullScreenDisplay->outputArea.getHeight(),
													true, OF_RECTMODE_CORNER);
	if (!wasHackEnabled) ofDisableTextureEdgeHack();
	perspectiveMatrix = glm::mat4();
}

void MTFullScreenView::setup()
{

}

void MTFullScreenView::update()
{

}

void MTFullScreenView::draw()
{
	ofClear(255, 0, 0);
	ofSetColor(ofColor::white);
	ofFill();

	if (outputQuad->hasChanged())
	{
		perspectiveMatrix = MTHomographyHelper::calculateHomography(getFrame(),
																	outputQuad->getOutline()[0].getVertices());
	}
	ofPushMatrix();
	ofMultMatrix(perspectiveMatrix);
	outputTexture.bind();
	outputMesh.draw();
	ofPopMatrix();
}

