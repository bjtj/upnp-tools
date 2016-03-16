#ifndef __UPNP_PROFILE_HPP__
#define __UPNP_PROFILE_HPP__

#include <string>
#include <vector>
#include <map>
#include "UPnPModels.hpp"

namespace UPNP {

	class UPnPDeviceProfile {
	private:
		std::string _udn;
		std::string _alias;
		std::string _deviceDescription;
		std::vector<UPnPService> _services;
		std::map<std::string, std::string> _scpds;
	public:
		UPnPDeviceProfile();
		virtual ~UPnPDeviceProfile();
		std::string & udn();
		std::string & alias();
		std::string & deviceDescription();
		std::vector<UPnPService> & services();
		std::string & scpd(const std::string & serviceType);
		bool hasServiceWithServiceType(const std::string & serviceType);
		bool hasServiceWithScpdUrl(const std::string & scpdUrl);
		bool hasServiceWithControlUrl(const std::string & controlUrl);
		bool hasServiceWithEventSubUrl(const std::string & eventSubUrl);
		UPnPService & getServiceWithServiceType(const std::string & serviceType);
		UPnPService & getServiceWithScpdUrl(const std::string & scpdUrl);
		UPnPService & getServiceWithControlUrl(const std::string & controlUrl);
		UPnPService & getServiceWithEventSubUrl(const std::string & eventSubUrl);
	};
}

#endif
