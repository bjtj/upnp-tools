#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include <string>
#include <map>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <liboslayer/Properties.hpp>
#include <liboslayer/StringElements.hpp>
#include <liboslayer/Timer.hpp>
#include "UPnPNotificationCenter.hpp"
#include "UPnPDeviceProfileSession.hpp"
#include "UPnPDeviceProfileSessionManager.hpp"
#include "UPnPActionHandler.hpp"
#include "SSDPServer.hpp"
#include "NetworkStateManager.hpp"

namespace UPNP {

	/**
	 * @brief upnp server
	 */
	class UPnPServer {
	public:
		class Config : public UTIL::Properties {
		public:
			Config(int port) {
				setProperty("listen.port", port);
			}
			virtual ~Config() {}
		};

	private:
		static std::string SERVER_INFO;
		
		UTIL::AutoRef<NetworkStateManager> networkStateManager;
		Config config;
		UPnPDeviceProfileSessionManager profileManager;
		HTTP::AnotherHttpServer * httpServer;
		UTIL::AutoRef<UPnPActionHandler> actionHandler;
		UPnPNotificationCenter notificationCenter;
		UPnPEventNotifyThread notifyThread;
		UTIL::TimerLooperThread timerThread;
		SSDP::SSDPServer ssdpServer;
		UTIL::AutoRef<SSDP::SSDPEventHandler> ssdpEventHandler;

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
		std::string makeLocation(UPnPDeviceProfile & profile);

		// announce
		void notifyAlive(UPnPDeviceProfile & profile);
		void notifyAliveWithDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		std::string makeNotifyAlive(const std::string & location, const std::string & uuid, const std::string & deviceType);
		void notifyByeBye(UPnPDeviceProfile & profile);
		void notifyByeByeWithDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		std::string makeNotifyByeBye(const std::string & uuid, const std::string & deviceType);

		// m-search response
		void respondMsearch(const std::string & st, OS::InetAddress & remoteAddr);
		std::string makeMsearchResponse(const std::string & location, const std::string & uuid, const std::string & st);

		// device profile management
		UPnPDeviceProfileSessionManager & getProfileManager();
		void registerDeviceProfile(const std::string & uuid, const UPnPDeviceProfile & profile);
		void unregisterDeviceProfile(const std::string & uuid);

		// functionality
		void setActionHandler(UTIL::AutoRef<UPnPActionHandler> actionHandler);
		UTIL::AutoRef<UPnPActionHandler> getActionHandler();

		// event notification
		UPnPNotificationCenter & getNotificationCenter();
		UPnPEventNotifyThread & getEventNotifyThread();

		// session timeout manager
		UTIL::TimerLooperThread & getTimerThread();
	};
}

#endif
