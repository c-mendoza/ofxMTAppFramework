//
// Created by Cristobal Mendoza on 4/4/18.
//

#include <ofMain.h>
#include "MTOffscreenWindow.hpp"

MTOffscreenWindow::MTOffscreenWindow(const std::string& name) : MTWindow(name)
{}



void MTOffscreenWindow::setupInternal(ofEventArgs& args)
{
	MTWindow::setupInternal(args);
	windowOutput.allocate(getWidth(), getHeight(), GL_RGBA, aaSamples);
}

void MTOffscreenWindow::draw(ofEventArgs& args)
{
	ofGetGLRenderer()->begin(windowOutput, ofFboBeginMode::Perspective | ofFboBeginMode::MatrixFlip);
	MTWindow::draw(args);
	ofGetGLRenderer()->end(windowOutput);
}

void MTOffscreenWindow::exit(ofEventArgs& args)
{
//	MTWindow::exit(args);
	contentView->exit(args);
	contentView = nullptr;
	if (isImGuiEnabled)
	{
		gui.close();
		if (imCtx) ImGui::DestroyContext(imCtx);
	}
	MTApp::sharedApp->removeWindow(shared_from_this());
}

ofFbo& MTOffscreenWindow::getWindowOutput() const
{
	return windowOutput;
}

void MTOffscreenWindow::setup(const ofGLFWWindowSettings& settings)
{
	int aaSamples = settings.numSamples;
	MTWindow::setup(settings);
//	windowOutput.allocate(getWidth(), getHeight(), GL_RGBA, settings.numSamples);
}

MTOffscreenWindow::~MTOffscreenWindow()
{
	ofEventArgs args;
	exit(args);
}
