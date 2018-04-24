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


namespace UPNP {


	/**
	 * @brief http event listener
	 */
	class HttpEventListener
	{
	public:
		HttpEventListener() {}
		virtual ~HttpEventListener() {}
		virtual void onDeviceDescriptionRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response) = 0;
		virtual void onScpdRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response) = 0;
		virtual void onControlRequest(HTTP::HttpRequest & request, HTTP::HttpResponse & response) = 0;
	};


	/**
	 * 
	 */
	class UPnPDeviceProfileManager {
	private:
		std::vector< OS::AutoRef<UPnPDeviceProfile> > _profiles;
	public:
		UPnPDeviceProfileManager();
		virtual ~UPnPDeviceProfileManager();
		std::vector<OS::AutoRef<UPnPDeviceProfile> > & profiles();
		void registerProfile(OS::AutoRef<UPnPDeviceProfile> profile);
		void unregisterProfile(const UDN & udn);
		std::vector<std::string> getAllTypes();
		std::vector<std::string> getTypes(const std::string & st);
		OS::AutoRef<UPnPDeviceProfile> getDeviceProfile(const UDN & udn);
		OS::AutoRef<UPnPDeviceProfile> getDeviceProfileHasScpdUrl(const std::string & scpdUrl);
		OS::AutoRef<UPnPDeviceProfile> getDeviceProfileHasControlUrl(const std::string & controlUrl);
		OS::AutoRef<UPnPDeviceProfile> getDeviceProfileHasEventSubUrl(const std::string & eventSubUrl);
	};
	

	/**
	 * @brief upnp server
	 */
	class UPnPServer {
	public:

		/**
		 * @brief config
		 */
		class Config : public UTIL::Properties {
		public:
			Config(int port) {
				setProperty("listen.port", port);
			}
			virtual ~Config() {}
		};

	private:
		
		static std::string DEFAULT_SERVER_INFO;

		OS::AutoRef<NetworkStateManager> networkStateManager;
		Config config;
		UPnPDeviceProfileManager profileManager;
		OS::AutoRef<HTTP::AnotherHttpServer> httpServer;
		OS::AutoRef<UPnPActionRequestHandler> actionRequestHandler;
		OS::AutoRef<HttpEventListener> httpEventListener;
		UPnPPropertyManager propertyManager;
		UPnPEventNotificationThread notificationThread;
		UTIL::TimerLooperThread timerThread;
		SSDP::SSDPServer ssdpServer;
		OS::AutoRef<SSDP::SSDPEventListener> ssdpListener;

	private:
		
		// do not allow copy or assign
		UPnPServer(const UPnPServer & other);
		UPnPServer & operator=(const UPnPServer & other);
		
	public:
		UPnPServer(const Config & config);
		UPnPServer(const Config & config,
				   OS::AutoRef<NetworkStateManager> networkStateManager);
		virtual ~UPnPServer();
		void startAsync();
		void stop();
		OS::AutoRef<HTTP::AnotherHttpServer> getHttpServer();
		std::string makeLocation(const UDN & udn);

		void activateDevice(const UDN & udn);
		void deactivateDevice(const UDN & udn);
		void activateAllDevices();
		void deactivateAllDevices();

		// announce
		void delayNotify(unsigned long delay, int type, OS::AutoRef<UPnPDeviceProfile> profile);
		void notifyAliveAll();
		void notifyAlive(OS::AutoRef<UPnPDeviceProfile> profile);
		void notifyAliveByDeviceType(OS::AutoRef<UPnPDeviceProfile> profile,
									 const std::string & deviceType);
		std::string makeNotifyAlive(const std::string & location,
									const UDN & udn,
									const std::string & deviceType);
		void notifyByeBye(OS::AutoRef<UPnPDeviceProfile> profile);
		void notifyByeByeByDeviceType(OS::AutoRef<UPnPDeviceProfile> profile,
									  const std::string & deviceType);
		std::string makeNotifyByeBye(const UDN & udn, const std::string & deviceType);

		// m-search response
		void respondMsearch(const std::string & st, OS::InetAddress & remoteAddr);
		std::string makeMsearchResponse(const std::string & location,
										const UDN & udn,
										const std::string & st);

		// device profile management
		UPnPDeviceProfileManager & getProfileManager();
		OS::AutoRef<UPnPDeviceProfile> registerDeviceProfile(const HTTP::Url & url);
		void registerDeviceProfile(OS::AutoRef<UPnPDeviceProfile> profile);
		void unregisterDeviceProfile(const UDN & udn);

		// application level control
		void setActionRequestHandler(OS::AutoRef<UPnPActionRequestHandler> actionRequestHandler);
		OS::AutoRef<UPnPActionRequestHandler> getActionRequestHandler();
		void setHttpEventListener(OS::AutoRef<HttpEventListener> httpEventListener);
		OS::AutoRef<HttpEventListener> getHttpEventListener();

		// event notification
		UPnPPropertyManager & getPropertyManager();
		void setProperty(const UDN & udn, const std::string & serviceyType,
						 const std::string & name, const std::string & value);
		void setProperties(const UDN & udn, const std::string & serviceyType,
						   UTIL::LinkedStringMap & props);
		void notifyEvent(const std::string & sid);
		void delayNotifyEvent(const std::string & sid, unsigned long delay);
		std::string onSubscribe(OS::AutoRef<UPnPDevice> device,
								OS::AutoRef<UPnPService> service,
								const std::vector<std::string> & callbacks,
								unsigned long timeout);
		bool onRenewSubscription(const std::string & sid, unsigned long timeout);
		bool onUnsubscribe(const std::string & sid);
		std::vector<std::string> parseCallbackUrls(const std::string & urls);
		unsigned long parseTimeoutMilli(const std::string & phrase);

		// session timeout manager
		UTIL::TimerLooperThread & getTimerThread();

		void collectExpired();

		// information
		std::string getServerInfo();
	};
}

#endif
