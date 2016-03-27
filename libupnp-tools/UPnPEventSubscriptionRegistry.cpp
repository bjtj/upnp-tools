#include "UPnPEventSubscriptionRegistry.hpp"

namespace UPNP {

	using namespace std;
	using namespace UTIL;

	UPnPEventSubscriptionRegistry::UPnPEventSubscriptionRegistry() {
	}
	UPnPEventSubscriptionRegistry::~UPnPEventSubscriptionRegistry() {
	}
	void UPnPEventSubscriptionRegistry::clear() {
		subscriptions.clear();
	}
	void UPnPEventSubscriptionRegistry::addSubscription(UPnPEventSubscription & subscription) {
		subscriptions[subscription.sid()] = subscription;
	}
	void UPnPEventSubscriptionRegistry::removeSubscription(const string & sid) {
		subscriptions.erase(sid);
	}
	bool UPnPEventSubscriptionRegistry::hasSubscription(const string & sid) {
		return subscriptions.find(sid) != subscriptions.end();
	}
	UPnPEventSubscription & UPnPEventSubscriptionRegistry::findSubscriptionWithUdnAndServiceType(const std::string & udn, const std::string & serviceType) {
		for (map<string, UPnPEventSubscription>::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++) {
			if (iter->second.udn() == udn && iter->second.serviceType() == serviceType) {
				return iter->second;
			}
		}
		throw OS::Exception("No subscription found");
	}
	UPnPEventSubscription & UPnPEventSubscriptionRegistry::operator[] (const string & sid) {
		return subscriptions[sid];
	}
}
