//
// Created by Cristobal Mendoza on 7/24/18.
//

#include "MTZoomableDocumentView.hpp"

MTZoomableDocumentView::MTZoomableDocumentView(const std::string& _name) : MTView(_name)
{
   setupDocumentRect(1920, 1080);
}

void MTZoomableDocumentView::draw()
{
   documentRectPath.draw();
}


void MTZoomableDocumentView::mouseDragged(int x, int y, int button)
{
   // TODO: Make view dragging code better
   if (button == 2)
   {
      //        ofLogVerbose("Dragging") << "contentMouse: " << x << " " << y;
      auto dist = getScreenMouse() - getPrevScreenMouse();
      setContentOrigin(getContentOrigin() + dist);
      //        shiftContentOrigin(glm::vec3(dist, 1));
   }
}

void MTZoomableDocumentView::mouseScrolled(int x, int y, float scrollX, float scrollY)
{
   auto startPos = glm::vec2(getContentOrigin().x, getContentOrigin().y);  // get a copy
   auto startContentMouse =
       glm::clamp(getContentMouse(), glm::vec2(0, 0), glm::vec2(documentRect.getWidth(), documentRect.getHeight()));
   float factor = (scrollY / 100.0f) + getContentScaleX();
   factor = ofClamp(factor, 0.1, 10);
   //    ofLogVerbose("Scrolling") << "contentMouse: " << startContentMouse.x << " " << startContentMouse.y;
   setContentScale(factor, factor);

   // TODO: Improve the zoom, it is very finicky at the moment
   auto newContentMouse = glm::vec2(getInvContentMatrix() * glm::vec4(getScreenMouse(), 1, 1));

   newContentMouse = glm::clamp(newContentMouse, glm::vec2(0, 0), glm::vec2(documentRect.getWidth(), documentRect.getHeight()));
   //    setContentOrigin(getContentOrigin() + glm::vec3((newContentMouse.xy() - startContentMouse), 1));

   auto diff = newContentMouse - startContentMouse;
   shiftContentOrigin(diff);
}

void MTZoomableDocumentView::setupDocumentRect(
    float width, float height, const ofColor& backgroundColor, const ofColor& lineColor, float strokeWidth)
{
   documentRect = ofRectangle(0, 0, width, height);
   documentRectPath.clear();
   documentRectPath.setStrokeWidth(strokeWidth);
   documentRectPath.setStrokeColor(lineColor);
   documentRectPath.setColor(backgroundColor);
   documentRectPath.setFilled(true);
   documentRectPath.rectangle(documentRect);
}
