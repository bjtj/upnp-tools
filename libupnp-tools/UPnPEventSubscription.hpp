#ifndef __UPNP_EVENT_SUBSCRIPTION_HPP__
#define __UPNP_EVENT_SUBSCRIPTION_HPP__

#include <string>

namespace UPNP {
	
	/**
	 * @brief
	 */
	class UPnPEventSubscription {
	private:
		std::string _sid;
		std::string _udn;
		std::string _serviceType;
		unsigned long _lastSeq;
		unsigned long _creationTick;
		unsigned long _lastUpdatedTick;
		unsigned long _timeoutTick;
	public:
		UPnPEventSubscription();
		UPnPEventSubscription(const std::string & sid);
		UPnPEventSubscription(const std::string & sid, unsigned long timeoutTick);
		virtual ~UPnPEventSubscription();
		std::string & sid();
		std::string & udn();
		std::string & serviceType();
		unsigned long & lastSeq();
		unsigned long & creationTick();
		unsigned long & lastUpdatedTick();
		unsigned long timeoutTick();
		void timeoutTick(unsigned long timeoutTick);
		bool outdated();
	};
	
}

#endif
