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
	public:
		UPnPDeviceBuilder(const HTTP::Url & url);
		virtual ~UPnPDeviceBuilder();
		HTTP::Url & url();
		OS::AutoRef<UPnPDevice> & device();
		OS::AutoRef<UPnPDevice> execute();
	};
}

#endif
