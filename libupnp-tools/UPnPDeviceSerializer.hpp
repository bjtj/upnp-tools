#ifndef __UPNP_DEVICE_SERIALIZER_HPP__
#define __UPNP_DEVICE_SERIALIZER_HPP__

#include "UPnPModels.hpp"

namespace UPNP {

	class UPnPDeviceSerializer {
	private:
		static std::string FIRSTLINE;
		static std::string NEWLINE;
	public:
		UPnPDeviceSerializer();
		virtual ~UPnPDeviceSerializer();

		static std::string serializeDeviceDescription(UPnPDevice & device);
		static std::string serializeDevice(UPnPDevice & device);
		static std::string serializeScpd(UPnPService & service);
		static std::string serializeAction(UPnPAction & action);
		static std::string serializeArgument(UPnPArgument & argument);
		static std::string serializeStateVariable(UPnPStateVariable & stateVariable);
	};

}

#endif
