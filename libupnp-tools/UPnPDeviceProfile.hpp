#ifndef __UPNP_PROFILE_HPP__
#define __UPNP_PROFILE_HPP__

#include <string>
#include <vector>
#include <map>
#include "UPnPModels.hpp"
#include "UPnPServiceProfile.hpp"

namespace UPNP {

	class UPnPDeviceProfile {
	private:
		std::string _uuid;
		std::string _deviceDescription;
		std::vector<std::string> _deviceTypes;
		std::vector<UPnPServiceProfile> _serviceProfiles;
	public:
		UPnPDeviceProfile();
		virtual ~UPnPDeviceProfile();
		std::string & uuid();
		std::string & deviceDescription();
		std::vector<std::string> & deviceTypes();
		std::string rootDeviceType();
		std::vector<UPnPServiceProfile> & serviceProfiles();
		std::string & scpd(const std::string & serviceType);
		bool match(const std::string & st);
		bool hasServiceWithServiceType(const std::string & serviceType);
		bool hasServiceWithScpdUrl(const std::string & scpdUrl);
		bool hasServiceWithControlUrl(const std::string & controlUrl);
		bool hasServiceWithEventSubUrl(const std::string & eventSubUrl);
		UPnPServiceProfile & getServiceProfileWithServiceType(const std::string & serviceType);
		UPnPServiceProfile & getServiceProfileWithScpdUrl(const std::string & scpdUrl);
		UPnPServiceProfile & getServiceProfileWithControlUrl(const std::string & controlUrl);
		UPnPServiceProfile & getServiceProfileWithEventSubUrl(const std::string & eventSubUrl);
	};
}

#endif
