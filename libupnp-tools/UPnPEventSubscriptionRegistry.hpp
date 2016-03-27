#ifndef __UPNP_EVENT_SUBSCRIPTION_REGISTRY_HPP__
#define __UPNP_EVENT_SUBSCRIPTION_REGISTRY_HPP__

#include <iostream>
#include <string>
#include <map>
#include <liboslayer/AutoRef.hpp>
#include "UPnPEventSubscription.hpp"

namespace UPNP {
	
	/**
	 * @brief
	 */
	class UPnPEventSubscriptionRegistry {
	private:
		std::map<std::string, UPnPEventSubscription> subscriptions;
	public:
		UPnPEventSubscriptionRegistry();
		virtual ~UPnPEventSubscriptionRegistry();
		void clear();
		void addSubscription(UPnPEventSubscription & subscription);
		void removeSubscription(const std::string & sid);
		bool hasSubscription(const std::string & sid);
		UPnPEventSubscription & findSubscriptionWithUdnAndServiceType(const std::string & udn, const std::string & serviceType);
		UPnPEventSubscription & operator[] (const std::string & sid);
	};
	
}

#endif
