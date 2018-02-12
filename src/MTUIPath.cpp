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
    for (auto handle : pathVertices)
    {
        handle->getPointHandle()->removeFromSuperview();
        handle->getCP1Handle()->removeFromSuperview();
        handle->getCP2Handle()->removeFromSuperview();
    }
    pathVertices.clear();
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
    pathVertices.clear();
    selectedVertices.clear();
    this->view = view;
    pathOptionFlags = std::bitset<4>(options);
    if (isClosed) arrangeClosedPath();
    addEventListeners();

    //Create handles for verts
    for (int i = 0; i < p->getCommands().size(); i++)
    {
        auto command = &p->getCommands()[i];

        // Do not add handles for close commands:
        if (command->type == ofPath::Command::close)
        {
            continue;
        }

        auto vertex = std::shared_ptr<MTUIPathVertex>(new MTUIPathVertex());
        vertex->setup(this, command);
        pathVertices.push_back(vertex);
    }

    updatePath();
}

void MTUIPath::updatePath()
{
    auto commands = path->getCommands();
    midpoints.clear();

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

void MTUIPath::arrangeClosedPath()
{
    auto commands = path->getCommands();

    // If the path is not empty, and the last command is not close, and the first command isn't close
    // either, take the coords of the first command.
    // Otherwise, position is 0,0
    if (commands.size() > 0)
    {
        if (commands.back().type == ofPath::Command::close)
        {
            return;
        }
        else
        {
            auto frontCommand = path->getCommands().front();
            if (frontCommand.type != ofPath::Command::close)
            {
                ofPoint pos = ofPoint(path->getCommands().front().to);
                auto command = ofPath::Command(ofPath::Command::close, pos);
                path->getCommands().push_back(std::move(command));
            }
        }
    }
    else
    {
        ofPoint pos = ofPoint(0, 0);
        auto command = ofPath::Command(ofPath::Command::close, pos);
        path->getCommands().push_back(std::move(command));
    }
}

void MTUIPath::arrangeOpenPath()
{
    if (path->getCommands().back().type == ofPath::Command::close)
    {
        path->getCommands().pop_back();
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
        arrangeClosedPath();

    }
    else
    {
        arrangeOpenPath();
    }

    //I think that this updates the path:
    path->getCommands();
    setup(path, view, (unsigned int) pathOptionFlags.to_ulong()); // Necessary??? TODO: Check
}

///The actual drawing method
void MTUIPath::draw()
{
    if (isVisible)
    {
        path->draw(0, 0);

        auto previous = pathVertices.back();

        for (int i = 0; i < pathVertices.size(); i++)
        {
            auto handle = pathVertices[i];
            if (handle->getCommand()->type == ofPathCommand::bezierTo)
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

void MTUIPath::handlePressed(MTUIPathVertex* handle, ofMouseEventArgs& args)
{
    if (args.button == 0)
    {
        auto it = std::find_if(pathVertices.begin(),
                               pathVertices.end(),
                               [&](std::shared_ptr<MTUIPathVertex> const& current)
                               {
                                   return current.get() == handle;
                               });

        // Transform lineTo <-> bezierTo
#pragma mark VERTEX TRANSFORM
        if (ofGetKeyPressed(OF_KEY_SHIFT) &&
            pathOptionFlags.test(CanConvertPoints))
        {
            // Don't transform the first vertex, it has to be moveTo
            if (it != pathVertices.begin())
            {
                ofPath::Command* command = (*it)->getCommand();
                if (command->type == ofPath::Command::lineTo)
                {
                    auto polyline = path->getOutline()[0];
                    unsigned int polyIndex;
                    auto tangent = polyline.getClosestPoint(command->to, &polyIndex);

                    auto toHandle = (*it)->getPointHandle();
                    auto cp1 = (*it)->getCP1Handle();
                    auto tNorm = glm::normalize(tangent);
                    cp1->setFrameOrigin(toHandle->getFrameOrigin() - (tNorm * 50.0f));

                    auto cp2 = (*it)->getCP2Handle();
                    cp2->setFrameOrigin(toHandle->getFrameOrigin() + (tNorm * 50.0f));
                    command->type = ofPath::Command::bezierTo;
//                    (*it)->getCP1Handle()->setFrameOrigin(command->to.x, command->to.y);
//                    (*it)->getCP2Handle()->setFrameOrigin(command->to.x, command->to.y);
                    view->addSubview(cp1);
                    view->addSubview(cp2);
                    this->pathChangedEvent.notify(this);
                }
                else if ((*it)->getCommand()->type == ofPath::Command::bezierTo)
                {
                    (*it)->getCommand()->type = ofPath::Command::lineTo;
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

void MTUIPath::handleReleased(MTUIPathVertex* handle, ofMouseEventArgs& args)
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
        closestMidpoint = getClosestMidpoint(args.xyy());
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

bool MTUIPath::deleteHandle(std::shared_ptr<MTUIPathVertex> handle)
{

    bool success = false;

    // First we deal with the ofPath:
    auto commands = handle->getPath()->getCommands();

    int index = 0;
    for (; index < commands.size(); index++)
    {
        ofPath::Command* com = &commands[index];
        if (com == handle->getCommand())
        {
            success = true;
            break;
        }
    }

    if (success)
    {
        // If we are deleting the first command
        // there are some things to check before we decide
        // what to do:
        if (index == 0)
        {
            // If the path has more than 1 command...
            if (commands.size() > 1)
            {
                // And if the next command is "close"... then it is a closed path and we should delete both commands
                if (commands.at(index + 1).type == ofPath::Command::close)
                {
                    commands.clear();
                }
                else // If it isn't "close"... then set the next command's type to moveTo and erase the current command.
                    // This is necessary because ofPaths need to start with a moveTo command.
                {
                    commands.at(index + 1).type = ofPath::Command::moveTo;
                    commands.erase(commands.begin());
                }

            }
            else // If the path has just one command, delete it:
            {
                commands.clear();
            }
        }
        else // When index is greater than 0 it is simpler:
        {
            commands.erase(commands.begin() + index);
        }
    }

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

    if (success)
    {
        auto commands = path->getCommands();

        // Check to see if the last command has been deleted:
        if ((commands.size() == 0) || (commands.size() == 1 && commands.back().type == ofPath::Command::close))
        {
            // If so, notify listeners.
            lastHandleDeletedEvent.notify(this);
        }
        else
        {
            setup(path, view, (unsigned int) pathOptionFlags.to_ulong());
            pathChangedEvent.notify(this);
            if (selectsLastInsertion)
            {
                setSelection(pathVertices.back());
            }
        }
    }

    return success;
}

void MTUIPath::deleteSelected()
{
    for (auto handle : selectedVertices)
    {
        deleteHandle(handle);
    }

    deselectAll();
}

MTUIPath::Midpoint& MTUIPath::getClosestMidpoint(const glm::vec3& point)
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

void MTUIPath::addCommand(ofPath::Command& command)
{
    if (!pathOptionFlags.test(CanAddPoints)) return;

    if (path->getCommands().size() < 3)
    {
        insertCommand(command, path->getCommands().size());
    }
    else
    {
        // Check to see if we are in the last midpoint.
        // index2 should be 0 in the last one:
        if (closestMidpoint.index2 == 0)
        {
            insertCommand(command, closestMidpoint.index1 + 1);
        }
        else
        {
            insertCommand(command, closestMidpoint.index2);
        }
    }
}

void MTUIPath::insertCommand(ofPath::Command& command, int index)
{
    if (!pathOptionFlags.test(CanAddPoints)) return;

    if (index >= path->getCommands().size())
    {
        // if the path is closed we need to add the point second-to-last
        if (isClosed)
        {
            auto pos = path->getCommands().end() - 1;
            path->getCommands().insert(pos, std::move(command));
        }
        else
        {
            path->getCommands().push_back(std::move(command));
        }
    }
    else
    {
        path->getCommands().insert(path->getCommands().begin() + index,
                                   std::move(command));
    }

    //	auto vertexHandle = shared_ptr<MTUIPathHandle> (new MTUIPathHandle());
    //	vertexHandle->setup(this, &path->getCommands()[index]);
    //	vertexHandles.insert(vertexHandles.begin() + index, vertexHandle);

    setup(path, view, (unsigned int) pathOptionFlags.to_ulong());
    pathChangedEvent.notify(this);

    if (selectsLastInsertion)
    {
        setSelection(pathVertices.back());
    }
}

//SELECTION
/////////////////////////////////

void MTUIPath::addToSelection(std::shared_ptr<MTUIPathVertex> vertex)
{
    selectedVertices.push_back(vertex);
    vertex->setStyle(selectedVextexHandleStyle);
}

void MTUIPath::removeFromSelection(std::shared_ptr<MTUIPathVertex> vertex)
{
    selectedVertices.erase(std::remove_if(selectedVertices.begin(),
                                          selectedVertices.end(),
                                          [&](std::shared_ptr<MTUIPathVertex> const& current)
                                          {
                                              if (current.get() == vertex.get())
                                              {
                                                  return true;
                                              }
                                              return false;
                                          }));

    vertex->setStyle(vertexHandleStyle);

}

void MTUIPath::setSelection(std::shared_ptr<MTUIPathVertex> vertex)
{
    deselectAll();
    addToSelection(vertex);
}

void MTUIPath::deselectAll()
{
    for (auto handle : selectedVertices)
    {
        handle->setStyle(vertexHandleStyle);
    }

    selectedVertices.clear();
}

void MTUIPath::selectAll()
{
    for (auto handle : selectedVertices)
    {
        addToSelection(handle);
    }
}


#pragma mark MTUIPathHandle

//////////////////////////
//MTUIPathHandle
//////////////////////////

MTUIPathVertex::~MTUIPathVertex()
{
    ofLogVerbose("MTUIPathHandle") << "Destroyed";
    toHandle->removeFromSuperview();
    cp1Handle->removeFromSuperview();
    cp2Handle->removeFromSuperview();
}

void MTUIPathVertex::setup(MTUIPath* uiPath, ofPath::Command* com)
{
    this->uiPath = uiPath;
    command = com;
    clearEventListeners();

    toHandle = std::make_shared<MTUIHandle>("To Handle");
    toHandle->setFrameFromCenter(command->to,
                                 glm::vec2(MTUIPath::vertexHandleSize,
                                           MTUIPath::vertexHandleSize));

    cp1Handle = std::make_shared<MTUIHandle>("CP 1");
    cp1Handle->setFrameFromCenter(command->cp1,
                                  glm::vec2(MTUIPath::vertexHandleSize,
                                            MTUIPath::vertexHandleSize));
    //    cp1Handle->handleType = ofxMTHandle<ofPoint>::ControlPointHandleType;

    cp2Handle = std::make_shared<MTUIHandle>("CP 2");
    cp2Handle->setFrameFromCenter(command->cp2,
                                  glm::vec2(MTUIPath::vertexHandleSize,
                                            MTUIPath::vertexHandleSize));
    //    cp2Handle->handleType = MTUIHandle::HandleType::SQUARE;

    uiPath->view->addSubview(toHandle);

    if (command->type == ofPath::Command::bezierTo ||
        command->type == ofPath::Command::quadBezierTo)
    {
        uiPath->view->addSubview(cp1Handle);
        uiPath->view->addSubview(cp2Handle);
    }

    addEventListener(toHandle->mouseDraggedEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {

                 auto h = (MTUIHandle*) handle;

                 if (command->type == ofPath::Command::bezierTo ||
                     command->type == ofPath::Command::quadBezierTo)
                 {
                     // Mouse coordinate in the handle's frame coordinate system:
                     /* disabled for the moment:
                     auto mouse = toHandle->transformPoint(args, toHandle->getSuperview());

                     glm::vec3 diff = mouse.xyy() - toHandle->getFrameCenter();
                     cp1Handle->shiftFrameOrigin(diff);
                     cp2Handle->shiftFrameOrigin(diff);
                      */
                 }

//		this->uiPath->pathHandleMovedEvent.notify(this, args);
             }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(toHandle->mouseDraggedEndEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {
//		this->uiPath->pathChangedEvent.notify(this->uiPath);
                 this->uiPath->pathHandleMovedEvent.notify(this, args);
             }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(toHandle->mousePressedEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {
                 this->uiPath->handlePressed(this, args);
                 this->uiPath->pathHandlePressedEvent.notify(this, args);
             }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(toHandle->mouseReleasedEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {
                 this->uiPath->handleReleased(this, args);
                 this->uiPath->pathHandleReleasedEvent.notify(this, args);
             }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(toHandle->mouseDraggedEvent.newListener
            ([this](ofEventArgs& args)
             {
                 updateCommand();
             }, OF_EVENT_ORDER_AFTER_APP));

    addEventListener(cp1Handle->mouseDraggedEvent.newListener
            ([this](ofEventArgs& args)
             {
                 updateCommand();
             }, OF_EVENT_ORDER_AFTER_APP));

    addEventListener(cp2Handle->mouseDraggedEvent.newListener
            ([this](ofEventArgs& args)
             {
                 updateCommand();
             }, OF_EVENT_ORDER_AFTER_APP));

    setControlPoints();

    // This call is to clamp path points to the region
    // at instantiation if necessary.
    updateCommand();
    currentStyle = MTUIPath::vertexHandleStyle;

}

void MTUIPathVertex::setControlPoints()
{
    cp1Handle->setFrameFromCenter(command->cp1,
                                  glm::vec2(MTUIPath::cpHandleSize,
                                            MTUIPath::cpHandleSize));

    addEventListener(cp1Handle->mouseDraggedEvent.newListener(
            [this](const void* handle, ofMouseEventArgs& args)
            {
                auto h = (MTUIHandle*) handle;
                command->cp1 = args.xyx();
                this->updateCommand();
                uiPath->pathHandleMovedEvent.notify(this, args);
            }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp1Handle->mousePressedEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {
                 this->uiPath->handlePressed(this, args);
                 this->uiPath->pathHandlePressedEvent.notify(this, args);
             }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp1Handle->mouseReleasedEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {
                 this->uiPath->handleReleased(this, args);
                 this->uiPath->pathHandleReleasedEvent.notify(this, args);
             }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp1Handle->mouseDraggedEndEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {
                 this->uiPath->pathChangedEvent.notify(this->uiPath);
             }, OF_EVENT_ORDER_BEFORE_APP));

    cp2Handle->setFrameFromCenter(command->cp2,
                                  glm::vec2(MTUIPath::cpHandleSize,
                                            MTUIPath::cpHandleSize));

    addEventListener(cp2Handle->mouseDraggedEvent.newListener(
            [this](const void* handle, ofMouseEventArgs& args)
            {
                auto h = (MTUIHandle*) handle;
                command->cp2 = args.xyx();
                this->updateCommand();
                uiPath->pathHandleMovedEvent.notify(this, args);
            }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp2Handle->mousePressedEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {
                 this->uiPath->handlePressed(this, args);
                 this->uiPath->pathHandlePressedEvent.notify(this, args);
             }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp2Handle->mouseReleasedEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {
                 this->uiPath->handleReleased(this, args);
                 this->uiPath->pathHandleReleasedEvent.notify(this, args);
             }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp2Handle->mouseDraggedEndEvent.newListener
            ([this](const void* handle, ofMouseEventArgs& args)
             {
                 this->uiPath->pathChangedEvent.notify(this->uiPath);
             }, OF_EVENT_ORDER_BEFORE_APP));
}

void MTUIPathVertex::updateCommand()
{
    if (command->type == ofPath::Command::bezierTo ||
        command->type == ofPath::Command::quadBezierTo)
    {
        if (uiPath->pathOptionFlags.test(MTUIPath::LimitToRegion))
        {
            auto pos = cp1Handle->getFrameCenter();
            cp1Handle->setFrameCenter(glm::vec2(ofClamp(pos.x,
                                                        uiPath->region.getMinX(),
                                                        uiPath->region.getMaxX()),
                                                ofClamp(pos.y,
                                                        uiPath->region.getMinY(),
                                                        uiPath->region.getMaxY())));
            pos = cp2Handle->getFrameCenter();
            cp2Handle->setFrameCenter(glm::vec2(ofClamp(pos.x,
                                                        uiPath->region.getMinX(),
                                                        uiPath->region.getMaxX()),
                                                ofClamp(pos.y,
                                                        uiPath->region.getMinY(),
                                                        uiPath->region.getMaxY())));
        }
        command->cp1 = cp1Handle->getFrameCenter();
        command->cp2 = cp2Handle->getFrameCenter();
    }

    if (uiPath->pathOptionFlags.test(MTUIPath::LimitToRegion))
    {
        auto pos = toHandle->getFrameCenter();
        toHandle->setFrameCenter(glm::vec2(ofClamp(pos.x,
                                                   uiPath->region.getMinX(),
                                                   uiPath->region.getMaxX()),
                                           ofClamp(pos.y,
                                                   uiPath->region.getMinY(),
                                                   uiPath->region.getMaxY())));
    }


    command->to = toHandle->getFrameCenter();
    uiPath->updatePath();
}

void MTUIPathVertex::setStyle(ofStyle newStyle)
{
    currentStyle = newStyle;
}

//bool MTUIPathHandle::hitTest(glm::vec2& point)
//{
//    bool success = toHandle->inside(point);
//    if (command->type == ofPath::Command::bezierTo || command->type == ofPath::Command::quadBezierTo)
//    {
//        if (cp1Handle != nullptr)
//        {
//            success = success || cp1Handle->hitTest(point);
//        }

//        if (cp2Handle != nullptr)
//        {
//            success = success || cp2Handle->hitTest(point);
//        }
//    }

//    return success;
//}

void MTUIPathVertex::draw()
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

void MTUIHandle::mousePressed(int x, int y, int button)
{
    MTView::mousePressed(x, y, button);
}

void MTUIHandle::mouseReleased(int x, int y, int button)
{
    MTView::mouseReleased(x, y, button);
}

void MTUIHandle::mouseEntered(int x, int y)
{
    MTView::mouseEntered(x, y);
}

void MTUIHandle::mouseExited(int x, int y)
{
    MTView::mouseExited(x, y);
}

void MTUIHandle::superviewContentChanged()
{
    auto su = superview.lock();
    if (!su) return;

    auto sx = 1.0 / su->getContentScaleX();
    auto sy = 1.0 / su->getContentScaleY();
    setFrameSize(getScreenFrame().getWidth() * sx, getScreenFrame().getHeight() * sy);
}






