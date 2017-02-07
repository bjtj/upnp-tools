#ifndef __UPNP_NOTIFICATION_CENTER_HPP__
#define __UPNP_NOTIFICATION_CENTER_HPP__

#include <liboslayer/StringElements.hpp>
#include <liboslayer/os.hpp>
#include <liboslayer/MessageQueue.hpp>
#include "UPnPEventSubscription.hpp"
#include "UPnPCache.hpp"

namespace UPNP {

	class UPnPPropertyManager;

	/**
	 * @brief 
	 */
	class UPnPEventSubscriptionSession : public UPnPEventSubscription, public UPnPCache {
	private:
		std::vector<std::string> _callbackUrls;
		
	public:
		UPnPEventSubscriptionSession();
		virtual ~UPnPEventSubscriptionSession();
		std::vector<std::string> & callbackUrls();
	};

	/**
	 * @brief 
	 */
	class OnSubscriptionOutdatedListener {
	public:
		OnSubscriptionOutdatedListener() {}
		virtual ~OnSubscriptionOutdatedListener() {}
		virtual void onSessionOutdated(UPnPEventSubscriptionSession & session) = 0;
	};

	/**
	 * notification request
	 */
	class NotificationRequest {
	private:
		unsigned long creationTick;
		std::string _sid;
		unsigned long delay;
	public:
		NotificationRequest();
		NotificationRequest(const std::string & sid);
		NotificationRequest(const std::string & sid, unsigned long delay);
		virtual ~NotificationRequest();
		std::string & sid();
		bool prepared();
	};

	typedef UTIL::AutoRef<NotificationRequest> AutoNotificationRequest;
	typedef UTIL::Message<AutoNotificationRequest> NotificationRequestMessage;

	/**
	 * @brief notify thread
	 */
	class UPnPEventNotificationThread : public OS::Thread {
	private:
		UTIL::MessageQueue<AutoNotificationRequest> messageQueue;
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
		std::map<std::string, UTIL::AutoRef<UPnPEventSubscriptionSession> > sessions;
		UTIL::AutoRef<OnSubscriptionOutdatedListener> outdatedListener;

	private:
		std::string makeKey(const std::string & udn, const std::string serviceType);
		
	public:
		UPnPPropertyManager();
		virtual ~UPnPPropertyManager();
		void clear();
		bool isRegisteredService(const std::string & udn, const std::string serviceType);
		void registerService(const std::string & udn, const std::string serviceType, const UTIL::LinkedStringMap & props);
		void addSubscriptionSession(const UTIL::AutoRef<UPnPEventSubscriptionSession> session);
		bool hasSubscriptionSession(const std::string & sid);
		void removeSubscriptionSession(const std::string & sid);
		UTIL::AutoRef<UPnPEventSubscriptionSession> getSession(const std::string & sid);
		std::vector<UTIL::AutoRef<UPnPEventSubscriptionSession> > getSessionsByUdnAndServiceType(const std::string & udn, const std::string & serviceType);
		void setProperties(const std::string & udn, const std::string & serviceType, const UTIL::LinkedStringMap & props);
		UTIL::LinkedStringMap & getProperties(const std::string & udn, const std::string & serviceType);
		UTIL::LinkedStringMap & getPropertiesBySid(const std::string & sid);
		void notify(const std::string & sid);
		void notify(const std::vector<UTIL::AutoRef<UPnPEventSubscriptionSession> > & sessions, const UTIL::LinkedStringMap & props);
		void notify(UTIL::AutoRef<UPnPEventSubscriptionSession> session, const UTIL::LinkedStringMap & props);
		std::string makePropertiesXml(const UTIL::LinkedStringMap & props);
		void collectOutdated();
		void setOnSubscriptionOutdatedListener(UTIL::AutoRef<OnSubscriptionOutdatedListener> listener);
	};
}

#endif
