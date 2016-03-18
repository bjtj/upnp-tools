#ifndef __UPNP_SERVICE_PROFILE_HPP__
#define __UPNP_SERVICE_PROFILE_HPP__

#include <string>

namespace UPNP {
	
	class UPnPServiceProfile {
	private:
		std::string _serviceType;
		std::string _scpd;
		std::string _scpdUrl;
		std::string _controlUrl;
		std::string _eventSubUrl;
	public:
		UPnPServiceProfile() {}
		virtual ~UPnPServiceProfile() {}

		std::string & serviceType() {return _serviceType;}
		std::string & scpd() {return _scpd;}
		std::string & scpdUrl() {return _scpdUrl;}
		std::string & controlUrl() {return _controlUrl;}
		std::string & eventSubUrl() {return _eventSubUrl;}
	};

	
}
#endif
