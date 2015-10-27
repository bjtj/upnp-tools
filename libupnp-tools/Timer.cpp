#include "Timer.hpp"

namespace UPNP {

	using namespace OS;


	TimerEvent::TimerEvent()
		: timer(NULL), fireTick(0), repeatCount(0), repeatInterval(0),
		  repeatIndex(0) {
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

	void TimerEvent::setTimer(Timer * timer) {
		this->timer = timer;
	}

	bool TimerEvent::isFireTime(unsigned long currentTick) {
		if (timer) {
			return (currentTick >= fireTick);
		}
		return false;
	}

	void TimerEvent::fire() {
		onFire();
		repeatIndex++;
		if (repeatCount > repeatIndex) {
			fireTick += repeatInterval;
		}
	}



	Timer::Timer() : sem(1) {
	}
	
	Timer::~Timer() {
	}

	unsigned long Timer::getCurrentTick() {
		return tick_milli();
	}

	void Timer::setTimerEvent(TimerEvent * event) {
	}

	void Timer::loop() {
		sem.wait();
		for (size_t i = 0; i < events.size(); i++) {
			TimerEvent * event = events[i];
			if (event->isFireTime(getCurrentTick())) {
				event->fire();
			}
		}
		sem.post();
	}
}
