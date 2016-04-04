#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include "UPnPDeviceProfile.hpp"
#include "UPnPActionHandler.hpp"
#include <string>
#include <map>
#include <libhttp-server/AnotherHttpServer.hpp>
#include <liboslayer/Properties.hpp>
#include <liboslayer/StringElement.hpp>
#include <liboslayer/Timer.hpp>
#include "UPnPNotificationCenter.hpp"

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
		UPnPServerConfig config;
		std::map<std::string, UPnPDeviceProfile> deviceProfiles;
		HTTP::AnotherHttpServer * httpServer;
		UTIL::AutoRef<UPnPActionHandler> actionHandler;
		UPnPNotificationCenter notificationCenter;
		UPnPEventNotifyThread notifyThread;
		UTIL::TimerLooperThread timerThread;

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
		void notifyAliveWithDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		void notifyByeByeWithDeviceType(UPnPDeviceProfile & profile, const std::string & deviceType);
		bool hasDeviceProfileWithScpdUrl(const std::string & scpdUrl);
		bool hasDeviceProfileWithControlUrl(const std::string & controlUrl);
		bool hasDeviceProfileWithEventSubUrl(const std::string & eventSubUrl);
		UPnPDeviceProfile & getDeviceProfileWithUdn(const std::string & udn);
		UPnPDeviceProfile & getDeviceProfileWithAlias(const std::string & alias);
		UPnPDeviceProfile & getDeviceProfileHasScpdUrl(const std::string & scpdUrl);
		UPnPDeviceProfile & getDeviceProfileHasEventSubUrl(const std::string & eventSubUrl);
		UPnPDeviceProfile & operator[] (const std::string & udn);
		void setActionHandler(UTIL::AutoRef<UPnPActionHandler> actionHandler);
		UTIL::AutoRef<UPnPActionHandler> getActionHandler();
		UPnPNotificationCenter & getNotificationCenter();
		UPnPEventNotifyThread & getEventNotifyThread();
		UTIL::TimerLooperThread & getTimerThread();	
	};
}

#endif
