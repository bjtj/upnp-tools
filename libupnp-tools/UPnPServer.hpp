#ifndef __UPNP_SERVER_HPP__
#define __UPNP_SERVER_HPP__

#include "UPnPDeviceProfile.hpp"
#include "UPnPServerProfile.hpp"
#include "UPnPActionHandler.hpp"
#include <string>
#include <map>
#include <libhttp-server/AnotherHttpServer.hpp>

namespace UPNP {

	/**
	 * @breif upnp server
	 */
	class UPnPServer {
	private:
		UPnPServerProfile profile;
		std::map<std::string, UPnPDeviceProfile> deviceProfiles;
		HTTP::AnotherHttpServer * httpServer;
		UTIL::AutoRef<UPnPActionHandler> actionHandler;

	private:
		// do not allow copy or assign
		UPnPServer(const UPnPServer & other);
		UPnPServer & operator=(const UPnPServer & other);
		
	public:
		UPnPServer(UPnPServerProfile & profile);
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
		UPnPDeviceProfile & operator[] (const std::string & udn);
		void setActionHandler(UTIL::AutoRef<UPnPActionHandler> actionHandler);
		UTIL::AutoRef<UPnPActionHandler> getActionHandler();
	};
}

#endif
