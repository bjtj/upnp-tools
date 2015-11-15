#include "Timer.hpp"
#include <algorithm>
#include <liboslayer/Logger.hpp>

namespace UPNP {

    using namespace std;
	using namespace OS;
    using namespace UTIL;
    
    static const Logger & logger = LoggerFactory::getDefaultLogger();

	TimerEvent::TimerEvent(bool heap)
		: heap(heap), fireTick(0), repeatCount(0), repeatInterval(0), repeatIndex(0), done(false) {
	}
	
	TimerEvent::~TimerEvent() {
	}
    
    void TimerEvent::scheduleFixedTimer(unsigned long fireTick) {
        scheduleRepeatableFixedTimer(fireTick, 1, 0);
    }
    void TimerEvent::scheduleRelativeTimer(unsigned long after) {
        scheduleRepeatableRelativeTimer(after, 1, 0);
    }
    void TimerEvent::scheduleRepeatableFixedTimer(unsigned long fireTick, int repeatCount, unsigned long repeatInterval) {
        this->fireTick = fireTick;
        this->repeatCount = repeatCount;
        this->repeatInterval = repeatInterval;
        
        repeatIndex = 0;
        
    }
    void TimerEvent::scheduleRepeatableRelativeTimer(unsigned long after, int repeatCount, unsigned long repeatInterval) {
        scheduleRepeatableFixedTimer(tick_milli() + after, repeatCount, repeatInterval);
    }

	bool TimerEvent::isFireTime(unsigned long currentTick) {
        return (currentTick >= fireTick);
	}

	void TimerEvent::fire() {
		onFire();
		repeatIndex++;
		if (repeatCount < 0 || repeatCount > repeatIndex) {
			fireTick += repeatInterval;
        } else {
            done = true;
        }
	}
    
    bool TimerEvent::isDone() {
        return done;
    }
    
    bool TimerEvent::isHeap() {
        return heap;
    }

    /**
     * @brief Timer
     */

	Timer::Timer() : eventsLock(1), listener(NULL), started(false) {
	}
	
	Timer::~Timer() {
	}
    
    void Timer::start() {
        started = true;
    }
    void Timer::stop() {
        started = false;
        
        clearEvents();
    }
    
    void Timer::clearEvents() {
        eventsLock.wait();
        for (vector<TimerEvent*>::iterator iter = events.begin(); iter != events.end();) {
            TimerEvent * event = *iter;
            removeEvent(event);
            iter = events.erase(iter);
        }
        eventsLock.post();
    }
    
    void Timer::removeEvent(TimerEvent * event) {
        if (event->isHeap()) {
            delete event;
        }
    }

	unsigned long Timer::getCurrentTick() {
		return tick_milli();
	}

	void Timer::setTimerEvent(TimerEvent * event) {
        events.push_back(event);
	}
    
    void Timer::removeTimerEvent(TimerEvent * event) {
        events.erase(std::remove(events.begin(), events.end(), event), events.end());
    }
    
    void Timer::onIdle() {
        
        eventsLock.wait();
        for (vector<TimerEvent*>::iterator iter = events.begin(); iter != events.end();) {
            
            TimerEvent * event = *iter;
            if (event->isFireTime(getCurrentTick())) {
                event->fire();
            }
            
            if (event->isDone()) {
                onTimerEventDone(event);
                removeEvent(event);
                iter = events.erase(iter);
            } else {
                iter++;
            }
        }
        eventsLock.post();
    }
    
    void Timer::listen(Poller & poller) {
    }
    
    void Timer::setTimerListener(TimerListener * listener) {
        this->listener = listener;
    }
    
    void Timer::onTimerEventDone(TimerEvent * event) {
        if (listener) {
            listener->onTimerEventDone(event);
        }
    }
}
