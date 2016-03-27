#include "UPnPEventSubscription.hpp"
#include <liboslayer/os.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;

	UPnPEventSubscription::UPnPEventSubscription() :
		_lastSeq(0), _creationTick(0), _lastUpdatedTick(0), _timeoutTick(0) {
	}

	UPnPEventSubscription::UPnPEventSubscription(const string & sid) :
		_sid(sid), _lastSeq(0), _creationTick(0), _lastUpdatedTick(0), _timeoutTick(0) {
		
		_lastUpdatedTick = _creationTick = tick_milli();
	}
	
	UPnPEventSubscription::UPnPEventSubscription(const string & sid, unsigned long timeoutTick) :
		_sid(sid), _lastSeq(0), _creationTick(0), _lastUpdatedTick(0), _timeoutTick(timeoutTick) {

		_lastUpdatedTick = _creationTick = tick_milli();
	}
	UPnPEventSubscription::~UPnPEventSubscription() {
	}
	string & UPnPEventSubscription::sid() {
		return _sid;
	}
	string & UPnPEventSubscription::udn() {
		return _udn;
	}
	string & UPnPEventSubscription::serviceType() {
		return _serviceType;
	}
	unsigned long & UPnPEventSubscription::lastSeq() {
		return _lastSeq;
	}
	unsigned long & UPnPEventSubscription::creationTick() {
		return _creationTick;
	}
	unsigned long & UPnPEventSubscription::lastUpdatedTick() {
		return _lastUpdatedTick;
	}
	unsigned long UPnPEventSubscription::timeoutTick() {
		return _timeoutTick;
	}
	void UPnPEventSubscription::timeoutTick(unsigned long timeoutTick) {
		this->_timeoutTick = timeoutTick;
	}
	bool UPnPEventSubscription::outdated() {
		return (tick_milli() - _lastUpdatedTick >= _timeoutTick);
	}
	
}
