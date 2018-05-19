#ifndef __UPNP_EVENT_SUBSCRIPTION_HPP__
#define __UPNP_EVENT_SUBSCRIPTION_HPP__

#include <string>
#include "UPnPUsn.hpp"

namespace upnp {
	

	/**
	 * @brief
	 */
	class UPnPEventSubscription {
	private:
		std::string _sid;
		std::string _udn;
		std::string _serviceType;
		unsigned long _lastSeq;
		
	public:
		UPnPEventSubscription();
		UPnPEventSubscription(const std::string & sid);
		virtual ~UPnPEventSubscription();
		std::string & sid();
		std::string & udn();
		std::string & serviceType();
		unsigned long & lastSeq();
	};
}

#endif
