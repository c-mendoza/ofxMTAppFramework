#include "testApp.h"
#include "MTView.hpp"
#include "ofGraphics.h"

void testApp::initialize()
{
	model = std::make_shared<MTModel>("test model");
}

void testApp::createAppViews()
{
	view1 = std::make_shared<MTView>("one");
	//    view1 = ofxMTView::createView("One");
	view1->setSize(glm::vec2(500, 500));
	view1->setFrameOrigin(glm::vec2(30, 30));
	view1->backgroundColor = ofColor(255, 10, 10);
	view1->setContentScale(1.5, 1);

//    view1->setContentScale(0.5, 0.5);

	view1_2 = std::make_shared<MTView>("One_two");
	view1_2->setSize(80, 80);
	view1_2->setFrameOrigin(190, 30);
	view1_2->backgroundColor = ofColor(10, 10, 255);
//	view1->addSubview(view1_2);

	view2 = std::make_shared<MTView>("Two");
	view2->setSize(200, 300);
	view2->setFrameOrigin(glm::vec2(400, 100));
	view2->backgroundColor = ofColor(10, 200, 10);

	ofGLFWWindowSettings windowSettings;
	windowSettings.setSize(1920, 1080);
	mainWindow = createWindow("Main Window", windowSettings);

	mainWindow->contentView->addSubview(view1);
	mainWindow->contentView->addSubview(view2);
	mainWindow->contentView->backgroundColor = ofColor::white;


//	offscreenView = std::make_shared<MTOffscreenView>("offscreen view");
//	offscreenView->setSize(300, 300);
//
//	offscreenView->onDraw = [this]()
//	{
//		ofClear(0, 0);
//		ofSetColor(127);
//		ofFill();
//		ofDrawCircle(0, 0, 80);
//	};


	view1->onMouseMoved = [this](int x, int y) {
		auto coord = glm::vec2(x, y);
		auto tcoord = view1->transformFramePointToContent(coord);
		ofLogVerbose() << view1->name << " Pased: " << coord <<
						  " Frame: " << view1->getContentMouse() <<
						  " To Content: " << tcoord;
	};

	view1->onDraw = [this]() {
//		offscreenView->drawOffscreen();
		ofSetColor(255);
//		offscreenView->getViewTexture().draw(0, 0);
		ofSetColor(ofColor::aquamarine);
		ofFill();

		ofDrawCircle(view1->getFrameSize() / 2, 40);


	};

	ofSetLogLevel(OF_LOG_VERBOSE);
	view1->onMousePressed = [this](int x, int y, int b) {
		ofLogVerbose() << view1->name << " " << view1->getContentMouse();
		//        mainWindow->contentView->removeSubview(view1);
		//		view1->removeFromSuperview();

	};

	view1->onMouseDragged = [this](int x, int y, int b) {
		dragView(view1, x, y);
	};

	view2->onMouseDragged = [this](int x, int y, int b) {
		dragView(view2, x, y);
	};

	view1_2->onMouseDragged = [this](int x, int y, int b) {
		dragView(view1_2, x, y);
	};

	view1_2->onMousePressed = [this](int x, int y, int b) {
		ofLogVerbose() << view1_2->name << " " << view1_2->getContentMouse();
	};

	view2->onMousePressed = [this](int x, int y, int b) {
		ofLogVerbose() << view2->name << " " << view2->getContentMouse();
	};

	mainWindow->contentView->onMousePressed = [this](int x, int y, int b) {
		ofLogVerbose() << mainWindow->contentView->name;
	};
}

void testApp::dragView(std::shared_ptr<MTView> view, int x, int y)
{
	glm::vec3 pos = view->getFrameOrigin();
	view->setFrameOrigin(pos +
						 (view->getContentMouse() - view->getContentMouseDown()));
	ofLogVerbose("Dragged " + view->name.get()) << view->getScreenFrame();
}
