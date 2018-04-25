#include "UPnPCache.hpp"
#include <liboslayer/os.hpp>


namespace upnp {
	

	using namespace osl;
	
	
	UPnPCache::UPnPCache()
		: _creationTime(0), _lastUpdateTime(0), _timeout(0)
	{
		_lastUpdateTime = _creationTime = tick_milli();
	}

	UPnPCache::UPnPCache(unsigned long timeout)
		: _creationTime(0), _lastUpdateTime(0), _timeout(timeout)
	{
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

	unsigned long UPnPCache::lifetime() {
		return (tick_milli() - _lastUpdateTime);
	}

	bool UPnPCache::expired() {
		return (lifetime() >= _timeout);
	}

	void UPnPCache::updateTime() {
		_lastUpdateTime = tick_milli();
	}
}
