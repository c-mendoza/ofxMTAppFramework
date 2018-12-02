
//
//  MTViewModePathEditor.cpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 09/14/2017.
//
//

#include "MTViewModePathEditor.hpp"
#include "MTUIPath.hpp"

MTViewModePathEditor::MTViewModePathEditor(PathEditorSettings& settings)
		: MTViewMode(settings.appModeName, settings.view)
{
	addAllEventListeners();
	pathCollection = settings.paths;
	path = settings.path;
	options = settings.options;
	validRegion = settings.validRegion;
	validRegionsMap = settings.validRegionsMap;
	pathStrokeWidth = settings.pathStrokeWidth;
	pathColor = settings.pathColor;

	pEventArgs.pathEditor = this;

	if (options.test(PathEditorSettings::LimitToView))
	{
		options.set(PathEditorSettings::LimitToRegion);
		validRegion = view->getContent();
	}
	else if (options.test(PathEditorSettings::LimitToRegion))
	{
		if (path != nullptr)
		{
			if (validRegion.getWidth() == 0)
			{
				ofLogError("MTViewModePathEditor") << "Settings specify LimitToRegion, but no region was passed. "
												   << "Using view as region";
				options.set(PathEditorSettings::LimitToView);
			}
		}
		else //TODO: Valid Regions
		{
			ofLogWarning("MTViewModePathEditor") << "validRegions not implemented yet!! Turning off LimitToRegion";
			options.reset(PathEditorSettings::LimitToRegion);
//			if (validRegions->size() != pathCollection.size())
//			{
//				ofLogError("MTViewModePathEditor")
//						<< "Settings specify LimitToRegion using a vector of paths "
//						<< "but no equivalent vector of regions was passed.\n "
//						<< "We should probably kill this here...";
//			}
		}
	}

}

void MTViewModePathEditor::setup()
{

	ofLogVerbose("MTViewModePathEditor::setup") << getName();
	MTUIPath::vertexHandleStyle.bFill = false;
	MTUIPath::selectedVextexHandleStyle.bFill = true;
	uiPaths.clear();
	

	if (path == nullptr)
	{
		for (const auto& path : pathCollection)
		{
			auto uiPath = createUIPath(path);
			activeUIPath = uiPath;
		}
	}
	else
	{
		auto uiPath = createUIPath(path);
		activeUIPath = uiPath;
	}

	view->enqueueUpdateOperation([this]()
								 { ofShowCursor(); });
}

std::shared_ptr<MTUIPath> MTViewModePathEditor::createUIPath(
		std::shared_ptr<ofPath> p, ofRectangle validRegion)
{
	// There has to be a less stupid way of doing this...
	// I feel that OR'd flags would be simpler....
	auto uiPath = std::make_shared<MTUIPath>();
	std::bitset<5> uiPathOptions;

	if (options.test(PathEditorSettings::LimitToRegion))
	{
		uiPathOptions.set(MTUIPath::LimitToRegion);
		auto regions = validRegionsMap;
		if (regions.size() > 0)
		{
			auto region = regions.find(p.get());
			if (region != regions.end())
			{
				uiPath->setRegion(region->second);
			}
			else
			{
				ofLogFatalError("PathEditorSettings") << "Failed to find region in validRegionsMap";
				std::exit(-1);
			}
		}
		else
		{
			uiPath->setRegion(validRegion);
		}
	}
	if (options.test(PathEditorSettings::CanAddPoints))
	{
		uiPathOptions.set(MTUIPath::CanAddPoints);
	}
	if (options.test(PathEditorSettings::CanDeletePoints))
	{
		uiPathOptions.set(MTUIPath::CanDeletePoints);
	}
	if (options.test(PathEditorSettings::CanConvertPoints))
	{
		uiPathOptions.set(MTUIPath::CanConvertPoints);
	}
	if (options.test(PathEditorSettings::NotifyOnHandleDragged))
	{
		uiPathOptions.set(MTUIPath::NotifyOnHandleDragged);
	}

	uiPath->setup(p, view, (unsigned int) uiPathOptions.to_ulong());

	if (options.test(PathEditorSettings::PathsAreClosed))
	{
		uiPath->setClosed(true);
	}
	uiPath->getPath()->setColor(pathColor);
	uiPath->getPath()->setFilled(false);
	uiPath->getPath()->setStrokeWidth(pathStrokeWidth);

#pragma mark listeners

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
				auto h = (MTUIPathVertexHandle*) handle;
				auto uiPathPtr = h->getUIPath().lock();
				activeUIPath = uiPathPtr;
			},
			OF_EVENT_ORDER_AFTER_APP));

	addEventListener(uiPath->pathHandleMovedEvent.newListener(
			[this](const void* handle, ofMouseEventArgs& args)
			{
				auto h = (MTUIPathVertexHandle*) handle;
				auto uiPathPtr = h->getUIPath().lock();
				pEventArgs.path = uiPathPtr->getPath();
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

bool MTViewModePathEditor::removeUIPath(std::shared_ptr<MTUIPath> p)
{
	auto iter = std::find(uiPaths.begin(), uiPaths.end(), p);
	if (iter != uiPaths.end())
	{

		auto path = p->getPath();
		auto iterOfPath = std::find_if(pathCollection.begin(),
									   pathCollection.end(),
									   [&](std::shared_ptr<ofPath> const& current)
									   {
										   return current == path;
									   });

		if (iterOfPath != pathCollection.end())
		{
			// Clear the path first:
			(*iterOfPath)->clear();

			// Now remove the path from the model:
			pathCollection.erase(iterOfPath);
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

void MTViewModePathEditor::mouseReleased(int x, int y, int button)
{
	if (button == 0)
	{
		//        if (!handleWasPressed)
		//        {
		if (ofGetKeyPressed(OF_KEY_SHIFT))
		{
			if (activeUIPath != nullptr &&
				options.test(PathEditorSettings::CanAddPoints))
			{
				if (activeUIPath->getSelection().size() > 0)
				{
					auto dex = activeUIPath->getIndexForHandle(activeUIPath->getSelection().back());
					activeUIPath->insertHandle(glm::vec3(x, y, 0), dex + 1);
				}
				else
				{
					activeUIPath->addHandle(glm::vec3(x, y, 0));
				}
			}
			else
			{
				if (options.test(PathEditorSettings::AllowsMultiplePaths))
				{
					if (pathCollection.size() < maxPaths)
					{
						auto pathPtr = std::make_shared<ofPath>();
						pathPtr->moveTo(glm::vec3(x, y, 0));
						activeUIPath = createUIPath(pathPtr);
//					activeUIPath->addHandle(glm::vec3(x, y, 0));
						pathCollection.push_back(pathPtr);
						pEventArgs.path = pathPtr;
						pathCreatedEvent.notify(pEventArgs);
						onPathCreated(pEventArgs);
						ofLogVerbose() << "Active UI Path: " << activeUIPath;
					}
				}
			}
		}
	}
}

void MTViewModePathEditor::keyReleased(int key)
{
	if (key == OF_KEY_RETURN)
	{
		activeUIPath = nullptr;
	}
}

void MTViewModePathEditor::draw()
{
	for (auto uiPath : uiPaths)
	{
		uiPath->draw();
	}
}


void MTViewModePathEditor::exit()
{
	ofLogVerbose("MTViewModePathEditor::exit()") << getName();
	activeUIPath = nullptr;
	uiPaths.clear();
	onExit();
}

std::vector<std::shared_ptr<ofPath>> MTViewModePathEditor::getPathCollection()
{
	return pathCollection;
}
