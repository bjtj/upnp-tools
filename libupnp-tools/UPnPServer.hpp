#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include <string>
#include <map>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <libhttp-server/Url.hpp>
#include <liboslayer/Properties.hpp>
#include <liboslayer/StringElements.hpp>
#include <liboslayer/Timer.hpp>
#include "UPnPPropertyManager.hpp"
#include "UPnPActionRequestHandler.hpp"
#include "SSDPServer.hpp"
#include "NetworkStateManager.hpp"
#include "UPnPDeviceProfile.hpp"
#include "UPnPDebug.hpp"


namespace upnp {


	/**
	 * @brief http event listener
	 */
	class HttpEventListener
	{
	public:
		HttpEventListener() {}
		virtual ~HttpEventListener() {}
		virtual void onDeviceDescriptionRequest(http::HttpRequest & request, http::HttpResponse & response) = 0;
		virtual void onScpdRequest(http::HttpRequest & request, http::HttpResponse & response) = 0;
		virtual void onControlRequest(http::HttpRequest & request, http::HttpResponse & response) = 0;
	};


	/**
	 * 
	 */
	class UPnPDeviceProfileManager {
	private:
		std::vector< osl::AutoRef<UPnPDeviceProfile> > _profiles;
	public:
		UPnPDeviceProfileManager();
		virtual ~UPnPDeviceProfileManager();
		std::vector<osl::AutoRef<UPnPDeviceProfile> > & profiles();
		void registerProfile(osl::AutoRef<UPnPDeviceProfile> profile);
		void unregisterProfile(const std::string & udn);
		std::vector<std::string> getAllTypes();
		std::vector<std::string> getTypes(const std::string & st);
		osl::AutoRef<UPnPDeviceProfile> getDeviceProfile(const std::string & udn);
		osl::AutoRef<UPnPDeviceProfile> getDeviceProfileHasScpdUrl(const std::string & scpdUrl);
		osl::AutoRef<UPnPDeviceProfile> getDeviceProfileHasControlUrl(const std::string & controlUrl);
		osl::AutoRef<UPnPDeviceProfile> getDeviceProfileHasEventSubUrl(const std::string & eventSubUrl);
	};
	

	/**
	 * @brief upnp server
	 */
	class UPnPServer {
	public:

		/**
		 * @brief config
		 */
		class Config : public osl::Properties {
		public:
			Config(int port) {
				setProperty("listen.port", port);
			}
			virtual ~Config() {}
		};

	private:
		
		static std::string DEFAULT_SERVER_INFO;

		osl::AutoRef<NetworkStateManager> networkStateManager;
		Config config;
		UPnPDeviceProfileManager profileManager;
		osl::AutoRef<http::AnotherHttpServer> httpServer;
		osl::AutoRef<UPnPActionRequestHandler> actionRequestHandler;
		osl::AutoRef<HttpEventListener> httpEventListener;
		UPnPPropertyManager propertyManager;
		UPnPEventNotificationThread notificationThread;
		osl::TimerLooperThread timerThread;
		ssdp::SSDPServer ssdpServer;
		osl::AutoRef<ssdp::SSDPEventListener> ssdpListener;

	private:
		
		// do not allow copy or assign
		UPnPServer(const UPnPServer & other);
		UPnPServer & operator=(const UPnPServer & other);
		
	public:
		UPnPServer(const Config & config);
		UPnPServer(const Config & config,
				   osl::AutoRef<NetworkStateManager> networkStateManager);
		virtual ~UPnPServer();
		void startAsync();
		void stop();
		osl::AutoRef<http::AnotherHttpServer> getHttpServer();
		std::string makeLocation(const std::string & udn);

		void activateDevice(const std::string & udn);
		void deactivateDevice(const std::string & udn);
		void activateAllDevices();
		void deactivateAllDevices();

		// announce
		void delayNotify(unsigned long delay, int type, osl::AutoRef<UPnPDeviceProfile> profile);
		void notifyAliveAll();
		void notifyAlive(osl::AutoRef<UPnPDeviceProfile> profile);
		void notifyAliveByDeviceType(osl::AutoRef<UPnPDeviceProfile> profile,
									 const std::string & deviceType);
		std::string makeNotifyAlive(const std::string & location,
									const std::string & udn,
									const std::string & deviceType);
		void notifyByeBye(osl::AutoRef<UPnPDeviceProfile> profile);
		void notifyByeByeByDeviceType(osl::AutoRef<UPnPDeviceProfile> profile,
									  const std::string & deviceType);
		std::string makeNotifyByeBye(const std::string & udn, const std::string & deviceType);

		// m-search response
		void respondMsearch(const std::string & st, osl::InetAddress & remoteAddr);
		std::string makeMsearchResponse(const std::string & location,
										const std::string & udn,
										const std::string & st);

		// device profile management
		UPnPDeviceProfileManager & getProfileManager();
		osl::AutoRef<UPnPDeviceProfile> registerDeviceProfile(const http::Url & url);
		void registerDeviceProfile(osl::AutoRef<UPnPDeviceProfile> profile);
		void unregisterDeviceProfile(const std::string & udn);

		// application level control
		void setActionRequestHandler(osl::AutoRef<UPnPActionRequestHandler> actionRequestHandler);
		osl::AutoRef<UPnPActionRequestHandler> getActionRequestHandler();
		void setHttpEventListener(osl::AutoRef<HttpEventListener> httpEventListener);
		osl::AutoRef<HttpEventListener> getHttpEventListener();

		// event notification
		UPnPPropertyManager & getPropertyManager();
		void setProperty(const std::string & udn, const std::string & serviceyType,
						 const std::string & name, const std::string & value);
		void setProperties(const std::string & udn, const std::string & serviceyType,
						   osl::LinkedStringMap & props);
		void notifyEvent(const std::string & sid);
		void delayNotifyEvent(const std::string & sid, unsigned long delay);
		std::string onSubscribe(osl::AutoRef<UPnPDevice> device,
								osl::AutoRef<UPnPService> service,
								const std::vector<std::string> & callbacks,
								unsigned long timeout);
		bool onRenewSubscription(const std::string & sid, unsigned long timeout);
		bool onUnsubscribe(const std::string & sid);
		std::vector<std::string> parseCallbackUrls(const std::string & urls);
		unsigned long parseTimeoutMilli(const std::string & phrase);

		// session timeout manager
		osl::TimerLooperThread & getTimerThread();

		void collectExpired();

		// information
		std::string getServerInfo();
	};
}

#endif
