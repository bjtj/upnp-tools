#include "UPnPCache.hpp"
#include <liboslayer/os.hpp>

namespace UPNP {

	using namespace OS;
	
	UPnPCache::UPnPCache() : _creationTime(0), _lastUpdateTime(0), _timeout(0) {
		_lastUpdateTime = _creationTime = tick_milli();
	}

	UPnPCache::UPnPCache(unsigned long timeout)
		: _creationTime(0), _lastUpdateTime(0), _timeout(timeout) {
		_lastUpdateTime = _creationTime = tick_milli();
	}
	
	UPnPCache::~UPnPCache() {
	}

	unsigned long & UPnPCache::creationTime() {
		return _creationTime;
	}

	unsigned long & UPnPCache::lastUpdateTime() {
		return _lastUpdateTime;
	}

	unsigned long & UPnPCache::timeout() {
		return _timeout;
	}

	unsigned long UPnPCache::lifetimeRecent() {
		return (tick_milli() - _lastUpdateTime);
	}

	unsigned long UPnPCache::lifetimeFull() {
		return (tick_milli() - _creationTime);
	}
	
	unsigned long UPnPCache::lifetimeRemaining() {
		unsigned long curr = tick_milli();
		if (curr < _lastUpdateTime + _timeout) {
			return (_lastUpdateTime + _timeout) - curr;
		}
		return 0;
	}

	bool UPnPCache::outdated() {
		return (lifetimeRecent() >= _timeout);
	}

	void UPnPCache::prolong() {
		_lastUpdateTime = tick_milli();
	}
	
	void UPnPCache::prolong(unsigned long newTimeout) {
		prolong();
		_timeout = newTimeout;
	}
}
