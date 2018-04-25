#ifndef __UPNP_DEVICE_LISTENER_HPP__
#define __UPNP_DEVICE_LISTENER_HPP__

#include <liboslayer/AutoRef.hpp>
#include "UPnPModels.hpp"

namespace upnp {

	/**
	 * upnp device listener
	 */
	class UPnPDeviceListener {
	private:
	public:
		UPnPDeviceListener() {}
		virtual ~UPnPDeviceListener() {}
		virtual void onDeviceAdded(osl::AutoRef<UPnPDevice> device) {}
		virtual void onDeviceUpdated(osl::AutoRef<UPnPDevice> device) {}
		virtual void onDeviceRemoved(osl::AutoRef<UPnPDevice> device) {}
	};
}

#endif
