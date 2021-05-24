#pragma once

#include "ofxMTAppFramework.h"
#include "MTApp.hpp"

class MTView;

class testApp : public MTApp{

    public:
    void initialize();
    void createAppViews();


//	std::shared_ptr<MTOffscreenView> offscreenView;

    void dragView(MTView* view, int x, int y);
    void pressView(MTView* view, int x, int y);
};
