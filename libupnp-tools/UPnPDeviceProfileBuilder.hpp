#ifndef __UPNP_DEVICE_PROFILE_BUILDER_HPP__
#define __UPNP_DEVICE_PROFILE_BUILDER_HPP__

#include <string>
#include <liboslayer/AutoRef.hpp>
#include "UPnPDeviceProfile.hpp"
#include "UPnPModels.hpp"

namespace UPNP {
	
	/**
	 * @brief 
	 */
	class UPnPDeviceProfileBuilder {
	private:
		std::string _uuid;
		UTIL::AutoRef<UPnPDevice> _device;
		
	public:
		UPnPDeviceProfileBuilder(UTIL::AutoRef<UPnPDevice> device);
		UPnPDeviceProfileBuilder(const std::string uuid, UTIL::AutoRef<UPnPDevice> device);
		virtual ~UPnPDeviceProfileBuilder();
		std::string & uuid();
		UTIL::AutoRef<UPnPDevice> device();
		std::vector<UTIL::AutoRef<UPnPDevice> > selectAllDevices(UTIL::AutoRef<UPnPDevice> device);
		UPnPDeviceProfile build();
		void setUdnRecursive(UTIL::AutoRef<UPnPDevice> device, const std::string & udn);
	};
}

#endif
