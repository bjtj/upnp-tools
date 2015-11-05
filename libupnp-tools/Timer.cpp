#include "Timer.hpp"
#include <algorithm>

namespace UPNP {

    using namespace std;
	using namespace OS;

	TimerEvent::TimerEvent()
		: fireTick(0), repeatCount(0), repeatInterval(0), repeatIndex(0), done(false) {
	}
	
	TimerEvent::~TimerEvent() {
	}

	void TimerEvent::scheduleTimer(unsigned long fireTick, int repeatCount,
							  unsigned long repeatInterval) {
		this->fireTick = fireTick;
		this->repeatCount = repeatCount;
		this->repeatInterval = repeatInterval;

		repeatIndex = 0;
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



	Timer::Timer() : sem(1), listener(NULL) {
	}
	
	Timer::~Timer() {
	}
    
    void Timer::start() {
    }
    void Timer::stop() {
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

	void Timer::poll() {
		sem.wait();
		for (size_t i = 0; i < events.size(); i++) {
			TimerEvent * event = events[i];
			if (event->isFireTime(getCurrentTick())) {
				event->fire();
			}
		}
        for (vector<TimerEvent*>::iterator iter = events.begin(); iter != events.end();) {
            if ((*iter)->isDone()) {
                if (listener) {
                    listener->onTimerEventDone(*iter);
                }
                iter = events.erase(iter);
            } else {
                iter++;
            }
        }
		sem.post();
	}
    
    void Timer::setTimerListener(TimerListener * listener) {
        this->listener = listener;
    }
}
