
//
//  MTAppModePathEditor.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 09/14/2017.
//
//

#include "MTAppModePathEditor.hpp"
#include "MTUIPath.hpp"

MTAppModePathEditor::MTAppModePathEditor(PathEditorSettings& settings)
        : MTAppMode(settings.appModeName, settings.view)
{
    addAllEventListeners();
    this->settings = settings;
//    onFirstPathCreated = [](PathEditorEventArgs args)
//    {};
//    onPathCreated = [](PathEditorEventArgs args)
//    {};
//    onPathModified = [](PathEditorEventArgs args)
//    {};
//    onPathDeleted = [](PathEditorEventArgs args)
//    {};
//    onLastPathDeleted = []()
//    {};

    if (settings.options.test(PathEditorSettings::LimitToRegion))
    {
        if (settings.validRegion.getWidth() == 0)
        {
            ofLogError("MTAppModePathEditor") << "Settings specify LimitToRegion, but no region was passed. "
                                              << "Using view as region";
        }
    }
    if (settings.options.test(PathEditorSettings::LimitToView))
    {
        this->settings.options.set(PathEditorSettings::LimitToRegion);
        this->settings.validRegion = settings.view->getContent();
    }
}

void MTAppModePathEditor::setup()
{

    ofLogVerbose("MTAppModePathEditor::setup") << settings.appModeName;
    MTUIPath::vertexHandleStyle.bFill = false;
    MTUIPath::selectedVextexHandleStyle.bFill = true;

    if (settings.path == nullptr)
    {
        for (int j = 0; j < settings.paths->size(); j++)
        {
            auto uiPath = createUIPath(settings.paths->at(j));
            activeUIPath = uiPath;
        }
    }
    else
    {
        auto uiPath = createUIPath(settings.path);
        activeUIPath = uiPath;
    }

    view->enqueueUpdateOperation([this]()
                                 { ofShowCursor(); });
}

std::shared_ptr<MTUIPath> MTAppModePathEditor::createUIPath(
        std::shared_ptr<ofPath> p)
{
    // There has to be a less stupid way of doing this...
    // I feel that OR'd flags would be simpler....
    auto uiPath = std::make_shared<MTUIPath>();
    std::bitset<4> uiPathOptions;

    if (settings.options.test(PathEditorSettings::LimitToRegion))
    {
        uiPathOptions.set(MTUIPath::LimitToRegion);
        uiPath->setRegion(settings.validRegion);
    }
    if (settings.options.test(PathEditorSettings::CanAddPaths))
    {
        uiPathOptions.set(MTUIPath::CanAddPoints);
    }
    if (settings.options.test(PathEditorSettings::CanDeletePoints))
    {
        uiPathOptions.set(MTUIPath::CanDeletePoints);
    }
    if (settings.options.test(PathEditorSettings::CanConvertPoints))
    {
        uiPathOptions.set(MTUIPath::CanConvertPoints);
    }
    if (settings.options.test(PathEditorSettings::PathsAreClosed))
    {
        uiPath->setClosed(true);
    }

    uiPath->setup(p, view, (unsigned int) uiPathOptions.to_ulong());

    uiPath->getPath()->setColor(settings.pathColor);
    uiPath->getPath()->setFilled(false);
    uiPath->getPath()->setStrokeWidth(settings.pathStrokeWidth);

#pragma mark MTAppModePathEditor listeners

    addEventListener(uiPath->pathChangedEvent.newListener(
            [this](const void* theUiPath)
            {
                auto up = *(std::shared_ptr<MTUIPath>*) theUiPath;
                pEventArgs.path = up->getPath();
                pathModifiedEvent.notify(pEventArgs);
                onPathModified(pEventArgs);
            },
            OF_EVENT_ORDER_AFTER_APP));

    addEventListener(uiPath->pathHandlePressedEvent.newListener(
            [this](const void* handle, ofMouseEventArgs& args)
            {
                //                                  handleWasPressed = true;
                auto h = (MTUIPathVertex*) handle;
                activeUIPath = h->getUIPath()->shared_from_this();
            },
            OF_EVENT_ORDER_AFTER_APP));

    addEventListener(uiPath->pathHandleMovedEvent.newListener(
            [this](const void* handle, ofMouseEventArgs& args)
            {
                auto h = (MTUIPathVertex*) handle;
                pEventArgs.path = h->getUIPath()->getPath();
                pathModifiedEvent.notify(pEventArgs);
                onPathModified(pEventArgs);
            },
            OF_EVENT_ORDER_AFTER_APP));

    addEventListener(uiPath->lastHandleDeletedEvent.newListener(
            [this](const void* theUiPath)
            {
                auto up = *(std::shared_ptr<MTUIPath>*) theUiPath;
                removeUIPath(up);
                pEventArgs.path = up->getPath();
                pathDeletedEvent.notify(pEventArgs);
                onPathDeleted(pEventArgs);
            },
            OF_EVENT_ORDER_AFTER_APP));

    uiPaths.push_back(uiPath);
    return uiPath;
}

bool MTAppModePathEditor::removeUIPath(std::shared_ptr<MTUIPath> p)
{
    auto iter = std::find(uiPaths.begin(), uiPaths.end(), p);
    if (iter != uiPaths.end())
    {

        auto path = p->getPath();
        auto iterOfPath = std::find_if(settings.paths->begin(),
                                       settings.paths->end(),
                                       [&](std::shared_ptr<ofPath> const& current)
                                       {
                                           return current == path;
                                       });

        if (iterOfPath != settings.paths->end())
        {
            // Clear the path first:
            (*iterOfPath)->clear();

            // Now remove the path from the model:
            settings.paths->erase(iterOfPath);
            activeUIPath = nullptr;
        }

        uiPaths.erase(iter);
        pEventArgs.path = path;
        pathDeletedEvent.notify(pEventArgs);
        onPathDeleted(pEventArgs);

        if (uiPaths.size() == 0)
        {
            ofEventArgs args;
            lastPathDeletedEvent.notify(args);
            onLastPathDeleted();
        }
        return true;
    }

    return false;
}

void MTAppModePathEditor::mouseReleased(int x, int y, int button)
{
    if (button == 0)
    {
        //        if (!handleWasPressed)
        //        {
        if (ofGetKeyPressed(OF_KEY_SHIFT))
        {
            if (activeUIPath != nullptr &&
                settings.options.test(PathEditorSettings::PathEditorOptions::CanAddPoints))
            {
                auto command =
                        ofPath::Command(ofPath::Command::lineTo, glm::vec3(x, y, 0));
                activeUIPath->addCommand(command);
            }
            else
            {
                if (settings.options.test(PathEditorSettings::PathEditorOptions::AllowsMultiplePaths))
                {
                    auto pathPtr = std::shared_ptr<ofPath>(new ofPath);
                    activeUIPath = createUIPath(pathPtr);
                    auto command =
                            ofPath::Command(ofPath::Command::moveTo, glm::vec3(x, y, 0));
                    activeUIPath->addCommand(command);
                    settings.paths->push_back(pathPtr);
                    pEventArgs.path = pathPtr;
                    pathCreatedEvent.notify(pEventArgs);
                    ofLogVerbose() << "Active UI Path: " << activeUIPath;
                }
            }
        }
    }
    //        else
    //        {
    //            handleWasPressed = false;
    //        }
    //    }
}

void MTAppModePathEditor::keyReleased(int key)
{
    if (key == OF_KEY_RETURN)
    {
        activeUIPath = nullptr;
    }
}

void MTAppModePathEditor::draw()
{
    for (auto uiPath : uiPaths)
    {
        uiPath->draw();
    }
}


void MTAppModePathEditor::exit()
{
    ofLogVerbose("MTAppModePathEditor::exit()") << settings.appModeName;
    activeUIPath = nullptr;
    uiPaths.clear();
    onExit();
}
