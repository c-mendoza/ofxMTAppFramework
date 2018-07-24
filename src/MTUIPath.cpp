//
//  ofxUIPath.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 8/1/16.
//
//

#include "MTUIPath.hpp"

int MTUIPath::vertexHandleSize = 10;
int MTUIPath::cpHandleSize = 10;
ofStyle MTUIPath::vertexHandleStyle;
ofStyle MTUIPath::selectedVextexHandleStyle;
ofStyle MTUIPath::cpHandleStyle;

//////////////////////////
//MTUIPath
//////////////////////////

MTUIPath::~MTUIPath()
{
	ofLogVerbose("MTUIPath") << "Destructor";
	removeEventListeners();
	for (auto handle : pathHandles)
	{
		handle->getPointHandle()->removeFromSuperview();
		handle->getCP1Handle()->removeFromSuperview();
		handle->getCP2Handle()->removeFromSuperview();
	}
	pathHandles.clear();
}

void MTUIPath::setup(std::shared_ptr<ofPath> p,
					 std::shared_ptr<MTView> view)
{
	setup(p, view, (unsigned int) pathOptionFlags.to_ulong());
}

void MTUIPath::setup(std::shared_ptr<ofPath> p,
					 std::shared_ptr<MTView> view,
					 unsigned int options)
{
	path = p;
	pathHandles.clear();
	selectedHandles.clear();
	this->view = view;
	pathOptionFlags = std::bitset<5>(options);
	addEventListeners();

	//Create handles for verts
	auto commands = p->getCommands();
	for (auto command : commands)
	{
		// Do not add handles for close commands:
		if (command.type == ofPath::Command::close)
		{
			continue;
		}

		auto handle = std::shared_ptr<MTUIPathVertexHandle>(new MTUIPathVertexHandle());
		handle->setup(shared_from_this(), command);
		pathHandles.push_back(handle);
	}

	updatePath();
}

void MTUIPath::updatePath()
{

	outputPath.clear();
	path->clear();
//	auto commands = outputPath.getCommands();
//	path->getCommands().assign(commands.begin(), commands.end());
	std::vector<ofPath::Command> commands;
	for (auto& handle : pathHandles)
	{
		commands.push_back(handle->getCommand());
	}
	outputPath.getCommands().assign(commands.begin(), commands.end());

	if (isClosed) outputPath.close();

	path->append(outputPath);
	midpoints.clear();
	commands = outputPath.getCommands();
	if (commands.size() > 1)
	{
		int max;

		if (isClosed)
		{
			max = commands.size() - 1;
		}
		else
		{
			max = commands.size();
		}

		for (int i = 0; i < max; i++)
		{
			Midpoint mp;
			mp.index1 = i;
			mp.index2 = (i + 1) % (max);
			mp.pos = (commands[mp.index1].to + commands[mp.index2].to) / 2.0f;
			midpoints.push_back(std::move(mp));
		}
	}
}

void MTUIPath::setClosed(bool closed)
{
	// No change in state? Do nothing and return
	if (closed == isClosed) return;

	isClosed = closed;

	// If the path hasn't been set yet, return. We will set this up in MTUIPath::setup
	if (path == NULL) return;

	if (closed)
	{
		path->close();

	}
	else
	{
		auto commands = path->getCommands();
		auto com = commands.back();
		if (com.type == ofPath::Command::close)
		{
			commands.pop_back();
		}
		path->getCommands();
	}

	//I think that this updates the path:
	path->getCommands();
	updatePath();
//	setup(path, view, (unsigned int) pathOptionFlags.to_ulong()); // Necessary??? TODO: Check
}

///The actual drawing method
void MTUIPath::draw()
{
	if (isVisible)
	{
		path->draw(0, 0);

		auto previous = pathHandles.back();

		for (int i = 0; i < pathHandles.size(); i++)
		{
			auto handle = pathHandles[i];
			if (handle->getCommand().type == ofPathCommand::bezierTo)
			{
				ofDrawLine(handle->getCP2Handle()->getFrameCenter(),
						   handle->getPointHandle()->getFrameCenter());

				ofDrawLine(handle->getCP1Handle()->getFrameCenter(),
						   previous->getPointHandle()->getFrameCenter());
			}

//            handle->draw();
			previous = handle;
		}

//        ofSetRectMode(OF_RECTMODE_CENTER);
//        for (auto & mid : midpoints)
//        {
//            ofDrawCircle(mid.pos, 5);
//        }
//        ofSetRectMode(OF_RECTMODE_CORNER);
	}
}

void MTUIPath::handlePressed(MTUIPathVertexHandle* handle, ofMouseEventArgs& args)
{
	if (args.button == 0)
	{
		auto it = std::find_if(pathHandles.begin(),
							   pathHandles.end(),
							   [&](std::shared_ptr<MTUIPathVertexHandle> const& current)
							   {
								   return current.get() == handle;
							   });

		// Transform lineTo <-> bezierTo
#pragma mark VERTEX TRANSFORM
		if (ofGetKeyPressed(OF_KEY_SHIFT) &&
			pathOptionFlags.test(CanConvertPoints))
		{
			// Don't transform the first vertex, it has to be moveTo
			if (it != pathHandles.begin())
			{
				auto command = (*it)->getCommand();
				if (command.type == ofPath::Command::lineTo)
				{
					auto polyline = path->getOutline()[0];
					unsigned int polyIndex;
					auto tangent = polyline.getClosestPoint(command.to, &polyIndex);

					auto toHandle = (*it)->getPointHandle();
					auto cp1 = (*it)->getCP1Handle();
					auto tNorm = glm::normalize(tangent);
					cp1->setFrameOrigin(toHandle->getFrameOrigin() - (tNorm * 50.0f));

					auto cp2 = (*it)->getCP2Handle();
					cp2->setFrameOrigin(toHandle->getFrameOrigin() + (tNorm * 50.0f));
					command.type = ofPath::Command::bezierTo;
					(*it)->setCommand(command);
//                    (*it)->getCP1Handle()->setFrameOrigin(command->to.x, command->to.y);
//                    (*it)->getCP2Handle()->setFrameOrigin(command->to.x, command->to.y);
					view->addSubview(cp1);
					view->addSubview(cp2);
					this->pathChangedEvent.notify(this);
				}
				else if (command.type == ofPath::Command::bezierTo)
				{
					command.type = ofPath::Command::lineTo;
					(*it)->setCommand(command);
					(*it)->getCP1Handle()->removeFromSuperview();
					(*it)->getCP2Handle()->removeFromSuperview();
					this->pathChangedEvent.notify(this);
				}

				(*it)->updateCommand();
			}
		}
			// Add to the selection
		else if (ofGetKeyPressed(OF_KEY_COMMAND))
		{
			addToSelection(*it);
		}
			// Remove from selection
		else if (ofGetKeyPressed(OF_KEY_ALT))
		{
			removeFromSelection(*it);
		}
			// Default case, set as the selection
		else
		{
			setSelection(*it);
		}

		handleWasPressed = true;
	}
}

void MTUIPath::handleReleased(MTUIPathVertexHandle* handle, ofMouseEventArgs& args)
{

}

void MTUIPath::addEventListeners()
{
	ofAddListener(view->mousePressedEvent, this, &MTUIPath::mousePressed, 100);
	ofAddListener(view->keyReleasedEvent, this, &MTUIPath::keyReleased, 100);
	ofAddListener(view->mouseMovedEvent, this, &MTUIPath::mouseMoved, 100);
}

void MTUIPath::removeEventListeners()
{
	ofRemoveListener(view->mousePressedEvent, this, &MTUIPath::mousePressed, 100);
	ofRemoveListener(view->keyReleasedEvent, this, &MTUIPath::keyReleased, 100);
	ofRemoveListener(view->mouseMovedEvent, this, &MTUIPath::mouseMoved, 100);
}

//VIEW EVENTS
/////////////////////////////////

void MTUIPath::mousePressed(ofMouseEventArgs& args)
{

	if (!handleWasPressed)
	{
		deselectAll();
	}
	else
	{
		handleWasPressed = false;
	}
}

void MTUIPath::mouseMoved(ofMouseEventArgs& args)
{
	if (path->getCommands().size() > 2)
	{
		auto pt = glm::vec3(args, 0);
//		closestMidpoint = getClosestMidpoint(pt);
	}
}

void MTUIPath::keyReleased(ofKeyEventArgs& args)
{
	if (pathOptionFlags.test(MTUIPathOptions::CanDeletePoints))
	{
		if (args.key == OF_KEY_DEL || args.key == OF_KEY_BACKSPACE)
		{
			deleteSelected();
		}
	}
}


//DATA HANDLING
/////////////////////////////////

bool MTUIPath::deleteHandle(std::shared_ptr<MTUIPathVertexHandle> handle)
{

	auto iter = std::find(pathHandles.begin(), pathHandles.end(), handle);

	bool success = true;

	if (iter == pathHandles.end()) return false;

	pathHandles.erase(iter);
	// This might be overkill, but for extra-checking it is here...
	//	vertexHandles.erase(std::find_if(vertexHandles.begin(), vertexHandles.end(), [&](shared_ptr<MTUIPathHandle> const& current)
	//	{
	//		if (current.get() == handle.get())
	//		{
	//			success = true;
	//			return true;
	//		}
	//        return false;
	//	}));


	// Check to see if the last command has been deleted:
	if (pathHandles.size() == 0)
	{
		// If so, notify listeners.
		lastHandleDeletedEvent.notify(this);
	}
	else
	{
//		setup(path, view, (unsigned int) pathOptionFlags.to_ulong());
		updatePath();
		pathChangedEvent.notify(this);
		if (selectsLastInsertion)
		{
			setSelection(pathHandles.back());
		}
	}


	return success;
}

void MTUIPath::deleteSelected()
{
	for (auto handle : selectedHandles)
	{
		deleteHandle(handle);
	}

	deselectAll();
}

MTUIPath::Midpoint& MTUIPath::getClosestMidpoint(glm::vec3& point)
{
	float minDistance = 5000;
	Midpoint closest;
	int currentIndex = 0;
	int closestIndex = 0;

	for (auto& mid : midpoints)
	{
		float distance = glm::distance(point, mid.pos);
		if (distance < minDistance)
		{
			closest = mid;
			minDistance = distance;
			closestIndex = currentIndex;

		}
		currentIndex++;
	}

	return midpoints[closestIndex];
}

void MTUIPath::addHandle(std::shared_ptr<MTUIPathVertexHandle> handle)
{
	if (!pathOptionFlags.test(CanAddPoints)) return;
	pathHandles.push_back(handle);
	updatePath();
	pathChangedEvent.notify(this);

	if (selectsLastInsertion)
	{
		setSelection(pathHandles.back());
	}
}

void MTUIPath::addHandle(glm::vec3 point)
{
	auto handle = std::make_shared<MTUIPathVertexHandle>();
	ofPath::Command com = ofPath::Command(ofPath::Command::lineTo, point);
	if (pathHandles.size() > 0)
	{
		com = ofPath::Command(ofPath::Command::lineTo, point);
	}
	else
	{
		com = ofPath::Command(ofPath::Command::moveTo, point);

	}

	handle->setup(shared_from_this(), com);
	addHandle(handle);
}

void MTUIPath::insertHandle(std::shared_ptr<MTUIPathVertexHandle> handle, unsigned int index)
{
	if (!pathOptionFlags.test(CanAddPoints)) return;
	if (index >= pathHandles.size())
	{
		pathHandles.push_back(handle);
	}
	else
	{
		pathHandles.insert(pathHandles.begin() + index, handle);
	}
	updatePath();
	pathChangedEvent.notify(this);

	if (selectsLastInsertion)
	{
		setSelection(pathHandles.back());
	}
}

void MTUIPath::insertHandle(glm::vec3 point, unsigned int index)
{
	auto handle = std::make_shared<MTUIPathVertexHandle>();
	if (pathHandles.size() > 0)
	{
		auto com = ofPath::Command(ofPath::Command::lineTo, point);
		handle->setup(shared_from_this(), com);
		insertHandle(handle, index);
	}
}

//SELECTION
/////////////////////////////////

void MTUIPath::addToSelection(std::shared_ptr<MTUIPathVertexHandle> vertex)
{
	selectedHandles.push_back(vertex);
	vertex->setStyle(selectedVextexHandleStyle);
}

void MTUIPath::removeFromSelection(std::shared_ptr<MTUIPathVertexHandle> vertex)
{
	selectedHandles.erase(std::remove_if(selectedHandles.begin(),
										 selectedHandles.end(),
										 [&](std::shared_ptr<MTUIPathVertexHandle> const& current)
										 {
											 if (current.get() == vertex.get())
											 {
												 return true;
											 }
											 return false;
										 }));

	vertex->setStyle(vertexHandleStyle);

}

void MTUIPath::setSelection(std::shared_ptr<MTUIPathVertexHandle> vertex)
{
	deselectAll();
	addToSelection(vertex);
}

void MTUIPath::deselectAll()
{
	for (auto handle : selectedHandles)
	{
		handle->setStyle(vertexHandleStyle);
	}

	selectedHandles.clear();
}

void MTUIPath::selectAll()
{
	for (auto handle : selectedHandles)
	{
		addToSelection(handle);
	}
}



#pragma mark MTUIPathHandle

//////////////////////////
//MTUIPathHandle
//////////////////////////

MTUIPathVertexHandle::~MTUIPathVertexHandle()
{
	ofLogVerbose("MTUIPathHandle") << "Destroyed";
	toHandle->removeFromSuperview();
	cp1Handle->removeFromSuperview();
	cp2Handle->removeFromSuperview();
}

void MTUIPathVertexHandle::setup(std::weak_ptr<MTUIPath> uiPath, ofPath::Command com)
{
	this->uiPath = uiPath;
	command = com;
	clearEventListeners();

	toHandle = std::make_shared<MTUIHandle>("To Handle");
	toHandle->resizePolicy = MTViewResizePolicy::ResizePolicyNone;
	toHandle->setFrameFromCenter(command.to,
								 glm::vec2(MTUIPath::vertexHandleSize,
										   MTUIPath::vertexHandleSize));

	cp1Handle = std::make_shared<MTUIHandle>("CP 1");
	cp1Handle->resizePolicy = MTViewResizePolicy::ResizePolicyNone;
	cp1Handle->setFrameFromCenter(command.cp1,
								  glm::vec2(MTUIPath::vertexHandleSize,
											MTUIPath::vertexHandleSize));
	//    cp1Handle->handleType = ofxMTHandle<ofPoint>::ControlPointHandleType;

	cp2Handle = std::make_shared<MTUIHandle>("CP 2");
	cp2Handle->resizePolicy = MTViewResizePolicy::ResizePolicyNone;
	cp2Handle->setFrameFromCenter(command.cp2,
								  glm::vec2(MTUIPath::vertexHandleSize,
											MTUIPath::vertexHandleSize));
	//    cp2Handle->handleType = MTUIHandle::HandleType::SQUARE;
	auto uiPathPtr = uiPath.lock();
	uiPathPtr->view->addSubview(toHandle);

	if (command.type == ofPath::Command::bezierTo ||
		command.type == ofPath::Command::quadBezierTo)
	{
		uiPathPtr->view->addSubview(cp1Handle);
		uiPathPtr->view->addSubview(cp2Handle);
	}

	addEventListener(toHandle->mouseDraggedEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {

				 auto h = (MTUIHandle*) handle;

				 if (command.type == ofPath::Command::bezierTo ||
					 command.type == ofPath::Command::quadBezierTo)
				 {
					 // Mouse coordinate in the handle's frame coordinate system:
					 /* disabled for the moment:
					 auto mouse = toHandle->transformPoint(args, toHandle->getSuperview());

					 glm::vec3 diff = mouse.xyy() - toHandle->getFrameCenter();
					 cp1Handle->shiftFrameOrigin(diff);
					 cp2Handle->shiftFrameOrigin(diff);
					  */
				 }
				 auto uiPathPtr = this->getUIPath().lock();
				 if (uiPathPtr->pathOptionFlags.test(MTUIPath::NotifyOnHandleDragged))
				 {
					 uiPathPtr->pathHandleMovedEvent.notify(this, args);
				 }
			 }, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(toHandle->mouseDraggedEndEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {
//		uiPathPtr->pathChangedEvent.notify(this->uiPath);
				 auto uiPathPtr = this->getUIPath().lock();
				 uiPathPtr->pathHandleMovedEvent.notify(this, args);
			 }, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(toHandle->mousePressedEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {
				 auto uiPathPtr = this->getUIPath().lock();
				 uiPathPtr->handlePressed(this, args);
				 uiPathPtr->pathHandlePressedEvent.notify(this, args);
			 }, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(toHandle->mouseReleasedEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {
				 auto uiPathPtr = this->getUIPath().lock();
				 uiPathPtr->handleReleased(this, args);
				 uiPathPtr->pathHandleReleasedEvent.notify(this, args);
			 }, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(toHandle->mouseDraggedEvent.newListener
			([this](const void* sender, ofMouseEventArgs& args)
			 {
				 if (command.type == ofPath::Command::bezierTo ||
					 command.type == ofPath::Command::quadBezierTo)
				 {
					 auto uiPathPtr = this->getUIPath().lock();
					 MTView* view = (MTView*) sender;
					 auto parent = view->getSuperview();
					 if (uiPathPtr->pathOptionFlags.test(MTUIPath::LimitToRegion))
					 {
						 auto pos = toHandle->getFrameCenter();

						 if (uiPathPtr->region.inside(args))
						 {
							 auto delta = view->getContentMouse() - view->getContentMouseDown();
							 cp1Handle->setFrameOrigin(cp1Handle->getFrameOrigin() + delta);
							 cp2Handle->setFrameOrigin(cp2Handle->getFrameOrigin() + delta);
						 }
					 }
					 else
					 {
						 auto delta = view->getContentMouse() - view->getContentMouseDragStart();
						 cp1Handle->setFrameOrigin(cp1Handle->getFrameOrigin() + delta);
						 cp2Handle->setFrameOrigin(cp2Handle->getFrameOrigin() + delta);
					 }
				 }
				 updateCommand();
			 }, OF_EVENT_ORDER_AFTER_APP));

	addEventListener(cp1Handle->mouseDraggedEvent.newListener
			([this](ofMouseEventArgs& args)
			 {
				 updateCommand();
			 }, OF_EVENT_ORDER_AFTER_APP));

	addEventListener(cp2Handle->mouseDraggedEvent.newListener
			([this](ofMouseEventArgs& args)
			 {
				 updateCommand();
			 }, OF_EVENT_ORDER_AFTER_APP));

	setControlPoints();

	// This call is to clamp path points to the region
	// at instantiation if necessary.
	updateCommand();
	currentStyle = MTUIPath::vertexHandleStyle;

}

void MTUIPathVertexHandle::setControlPoints()
{
	cp1Handle->setFrameFromCenter(command.cp1,
								  glm::vec2(MTUIPath::cpHandleSize,
											MTUIPath::cpHandleSize));

	addEventListener(cp1Handle->mouseDraggedEvent.newListener(
			[this](const void* handle, ofMouseEventArgs& args)
			{
				auto h = (MTUIHandle*) handle;
				command.cp1 = glm::vec3(args, 0);
				this->updateCommand();
				auto uiPathPtr = uiPath.lock();
				if (uiPathPtr->pathOptionFlags.test(MTUIPath::NotifyOnHandleDragged))
				{
					uiPathPtr->pathHandleMovedEvent.notify(this, args);
				}
			}, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(cp1Handle->mousePressedEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {
				 auto uiPathPtr = uiPath.lock();
				 uiPathPtr->handlePressed(this, args);
				 uiPathPtr->pathHandlePressedEvent.notify(this, args);
			 }, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(cp1Handle->mouseReleasedEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {
				 auto uiPathPtr = uiPath.lock();
				 uiPathPtr->handleReleased(this, args);
				 uiPathPtr->pathHandleReleasedEvent.notify(this, args);
			 }, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(cp1Handle->mouseDraggedEndEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {
				 auto uiPathPtr = uiPath.lock();
				 uiPathPtr->pathHandleMovedEvent.notify(this, args);
			 }, OF_EVENT_ORDER_BEFORE_APP));

	cp2Handle->setFrameFromCenter(command.cp2,
								  glm::vec2(MTUIPath::cpHandleSize,
											MTUIPath::cpHandleSize));

	addEventListener(cp2Handle->mouseDraggedEvent.newListener(
			[this](const void* handle, ofMouseEventArgs& args)
			{
				auto h = (MTUIHandle*) handle;
				command.cp2 = glm::vec3(args, 0);
				this->updateCommand();
				auto uiPathPtr = uiPath.lock();
				if (uiPathPtr->pathOptionFlags.test(MTUIPath::NotifyOnHandleDragged))
				{
					uiPathPtr->pathHandleMovedEvent.notify(this, args);
				}
			}, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(cp2Handle->mousePressedEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {
				 auto uiPathPtr = uiPath.lock();
				 uiPathPtr->handlePressed(this, args);
				 uiPathPtr->pathHandlePressedEvent.notify(this, args);
			 }, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(cp2Handle->mouseReleasedEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {
				 auto uiPathPtr = uiPath.lock();
				 uiPathPtr->handleReleased(this, args);
				 uiPathPtr->pathHandleReleasedEvent.notify(this, args);
			 }, OF_EVENT_ORDER_BEFORE_APP));

	addEventListener(cp2Handle->mouseDraggedEndEvent.newListener
			([this](const void* handle, ofMouseEventArgs& args)
			 {
				 auto uiPathPtr = uiPath.lock();
				 uiPathPtr->pathHandleMovedEvent.notify(this, args);
//                 uiPathPtr->pathChangedEvent.notify(this->uiPath);
			 }, OF_EVENT_ORDER_BEFORE_APP));
}

void MTUIPathVertexHandle::updateCommand()
{
	if (command.type == ofPath::Command::bezierTo ||
		command.type == ofPath::Command::quadBezierTo)
	{
		command.cp1 = cp1Handle->getFrameCenter();
		command.cp2 = cp2Handle->getFrameCenter();
	}
	auto uiPathPtr = uiPath.lock();

	if (uiPathPtr->pathOptionFlags.test(MTUIPath::LimitToRegion))
	{
		auto pos = toHandle->getFrameCenter();
		toHandle->setFrameCenter(glm::vec2(ofClamp(pos.x,
												   uiPathPtr->region.getMinX(),
												   uiPathPtr->region.getMaxX()),
										   ofClamp(pos.y,
												   uiPathPtr->region.getMinY(),
												   uiPathPtr->region.getMaxY())));
	}


	command.to = toHandle->getFrameCenter();
	uiPathPtr->updatePath();
}

void MTUIPathVertexHandle::setStyle(ofStyle newStyle)
{
	currentStyle = newStyle;
}


void MTUIPathVertexHandle::draw()
{
//    ofPushStyle();
////    ofSetStyle(currentStyle);
//    if (command->type == ofPath::Command::bezierTo || command->type == ofPath::Command::quadBezierTo)
//    {
//        //		ofDrawLine(*toHandle->getRepresentedObject(), *cp1Handle->getRepresentedObject());
//        //		ofDrawLine(*toHandle->getRepresentedObject(), *cp2Handle->getRepresentedObject());
//        cp1Handle->draw();
//        cp2Handle->draw();
//    }
//    toHandle->draw();
//
//    ofPopStyle();
}

#pragma mark MTUIHandle

MTUIHandle::MTUIHandle(std::string _name) : MTView(_name)
{
	wantsFocus = false;
}

void MTUIHandle::draw()
{
	ofSetColor(255);
	ofFill();
	ofDrawRectangle(0, 0,
					getFrameSize().x,
					getFrameSize().y);

	ofPushMatrix();
	ofLoadIdentityMatrix();
	ofSetColor(ofColor::yellow, 50);
	ofSetLineWidth(1);
	ofDrawRectangle(getScreenFrame());
	ofPopMatrix();
}

void MTUIHandle::mouseDragged(int x, int y, int button)
{
	if (button == 0)
	{
		setFrameOrigin(getFrameOrigin() +
					   (getContentMouse() - getContentMouseDown()));
	}
}

void MTUIHandle::superviewContentChanged()
{
	scaleToScreen();
}

/**
 * @brief Resizes the handle so that its size appears consistent regardless of the
 * scale (zoom) of its superview(s)
 */
void MTUIHandle::scaleToScreen()
{
	auto su = superview.lock();
	auto invMatrix = getInvFrameMatrix();
	/*
	 * http://www.c-jump.com/bcc/common/Talk3/Math/GLM/W01_0100_scaling_matrix_exampl.htm
	 *     m4[0].x = scale.x;
    m4[1].y = scale.y;
    m4[2].z = scale.z;
	 */

	// Multiply the wanted screen size by the x and y components of the scale portion
	// of the inverse matrix. That gets us the size that we need to render the handle at
	// in order for it to appear at the right size.
	auto sx = MTUIPath::vertexHandleSize * (invMatrix[0].x);
	auto sy = MTUIPath::vertexHandleSize * (invMatrix[1].y);

	// We need to recenter the frame, since scaling occurs from corner:
	auto center = getFrameCenter();
	setFrameFromCenter(glm::vec2(center.x, center.y), glm::vec2(sx, sy));
}

MTUIHandle::HandleState MTUIHandle::getState()
{
	return state;
}

void MTUIHandle::setState(MTUIHandle::HandleState newState)
{
	state = newState;
}

void MTUIHandle::setup()
{
	scaleToScreen();
}

void MTUIHandle::setHandleStyleForState(MTUIHandle::HandleStyle style, MTUIHandle::HandleState state)
{

}




