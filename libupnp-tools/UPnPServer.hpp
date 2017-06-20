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
	 * @brief upnp device profile session
	 */
	class UPnPDeviceProfileSession {
	private:
		bool enabled;
		UPnPDeviceProfile _profile;
	public:
		UPnPDeviceProfileSession(const UPnPDeviceProfile & profile);
		virtual ~UPnPDeviceProfileSession();
		UPnPDeviceProfile & profile();
		void setEnable(bool enable);
		bool isEnabled();
	};

	/**
	 * @brief upnp device profile session manager
	 */
	class UPnPDeviceProfileSessionManager {
	private:
		std::map< UDN, OS::AutoRef<UPnPDeviceProfileSession> > _sessions;
	public:
		UPnPDeviceProfileSessionManager();
		virtual ~UPnPDeviceProfileSessionManager();
		std::map<UDN, OS::AutoRef<UPnPDeviceProfileSession> > & sessions();
		std::vector<OS::AutoRef<UPnPDeviceProfileSession> > sessionList();
		void registerProfile(const UPnPDeviceProfile & profile);
		void registerProfile(const UDN & udn, const UPnPDeviceProfile & profile);
		void unregisterProfile(const UDN & udn);
		std::vector<std::string> getAllTypes();
		std::vector<std::string> getTypes(const std::string & st);
		bool hasDeviceProfileSessionByUDN(const UDN & udn);
		bool hasDeviceProfileSessionByScpdUrl(const std::string & scpdUrl);
		bool hasDeviceProfileSessionByControlUrl(const std::string & controlUrl);
		bool hasDeviceProfileSessionByEventSubUrl(const std::string & eventSubUrl);
		OS::AutoRef<UPnPDeviceProfileSession> getDeviceProfileSessionByUDN(const UDN & udn);
		OS::AutoRef<UPnPDeviceProfileSession> getDeviceProfileSessionHasScpdUrl(const std::string & scpdUrl);
		OS::AutoRef<UPnPDeviceProfileSession> getDeviceProfileSessionHasEventSubUrl(const std::string & eventSubUrl);
	};

	/**
	 * @brief upnp server
	 */
	class UPnPServer : public UPnPDebuggable {
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
		UPnPDeviceProfileSessionManager profileManager;
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
		UPnPServer(const Config & config, OS::AutoRef<NetworkStateManager> networkStateManager);
		virtual ~UPnPServer();
		void startAsync();
		void stop();
		OS::AutoRef<HTTP::AnotherHttpServer> getHttpServer();
		std::string makeLocation(const UDN & udn);

		void setEnableDevice(const UDN & udn, bool enable);
		void setEnableAllDevices(bool enable);

		// announce
		void delayNotify(unsigned long delay, int type, const UPnPDeviceProfile & profile);
		void notifyAliveAll();
		void notifyAlive(UPnPDeviceProfile & profile);
		void notifyAliveByDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		std::string makeNotifyAlive(const std::string & location, const UDN & udn, const std::string & deviceType);
		void notifyByeBye(UPnPDeviceProfile & profile);
		void notifyByeByeByDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		std::string makeNotifyByeBye(const UDN & udn, const std::string & deviceType);

		// m-search response
		void respondMsearch(const std::string & st, OS::InetAddress & remoteAddr);
		std::string makeMsearchResponse(const std::string & location, const UDN & udn, const std::string & st);

		// device profile management
		UPnPDeviceProfileSessionManager & getProfileManager();
		void registerDeviceProfile(const HTTP::Url & url);
		void registerDeviceProfile(const UDN & udn, const HTTP::Url & url);
		void registerDeviceProfile(const UPnPDeviceProfile & profile);
		void registerDeviceProfile(const UDN & udn, const UPnPDeviceProfile & profile);
		void unregisterDeviceProfile(const UDN & udn);

		// application level control
		void setActionRequestHandler(OS::AutoRef<UPnPActionRequestHandler> actionRequestHandler);
		OS::AutoRef<UPnPActionRequestHandler> getActionRequestHandler();
		void setHttpEventListener(OS::AutoRef<HttpEventListener> httpEventListener);
		OS::AutoRef<HttpEventListener> getHttpEventListener();

		// event notification
		UPnPPropertyManager & getPropertyManager();
		void setProperties(const UDN & udn, const std::string & serviceyType, UTIL::LinkedStringMap & props);
		void notifyEvent(const std::string & sid);
		void delayNotifyEvent(const std::string & sid, unsigned long delay);
		std::string onSubscribe(const UPnPDeviceProfile & device, const UPnPServiceProfile & service, const std::vector<std::string> & callbacks, unsigned long timeout);
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
