#ifndef __UPNP_DEVICE_HPP__
#define __UPNP_DEVICE_HPP__

#include <vector>
#include <string>

#include "UPnPService.hpp"

namespace UPNP {

	/**
	 * @brief upnp device
	 */
	class UPnPDevice {
	private:

		std::string udn;
		std::string friendlyName;
		
		UPnPDevice * parent;
		std::vector<UPnPDevice> embeddedDevices;
		std::vector<UPnPService> services;
		
	public:
		UPnPDevice();
		virtual ~UPnPDevice();


		void setUdn(std::string udn);
		std::string getUdn();
		void setFriendlyName(std::string friendlyName);
		std::string getFriendlyName();
		
		void setParentDevice(UPnPDevice * parent);
		UPnPDevice * getParentDevice();
		bool isRootDevice();
		void addEmbeddedDevice(UPnPDevice & embeddedDevice);
		void removeEmbeddedDevice(UPnPDevice & embeddedDevice);
		UPnPDevice getEmbeddedDevice(int index);
		void addService(UPnPService & service);
		void removeService(UPnPService & service);
		UPnPService getService(std::string serviceType);
	};
}

#endif
