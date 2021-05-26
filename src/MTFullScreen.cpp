//
// Created by Cristobal Mendoza on 2/15/18.
//
#ifndef TARGET_RASPBERRY_PI

#include "MTFullScreen.hpp"
//#include "ofGraphics.h"
//#include "ofPath.h"

using namespace MTFullScreen;

std::vector<std::shared_ptr<MTWindow>> fullScreenWindows;
bool isFullScreenActive = false;
std::vector<std::shared_ptr<MTFullScreenDisplayInfo>> displayOutputs;
std::shared_ptr<MTWindow> windowWithOutput;
float outputWidth;
float outputHeight;
ofTexture outputTexture;
glm::vec2 windowPos;
int frameRate = 60;

namespace priv
{
	void enterFullScreen();
	void exitFullScreen();
}

void MTFullScreen::setup(std::shared_ptr<MTWindow> _windowWithOutput, ofTexture& _outputTexture, int _frameRate = 60)
{
	windowWithOutput = _windowWithOutput;
	outputTexture = _outputTexture;
	outputWidth = outputTexture.getWidth();
	outputHeight = outputTexture.getHeight();
	frameRate = _frameRate;
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

void MTFullScreen::updatePerspectiveTransforms()
{
	for (auto win : fullScreenWindows)
	{
		auto& view = win->contentView->getSubviews()[0];
		auto* fsView = dynamic_cast<MTFullScreenView*>(view.get());
		fsView->updatePerspectiveTransform();
	}
}

void MTFullScreen::toggleFullScreen()
{
	setFullScreen(!isFullScreenActive);
}

void MTFullScreen::setFullScreen(bool fs)
{
	// Return immediately if the state is not changed:
	if (fs == isFullScreenActive)
	{
		return;
	}

	isFullScreenActive = fs;
	if (fs)
	{
		priv::enterFullScreen();
	}
	else
	{
		priv::exitFullScreen();
	}
}

bool MTFullScreen::isFullScreen()
{
	return isFullScreenActive;
}

void priv::enterFullScreen()
{
	int count = 0;
	glm::vec3 lastPosition;
	glm::vec2 lastSize;
	float oneWidth = outputWidth/(float) displayOutputs.size();
	for (auto fsDisplay : displayOutputs)
	{
		ofGLFWWindowSettings glfwWindowSettings;
		glfwWindowSettings.shareContextWith = windowWithOutput;
		glfwWindowSettings.monitor = fsDisplay->display->getId();
		glfwWindowSettings.windowMode = OF_FULLSCREEN;
		glfwWindowSettings.visible = true;
		auto frame = fsDisplay->display->getFrame();
		glfwWindowSettings.setSize(frame.width, frame.height);

		// TODO:
		// fsDisplay->display.frame.position is not reporting the actual position of the display


		glfwWindowSettings.setPosition(glm::vec2(frame.position));
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

		auto window = MTApp::Instance()->createWindow("FS " + ofToString(count), glfwWindowSettings);

		std::string name = "FS View " + ofToString(count);
		auto fv = MTView::CreateView<MTFullScreenView>(name, fsDisplay,
													   outputTexture);


		auto fsView = MTView::CreateView<MTFullScreenView>(name,
														 fsDisplay,
														 outputTexture);
		window->setWindowPosition(frame.position.x, frame.position.y);
//		window->events().setFrameRate(frameRate);
//		window->setVerticalSync(false);
		fsView->setSize(window->contentView->getFrameSize());
		window->contentView->addSubview(std::move(fsView));
		fullScreenWindows.push_back(window);
		count++;
	}

	windowPos = windowWithOutput->getWindowPosition();
//	glfwHideWindow(windowWithOutput->getGLFWWindow());
//	windowWithOutput->setWindowPosition(-windowWithOutput->getWidth(), -windowWithOutput->getHeight());
}

void priv::exitFullScreen()
{
	for (const auto& window : fullScreenWindows)
	{
		window->setFullscreen(false);
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
															  fsDisplay->display->getFrame().getWidth(),
															  fsDisplay->display->getFrame().getHeight());
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
														  fsDisplay->display->getFrame().getWidth(),
														  fsDisplay->display->getFrame().getHeight());
	fsDisplay->perspectiveQuad = std::make_shared<ofPath>();
	fsDisplay->perspectiveQuad->rectangle(*fsDisplay->outputArea.get());
	addFullScreenDisplay(fsDisplay);
	displayCountChanged.notify();
}

void MTFullScreen::removeFullScreenDisplay()
{
	displayOutputs.pop_back();
	updateFullscreenDisplays();
	displayCountChanged.notify();
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
	resizePolicy = MTViewResizePolicy::ResizePolicySuperview;
	perspectiveMatrix = glm::mat4();
	backgroundColor = ofColor(255, 0, 0);
	setDrawBackground(true);
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

	//TODO: Re-enable perspective warping later
//	ofPushMatrix();
//	ofMultMatrix(perspectiveMatrix);
//	outputTexture.bind();
//	outputMesh.draw();
//	ofPopMatrix();

	outputTexture.draw(0, 0, getWidth(), getHeight());
}

void MTFullScreenView::updatePerspectiveTransform()
{
	enqueueUpdateOperation([this]()
						   {
							   perspectiveMatrix = MTHomographyHelper::calculateHomography(getFrame(),
																						   perspectiveQuad->getOutline()[0].getVertices());
						   }
	);
}

void MTHomographyHelper::gaussian_elimination(float *input, int n)
{
	// ported to c from pseudocode in
	// http://en.wikipedia.org/wiki/Gaussian_elimination

	float *A = input;
	int i = 0;
	int j = 0;
	int m = n-1;
	while (i < m && j < n)
	{
		// Find pivot in column j, starting in row i:
		int maxi = i;
		for (int k = i+1; k < m; k++)
		{
			if (fabs(A[k*n+j]) > fabs(A[maxi*n+j]))
			{
				maxi = k;
			}
		}
		if (A[maxi*n+j] != 0)
		{
			//swap rows i and maxi, but do not change the value of i
			if (i != maxi)
				for (int k = 0; k < n; k++)
				{
					float aux = A[i*n+k];
					A[i*n+k] = A[maxi*n+k];
					A[maxi*n+k] = aux;
				}
			//Now A[i,j] will contain the old value of A[maxi,j].
			//divide each entry in row i by A[i,j]
			float A_ij = A[i*n+j];
			for (int k = 0; k < n; k++)
			{
				A[i*n+k] /= A_ij;
			}
			//Now A[i,j] will have the value 1.
			for (int u = i+1; u < m; u++)
			{
				//subtract A[u,j] * row i from row u
				float A_uj = A[u*n+j];
				for (int k = 0; k < n; k++)
				{
					A[u*n+k] -= A_uj*A[i*n+k];
				}
				//Now A[u,j] will be 0, since A[u,j] - A[i,j] * A[u,j] = A[u,j] - 1 * A[u,j] = 0.
			}

			i++;
		}
		j++;
	}

	//back substitution
	for (int i = m-2; i >= 0; i--)
	{
		for (int j = i+1; j < n-1; j++)
		{
			A[i*n+m] -= A[i*n+j]*A[j*n+m];
			//A[i*n+j]=0;
		}
	}
}

glm::mat4 MTHomographyHelper::findHomography(float src[4][2], float dst[4][2])
{

	// create the equation system to be solved
	//
	// from: Multiple View Geometry in Computer Vision 2ed
	//       Hartley R. and Zisserman A.
	//
	// x' = xH
	// where H is the homography: a 3 by 3 matrix
	// that transformed to inhomogeneous coordinates for each point
	// gives the following equations for each point:
	//
	// x' * (h31*x + h32*y + h33) = h11*x + h12*y + h13
	// y' * (h31*x + h32*y + h33) = h21*x + h22*y + h23
	//
	// as the homography is scale independent we can let h33 be 1 (indeed any of the terms)
	// so for 4 points we have 8 equations for 8 terms to solve: h11 - h32
	// after ordering the terms it gives the following matrix
	// that can be solved with gaussian elimination:

	float P[8][9] = {
			{-src[0][0], -src[0][1], -1, 0,          0,          0,  src[0][0]*dst[0][0], src[0][1]*dst[0][0], -dst[0][0]}, // h11
			{0,          0,          0,  -src[0][0], -src[0][1], -1, src[0][0]*dst[0][1], src[0][1]*dst[0][1], -dst[0][1]}, // h12

			{-src[1][0], -src[1][1], -1, 0,          0,          0,  src[1][0]*dst[1][0], src[1][1]*dst[1][0], -dst[1][0]}, // h13
			{0,          0,          0,  -src[1][0], -src[1][1], -1, src[1][0]*dst[1][1], src[1][1]*dst[1][1], -dst[1][1]}, // h21

			{-src[2][0], -src[2][1], -1, 0,          0,          0,  src[2][0]*dst[2][0], src[2][1]*dst[2][0], -dst[2][0]}, // h22
			{0,          0,          0,  -src[2][0], -src[2][1], -1, src[2][0]*dst[2][1], src[2][1]*dst[2][1], -dst[2][1]}, // h23

			{-src[3][0], -src[3][1], -1, 0,          0,          0,  src[3][0]*dst[3][0], src[3][1]*dst[3][0], -dst[3][0]}, // h31
			{0,          0,          0,  -src[3][0], -src[3][1], -1, src[3][0]*dst[3][1], src[3][1]*dst[3][1], -dst[3][1]}, // h32
	};

	gaussian_elimination(&P[0][0], 9);

	// gaussian elimination gives the results of the equation system
	// in the last column of the original matrix.
	// opengl needs the transposed 4x4 matrix:
	float aux_H[] = {P[0][8], P[3][8], 0, P[6][8], // h11  h21 0 h31
					 P[1][8], P[4][8], 0, P[7][8], // h12  h22 0 h32
					 0, 0, 0, 0,       // 0    0   0 0
					 P[2][8], P[5][8], 0, 1};      // h13  h23 0 h33

	glm::mat4 homography(P[0][8], P[3][8], 0, P[6][8], // h11  h21 0 h31
						 P[1][8], P[4][8], 0, P[7][8], // h12  h22 0 h32
						 0, 0, 0, 0,       // 0    0   0 0
						 P[2][8], P[5][8], 0, 1);      // h13  h23 0 h33)
//	for(int i=0;i<16;i++) homography[i] = aux_H[i];
	return homography;
}

glm::mat4 MTHomographyHelper::calculateHomography(ofRectangle source, std::vector<glm::vec3> destination)
{
	float src[4][2];
	float dst[4][2];
	assert(destination.size() == 4);

	src[0][0] = 0;
	src[0][1] = 0;
	src[1][0] = source.width;
	src[1][1] = 0;
	src[2][0] = source.width;
	src[2][1] = source.height;
	src[3][0] = 0;
	src[3][1] = source.height;

//	src[0][0] = 0;
//	src[0][1] = 0;
//	src[1][0] = 1920;
//	src[1][1] = 0;
//	src[2][0] = 1920;
//	src[2][1] = 1200;
//	src[3][0] = 0;
//	src[3][1] = 1200;

	glm::vec3 p0 = destination[0];
	glm::vec3 p1 = destination[1];
	glm::vec3 p2 = destination[2];
	glm::vec3 p3 = destination[3];

	dst[0][0] = p0.x;
	dst[0][1] = p0.y;
	dst[1][0] = p1.x;
	dst[1][1] = p1.y;
	dst[2][0] = p2.x;
	dst[2][1] = p2.y;
	dst[3][0] = p3.x;
	dst[3][1] = p3.y;

	return MTHomographyHelper::findHomography(src, dst);
}

#endif
