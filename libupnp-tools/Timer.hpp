#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <vector>
#include <liboslayer/os.hpp>
#include <liboslayer/PollablePool.hpp>

namespace UPNP {

	class Timer;

    /**
     * @brief TimerEvent
     */
	class TimerEvent {
	private:
        bool heap;
		unsigned long fireTick;
		int repeatCount;
		unsigned long repeatInterval;
		int repeatIndex;
        bool done;
		
	public:
		TimerEvent(bool heap);
		virtual ~TimerEvent();

        void scheduleFixedTimer(unsigned long fireTick);
        void scheduleRelativeTimer(unsigned long after);
		void scheduleRepeatableFixedTimer(unsigned long fireTick, int repeatCount, unsigned long repeatInterval);
        void scheduleRepeatableRelativeTimer(unsigned long after, int repeatCount, unsigned long repeatInterval);
        
		bool isFireTime(unsigned long currentTick);
		void fire();
        bool isDone();
		virtual void onFire() = 0;
        
        bool isHeap();
	};
    
    /**
     * @brief TimerListener
     */
    class TimerListener {
    private:
    public:
        TimerListener() {}
        virtual ~TimerListener() {}
        virtual void onTimerEventDone(TimerEvent * event) = 0;
    };

    /**
     * @brief Timer
     */
    class Timer : public TimerListener, public UTIL::Pollee {
	private:
		std::vector<TimerEvent*> events;
		OS::Semaphore eventsLock;
        TimerListener * listener;
        bool started;
        
    public:
        static const unsigned long SECOND = 1000UL;
		
	public:
		Timer();
		virtual ~Timer();
        
        void start();
        void stop();
        
        void clearEvents();
        void removeEvent(TimerEvent * event);

		unsigned long getCurrentTick();
		void setTimerEvent(TimerEvent * event);
        void removeTimerEvent(TimerEvent * event);
        
        virtual void onIdle();
        virtual void listen(UTIL::Poller & poller);
        
        void setTimerListener(TimerListener * listener);
        
        virtual void onTimerEventDone(TimerEvent * event);
	};
}

#endif
