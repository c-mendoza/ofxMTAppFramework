#pragma once

#include "ofxMTAppFramework.h"
#include "MTApp.hpp"

class MTView;

class testApp : public MTApp{

    public:
    void initialize();
    void createAppViews();

    std::shared_ptr<MTView> view1;
    std::shared_ptr<MTView> view2;
    std::shared_ptr<MTView> view1_2;
//	std::shared_ptr<MTOffscreenView> offscreenView;

    void dragView(std::shared_ptr<MTView> view, int x, int y);
    void pressView(std::shared_ptr<MTView> view, int x, int y);
};
