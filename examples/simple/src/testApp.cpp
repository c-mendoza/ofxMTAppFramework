#include "testApp.h"
#include "MTView.hpp"
#include "ofGraphics.h"

void testApp::initialize()
{
}

void testApp::createAppViews()
{
	ofSetLogLevel(OF_LOG_VERBOSE);
	std::shared_ptr<MTView> view1;
	std::shared_ptr<MTView> view2;
	std::shared_ptr<MTView> view1_2;
	view1 = std::make_shared<MTView>("one");
	ofLogVerbose("10 VIEW 1 USE COUNT") << view1.use_count();

	//    view1 = ofxMTView::createView("One");
	view1->setSize(glm::vec2(500, 500));
	view1->setFrameOrigin(glm::vec2(30, 30));
	view1->backgroundColor = ofColor(255, 10, 10);
	view1->setContentScale(1, 1);

//    view1->setContentScale(0.5, 0.5);

	view1_2 = std::make_shared<MTView>("One_two");
	view1_2->setSize(80, 80);
	view1_2->setFrameOrigin(190, 30);
	view1_2->backgroundColor = ofColor(10, 10, 255);
	view1->addSubview(view1_2);
	ofLogVerbose("20 VIEW 1 USE COUNT") << view1.use_count();

	view2 = std::make_shared<MTView>("Two");
	view2->setSize(200, 200);
	view2->setFrameOrigin(glm::vec2(400, 100));
	view2->backgroundColor = ofColor(10, 200, 10);

//	ofGLFWWindowSettings windowSettings;
//	windowSettings.setSize(1920, 1080);
//	mainWindow = createWindow("Main Window", windowSettings);

	auto mainWindow = getMainWindow().lock();
	mainWindow->contentView->addSubview(view1);
	ofLogVerbose("30 VIEW 1 USE COUNT") << view1.use_count();

	mainWindow->contentView->addSubview(view2);
	ofLogVerbose("40 VIEW 1 USE COUNT") << view1.use_count();

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


	view1->onMouseMoved = [](MTView* view, int x, int y) {
		auto coord = glm::vec2(x, y);
		auto tcoord = view->transformFramePointToContent(coord);
//		ofLogVerbose() << view->name << " Pased: " << coord <<
//						  " Frame: " << view->getContentMouse() <<
//						  " To Content: " << tcoord;
	};

	view1->onDraw = [](MTView* view) {
//		offscreenView->drawOffscreen();
		ofSetColor(255);
//		offscreenView->getViewTexture().draw(0, 0);
		ofSetColor(ofColor::aquamarine);
		ofFill();

		ofDrawCircle(view->getFrameSize() / 2, 40);


	};

	ofLogVerbose("VIEW 1 USE COUNT") << view1.use_count();
	view1->onMousePressed = [](MTView* view, int x, int y, int b) {
		ofLogVerbose() << view->name << " " << view->getContentMouse();
		//        mainWindow->contentView->removeSubview(view1);
		//		view1->removeFromSuperview();

	};

	ofLogVerbose("DB") << view1.use_count();
	view1->onMouseDragged = [this](MTView* view, int x, int y, int b) {
		dragView(view, x, y);
	};

	view2->onMouseDragged = [this](MTView* view, int x, int y, int b) {
		dragView(view, x, y);
	};

	view1_2->onMouseDragged = [this](MTView* view, int x, int y, int b) {
		dragView(view, x, y);
	};

	view1_2->onMousePressed = [](MTView* view, int x, int y, int b) {
		ofLogVerbose() << view->name << " " << view->getContentMouse();
	};

	view2->onMousePressed = [](MTView* view, int x, int y, int b) {
		ofLogVerbose() << view->name << " " << view->getContentMouse();
	};

	mainWindow->contentView->onMousePressed = [this](MTView* view, int x, int y, int b) {
		ofLogVerbose() << getMainWindow().lock()->contentView->name;
	};
}

void testApp::dragView(MTView* view, int x, int y)
{
	glm::vec3 pos = view->getFrameOrigin();
	view->setFrameOrigin(pos +
						 (view->getContentMouse() - view->getContentMouseDown()));
	ofLogVerbose("Dragged " + view->name.get()) << view->getScreenFrame();
}
