//
// Created by Cristobal Mendoza on 7/24/18.
//

#ifndef OFXMTAPPFRAMEWORK_MTZOOMABLEDOCUMENTVIEW_HPP
#define OFXMTAPPFRAMEWORK_MTZOOMABLEDOCUMENTVIEW_HPP

#include "MTView.hpp"

/**
 * @brief Adds zooming and panning behavior to a view using a mouse. Zooming is controlled by the scrollwheel,
 * panning is done by dragging the mouse while pressing button 2 (which is typically the right mouse button).
 *
 * To use:
 * 1. Make your view inherit from MTZoomableDocumentView.
 * 2. Call setupDocumentRect as soon as you know what your document values should be.
 * 3. If your view overrides mouseDragged or mouseScrolled, make sure to call this class's corresponding
 * methods. For example:
 *
 * 		void yourViewClass::mouseDragged(int x, int y, int button) {
 * 			MTZoomableDocumentView::mouseDragged(x, y, button);
 * 			(your code here)
 * 			...
 * 		}
 */
class MTZoomableDocumentView : public MTView
{
 public:
   explicit MTZoomableDocumentView(const string& _name);
   void draw() override;
   void mouseDragged(int x, int y, int button) override;
   void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
   void setupDocumentRect(float width,
                          float height,
                          const ofColor& backgroundColor = ofColor::black,
                          const ofColor& lineColor = ofColor::white,
                          float strokeWidth = 1);
 private:
   ofRectangle documentRect;
   ofPath documentRectPath;
};


#endif  //OFXMTAPPFRAMEWORK_MTZOOMABLEDOCUMENTVIEW_HPP
