#ifndef __UPNP_NOTIFICATION_CENTER_HPP__
#define __UPNP_NOTIFICATION_CENTER_HPP__

#include <liboslayer/StringElements.hpp>
#include <liboslayer/os.hpp>
#include <liboslayer/Thread.hpp>
#include <liboslayer/MessageQueue.hpp>
#include "UPnPEventSubscription.hpp"
#include "UPnPCache.hpp"

namespace upnp {

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

	typedef osl::AutoRef<NotificationRequest> AutoNotificationRequest;
	typedef osl::Message<AutoNotificationRequest> NotificationRequestMessage;

	/**
	 * @brief notify thread
	 */
	class UPnPEventNotificationThread : public osl::Thread {
	private:
		osl::MessageQueue<AutoNotificationRequest> messageQueue;
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
		std::map< std::string, osl::LinkedStringMap> registry;
		std::map< std::string, osl::AutoRef<UPnPEventSubscriptionSession> > sessions;
		osl::AutoRef<OnSubscriptionOutdatedListener> outdatedListener;

	private:
		std::string getKey(const std::string & udn, const std::string serviceType);
		
	public:
		UPnPPropertyManager();
		virtual ~UPnPPropertyManager();
		void clear();
		bool isRegisteredService(const std::string & udn, const std::string serviceType);
		void registerService(const std::string & udn, const std::string serviceType, const osl::LinkedStringMap & props);
		void addSubscriptionSession(const osl::AutoRef<UPnPEventSubscriptionSession> session);
		bool hasSubscriptionSession(const std::string & sid);
		void removeSubscriptionSession(const std::string & sid);
		osl::AutoRef<UPnPEventSubscriptionSession> getSession(const std::string & sid);
		std::vector<osl::AutoRef<UPnPEventSubscriptionSession> > getSessionsByUdnAndServiceType(const std::string & udn, const std::string & serviceType);
		void setProperty(const std::string & udn, const std::string & serviceType,
						 const std::string & name, const std::string & value);
		void setProperties(const std::string & udn, const std::string & serviceType,
						   const osl::LinkedStringMap & props);
		osl::LinkedStringMap & getProperties(const std::string & udn, const std::string & serviceType);
		osl::LinkedStringMap & getPropertiesBySid(const std::string & sid);
		void notify(const std::string & sid);
		void notify(const std::vector<osl::AutoRef<UPnPEventSubscriptionSession> > & sessions, const osl::LinkedStringMap & props);
		void notify(osl::AutoRef<UPnPEventSubscriptionSession> session, const osl::LinkedStringMap & props);
		std::string makePropertiesXml(const osl::LinkedStringMap & props);
		void collectExpired();
		void setOnSubscriptionOutdatedListener(osl::AutoRef<OnSubscriptionOutdatedListener> listener);
	};
}

#endif
