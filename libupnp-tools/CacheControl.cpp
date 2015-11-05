#include "CacheControl.hpp"

#include <liboslayer/os.hpp>

namespace UPNP {

	using namespace OS;

	CacheControl::CacheControl() : createdTick(tick_milli()), timeoutMilli(0) {
	}
	CacheControl::~CacheControl() {
	}
	void CacheControl::renew() {
		this->createdTick = tick_milli();
	}
	void CacheControl::setTimeout(unsigned long timeoutMilli) {
		this->timeoutMilli = timeoutMilli;
	}
	bool CacheControl::outdated() const {
		return (tick_milli() > (createdTick + timeoutMilli));
	}
}