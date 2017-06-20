#ifndef __UPNP_DEVICE_PROFILE_BUILDER_HPP__
#define __UPNP_DEVICE_PROFILE_BUILDER_HPP__

#include <string>
#include <liboslayer/AutoRef.hpp>
#include "UPnPDeviceProfile.hpp"
#include "UPnPModels.hpp"
#include "UPnPLocationResolver.hpp"
#include "UPnPTerms.hpp"

namespace UPNP {

	/**
	 * @brief 
	 */
	class UPnPDeviceProfileBuilder {
	private:
		UDN _udn;
		OS::AutoRef<UPnPDevice> _device;
		OS::AutoRef<UPnPLocationResolver> _locationResolver;
		
	public:
		UPnPDeviceProfileBuilder(OS::AutoRef<UPnPDevice> device);
		UPnPDeviceProfileBuilder(const UDN & udn, OS::AutoRef<UPnPDevice> device);
		virtual ~UPnPDeviceProfileBuilder();
		UDN & udn();
		OS::AutoRef<UPnPDevice> device();
		std::vector<OS::AutoRef<UPnPDevice> > selectAllDevices(OS::AutoRef<UPnPDevice> device);
		UPnPDeviceProfile build();
		void setLocationResolver(OS::AutoRef<UPnPLocationResolver> locationResolver);
	};
}

#endif
