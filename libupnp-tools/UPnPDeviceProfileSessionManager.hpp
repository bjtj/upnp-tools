#ifndef __UPNP_DEVICE_PROFILE_SESSION_MANAGER_HPP__
#define __UPNP_DEVICE_PROFILE_SESSION_MANAGER_HPP__

#include <string>
#include <map>
#include "UPnPDeviceProfileSession.hpp"

namespace UPNP {
	
	class UPnPDeviceProfileSessionManager {
	private:
		std::map<std::string, UTIL::AutoRef<UPnPDeviceProfileSession> > _sessions;
	public:
		UPnPDeviceProfileSessionManager();
		virtual ~UPnPDeviceProfileSessionManager();
		std::map<std::string, UTIL::AutoRef<UPnPDeviceProfileSession> > & sessions();

		void registerProfile(const std::string & uuid, const UPnPDeviceProfile & profile);
		std::vector<UTIL::AutoRef<UPnPDeviceProfileSession> > searchProfileSessions(const std::string & st);
		bool hasDeviceProfileSessionWithScpdUrl(const std::string & scpdUrl);
		bool hasDeviceProfileSessionWithControlUrl(const std::string & controlUrl);
		bool hasDeviceProfileSessionWithEventSubUrl(const std::string & eventSubUrl);
		UTIL::AutoRef<UPnPDeviceProfileSession> getDeviceProfileSessionWithUuid(const std::string & udn);
		UTIL::AutoRef<UPnPDeviceProfileSession> getDeviceProfileSessionHasScpdUrl(const std::string & scpdUrl);
		UTIL::AutoRef<UPnPDeviceProfileSession> getDeviceProfileSessionHasEventSubUrl(const std::string & eventSubUrl);
	};
}

#endif
