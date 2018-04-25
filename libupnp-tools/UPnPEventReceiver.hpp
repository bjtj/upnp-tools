#ifndef __UPNP_EVENT_SUBSCRIBER_SERVER_HPP__
#define __UPNP_EVENT_SUBSCRIBER_SERVER_HPP__

#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Properties.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include "UPnPEventSubscriber.hpp"
#include "UPnPEventSubscription.hpp"
#include "UPnPTerms.hpp"

namespace upnp {

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
		UPnPEventSubscription & findSubscriptionByUdnAndServiceType(const UDN & udn, const std::string & serviceType);
		UPnPEventSubscription & operator[] (const std::string & sid);
	};

	/**
	 * @brief
	 */
	class UPnPEventListener {
	private:
	public:
		UPnPEventListener() {}
		virtual ~UPnPEventListener() {}
		virtual void onNotify(UPnPNotify & notify) = 0;
	};

	/**
	 * @brief
	 */

	class UPnPEventReceiverConfig : public osl::Properties {
	private:
	public:
		UPnPEventReceiverConfig() {}
		UPnPEventReceiverConfig(int port) { setProperty("listen.port", port); }
		virtual ~UPnPEventReceiverConfig() {}
	};

	/**
	 * @brief
	 */
	class UPnPEventReceiver : public UPnPEventListener {
	private:
		UPnPEventReceiverConfig config;
		UPnPEventSubscriptionRegistry registry;
		std::vector<osl::AutoRef<UPnPEventListener> > listeners;
		http::AnotherHttpServer * server;
		
	public:
		UPnPEventReceiver(UPnPEventReceiverConfig & config);
		virtual ~UPnPEventReceiver();
		void startAsync();
		void stop();
		void addSubscription(UPnPEventSubscription & subscription);
		void removeSubscription(UPnPEventSubscription & subscription);
		void addEventListener(osl::AutoRef<UPnPEventListener> listener);
		UPnPEventSubscription & findSubscriptionByUdnAndServiceType(const UDN & udn, const std::string & serviceType);
		virtual void onNotify(UPnPNotify & notify);
		std::string getCallbackUrl(const std::string & host);
		std::map<std::string, std::string> parseEventNotify(const std::string & xml);
	};
}

#endif
