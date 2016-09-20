#ifndef __UPNP_PROFILE_HPP__
#define __UPNP_PROFILE_HPP__

#include <string>
#include <vector>
#include <map>
#include "UPnPModels.hpp"
#include "UPnPServiceProfile.hpp"

namespace UPNP {

	/**
	 * @brief 
	 */
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
		std::string const_uuid() const;
		std::string & deviceDescription();
		std::string const_deviceDescription() const;
		std::vector<std::string> & deviceTypes();
		std::string rootDeviceType();
		std::vector<UPnPServiceProfile> & serviceProfiles();
		std::string & scpd(const std::string & serviceType);
		bool hasDeviceType(const std::string & deviceType);
		bool hasServiceByServiceType(const std::string & serviceType);
		bool hasServiceByScpdUrl(const std::string & scpdUrl);
		bool hasServiceByControlUrl(const std::string & controlUrl);
		bool hasServiceByEventSubUrl(const std::string & eventSubUrl);
		UPnPServiceProfile & getServiceProfileByServiceType(const std::string & serviceType);
		UPnPServiceProfile & getServiceProfileByScpdUrl(const std::string & scpdUrl);
		UPnPServiceProfile & getServiceProfileByControlUrl(const std::string & controlUrl);
		UPnPServiceProfile & getServiceProfileByEventSubUrl(const std::string & eventSubUrl);
	};
}

#endif
