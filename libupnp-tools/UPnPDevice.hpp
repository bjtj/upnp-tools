#ifndef __UPNP_DEVICE_HPP__
#define __UPNP_DEVICE_HPP__

#include <map>
#include <vector>
#include <string>

#include <liboslayer/StringElement.hpp>

#include "CacheControl.hpp"
#include "UPnPService.hpp"

namespace UPNP {

	/**
	 * @brief upnp device
	 */
	class UPnPDevice {
	private:
		UTIL::LinkedStringProperties properties;
		UPnPDevice * parent;
		std::vector<UPnPDevice> embeddedDevices;
		std::vector<UPnPService> services;
        std::string baseUrl;
		CacheControl cacheControl;
		
	public:
		UPnPDevice();
        UPnPDevice(const UPnPDevice & other);
		virtual ~UPnPDevice();

        UPnPDevice copy();
        void rebaseParents();
        void rebaseParents(UPnPDevice * parent);
        
		UTIL::NameProperty & getProperty(const std::string & name);
		std::string & getPropertyValue(const std::string & name);
		std::string getConstPropertyValue(const std::string & name) const;
        UTIL::LinkedStringProperties & getProperties();
        const UTIL::LinkedStringProperties & getProperties() const;

		void setUdn(const std::string & udn);
		std::string getUdn() const;
		void setFriendlyName(const std::string & friendlyName);
		std::string getFriendlyName() const;
        void setDeviceType(const std::string & deviceType);
        std::string getDeviceType() const;
		
		void setParentDevice(UPnPDevice * parent);
		UPnPDevice * getParentDevice();
        UPnPDevice * getRootDevice();
		bool isRootDevice() const;
		void addEmbeddedDevice(UPnPDevice & embeddedDevice);
		void removeEmbeddedDevice(size_t index);
        UPnPDevice & getEmbeddedDevice(size_t index);
		const UPnPDevice & getEmbeddedDevice(size_t index) const;
        std::vector<UPnPDevice> & getEmbeddedDevices();
		const std::vector<UPnPDevice> & getEmbeddedDevices() const;
		void setServices(const std::vector<UPnPService> & services);
		void addService(UPnPService & service);
		void removeService(size_t index);

		bool hasServiceWithProperty(const std::string & name, const std::string & value) const;
		bool hasServiceWithPropertyRecursive(const std::string & name, const std::string & value) const;

		UPnPService & getServiceWithProperty(const std::string & name, const std::string & value);
		const UPnPService & getServiceWithProperty(const std::string & name, const std::string & value) const;
		UPnPService & getServiceWithPropertyRecursive(const std::string & name, const std::string & value);
		const UPnPService & getServiceWithPropertyRecursive(const std::string & name, const std::string & value) const;

		UPnPService & getServiceWithServiceType(const std::string & value);
		const UPnPService & getServiceWithServiceType(const std::string & value) const;
		UPnPService & getServiceWithServiceTypeRecursive(const std::string & value);
		const UPnPService & getServiceWithServiceTypeRecursive(const std::string & value) const;

        UPnPService & getService(size_t index);
        const UPnPService & getService(size_t index) const;
        const std::vector<UPnPService> & getServices() const;
        
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

		UTIL::NameProperty & operator[] (const std::string & name);
	};
}

#endif
