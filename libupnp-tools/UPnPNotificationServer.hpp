#ifndef __UPNP_EVENT_SUBSCRIBER_SERVER_HPP__
#define __UPNP_EVENT_SUBSCRIBER_SERVER_HPP__

#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/Properties.hpp>
#include <libhttp-server/AnotherHttpServer.hpp>
#include "UPnPEventSubscriber.hpp"
#include "UPnPEventSubscription.hpp"
#include "UPnPEventSubscriptionRegistry.hpp"

namespace UPNP {

	/**
	 * @brief
	 */

	class UPnPNotificationServerConfig : public UTIL::Properties {
	private:
	public:
		UPnPNotificationServerConfig() {}
		UPnPNotificationServerConfig(int port) { setProperty("listen.port", port); }
		virtual ~UPnPNotificationServerConfig() {}
	};

	/**
	 * @brief
	 */
	class UPnPNotificationServer : public UPnPNotificationListener {
	private:
		UPnPNotificationServerConfig config;
		UPnPEventSubscriptionRegistry registry;
		std::vector<UTIL::AutoRef<UPnPNotificationListener> > listeners;
		HTTP::AnotherHttpServer * server;
	public:
		UPnPNotificationServer(UPnPNotificationServerConfig & config);
		virtual ~UPnPNotificationServer();
		void startAsync();
		void stop();
		void addSubscription(UPnPEventSubscription & subscription);
		void removeSubscription(UPnPEventSubscription & subscription);
		void addEventNoitfyListener(UTIL::AutoRef<UPnPNotificationListener> listener);
		UPnPEventSubscription & findSubscriptionWithUdnAndServiceType(const std::string & udn, const std::string & serviceType);
		virtual void onNotify(UPnPNotify & notify);
		std::string getCallbackUrl(const std::string & host);
	};
}

#endif
