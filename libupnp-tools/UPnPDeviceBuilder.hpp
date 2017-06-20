#ifndef __UPNP_DEVICE_BUILDER_HPP__
#define __UPNP_DEVICE_BUILDER_HPP__

#include <libhttp-server/Url.hpp>
#include "UPnPModels.hpp"

namespace UPNP {

	/**
	 * upnp device builder
	 */
	class UPnPDeviceBuilder {
	private:
		HTTP::Url _url;
		OS::AutoRef<UPnPDevice> _device;
		bool _allow_fail_scpd;
	public:
		UPnPDeviceBuilder(const HTTP::Url & url);
		virtual ~UPnPDeviceBuilder();
		HTTP::Url & url();
		bool & allow_fail_scpd();
		OS::AutoRef<UPnPDevice> & device();
		OS::AutoRef<UPnPDevice> execute();
	};
}

#endif
