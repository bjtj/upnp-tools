#include "UPnPEventSubscriptionSession.hpp"
#include <liboslayer/os.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;
	
	UPnPEventSubscriptionSession::UPnPEventSubscriptionSession() {
		creationTick = lastUpdatedTick = tick_milli();
	}
	UPnPEventSubscriptionSession::~UPnPEventSubscriptionSession() {
	}
	vector<string> & UPnPEventSubscriptionSession::callbackUrls() {
		return _callbackUrls;
	}
	void UPnPEventSubscriptionSession::setTimeout(unsigned long timeoutTick) {
		this->timeoutTick = timeoutTick;
	}
	void UPnPEventSubscriptionSession::renew() {
		lastUpdatedTick = tick_milli();
	}
	bool UPnPEventSubscriptionSession::outdated() {
		return (tick_milli() - lastUpdatedTick >= timeoutTick);
	}
	
}
