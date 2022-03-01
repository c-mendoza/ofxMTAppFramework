//
//  ofxMTAppMode.hpp
//  NervousStructureOF
//
//  Created by Cristobal Mendoza on 6/7/16.
//
//

#ifndef MTViewMode_hpp
#define MTViewMode_hpp

#include "ofxMTAppFramework.h"

class MTView;

class MTViewMode : public MTEventListenerStore
{
 public:
   MTViewMode(std::string name, MTView* view)
   {
      this->name = name;
      this->view = view;
      //        addAllEventListeners();
   }
   ~MTViewMode();
   void addAllEventListeners();
   //    void removeAllEventListeners();
   std::string getName()
   {
      return name;
   }
   void setName(std::string name)
   {
      this->name = name;
   }

 protected:
   virtual void setup() = 0;
 public:
   virtual void exit(){};
   virtual void update(){};
   virtual void draw(){};
   virtual void keyPressed(int key){};
   virtual void keyReleased(int key){};
   virtual void mouseMoved(int x, int y){};
   virtual void mouseDragged(int x, int y, int button){};
   virtual void mouseDraggedEnd(int x, int y, int button){};
   virtual void mousePressed(int x, int y, int button){};
   virtual void mouseReleased(int x, int y, int button){};
   virtual void mouseEntered(int x, int y){};
   virtual void mouseExited(int x, int y){};
   virtual void windowResized(int w, int h){};

   //	virtual void dragEvent(ofDragInfo dragInfo);
   //	virtual void gotMessage(ofMessage msg);
 protected:
   MTView* view = nullptr;
   std::string name;

   friend class MTView;
};

class MTViewModeVoid : public MTViewMode
{
 public:
   MTViewModeVoid(MTView* view) : MTViewMode("View Mode Void", view)
   {
   }

   virtual void exit()
   {
   }
 protected:
   virtual void setup()
   {
   }
};

#endif /* MTViewMode_hpp */
