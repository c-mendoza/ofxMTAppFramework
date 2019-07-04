//
// Created by Cristobal Mendoza on 4/4/18.
//

#ifndef NERVOUSSTRUCTUREOF_MTOFFSCREENWINDOW_HPP
#define NERVOUSSTRUCTUREOF_MTOFFSCREENWINDOW_HPP

#include "MTWindow.hpp"

class MTOffscreenWindow : public MTWindow
{
public:
	explicit MTOffscreenWindow(const std::string& name, bool useTextureRectangle = true);
	~MTOffscreenWindow() override;
	void setup(ofGLFWWindowSettings& settings) override;
	void setupInternal(ofEventArgs& args) override;
	void draw(ofEventArgs& args) override;
	void exit(ofEventArgs& args) override;
	ofFbo & getWindowOutput();
protected:
	ofFbo windowOutput;
	bool useTextureRectangle;
	int aaSamples;
};


#endif //NERVOUSSTRUCTUREOF_MTOFFSCREENWINDOW_HPP
