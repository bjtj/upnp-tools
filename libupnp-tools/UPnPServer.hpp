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
	 * @brief 
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
	 * @brief 
	 */
	class UPnPDeviceProfileSessionManager {
	private:
		std::map<std::string, UTIL::AutoRef<UPnPDeviceProfileSession> > _sessions;
	public:
		UPnPDeviceProfileSessionManager();
		virtual ~UPnPDeviceProfileSessionManager();
		std::map<std::string, UTIL::AutoRef<UPnPDeviceProfileSession> > & sessions();
		std::vector<UTIL::AutoRef<UPnPDeviceProfileSession> > sessionList();
		void registerProfile(const UPnPDeviceProfile & profile);
		void registerProfile(const std::string & uuid, const UPnPDeviceProfile & profile);
		void unregisterProfile(const std::string & uuid);
		std::vector<std::string> getAllTypes();
		std::vector<std::string> getTypes(const std::string & st);
		bool hasDeviceProfileSessionByUuid(const std::string & uuid);
		bool hasDeviceProfileSessionByScpdUrl(const std::string & scpdUrl);
		bool hasDeviceProfileSessionByControlUrl(const std::string & controlUrl);
		bool hasDeviceProfileSessionByEventSubUrl(const std::string & eventSubUrl);
		UTIL::AutoRef<UPnPDeviceProfileSession> getDeviceProfileSessionByUuid(const std::string & uuid);
		UTIL::AutoRef<UPnPDeviceProfileSession> getDeviceProfileSessionHasScpdUrl(const std::string & scpdUrl);
		UTIL::AutoRef<UPnPDeviceProfileSession> getDeviceProfileSessionHasEventSubUrl(const std::string & eventSubUrl);
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

		UTIL::AutoRef<NetworkStateManager> networkStateManager;
		Config config;
		UPnPDeviceProfileSessionManager profileManager;
		HTTP::AnotherHttpServer * httpServer;
		UTIL::AutoRef<UPnPActionRequestHandler> actionRequestHandler;
		UPnPPropertyManager propertyManager;
		UPnPEventNotificationThread notificationThread;
		UTIL::TimerLooperThread timerThread;
		SSDP::SSDPServer ssdpServer;
		UTIL::AutoRef<SSDP::SSDPEventListener> ssdpListener;

	private:
		
		// do not allow copy or assign
		UPnPServer(const UPnPServer & other);
		UPnPServer & operator=(const UPnPServer & other);
		
	public:
		UPnPServer(const Config & config);
		UPnPServer(const Config & config, UTIL::AutoRef<NetworkStateManager> networkStateManager);
		virtual ~UPnPServer();
		void startAsync();
		void stop();
		std::string makeLocation(const std::string & uuid);

		void setEnableDevice(const std::string & udn, bool enable);
		void setEnableAllDevices(bool enable);

		// announce
		void delayNotify(unsigned long delay, int type, const UPnPDeviceProfile & profile);
		void notifyAliveAll();
		void notifyAlive(UPnPDeviceProfile & profile);
		void notifyAliveByDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		std::string makeNotifyAlive(const std::string & location, const std::string & uuid, const std::string & deviceType);
		void notifyByeBye(UPnPDeviceProfile & profile);
		void notifyByeByeByDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		std::string makeNotifyByeBye(const std::string & uuid, const std::string & deviceType);

		// m-search response
		void respondMsearch(const std::string & st, OS::InetAddress & remoteAddr);
		std::string makeMsearchResponse(const std::string & location, const std::string & uuid, const std::string & st);

		// device profile management
		UPnPDeviceProfileSessionManager & getProfileManager();
		void registerDeviceProfile(const HTTP::Url & url);
		void registerDeviceProfile(const std::string & uuid, const HTTP::Url & url);
		void registerDeviceProfile(const UPnPDeviceProfile & profile);
		void registerDeviceProfile(const std::string & uuid, const UPnPDeviceProfile & profile);
		void unregisterDeviceProfile(const std::string & uuid);

		// functionality
		void setActionRequestHandler(UTIL::AutoRef<UPnPActionRequestHandler> actionRequestHandler);
		UTIL::AutoRef<UPnPActionRequestHandler> getActionRequestHandler();

		// event notification
		UPnPPropertyManager & getPropertyManager();
		void setProperties(const std::string & udn, const std::string & serviceyType, UTIL::LinkedStringMap & props);
		void notifyEvent(const std::string & sid);
		void delayNotifyEvent(const std::string & sid, unsigned long delay);
		std::string onSubscribe(const UPnPDeviceProfile & device, const UPnPServiceProfile & service, const std::vector<std::string> & callbacks, unsigned long timeout);
		void onRenewSubscription(const std::string & sid, unsigned long timeout);
		void onUnsubscribe(const std::string & sid);
		std::vector<std::string> parseCallbackUrls(const std::string & urls);
		unsigned long parseTimeout(const std::string & phrase);

		// session timeout manager
		UTIL::TimerLooperThread & getTimerThread();

		void collectOutdated();
	};
}

#endif
