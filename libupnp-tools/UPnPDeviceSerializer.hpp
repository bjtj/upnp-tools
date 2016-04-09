#ifndef __UPNP_DEVICE_SERIALIZER_HPP__
#define __UPNP_DEVICE_SERIALIZER_HPP__

#include "UPnPModels.hpp"

namespace UPNP {

	class UPnPDeviceSerializer {
	private:
	public:
		UPnPDeviceSerializer();
		virtual ~UPnPDeviceSerializer();

		static std::string serializeDeviceDescription(UPnPDevice & device);
		static std::string serializeScpd(UPnPService & service);
	};

}

#endif
