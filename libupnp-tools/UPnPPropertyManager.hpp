#ifndef __UPNP_NOTIFICATION_CENTER_HPP__
#define __UPNP_NOTIFICATION_CENTER_HPP__

#include <liboslayer/StringElements.hpp>
#include <liboslayer/os.hpp>
#include <liboslayer/Thread.hpp>
#include <liboslayer/MessageQueue.hpp>
#include "UPnPEventSubscription.hpp"
#include "UPnPCache.hpp"
#include "UPnPTerms.hpp"

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

	typedef OS::AutoRef<NotificationRequest> AutoNotificationRequest;
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
		std::map< std::string, UTIL::LinkedStringMap> registry;
		std::map< std::string, OS::AutoRef<UPnPEventSubscriptionSession> > sessions;
		OS::AutoRef<OnSubscriptionOutdatedListener> outdatedListener;

	private:
		std::string getKey(const UDN & udn, const std::string serviceType);
		
	public:
		UPnPPropertyManager();
		virtual ~UPnPPropertyManager();
		void clear();
		bool isRegisteredService(const UDN & udn, const std::string serviceType);
		void registerService(const UDN & udn, const std::string serviceType, const UTIL::LinkedStringMap & props);
		void addSubscriptionSession(const OS::AutoRef<UPnPEventSubscriptionSession> session);
		bool hasSubscriptionSession(const std::string & sid);
		void removeSubscriptionSession(const std::string & sid);
		OS::AutoRef<UPnPEventSubscriptionSession> getSession(const std::string & sid);
		std::vector<OS::AutoRef<UPnPEventSubscriptionSession> > getSessionsByUdnAndServiceType(const UDN & udn, const std::string & serviceType);
		void setProperty(const UDN & udn, const std::string & serviceType,
						 const std::string & name, const std::string & value);
		void setProperties(const UDN & udn, const std::string & serviceType,
						   const UTIL::LinkedStringMap & props);
		UTIL::LinkedStringMap & getProperties(const UDN & udn, const std::string & serviceType);
		UTIL::LinkedStringMap & getPropertiesBySid(const std::string & sid);
		void notify(const std::string & sid);
		void notify(const std::vector<OS::AutoRef<UPnPEventSubscriptionSession> > & sessions, const UTIL::LinkedStringMap & props);
		void notify(OS::AutoRef<UPnPEventSubscriptionSession> session, const UTIL::LinkedStringMap & props);
		std::string makePropertiesXml(const UTIL::LinkedStringMap & props);
		void collectExpired();
		void setOnSubscriptionOutdatedListener(OS::AutoRef<OnSubscriptionOutdatedListener> listener);
	};
}

#endif
