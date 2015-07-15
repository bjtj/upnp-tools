#ifndef __UPNP_DEVICE_HPP__
#define __UPNP_DEVICE_HPP__

#include <vector>
#include <string>

#include "UPnPService.hpp"

namespace UPNP {

	class UPnPDevice {
	private:
		UPnPDevice * parent;
		std::vector<UPnPDevice *> embeddedDevices;
		std::vector<UPnPService *> services;
		
	public:
		UPnPDevice();
		virtual ~UPnPDevice();
	};
	
	class UPnPDevicePool {
	private:
		std::vector<UPnPDevice*> devices;
	public:
		UPnPDevicePool();
		virtual ~UPnPDevicePool();

		void add(UPnPDevice * device);
		void remove(UPnPDevice * device);
		UPnPDevice & getDevice(std::string udn);
	};

}

#endif
