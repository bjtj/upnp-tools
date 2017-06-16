#ifndef __UPNP_DEVICE_LISTENER_HPP__
#define __UPNP_DEVICE_LISTENER_HPP__

#include <liboslayer/AutoRef.hpp>
#include "UPnPModels.hpp"

namespace UPNP {

	/**
	 * upnp device listener
	 */
	class UPnPDeviceListener {
	private:
	public:
		UPnPDeviceListener() {}
		virtual ~UPnPDeviceListener() {}
		virtual void onDeviceAdded(OS::AutoRef<UPnPDevice> device) {}
		virtual void onDeviceUpdated(OS::AutoRef<UPnPDevice> device) {}
		virtual void onDeviceRemoved(OS::AutoRef<UPnPDevice> device) {}
	};
}

#endif
