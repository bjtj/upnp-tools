#include "UPnPEventSubscription.hpp"
#include <liboslayer/os.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;

	UPnPEventSubscription::UPnPEventSubscription() : _lastSeq(0) {
	}

	UPnPEventSubscription::UPnPEventSubscription(const string & sid) : _sid(sid), _lastSeq(0) {
	}

	UPnPEventSubscription::~UPnPEventSubscription() {
	}

	string & UPnPEventSubscription::sid() {
		return _sid;
	}

	UDN & UPnPEventSubscription::udn() {
		return _udn;
	}

	string & UPnPEventSubscription::serviceType() {
		return _serviceType;
	}

	unsigned long & UPnPEventSubscription::lastSeq() {
		return _lastSeq;
	}
}
