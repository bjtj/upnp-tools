#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <vector>
#include <liboslayer/os.hpp>

namespace UPNP {

	class Timer;

	class TimerEvent {
	private:
		unsigned long fireTick;
		int repeatCount;
		unsigned long repeatInterval;
		int repeatIndex;
        bool done;
		
	public:
		TimerEvent();
		virtual ~TimerEvent();

		void scheduleTimer(unsigned long fireTick, int repeatCount,
					  unsigned long repeatInterval);
		bool isFireTime(unsigned long currentTick);
		void fire();
        bool isDone();
		virtual void onFire() = 0;
	};
    
    class TimerListener {
    private:
    public:
        TimerListener() {}
        virtual ~TimerListener() {}
        virtual void onTimerEventDone(TimerEvent * event) = 0;
    };

	class Timer {
	private:
		std::vector<TimerEvent*> events;
		OS::Semaphore sem;
        TimerListener * listener;
		
	public:
		Timer();
		virtual ~Timer();
        
        void start();
        void stop();

		unsigned long getCurrentTick();
		void setTimerEvent(TimerEvent * event);
        void removeTimerEvent(TimerEvent * event);
		void poll();
        void setTimerListener(TimerListener * listener);
	};
}

#endif
