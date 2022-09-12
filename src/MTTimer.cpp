//
// Created by cmendoza on 4/16/21.
//

#include <ofUtils.h>
#include <ofAppRunner.h>
#include <ofAppBaseWindow.h>

#include "MTTimer.hpp"

void MTTimer::setup(uint64_t interval, bool repeating, std::function<void(TimerResult)> callback)
{
   this->interval = interval;
   this->repeating = repeating;
   this->callback = callback;
}

void MTTimer::start(bool useLoopEvent)
{
   startTime = ofGetCurrentTime().getAsMilliseconds();
   ticks = 0;
   if (useLoopEvent)
   {
      eventListener = ofGetMainLoop()->loopEvent.newListener([this]() { update(); });
   }
   else
   {
      eventListener = ofGetWindowPtr()->events().update.newListener([this](const ofEventArgs& args) { update(); });
   }
}

void MTTimer::stop()
{
   eventListener.unsubscribe();
}

void MTTimer::update()
{
   ticks++;
   auto now = ofGetCurrentTime().getAsMilliseconds();
   remaining = (startTime + interval) - now;
   if (now >= startTime + interval)
   {
      TimerResult result;
      result.ticks = ticks;
      result.startTime = startTime;
      result.endTime = now;
      result.offset = (now - startTime) - interval;
      this->callback(std::move(result));
      if (repeating)
      {
         startTime = startTime + interval;
         ticks = 0;
      }
      else
      {
         eventListener.unsubscribe();
      }
   }
}
