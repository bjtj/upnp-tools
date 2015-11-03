#include <algorithm>
#include "UPnPDevice.hpp"

#include "macros.hpp"

namespace UPNP {

	using namespace std;

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

	void UPnPDevice::setUdn(std::string udn) {
		properties["UDN"] = udn;
	}

	std::string UPnPDevice::getUdn() {
		return properties["UDN"];
	}

	void UPnPDevice::setFriendlyName(std::string friendlyName) {
		properties["friendlyName"] = friendlyName;
	}

	std::string UPnPDevice::getFriendlyName() {
		return properties["friendlyName"];
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

	bool UPnPDevice::isRootDevice() {
		return parent == NULL;
	}

	void UPnPDevice::addEmbeddedDevice(UPnPDevice & embeddedDevice) {
		embeddedDevice.setParentDevice(this);
		embeddedDevices.push_back(embeddedDevice);
	}

	void UPnPDevice::removeEmbeddedDevice(size_t index) {
		embeddedDevices.erase(embeddedDevices.begin() + index);
	}

	UPnPDevice & UPnPDevice::getEmbeddedDevice(size_t index) const {
		return embeddedDevices[index];
	}

	vector<UPnPDevice> & UPnPDevice::getEmbeddedDevices() const {
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

	UPnPService UPnPDevice::getService(std::string serviceType) const {
		for (
			 vector<UPnPService>::iterator iter = services.begin();
			 iter != services.end(); iter++) {
			
			UPnPService & service = *iter;
			if (!service.getServiceType().compare(serviceType)) {
				return service;
			}
		}
		return UPnPService();
	}
    
    UPnPService & UPnPDevice::getService(size_t index) const {
        return services[index];
    }
    
    vector<UPnPService> & UPnPDevice::getServices() const {
        return services;
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

	string & UPnPDevice::operator[] (const string & name){
		return properties[name];
	}
}
