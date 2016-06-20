#ifndef __UPNP_NOTIFICATION_CENTER_HPP__
#define __UPNP_NOTIFICATION_CENTER_HPP__

#include "UPnPEventSubscriptionSession.hpp"
#include <liboslayer/StringElements.hpp>
#include <liboslayer/os.hpp>
#include <liboslayer/MessageQueue.hpp>

namespace UPNP {

	class UPnPNotificationCenter;

	/**
	 * @brief notify thread
	 */
	class UPnPEventNotifyThread : public OS::Thread {
	private:
		UTIL::MessageQueue messageQueue;
		UPnPNotificationCenter & notificationCenter;
	public:
		UPnPEventNotifyThread(UPnPNotificationCenter & nc);
		virtual ~UPnPEventNotifyThread();
		virtual void run();
		void scheduleNotify(const std::string & sid);
		void delayNotify(unsigned long delay, const std::string & sid);
	};

	/**
	 * @brief noticiation center
	 */
	class UPnPNotificationCenter {
	private:
		std::map<std::string, UTIL::LinkedStringMap> registry;
		std::map<std::string, UPnPEventSubscriptionSession> sessions;
	public:
		UPnPNotificationCenter();
		virtual ~UPnPNotificationCenter();
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
	};
}

#endif
