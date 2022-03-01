//
// Created by cmendoza on 4/16/21.
//

#ifndef NERVOUSSTRUCTUREOF_MTTIMER_HPP
#define NERVOUSSTRUCTUREOF_MTTIMER_HPP

#include "ofEvent.h"

class MTTimer
{
 public:
   struct TimerResult
   {
      /**
		 * @brief The starting time, in ms.
		 */
      uint64_t startTime;
      /**
		 * @brief The time at which the timer stopped. In ms.
		 */
      uint64_t endTime;
      /**
		 * @brief The difference in ms. between the measured endTime and the intended stopping time.
		 */
      uint64_t offset;
      /**
		 * @brief The number of times the timer was updated.
		 */
      int ticks;
   };

   /**
	 * @brief Sets up the timer. It does not start it, you need to call start() yourself!
	 * @param interval In ms.
	 * @param repeating If true, the timer repeats at an interval specified by the duration.
	 * @param callback The function called when a timer completes an interval.
	 */
   void setup(uint64_t interval, bool repeating, std::function<void(TimerResult)> callback);

   /**
	 * @brief Starts the timer. The timer will be updated on the `update` event of whichever window is current when
	 * start() is called. start() registers the event listener on its own, so you don't need to worry about calling
	 * for updates.
	 * @param useLoopEvent If true, the timer will instead be updated on the `loopEvent` of the ofMainLoop. This may
	 * be useful if you are using OF in headless mode, but keep in mind that you may not have access to an OpenGL
	 * context when your callback is executed.
	 */
   void start(bool useLoopEvent = false);

   /**
	 * @brief Stops the timer. It can be restarted using start(), but note that the timing interval begins when
	 * start() is called.
	 */
   void stop();

 private:
   ofEventListener eventListener;
   uint64_t interval;
   bool repeating;
   int ticks;
   uint64_t startTime;
   std::function<void(TimerResult)> callback;
   void update();
};


#endif  //NERVOUSSTRUCTUREOF_MTTIMER_HPP
