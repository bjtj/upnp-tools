#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <vector>
#include <liboslayer/os.hpp>

namespace UPNP {

	class Timer;

	class TimerEvent {
	private:
		Timer * timer;
		unsigned long fireTick;
		int repeatCount;
		unsigned long repeatInterval;
		int repeatIndex;
		
	public:
		TimerEvent();
		virtual ~TimerEvent();

		void scheduleTimer(unsigned long fireTick, int repeatCount,
					  unsigned long repeatInterval);
		void setTimer(Timer * timer);
		bool isFireTime(unsigned long currentTick);
		void fire();
		virtual void onFire() = 0;
	};


	class Timer {
	private:
		std::vector<TimerEvent*> events;
		OS::Semaphore sem;
		
	public:
		Timer();
		virtual ~Timer();

		unsigned long getCurrentTick();
		void setTimerEvent(TimerEvent * event);
		void loop();
	};
}

#endif
