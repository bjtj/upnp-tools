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
#include "UPnPActionHandler.hpp"
#include "SSDPServer.hpp"

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
	 * @breif upnp server
	 */
	class UPnPServer {
	private:
		static std::string SERVER_INFO;
		UPnPServerConfig config;
		std::map<std::string, UPnPDeviceProfile> deviceProfiles;
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
		std::vector<UPnPDeviceProfile> searchProfiles(const std::string & st);

		// resource retrieving
		bool hasDeviceProfileWithScpdUrl(const std::string & scpdUrl);
		bool hasDeviceProfileWithControlUrl(const std::string & controlUrl);
		bool hasDeviceProfileWithEventSubUrl(const std::string & eventSubUrl);
		UPnPDeviceProfile & getDeviceProfileWithUuid(const std::string & udn);
		UPnPDeviceProfile & getDeviceProfileHasScpdUrl(const std::string & scpdUrl);
		UPnPDeviceProfile & getDeviceProfileHasEventSubUrl(const std::string & eventSubUrl);
		UPnPDeviceProfile & operator[] (const std::string & uuid);

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
