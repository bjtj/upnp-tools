#ifndef __UPNP_DEVICE_BUILDER_HPP__
#define __UPNP_DEVICE_BUILDER_HPP__

#include <libhttp-server/Url.hpp>
#include "UPnPModels.hpp"

namespace upnp {

	/**
	 * upnp device builder
	 */
	class UPnPDeviceBuilder {
	private:
		http::Url _url;
		osl::AutoRef<UPnPDevice> _device;
		bool _allow_fail_scpd;
	public:
		UPnPDeviceBuilder(const http::Url & url);
		virtual ~UPnPDeviceBuilder();
		http::Url & url();
		bool & allow_fail_scpd();
		osl::AutoRef<UPnPDevice> & device();
		osl::AutoRef<UPnPDevice> execute();
	};
}

#endif
