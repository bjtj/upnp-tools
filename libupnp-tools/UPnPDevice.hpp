#ifndef __UPNP_DEVICE_HPP__
#define __UPNP_DEVICE_HPP__

#include <map>
#include <vector>
#include <string>

#include "CacheControl.hpp"
#include "UPnPService.hpp"

namespace UPNP {

	/**
	 * @brief upnp device
	 */
	class UPnPDevice {
	private:
		std::map<std::string, std::string> properties;
		UPnPDevice * parent;
		mutable std::vector<UPnPDevice> embeddedDevices;
		mutable std::vector<UPnPService> services;
        std::string baseUrl;
		CacheControl cacheControl;
		
	public:
		UPnPDevice();
        UPnPDevice(const UPnPDevice & other);
		virtual ~UPnPDevice();

        UPnPDevice copy();
        void rebaseParents();
        void rebaseParents(UPnPDevice * parent);

		void setUdn(std::string udn);
		std::string getUdn();
		void setFriendlyName(std::string friendlyName);
		std::string getFriendlyName();
		
		void setParentDevice(UPnPDevice * parent);
		UPnPDevice * getParentDevice();
        UPnPDevice * getRootDevice();
		bool isRootDevice();
		void addEmbeddedDevice(UPnPDevice & embeddedDevice);
		void removeEmbeddedDevice(size_t index);
		UPnPDevice & getEmbeddedDevice(size_t index) const;
		std::vector<UPnPDevice> & getEmbeddedDevices() const;
		void setServices(const std::vector<UPnPService> & services);
		void addService(UPnPService & service);
		void removeService(size_t index);
		UPnPService getService(std::string serviceType) const;
        UPnPService & getService(size_t index) const;
        std::vector<UPnPService> & getServices() const;
        
        void setBaseUrl(const std::string & baseUrl);
        std::string getBaseUrl();
		size_t getScpdBindCount() const;
		bool checkAllScpdBind() const;
		size_t getScpdBindCountRecursive() const;
		size_t getServiceRecursive() const;
		bool checkAllScpdBindRecursive() const;
		bool complete() const;

		void renew();
		void setTimeout(unsigned long timeoutMilli);
		bool outdated() const;

		std::string & operator[] (const std::string & name);
	};
}

#endif
