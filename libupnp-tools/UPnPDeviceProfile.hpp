#ifndef __UPNP_PROFILE_HPP__
#define __UPNP_PROFILE_HPP__

#include <string>
#include <vector>
#include <map>
#include "UPnPModels.hpp"
#include "UPnPTerms.hpp"


namespace upnp {

	
	/**
	 * @brief 
	 */
	class UPnPDeviceProfile {
	private:
		bool _enabled;
		osl::AutoRef<UPnPDevice> _device;
		
	public:
		UPnPDeviceProfile();
		UPnPDeviceProfile(osl::AutoRef<UPnPDevice> device);
		virtual ~UPnPDeviceProfile();
		bool & enabled();
		bool enabled() const;
		UDN udn() const;
		void setUdn(const UDN & udn);
		std::string deviceDescription() const;
		osl::AutoRef<UPnPDevice> & device();
		std::vector<std::string> deviceTypes() const;
		std::vector< osl::AutoRef<UPnPService> > allServices() const;
		std::vector<std::string> serviceTypes() const;
		osl::AutoRef<UPnPService> getService(const std::string & serviceType);
		osl::AutoRef<UPnPDevice> getDeviceByType(const std::string & deviceType);
	};
}

#endif
