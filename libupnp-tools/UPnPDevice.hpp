#ifndef __UPNP_DEVICE_HPP__
#define __UPNP_DEVICE_HPP__

#include <map>
#include <vector>
#include <string>

#include "UPnPService.hpp"

namespace UPNP {

	/**
	 * @brief upnp device
	 */
	class UPnPDevice {
	private:
		
		std::map<std::string, std::string> properties;
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
		void removeEmbeddedDevice(size_t index);
		UPnPDevice & getEmbeddedDevice(int index);
		std::vector<UPnPDevice> & getEmbeddedDevices();
		void addService(UPnPService & service);
		void removeService(size_t index);
		UPnPService getService(std::string serviceType);
        std::vector<UPnPService> & getServices();

		std::string & operator[] (const std::string & name);
	};
}

#endif
