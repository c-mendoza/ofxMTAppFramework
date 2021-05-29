////
//// Created by Cristobal Mendoza on 4/4/18.
////
//#ifndef TARGET_RASPBERRY_PI
//#include <ofMain.h>
//#include "MTOffscreenWindow.hpp"
//
//MTOffscreenWindow::MTOffscreenWindow(const std::string& name, bool useTextureRectangle) : MTWindow(name)
//{
//	this->useTextureRectangle = useTextureRectangle;
//}
//
//void MTOffscreenWindow::setupInternal(ofEventArgs& args)
//{
//	MTWindow::setupInternal(args);
////	windowOutput.allocate(getWidth(), getHeight(), GL_RGBA, aaSamples);
//}
//
//void MTOffscreenWindow::draw(ofEventArgs& args)
//{
//	ofGetGLRenderer()->begin(windowOutput, ofFboMode::OF_FBOMODE_PERSPECTIVE | ofFboMode::OF_FBOMODE_MATRIXFLIP);
//	MTWindow::draw(args);
//	ofGetGLRenderer()->end(windowOutput);
//}
//
////void MTOffscreenWindow::exit(ofEventArgs& args)
////{
//////	MTWindow::exit(args);
////
////}
//
//ofFbo& MTOffscreenWindow::getWindowOutput()
//{
//	return windowOutput;
//}
//
//void MTOffscreenWindow::setup(ofGLFWWindowSettings& settings)
//{
//	aaSamples = settings.numSamples;
//	settings.visible = false;
//	settings.decorated = false;
//	MTWindow::setup(settings);
//	ofFboSettings fboSettings;
//	fboSettings.width = settings.getWidth();
//	fboSettings.height = settings.getHeight();
//	fboSettings.internalformat = GL_RGBA;
//	fboSettings.numSamples = settings.numSamples;
//
//#ifdef TARGET_OPENGLES
//	fboSettings.useDepth		= false;
//	fboSettings.useStencil		= false;
//	//we do this as the fbo and the settings object it contains could be created before the user had the chance to disable or enable arb rect.
//	settings.textureTarget	= GL_TEXTURE_2D;
//#else
//	fboSettings.useDepth = true;
//	fboSettings.useStencil = true;
//	if (useTextureRectangle)
//	{
//		fboSettings.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
//	}
//	else
//	{
//		fboSettings.textureTarget = GL_TEXTURE_2D;
//	}
//#endif
//	windowOutput.allocate(fboSettings);
//}
//
//MTOffscreenWindow::~MTOffscreenWindow()
//{
//	ofLogVerbose("MTOffscreenWindow") << this->name << " closing";
//	if (contentView != nullptr) // Bit of a klugde
//	{
//		ofEventArgs args;
//		contentView->exit(args);
//		contentView = nullptr;
//		MTApp::Instance()->closeWindow(shared_from_this());
//	}
//	ofLogVerbose("MTOffscreenWindow") << "MTOF Destroyed";
//}
//
//#endif