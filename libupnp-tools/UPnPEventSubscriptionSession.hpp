#ifndef __UPNP_EVENT_SUBSCRIPTION_SESSION_HPP__
#define __UPNP_EVENT_SUBSCRIPTION_SESSION_HPP__

#include "UPnPEventSubscription.hpp"
#include <vector>
#include <string>

namespace UPNP {

	class UPnPEventSubscriptionSession : public UPnPEventSubscription {
	private:
		std::vector<std::string> _callbackUrls;
		unsigned long creationTick;
		unsigned long lastUpdatedTick;
		unsigned long timeoutTick;
	public:
		UPnPEventSubscriptionSession();
		virtual ~UPnPEventSubscriptionSession();
		std::vector<std::string> & callbackUrls();
		void setTimeout(unsigned long timeoutTick);
		void renew();
		bool outdated();
	};
}

#endif
