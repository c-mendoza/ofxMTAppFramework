//
// Created by Cristobal Mendoza on 4/4/18.
//

#ifndef NERVOUSSTRUCTUREOF_MTOFFSCREENWINDOW_HPP
#define NERVOUSSTRUCTUREOF_MTOFFSCREENWINDOW_HPP

#include "MTWindow.hpp"

class MTOffscreenWindow : public MTWindow
{
public:
	MTOffscreenWindow(const std::string& name, bool useTextureRectangle = true);
	~MTOffscreenWindow() override;
	void setup(ofGLFWWindowSettings& settings) override;
	virtual void setupInternal(ofEventArgs& args);
	virtual void draw(ofEventArgs& args);
	virtual void exit(ofEventArgs& args);
	ofFbo & getWindowOutput();
protected:
	ofFbo windowOutput;
	bool useTextureRectangle;
	int aaSamples;
};


#endif //NERVOUSSTRUCTUREOF_MTOFFSCREENWINDOW_HPP
