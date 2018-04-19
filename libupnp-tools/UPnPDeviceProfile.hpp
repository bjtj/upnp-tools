#ifndef __UPNP_PROFILE_HPP__
#define __UPNP_PROFILE_HPP__

#include <string>
#include <vector>
#include <map>
#include "UPnPModels.hpp"
#include "UPnPTerms.hpp"


namespace UPNP {

	
	/**
	 * @brief 
	 */
	class UPnPDeviceProfile {
	private:
		bool _enabled;
		OS::AutoRef<UPnPDevice> _device;
		
	public:
		UPnPDeviceProfile();
		UPnPDeviceProfile(OS::AutoRef<UPnPDevice> device);
		virtual ~UPnPDeviceProfile();
		bool & enabled();
		bool enabled() const;
		UDN udn() const;
		void setUdn(const UDN & udn);
		std::string deviceDescription() const;
		OS::AutoRef<UPnPDevice> & device();
		std::vector<std::string> deviceTypes() const;
		std::vector< OS::AutoRef<UPnPService> > allServices() const;
		std::vector<std::string> serviceTypes() const;
		OS::AutoRef<UPnPService> getService(const std::string & serviceType);
		OS::AutoRef<UPnPDevice> getDeviceByType(const std::string & deviceType);
	};
}

#endif
