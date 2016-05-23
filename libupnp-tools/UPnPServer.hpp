#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include <string>
#include <map>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <liboslayer/Properties.hpp>
#include <liboslayer/StringElement.hpp>
#include <liboslayer/Timer.hpp>
#include "UPnPNotificationCenter.hpp"
#include "UPnPDeviceProfileSession.hpp"
#include "UPnPDeviceProfileSessionManager.hpp"
#include "UPnPActionHandler.hpp"
#include "SSDPServer.hpp"
#include "NetworkStateManager.hpp"

namespace UPNP {

	/**
	 * @brief
	 */
	class UPnPServerConfig : public UTIL::Properties {
	private:
	public:
		UPnPServerConfig(int port) { setProperty("listen.port", port); };
		virtual ~UPnPServerConfig() {}
	};

	/**
	 * @brief upnp server
	 */
	class UPnPServer {
	private:
		static std::string SERVER_INFO;
		
		UTIL::AutoRef<NetworkStateManager> networkStateManager;
		UPnPServerConfig config;
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
		UPnPServer(UPnPServerConfig & config);
		UPnPServer(UPnPServerConfig & config, UTIL::AutoRef<NetworkStateManager> networkStateManager);
		virtual ~UPnPServer();
		void startAsync();
		void stop();
		std::string makeLocation(UPnPDeviceProfile & profile);

		// announcing
		void notifyAlive(UPnPDeviceProfile & profile);
		void notifyAliveWithDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		std::string makeNotifyAlive(const std::string & location, const std::string & uuid, const std::string & deviceType);
		void notifyByeBye(UPnPDeviceProfile & profile);
		void notifyByeByeWithDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		std::string makeNotifyByeBye(const std::string & uuid, const std::string & deviceType);

		// responding & searching
		void respondMsearch(const std::string & st, OS::InetAddress & remoteAddr);
		std::string makeMsearchResponse(const std::string & location, const std::string & uuid, const std::string & st);

		// profile management
		UPnPDeviceProfileSessionManager & getProfileManager();
		void registerDeviceProfile(const std::string & uuid, const UPnPDeviceProfile & profile);

		// functionality
		void setActionHandler(UTIL::AutoRef<UPnPActionHandler> actionHandler);
		UTIL::AutoRef<UPnPActionHandler> getActionHandler();

		// event notifying
		UPnPNotificationCenter & getNotificationCenter();
		UPnPEventNotifyThread & getEventNotifyThread();

		// session managin
		UTIL::TimerLooperThread & getTimerThread();
	};
}

#endif
