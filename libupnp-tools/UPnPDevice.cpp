#include <algorithm>
#include "UPnPDevice.hpp"

namespace UPNP {

	using namespace std;

	UPnPDevice::UPnPDevice() : parent(NULL) {
	}

	UPnPDevice::~UPnPDevice() {
	}

	void UPnPDevice::setUdn(std::string udn) {
		this->udn = udn;
		properties["UDN"] = udn;
	}

	std::string UPnPDevice::getUdn() {
		return properties["UDN"];
	}

	void UPnPDevice::setFriendlyName(std::string friendlyName) {
		this->friendlyName = friendlyName;
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

	bool UPnPDevice::isRootDevice() {
		return parent == NULL;
	}

	void UPnPDevice::addEmbeddedDevice(UPnPDevice & embeddedDevice) {
		embeddedDevice.setParentDevice(this);
		embeddedDevices.push_back(embeddedDevice);
	}

	void UPnPDevice::removeEmbeddedDevice(UPnPDevice & embeddedDevice) {
		embeddedDevice.setParentDevice(NULL);
		embeddedDevices.erase(std::remove(embeddedDevices.begin(),
										  embeddedDevices.end(),
										  embeddedDevice), embeddedDevices.end());
		
	}

	UPnPDevice UPnPDevice::getEmbeddedDevice(int index) {
		return embeddedDevices[index];
	}

	void UPnPDevice::addService(UPnPService & service) {
		services.push_back(service);
	}

	void UPnPDevice::removeService(UPnPService & service) {
		services.erase(std::remove(services.begin(),
								   services.end(),
								   service), services.end());
	}

	UPnPService UPnPDevice::getService(std::string serviceType) {
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

	bool UPnPDevice::operator==(const UPnPDevice &other) const {
		return (this->udn.compare(other.udn) == 0);
	}

	string & UPnPDevice::operator[] (const string & name){
		return properties[name];
	}
}
