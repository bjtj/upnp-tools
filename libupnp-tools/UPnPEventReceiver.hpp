#ifndef __UPNP_EVENT_SUBSCRIBER_SERVER_HPP__
#define __UPNP_EVENT_SUBSCRIBER_SERVER_HPP__

#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Properties.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include "UPnPEventSubscriber.hpp"
#include "UPnPEventSubscription.hpp"
#include "UPnPPropertySet.hpp"

namespace upnp {


	/**
	 * @brief
	 */
	class UPnPEventListener {
	private:
	public:
		UPnPEventListener() {}
		virtual ~UPnPEventListener() {}
		virtual void onNotify(UPnPPropertySet & propset) = 0;
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
		std::map< std::string, UPnPEventSubscription > subscriptions;
		std::vector< osl::AutoRef<UPnPEventListener> > listeners;
		http::AnotherHttpServer * server;
		
	public:
		UPnPEventReceiver(UPnPEventReceiverConfig & config);
		virtual ~UPnPEventReceiver();
		void startAsync();
		void stop();
		std::map< std::string, UPnPEventSubscription > getSubscriptions();
		void addSubscription(UPnPEventSubscription & subscription);
		void removeSubscription(UPnPEventSubscription & subscription);
		void addEventListener(osl::AutoRef<UPnPEventListener> listener);
		UPnPEventSubscription & findSubscriptionByUdnAndServiceType(const std::string & udn, const std::string & serviceType);
		virtual void onNotify(UPnPPropertySet & propset);
		std::string getCallbackUrl(const std::string & host);
		std::map<std::string, std::string> parsePropertySet(const std::string & xml);
	};
}

#endif
