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
    ofLogNotice("MTUIPath") << "Destructor";
    if (autoDraw)
    {
        ofRemoveListener(ofEvents().draw, this, &MTUIPath::drawEvent);
    }

}

void MTUIPath::setup(ofPath* p, shared_ptr<MTView> view)
{
    path = p;
    uiHandles.clear();
    selectedHandles.clear();
    this->view = view;

    if (isClosed) arrangeClosedPath();

    //Create handles for verts
    for (int i = 0; i < p->getCommands().size(); i++)
    {
        auto command = &p->getCommands()[i];

        // Do not add handles for close commands:
        if (command->type == ofPath::Command::close)
        {
            continue;
        }

        auto vertexHandle = shared_ptr<MTUIPathHandle> (new MTUIPathHandle());
        vertexHandle->setup(this, command);
        uiHandles.push_back(vertexHandle);
    }

    if (useAutoEventListeners)
    {
        addEventListeners();
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
                auto command  = ofPath::Command(ofPath::Command::close, pos);
                path->getCommands().push_back(std::move(command));
            }
        }
    }
    else
    {
        ofPoint pos = ofPoint(0, 0);
        auto command  = ofPath::Command(ofPath::Command::close, pos);
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
    setup(path, view); // Necessary??? TODO: Check
}

void MTUIPath::addEventListeners()
{
    ofAddListener(ofEvents().mousePressed, this, &MTUIPath::mousePressed);
    ofAddListener(ofEvents().mouseReleased, this, &MTUIPath::mouseReleased);
    ofAddListener(ofEvents().mouseDragged, this, &MTUIPath::mouseDragged);
    ofAddListener(ofEvents().keyPressed, this, &MTUIPath::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &MTUIPath::keyReleased);

    for (auto h : uiHandles)
    {
        h->setAutoEventListeners(true);
    }
}

void MTUIPath::removeEventListeners()
{
    ofRemoveListener(ofEvents().mousePressed, this, &MTUIPath::mousePressed);
    ofRemoveListener(ofEvents().mouseReleased, this, &MTUIPath::mouseReleased);
    ofRemoveListener(ofEvents().mouseDragged, this, &MTUIPath::mouseDragged);
    ofRemoveListener(ofEvents().keyPressed, this, &MTUIPath::keyPressed);
    ofRemoveListener(ofEvents().keyReleased, this, &MTUIPath::keyReleased);

    for (auto h : uiHandles)
    {
        h->setAutoEventListeners(false);
    }
}

void MTUIPath::setUseAutoEventListeners(bool use)
{
    if (!use)
    {
        if (useAutoEventListeners && path != nullptr) //setup has run, need to remove the listeners.
        {
            removeEventListeners();
        }
    }
    else
    {
        if (!useAutoEventListeners && path != nullptr) //setup has run, need to add the listeners again.
        {
            addEventListeners();
        }
    }

    useAutoEventListeners = use;

    //Tell the handles about our event handling preference:
    for (auto handle : uiHandles)
    {
        handle->setAutoEventListeners(useAutoEventListeners);
    }

}

void MTUIPath::setAutoDraw(bool _autoDraw)
{
    autoDraw = _autoDraw;

    if (autoDraw)
    {
        ofAddListener(ofEvents().draw, this, &MTUIPath::drawEvent);
    }
    else //TODO: Check if this works!!
    {
        ofRemoveListener(ofEvents().draw, this, &MTUIPath::drawEvent);
    }
}

///Event handlers
void MTUIPath::drawEvent(ofEventArgs& args)
{
    draw();
}

void MTUIPath::mouseMoved(ofMouseEventArgs &args)
{
    if (!useAutoEventListeners)
    {
        for (auto handle : uiHandles)
        {
            handle->mouseMoved(args);
        }
    }
}
void MTUIPath::mousePressed(ofMouseEventArgs& args)
{
    if (!useAutoEventListeners)
    {
        for (auto handle : uiHandles)
        {
            handle->mousePressed(args);
        }
    }

    bool handlePressed = false;

    for (auto h : uiHandles)
    {
        if(h->hitTest(args))
        {
            handlePressed = true;
            break;
        }
    }

    if (!handlePressed)
    {
        deselectAll();
    }
}

void MTUIPath::mouseReleased(ofMouseEventArgs &args)
{
    if (!useAutoEventListeners)
    {
        for (auto handle : uiHandles)
        {
            handle->mouseReleased(args);
        }
    }
}

void MTUIPath::mouseDragged(ofMouseEventArgs &args)
{
    if (!useAutoEventListeners)
    {
        for (auto handle : uiHandles)
        {
            handle->mouseDragged(args);
        }
    }
}

void MTUIPath::keyPressed(ofKeyEventArgs &args)
{
    if (!useAutoEventListeners)
    {
        for (auto handle : uiHandles)
        {
            handle->keyPressed(args);
        }
    }
}

void MTUIPath::keyReleased(ofKeyEventArgs& args)
{
    if (!useAutoEventListeners)
    {
        for (auto handle : uiHandles)
        {
            handle->keyReleased(args);
        }
    }

    if(args.key == OF_KEY_DEL || args.key == OF_KEY_BACKSPACE)
    {
        deleteSelected();
    }
}

///The actual drawing method
void MTUIPath::draw()
{
    if (isVisible)
    {
        path->draw();

        auto previous = uiHandles.back();

        for (int i = 0; i < uiHandles.size(); i++)
        {
            auto handle = uiHandles[i];
            if (handle->getCommand()->type == ofPathCommand::bezierTo)
            {
                ofDrawLine(handle->getCP2Handle()->getFrameCenter(),
                           handle->getPointHandle()->getFrameCenter());

                ofDrawLine(handle->getCP1Handle()->getFrameCenter(),
                           previous->getPointHandle()->getFrameCenter());
            }

            handle->draw();
            previous = handle;
        }
    }
}

void MTUIPath::handlePressed(MTUIPathHandle* handle, ofMouseEventArgs &args)
{
    if (args.button == 0)
    {
        auto it = std::find_if(uiHandles.begin(), uiHandles.end(), [&](shared_ptr<MTUIPathHandle> const& current) {
            return current.get() == handle;
        });

        if (ofGetKeyPressed(OF_KEY_SHIFT))
        {
            // Don't transform the first vertex, it has to be moveTo
            if (it != uiHandles.begin())
            {
                ofPath::Command* command = (*it)->getCommand();
                if(command->type == ofPath::Command::lineTo)
                {
                    command->type = ofPath::Command::bezierTo;
                    (*it)->getCP1Handle()->setFrameOrigin(command->to.x, command->to.y);
                    (*it)->getCP2Handle()->setFrameOrigin(command->to.x, command->to.y);
                    this->pathChangedEvent.notify(this);
                }
                else if((*it)->getCommand()->type == ofPath::Command::bezierTo)
                {
                    (*it)->getCommand()->type = ofPath::Command::lineTo;
                    this->pathChangedEvent.notify(this);
                }
            }
        }
        if (ofGetKeyPressed(OF_KEY_COMMAND))
        {
            addToSelection(*it);
        }
        else
        {
            setSelection(*it);
        }
    }
}

void MTUIPath::handleReleased(MTUIPathHandle* handle, ofMouseEventArgs &args)
{

}

//DATA HANDLING
/////////////////////////////////

bool MTUIPath::deleteHandle(shared_ptr<MTUIPathHandle> handle)
{

    bool success = false;

    // First we deal with the ofPath:
    vector<ofPath::Command>& commands = const_cast<vector<ofPath::Command>&>(handle->getPath()->getCommands());  // <-- Sometimes I hate this language

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
        // If we are deleting the first command there are some things to check before we decide
        // what to do:
        if (index == 0)
        {
            // If the path has more than 1 command...
            if (commands.size() > 1)
            {
                // And if the next command is "close"... then it is a closed path and we should delete both commands
                if (commands.at(index+1).type == ofPath::Command::close)
                {
                    commands.clear();
                }
                else // If it isn't "close"... then set the next command's type to moveTo and erase the current command.
                     // This is necessary because ofPaths need to start with a moveTo command.
                {
                    commands.at(index+1).type = ofPath::Command::moveTo;
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
        if ( (commands.size() == 0) || (commands.size() == 1 && commands.back().type == ofPath::Command::close))
        {
            // If so, notify listeners.
            lastHandleDeletedEvent.notify(this);
        }
        else
        {
            setup(path, view);
            pathChangedEvent.notify(this);
            if (selectsLastInsertion)
            {
                setSelection(uiHandles.back());
            }
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

void MTUIPath::addCommand(ofPath::Command &command)
{
    if (isClosed)
    {
        insertCommand(command, path->getCommands().size()-1);
    }
    else
    {
        insertCommand(command, path->getCommands().size());
    }
}

void MTUIPath::insertCommand(ofPath::Command &command, int index)
{
    if (index >= path->getCommands().size())
    {
        index = path->getCommands().size();
        path->getCommands().push_back(std::move(command));

    }
    else
    {
        path->getCommands().insert(path->getCommands().begin() + index,
                               std::move(command));
    }

//	auto vertexHandle = shared_ptr<MTUIPathHandle> (new MTUIPathHandle());
//	vertexHandle->setup(this, &path->getCommands()[index]);
//	vertexHandles.insert(vertexHandles.begin() + index, vertexHandle);

    setup(path, view);
    pathChangedEvent.notify(this);  //TODO: Should "this" refer to a vertex handle here??

    if (selectsLastInsertion)
    {
        setSelection(uiHandles.back());
    }
}

//SELECTION
/////////////////////////////////

void MTUIPath::addToSelection(shared_ptr<MTUIPathHandle> handle)
{
    selectedHandles.push_back(handle);
    handle->setStyle(selectedVextexHandleStyle);
}

void MTUIPath::removeFromSelection(shared_ptr<MTUIPathHandle> handle)
{
    selectedHandles.erase(std::remove_if(selectedHandles.begin(), selectedHandles.end(), [&](shared_ptr<MTUIPathHandle> const& current) {
                              if (current.get() == handle.get())
                              {
                                  return true;
                              }
                              return false;
                          }));

    handle->setStyle(vertexHandleStyle);

}

void MTUIPath::setSelection(shared_ptr<MTUIPathHandle> handle)
{
    deselectAll();
    addToSelection(handle);
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



//////////////////////////
//MTUIPathHandle
//////////////////////////

void MTUIPathHandle::setup(MTUIPath* uiPath, ofPath::Command* com)
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



    //Should events be class members instead of statics??
    //Why yes, yes they should
    addEventListener(toHandle->handleMovedEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {

                                  auto h = (ofxMTHandle<glm::vec3>*) handle;

                                  if (command->type == ofPathCommand::bezierTo ||
                                      command->type == ofPathCommand::quadBezierTo)
                                  {
                                      ofPoint diff = args - ofVec2f(*h->getRepresentedObject());
                                      ofLogVerbose() << diff;
//									  cp1Handle->moveHandleBy(diff.x, diff.y);
//									  cp2Handle->moveHandleBy(diff.x, diff.y);
//									  cp1Handle->getRepresentedObject()->set(cp1Handle->getX(), cp1Handle->getY());
//									  cp2Handle->getRepresentedObject()->set(cp2Handle->getX(), cp2Handle->getY());
                                  }

                                  h->getRepresentedObject()->x = args.x;
                                  h->getRepresentedObject()->y = args.y;
                                  this->path->getCommands();
                                  this->uiPath->pathHandleMovedEvent.notify(this, args);
                              }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(toHandle->handleMoveEndEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {
                                  this->uiPath->pathChangedEvent.notify(this->uiPath);
                              }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(toHandle->handlePressedEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {
                                  this->uiPath->handlePressed(this, args);
                                  this->uiPath->pathHandlePressedEvent.notify(this, args);
                              }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(toHandle->handleReleasedEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {
                                  this->uiPath->handleReleased(this, args);
                                  this->uiPath->pathHandleReleasedEvent.notify(this, args);
                              }, OF_EVENT_ORDER_BEFORE_APP));

//	if (command->type == ofPathCommand::bezierTo || command->type == ofPathCommand::quadBezierTo)
//	{
        setControlPoints();
//	}

    currentStyle = MTUIPath::vertexHandleStyle;
    setAutoEventListeners(uiPath->getUseAutoEventListeners());
}

void MTUIPathHandle::setControlPoints()
{
    cp1Handle->setRepresentedObject(&command->cp1);
    cp1Handle->setFromCenter(command->cp1, MTUIPath::cpHandleSize, MTUIPath::cpHandleSize);
    addEventListener(cp1Handle->handleMovedEvent.newListener([this](const void* handle, ofMouseEventArgs &args)
                                                                     {
                                                                         auto h = (ofxMTHandle<glm::vec3>*) handle;
                                                                         if (h != cp1Handle.get()) return;

                                                                         command->cp1.x = args.x;
                                                                         command->cp1.y = args.y;
                                                                         path->getCommands();
                                                                         uiPath->pathHandleMovedEvent.notify(this, args);
                                                                     }, OF_EVENT_ORDER_BEFORE_APP));

    cp2Handle->setRepresentedObject(&command->cp2);
    cp2Handle->setFromCenter(command->cp2, MTUIPath::cpHandleSize, MTUIPath::cpHandleSize);
    addEventListener(cp2Handle->handleMovedEvent.newListener([this](const void* handle, ofMouseEventArgs &args)
                                                                     {
                                                                         auto h = (ofxMTHandle<glm::vec3>*) handle;
                                                                         if (h != cp2Handle.get()) return;

                                                                         command->cp2.x = args.x;
                                                                         command->cp2.y = args.y;
                                                                         path->getCommands();
                                                                         uiPath->pathHandleMovedEvent.notify(this, args);
                                                                     }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp1Handle->handlePressedEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {
                                  this->uiPath->handlePressed(this, args);
                                  this->uiPath->pathHandlePressedEvent.notify(this, args);
                              }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp1Handle->handleReleasedEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {
                                  this->uiPath->handleReleased(this, args);
                                  this->uiPath->pathHandleReleasedEvent.notify(this, args);
                              }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp1Handle->handleMoveEndEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {
                                  this->uiPath->pathChangedEvent.notify(this->uiPath);
                              }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp2Handle->handlePressedEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {
                                  this->uiPath->handlePressed(this, args);
                                  this->uiPath->pathHandlePressedEvent.notify(this, args);
                              }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp2Handle->handleReleasedEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {
                                  this->uiPath->handleReleased(this, args);
                                  this->uiPath->pathHandleReleasedEvent.notify(this, args);
                              }, OF_EVENT_ORDER_BEFORE_APP));

    addEventListener(cp2Handle->handleMoveEndEvent.newListener
                             ([this](const void* handle, ofMouseEventArgs &args)
                              {
                                  this->uiPath->pathChangedEvent.notify(this->uiPath);
                              }, OF_EVENT_ORDER_BEFORE_APP));
}

void MTUIPathHandle::setAutoEventListeners(bool useAutoEvents)
{
    if (useAutoEvents)
    {
        toHandle->enableMouseEvents();
        toHandle->enableKeyEvents();
        cp1Handle->enableMouseEvents();
        cp1Handle->enableKeyEvents();
        cp2Handle->enableMouseEvents();
        cp2Handle->enableKeyEvents();

    } else
    {
        toHandle->disableMouseEvents();
        toHandle->disableKeyEvents();
        cp1Handle->disableMouseEvents();
        cp1Handle->disableKeyEvents();
        cp2Handle->disableMouseEvents();
        cp2Handle->disableKeyEvents();
    }
    useAutoEventListeners = useAutoEvents;
}

void MTUIPathHandle::mouseMoved(ofMouseEventArgs& args)
{
    if (command->type == ofPathCommand::bezierTo || command->type == ofPathCommand::quadBezierTo)
    {
        cp1Handle->_mouseMoved(args);
        cp2Handle->_mouseMoved(args);
    }
    toHandle->_mouseMoved(args);
}

void MTUIPathHandle::mousePressed(ofMouseEventArgs& args)
{
    if (command->type == ofPathCommand::bezierTo || command->type == ofPathCommand::quadBezierTo)
    {
        cp1Handle->_mousePressed(args);
        cp2Handle->_mousePressed(args);
    }
    toHandle->_mousePressed(args);
}

void MTUIPathHandle::mouseReleased(ofMouseEventArgs& args)
{
    if (command->type == ofPathCommand::bezierTo || command->type == ofPathCommand::quadBezierTo)
    {
        cp1Handle->_mouseReleased(args);
        cp2Handle->_mouseReleased(args);
    }
    toHandle->_mouseReleased(args);
}

void MTUIPathHandle::mouseDragged(ofMouseEventArgs& args)
{
    if (command->type == ofPathCommand::bezierTo || command->type == ofPathCommand::quadBezierTo)
    {
        cp1Handle->_mouseDragged(args);
        cp2Handle->_mouseDragged(args);
    }
    toHandle->_mouseDragged(args);

}

void MTUIPathHandle::keyPressed(ofKeyEventArgs& args)
{
    toHandle->_keyPressed(args);
    if (command->type == ofPathCommand::bezierTo || command->type == ofPathCommand::quadBezierTo)
    {
        cp1Handle->_keyPressed(args);
        cp2Handle->_keyPressed(args);
    }
}

void MTUIPathHandle::keyReleased(ofKeyEventArgs& args)
{
    toHandle->_keyReleased(args);
    if (command->type == ofPathCommand::bezierTo || command->type == ofPathCommand::quadBezierTo)
    {
        cp1Handle->_keyReleased(args);
        cp2Handle->_keyReleased(args);
    }
}

void MTUIPathHandle::setStyle(ofStyle newStyle)
{
    currentStyle = newStyle;
}

bool MTUIPathHandle::hitTest(glm::vec2& point)
{
    bool success = toHandle->inside(point);
    if (command->type == ofPathCommand::bezierTo || command->type == ofPathCommand::quadBezierTo)
    {
        if (cp1Handle != nullptr)
        {
            success = success || cp1Handle->inside(point);
        }

        if (cp2Handle != nullptr)
        {
            success = success || cp2Handle->inside(point);
        }
    }

    return success;
}

void MTUIPathHandle::draw()
{
    ofPushStyle();
    ofSetStyle(currentStyle);
    if (command->type == ofPathCommand::bezierTo || command->type == ofPathCommand::quadBezierTo)
    {
//		ofDrawLine(*toHandle->getRepresentedObject(), *cp1Handle->getRepresentedObject());
//		ofDrawLine(*toHandle->getRepresentedObject(), *cp2Handle->getRepresentedObject());
        cp1Handle->draw();
        cp2Handle->draw();
    }
    toHandle->draw();

    ofPopStyle();
}
