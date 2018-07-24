//
// Created by Cristobal Mendoza on 2/15/18.
//
#ifndef TARGET_RASPBERRY_PI

#include "MTFullScreen.hpp"
//#include "ofGraphics.h"
//#include "ofPath.h"

using namespace MTFullScreen;

std::vector<std::shared_ptr<MTWindow>> fullScreenWindows;
bool isFullScreen;
std::vector<std::shared_ptr<MTFullScreenDisplayInfo>> displayOutputs;
std::shared_ptr<MTWindow> windowWithOutput;
float outputWidth;
float outputHeight;
ofTexture outputTexture;
glm::vec2 windowPos;

namespace priv
{
	void enterFullScreen();
	void exitFullScreen();
}

void MTFullScreen::setup(std::shared_ptr<MTWindow> _windowWithOutput,
						 ofTexture& _outputTexture)
{
	windowWithOutput = _windowWithOutput;
	outputTexture = _outputTexture;
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
			fsDisplay->outputArea->setPosition(0, 0);
		}
		else
		{
			fsDisplay->outputArea->setPosition(oneWidth * count, 0);
		}
		fsDisplay->outputArea->setSize(oneWidth,
									   outputHeight);
		if (fsDisplay->perspectiveQuad == nullptr)
		{
			fsDisplay->perspectiveQuad = std::make_shared<ofPath>();
			fsDisplay->perspectiveQuad->rectangle(*fsDisplay->outputArea);
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
		priv::enterFullScreen();
	}
	else
	{
		priv::exitFullScreen();
	}
}

void priv::enterFullScreen()
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
			fsDisplay->outputArea->setPosition(0, 0);
		}
		else
		{
			fsDisplay->outputArea->setPosition(oneWidth * count, 0);
		}
		fsDisplay->outputArea->setSize(oneWidth,
									   outputHeight);

		auto window = MTApp::sharedApp->createWindow("FS " + ofToString(count), glfwWindowSettings);
		auto fsView = std::make_shared<MTFullScreenView>("FS View " + ofToString(count),
														 fsDisplay,
														 outputTexture);

		window->contentView->addSubview(fsView);
		fsView->setSize(window->contentView->getFrameSize());
		fullScreenWindows.push_back(window);
		count++;
	}

	windowPos = windowWithOutput->getWindowPosition();
	glfwHideWindow(windowWithOutput->getGLFWWindow());
//	windowWithOutput->setWindowPosition(-windowWithOutput->getWidth(), -windowWithOutput->getHeight());
}

void priv::exitFullScreen()
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
	if (fsDisplay->outputArea == nullptr)
	{
		float xStart = 0;
		for (auto& fsd : displayOutputs)
		{
			xStart += fsd->outputArea->getMaxX();
		}

		fsDisplay->outputArea = std::make_shared<ofRectangle>(xStart, 0,
															  fsDisplay->display.frame.getWidth(),
															  fsDisplay->display.frame.getHeight());
	}

	if (fsDisplay->perspectiveQuad == nullptr)
	{
		auto quad = std::make_shared<ofPath>();
		quad->rectangle(*fsDisplay->outputArea.get());
		fsDisplay->perspectiveQuad = quad;
	}
	if (fsDisplay->perspectiveQuad->getCommands().size() != 5)
	{
		fsDisplay->perspectiveQuad->clear();
		fsDisplay->perspectiveQuad->rectangle(*fsDisplay->outputArea.get());
	}

	displayOutputs.push_back(fsDisplay);
	updateFullscreenDisplays();
	auto bla = displayOutputs.begin();
}

void MTFullScreen::addFullScreenDisplay()
{
	auto fsDisplay = std::make_shared<MTFullScreenDisplayInfo>();
	fsDisplay->display = MTApp::getDisplays().front();

	float xStart = 0;
	for (auto& fsd : displayOutputs)
	{
		xStart += fsd->outputArea->getMaxX();
	}

	fsDisplay->outputArea = std::make_shared<ofRectangle>(xStart, 0,
														  fsDisplay->display.frame.getWidth(),
														  fsDisplay->display.frame.getHeight());
	fsDisplay->perspectiveQuad = std::make_shared<ofPath>();
	fsDisplay->perspectiveQuad->rectangle(*fsDisplay->outputArea.get());
	addFullScreenDisplay(fsDisplay);
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

std::vector<std::shared_ptr<MTFullScreenDisplayInfo>> MTFullScreen::getDisplayOutputs()
{
	return displayOutputs;
}

int MTFullScreen::getDisplayCount()
{
	return displayOutputs.size();
}

#pragma mark MTFullScreenView

MTFullScreenView::MTFullScreenView(std::string name,
								   std::shared_ptr<MTFullScreenDisplayInfo> fullScreenDisplay,
								   ofTexture& outputTexture) : MTView(name)
{
	this->fullScreenDisplay = fullScreenDisplay;
	this->outputTexture = outputTexture;
	this->perspectiveQuad = fullScreenDisplay->perspectiveQuad;
	bool wasHackEnabled = ofIsTextureEdgeHackEnabled();
	ofEnableTextureEdgeHack();
	outputMesh = outputTexture.getMeshForSubsection(0, 0, 0,
													fullScreenDisplay->outputArea->getWidth(),
													fullScreenDisplay->outputArea->getHeight(),
													fullScreenDisplay->outputArea->x, fullScreenDisplay->outputArea->y,
													fullScreenDisplay->outputArea->getWidth(),
													fullScreenDisplay->outputArea->getHeight(),
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
	ofClear(0, 0, 0);
	ofSetColor(ofColor::white);
	ofFill();

	if (perspectiveQuad->hasChanged())
	{
		perspectiveMatrix = MTHomographyHelper::calculateHomography(getFrame(),
																	perspectiveQuad->getOutline()[0].getVertices());
	}
	ofPushMatrix();
	ofMultMatrix(perspectiveMatrix);
	outputTexture.bind();
	outputMesh.draw();
	ofPopMatrix();
}

#endif
