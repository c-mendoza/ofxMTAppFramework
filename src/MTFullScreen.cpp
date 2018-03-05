//
// Created by Cristobal Mendoza on 2/15/18.
//

#include "MTFullScreen.hpp"
#include "ofGraphics.h"
#include "ofPath.h"

void MTFullScreen::setup(std::vector<MTFullScreenDisplayInfo> displayOutputs,
						 std::shared_ptr<MTWindow> windowWithOutput,
						 ofTexture& outputTexture)
{
	this->displayOutputs = displayOutputs;
	this->windowWithOutput = windowWithOutput;
	this->outputTexture = outputTexture;
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
	for (auto fsDisplay : displayOutputs)
	{
		ofGLFWWindowSettings glfwWindowSettings;
		glfwWindowSettings.shareContextWith = windowWithOutput;
		glfwWindowSettings.monitor = fsDisplay.display.id;
		glfwWindowSettings.windowMode = OF_FULLSCREEN;
		glfwWindowSettings.visible = true;
		auto window = MTApp::sharedApp->createWindow("FS " + ofToString(count), glfwWindowSettings);
		auto fsView = std::make_shared<MTFullScreenView>("FS View " + ofToString(count),
														 outputTexture,
														 fsDisplay.display.frame,
														 fsDisplay.outputQuad);

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


#pragma mark MTFullScreenView

MTFullScreenView::MTFullScreenView(std::string name,
								   ofTexture& outputTexture,
								   ofRectangle textureSubsection,
								   std::shared_ptr<ofPath> outputSurface) : MTView(name)
{
	this->outputTexture = outputTexture;
	this->outputSurface = outputSurface;
	outputMesh = outputTexture.getMeshForSubsection(0, 0, 0,
													textureSubsection.getWidth(), textureSubsection.getHeight(),
													textureSubsection.x, textureSubsection.y,
													textureSubsection.getWidth(), textureSubsection.getHeight(),
													true, OF_RECTMODE_CORNER);

	outputSurface->getOutline()[0].getBoundingBox();
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
	ofClear(0);
	ofSetColor(ofColor::white);
	ofFill();

	if (outputSurface->hasChanged())
	{
		perspectiveMatrix = MTHomographyHelper::calculateHomography(outputSurface->getOutline()[0].getVertices());
	}
	ofPushMatrix();
	ofMultMatrix(perspectiveMatrix);
	outputTexture.bind();
	outputMesh.draw();
	ofPopMatrix();
}

