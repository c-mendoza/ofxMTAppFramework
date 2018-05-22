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
//	windowOutput.allocate(getWidth(), getHeight(), GL_RGBA, aaSamples);
}

void MTOffscreenWindow::draw(ofEventArgs& args)
{
	ofGetGLRenderer()->begin(windowOutput, ofFboMode::OF_FBOMODE_PERSPECTIVE | ofFboMode::OF_FBOMODE_MATRIXFLIP);
	MTWindow::draw(args);
	ofGetGLRenderer()->end(windowOutput);
}

void MTOffscreenWindow::exit(ofEventArgs& args)
{
//	MTWindow::exit(args);
	ofLogVerbose("MTOffscreenWindow") << this->name << " closing";
	if (contentView != nullptr) // Bit of a klugde
	{
		contentView->exit(args);
		contentView = nullptr;
		if (isImGuiEnabled)
		{
			gui.close();
			if (imCtx) ImGui::DestroyContext(imCtx);
		}
		MTApp::sharedApp->removeWindow(shared_from_this());
	}
}

ofFbo& MTOffscreenWindow::getWindowOutput()
{
	return windowOutput;
}

void MTOffscreenWindow::setup(ofGLFWWindowSettings& settings)
{
	aaSamples = settings.numSamples;
	settings.visible = false;
	settings.decorated = false;
	MTWindow::setup(settings);
	windowOutput.allocate(getWidth(), getHeight(), GL_RGBA, settings.numSamples);
}

MTOffscreenWindow::~MTOffscreenWindow()
{
}

void MTOffscreenWindow::close()
{
	ofEventArgs args;
	exit(args);
	ofAppGLFWWindow::close();
}
