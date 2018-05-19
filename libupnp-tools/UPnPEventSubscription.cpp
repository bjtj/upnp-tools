#include "UPnPEventSubscription.hpp"
#include <liboslayer/os.hpp>

namespace upnp {

	using namespace std;
	using namespace osl;

	UPnPEventSubscription::UPnPEventSubscription() : _lastSeq(0) {
	}

	UPnPEventSubscription::UPnPEventSubscription(const string & sid) : _sid(sid), _lastSeq(0) {
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
}
