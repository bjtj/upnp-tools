#ifndef __UPNP_DEVICE_POLL_HPP__
#define __UPNP_DEVICE_POLL_HPP__

#include <libhttp-server/Http.hpp>
#include <vector>
#include <string>
#include "UPnPDevice.hpp"

namespace UPNP {

	/**
	 * @brief upnp device pool
	 */
	class UPnPDevicePool {
	private:
		std::vector<UPnPDevice> devices;
		
	public:
		UPnPDevicePool();
		virtual ~UPnPDevicePool();

		void requestDeviceDescription(std::string url);
		void requestScpd(std::string url);

		UPnPDevice getDevice(std::string udn);
		
	};

}

#endif
