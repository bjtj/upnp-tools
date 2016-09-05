#ifndef __UPNP_NOTIFICATION_CENTER_HPP__
#define __UPNP_NOTIFICATION_CENTER_HPP__

#include <liboslayer/StringElements.hpp>
#include <liboslayer/os.hpp>
#include <liboslayer/MessageQueue.hpp>
#include "UPnPEventSubscription.hpp"

namespace UPNP {

	class UPnPPropertyManager;

	/**
	 * @brief 
	 */
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
		void prolong();
		bool outdated();
	};
	

	/**
	 * @brief notify thread
	 */
	class UPnPEventNotificationThread : public OS::Thread {
	private:
		UTIL::MessageQueue messageQueue;
		UPnPPropertyManager & propertyManager;
	public:
		UPnPEventNotificationThread(UPnPPropertyManager & propertyManager);
		virtual ~UPnPEventNotificationThread();
		virtual void run();
		void notify(const std::string & sid);
		void delayNotify(const std::string & sid, unsigned long delay);
	};

	/**
	 * @brief noticiation center
	 */
	class UPnPPropertyManager {
	private:
		std::map<std::string, UTIL::LinkedStringMap> registry;
		std::map<std::string, UPnPEventSubscriptionSession> sessions;
	public:
		UPnPPropertyManager();
		virtual ~UPnPPropertyManager();
		void clear();
		void registerService(const std::string & udn, const std::string serviceType, UTIL::LinkedStringMap & props);
		void addSubscriptionSession(UPnPEventSubscriptionSession & session);
		void removeSubscriptionSession(const std::string & sid);
		UPnPEventSubscriptionSession & getSession(const std::string & sid);
		UPnPEventSubscriptionSession & getSessionWithUdnAndServiceType(const std::string & udn, const std::string & serviceType);
		void setProperties(const std::string & udn, const std::string & serviceyType, UTIL::LinkedStringMap & props);
		UTIL::LinkedStringMap & getProperties(const std::string & udn, const std::string & serviceType);
		UTIL::LinkedStringMap & getPropertiesWithSid(const std::string & sid);
		void notify(const std::string & sid);
		void notify(UPnPEventSubscriptionSession & session, UTIL::LinkedStringMap & props);
		std::string makePropertiesXml(UTIL::LinkedStringMap & props);
		void collectOutdated();
	};
}

#endif
