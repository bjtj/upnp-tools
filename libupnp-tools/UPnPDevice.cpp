#include <algorithm>
#include "UPnPDevice.hpp"
#include <liboslayer/os.hpp>
#include "macros.hpp"

namespace UPNP {

	using namespace std;
    using namespace OS;
    using namespace UTIL;

	UPnPDevice::UPnPDevice() : parent(NULL) {
	}
    
    UPnPDevice::UPnPDevice(const UPnPDevice & other) {
        this->properties = other.properties;
        this->parent = other.parent;
        this->embeddedDevices = other.embeddedDevices;
        this->services = other.services;
        
        rebaseParents(this);
    }

	UPnPDevice::~UPnPDevice() {
	}
    
    UPnPDevice UPnPDevice::copy() {
        UPnPDevice device(*this);
        return device;
    }
    
    void UPnPDevice::rebaseParents() {
        rebaseParents(this);
    }
    
    void UPnPDevice::rebaseParents(UPnPDevice * parent) {
        vector<UPnPDevice> & embeddedDevices = parent->getEmbeddedDevices();
        LOOP_VEC(embeddedDevices, i) {
            UPnPDevice & embeddedDevice = embeddedDevices[i];
            embeddedDevice.parent = parent;
            rebaseParents(&embeddedDevice);
        }
    }
    
    bool UPnPDevice::valid() {
        return !getUdn().empty() && complete();
    }

	NameProperty & UPnPDevice::getProperty(const std::string & name) {
		return properties.get(name);
	}
    
	string & UPnPDevice::getPropertyValue(const string & name) {
		return properties[name].getValue();
	}
	string UPnPDevice::getConstPropertyValue(const string & name) const {
		if (!properties.has(name)) {
			return "";
		}
		return properties.const_get(name).getValue();
	}

    LinkedStringProperties & UPnPDevice::getProperties() {
        return properties;
    }
    
    const LinkedStringProperties & UPnPDevice::getProperties() const {
        return properties;
    }

	void UPnPDevice::setUdn(const std::string & udn) {
		properties["UDN"] = udn;
	}

	std::string UPnPDevice::getUdn() const {
		return getConstPropertyValue("UDN");
	}

	void UPnPDevice::setFriendlyName(const std::string & friendlyName) {
		properties["friendlyName"] = friendlyName;
	}

	std::string UPnPDevice::getFriendlyName() const {
		return getConstPropertyValue("friendlyName");
	}
    
    void UPnPDevice::setDeviceType(const std::string & deviceType) {
        properties["deviceType"] = deviceType;
    }
    
    std::string UPnPDevice::getDeviceType() const {
		return getConstPropertyValue("deviceType");
    }

	void UPnPDevice::setParentDevice(UPnPDevice * parent) {
		this->parent = parent;
	}

	UPnPDevice * UPnPDevice::getParentDevice() {
		return parent;
	}
    
    UPnPDevice * UPnPDevice::getRootDevice() {
        UPnPDevice * device = this;
        while (device->getParentDevice()) {
            device = device->getParentDevice();
        }
        return device;
    }

	bool UPnPDevice::isRootDevice() const {
		return parent == NULL;
	}

	void UPnPDevice::addEmbeddedDevice(UPnPDevice & embeddedDevice) {
		embeddedDevice.setParentDevice(this);
		embeddedDevices.push_back(embeddedDevice);
	}

	void UPnPDevice::removeEmbeddedDevice(size_t index) {
		embeddedDevices.erase(embeddedDevices.begin() + index);
	}
    
    UPnPDevice & UPnPDevice::getEmbeddedDevice(size_t index) {
        return embeddedDevices[index];
    }

	const UPnPDevice & UPnPDevice::getEmbeddedDevice(size_t index) const {
		return embeddedDevices[index];
	}
    
    vector<UPnPDevice> & UPnPDevice::getEmbeddedDevices() {
        return embeddedDevices;
    }

	const vector<UPnPDevice> & UPnPDevice::getEmbeddedDevices() const {
		return embeddedDevices;
	}

	void UPnPDevice::setServices(const std::vector<UPnPService> & services) {
		this->services = services;
	}

	void UPnPDevice::addService(UPnPService & service) {
		services.push_back(service);
	}

	void UPnPDevice::removeService(size_t index) {
		services.erase(services.begin() + index);
	}


	bool UPnPDevice::hasServiceWithProperty(const string & name, const string & value) const {
		for (size_t i = 0; i < services.size(); i++) {
			const UPnPService & service = services[i];
			if (!service.getProperty(name).compare(value)) {
				return true;
			}
		}
		return false;
	}
	bool UPnPDevice::hasServiceWithPropertyRecursive(const string & name, const string & value) const {

		if (hasServiceWithProperty(name, value)) {
			return true;
		}

		for (size_t i = 0; i < embeddedDevices.size(); i++) {
			const UPnPDevice & embed = embeddedDevices[i];
			if (embed.hasServiceWithPropertyRecursive(name, value)) {
				return true;
			}
		}

		return false;
	}

	UPnPService & UPnPDevice::getServiceWithProperty(const string & name, const string & value) {
		for (size_t i = 0; i < services.size(); i++) {
			UPnPService & service = services[i];
			if (!service[name].compare(value)) {
				return service;
			}
		}
		throw Exception("no service found/property name: " + name + ", value: " + value, -1, 0);
	}

	const UPnPService & UPnPDevice::getServiceWithProperty(const string & name, const string & value) const {
		for (size_t i = 0; i < services.size(); i++) {
			const UPnPService & service = services[i];
			if (!service.getProperty(name).compare(value)) {
				return service;
			}
		}
		throw Exception("no service found/property name: " + name + ", value: " + value, -1, 0);
	}
	UPnPService & UPnPDevice::getServiceWithPropertyRecursive(const string & name, const string & value) {
		if (hasServiceWithProperty(name, value)) {
			return getServiceWithProperty(name, value);
		}

		for (size_t i = 0; i < embeddedDevices.size(); i++) {
			UPnPDevice & embed = embeddedDevices[i];
			if (embed.hasServiceWithPropertyRecursive(name, value)) {
				return getServiceWithPropertyRecursive(name, value);
			}
		}
		throw Exception("no service found/property name: " + name + ", value: " + value, -1, 0);
	}
	const UPnPService & UPnPDevice::getServiceWithPropertyRecursive(const string & name, const string & value) const {
		if (hasServiceWithProperty(name, value)) {
			return getServiceWithProperty(name, value);
		}

		for (size_t i = 0; i < embeddedDevices.size(); i++) {
			const UPnPDevice & embed = embeddedDevices[i];
			if (embed.hasServiceWithPropertyRecursive(name, value)) {
				return getServiceWithPropertyRecursive(name, value);
			}
		}
		throw Exception("no service found/property name: " + name + ", value: " + value, -1, 0);
	}


	UPnPService & UPnPDevice::getServiceWithServiceType(const std::string & value) {
		return getServiceWithProperty("serviceType", value);
	}
	const UPnPService & UPnPDevice::getServiceWithServiceType(const std::string & value) const {
		return getServiceWithProperty("serviceType", value);
	}
	UPnPService & UPnPDevice::getServiceWithServiceTypeRecursive(const std::string & value) {
		return getServiceWithPropertyRecursive("serviceType", value);
	}
	const UPnPService & UPnPDevice::getServiceWithServiceTypeRecursive(const std::string & value) const {
		return getServiceWithPropertyRecursive("serviceType", value);
	}

	
    UPnPService & UPnPDevice::getService(size_t index) {
        return services[index];
    }
    
    const UPnPService & UPnPDevice::getService(size_t index) const {
        return services[index];
    }
    
	vector<UPnPService> & UPnPDevice::getServices() {
		return services;
	}

    const vector<UPnPService> & UPnPDevice::getServices() const {
        return services;
    }

	vector<UPnPService> UPnPDevice::getServicesRecursive() {
		vector<UPnPService> ret = getServices();
		for (size_t i = 0; i < embeddedDevices.size(); i++) {
			UPnPDevice & embed = embeddedDevices[i];
			vector<UPnPService> append = embed.getServicesRecursive();
			ret.insert(ret.end(), append.begin(), append.end());
		}
		return ret;
	}
	const vector<UPnPService> UPnPDevice::getServicesRecursive() const {
		vector<UPnPService> ret = getServices();
		for (size_t i = 0; i < embeddedDevices.size(); i++) {
			const UPnPDevice & embed = embeddedDevices[i];
			vector<UPnPService> append = embed.getServicesRecursive();
			ret.insert(ret.end(), append.begin(), append.end());
		}
		return ret;
	}
    
    void UPnPDevice::setBaseUrl(const string & baseUrl) {
        this->baseUrl = baseUrl;
    }
    
    string UPnPDevice::getBaseUrl() {
        return baseUrl;
    }

	size_t UPnPDevice::getScpdBindCount() const {
		size_t cnt = 0;
		LOOP_VEC(services, i) {
			if (services[i].isScpdBind()) {
				cnt++;
			}
		}
		return cnt;
	}

	bool UPnPDevice::checkAllScpdBind() const {
		return getScpdBindCount() == services.size();
	}

	size_t UPnPDevice::getScpdBindCountRecursive() const {
		size_t cnt = getScpdBindCount();
		LOOP_VEC(embeddedDevices, i) {
			cnt += embeddedDevices[i].getScpdBindCountRecursive();
		}
		return cnt;
	}
	size_t UPnPDevice::getServiceRecursive() const {
		size_t cnt = services.size();
		LOOP_VEC(embeddedDevices, i) {
			cnt += embeddedDevices[i].getServiceRecursive();
		}
		return cnt;
	}
	bool UPnPDevice::checkAllScpdBindRecursive() const {
		if (!checkAllScpdBind()) {
			return false;
		}
		LOOP_VEC(embeddedDevices, i) {
			if (!embeddedDevices[i].checkAllScpdBindRecursive()) {
				return false;
			}
		}
		return true;
	}

	bool UPnPDevice::complete() const {
		return checkAllScpdBindRecursive();
	}

	void UPnPDevice::renew() {
		cacheControl.renew();
	}
	void UPnPDevice::setTimeout(unsigned long timeoutMilli) {
		cacheControl.setTimeout(timeoutMilli);
	}
	bool UPnPDevice::outdated() const {
		return cacheControl.outdated();
	}

	NameProperty & UPnPDevice::operator[] (const string & name){
		return properties[name];
	}
}
