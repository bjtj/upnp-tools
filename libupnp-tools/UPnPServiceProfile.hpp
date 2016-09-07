#ifndef __UPNP_SERVICE_PROFILE_HPP__
#define __UPNP_SERVICE_PROFILE_HPP__

#include <string>

namespace UPNP {
	
	class UPnPServiceProfile {
	private:
		std::string _serviceType;
		std::string _serviceId;
		std::string _scpd;
		std::string _scpdUrl;
		std::string _controlUrl;
		std::string _eventSubUrl;
		
	public:
		UPnPServiceProfile() {}
		virtual ~UPnPServiceProfile() {}

		std::string & serviceType() {return _serviceType;}
		std::string & serviceId() {return _serviceId;}
		std::string & scpd() {return _scpd;}
		std::string & scpdUrl() {return _scpdUrl;}
		std::string & controlUrl() {return _controlUrl;}
		std::string & eventSubUrl() {return _eventSubUrl;}

		std::string const_serviceType() const {return _serviceType;}
		std::string const_serviceId() const {return _serviceId;}
		std::string const_scpd() const {return _scpd;}
		std::string const_scpdUrl() const {return _scpdUrl;}
		std::string const_controlUrl() const {return _controlUrl;}
		std::string const_eventSubUrl() const {return _eventSubUrl;}
	};

	
}
#endif
