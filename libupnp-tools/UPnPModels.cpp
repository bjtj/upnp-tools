#include "UPnPModels.hpp"

namespace UPNP {

	using namespace std;
	using namespace HTTP;
	using namespace UTIL;

	
	UPnPService::UPnPService() : device(NULL) {}
	UPnPService::UPnPService(UPnPDevice * device) : device(device) {}
	UPnPService::~UPnPService() {}
	
	string UPnPService::getServiceType() {
		return getProperties()["serviceType"];
	}
	string UPnPService::getServiceId() {
		return getProperties()["serviceId"];
	}
	string UPnPService::getScpdUrl() {
		return getProperties()["SCPDURL"];
	}
	string UPnPService::getControlUrl() {
		return getProperties()["controlURL"];
	}
	string UPnPService::getEventSubUrl() {
		return getProperties()["eventSubURL"];
	}
	void UPnPService::setDevice(UPnPDevice * device) {
		this->device = device;
	}
	UPnPDevice * UPnPService::getDevice() {
		return device;
	}
	Url UPnPService::makeScpdUrl() {
		if (!device) {
			throw OS::Exception("no device", -1, 0);
		}
		return device->baseUrl().relativePath(getScpdUrl());
	}
	vector<UPnPAction> & UPnPService::actions() {
		return _actions;
	}
	UPnPAction UPnPService::getAction(const string & actionName) {
		for (vector<UPnPAction>::iterator iter = _actions.begin(); iter != _actions.end(); iter++) {
			if (iter->name() == actionName) {
				return *iter;
			}
		}
		throw OS::Exception("not found action / name: " + actionName, -1, 0);
	}
	void UPnPService::addAction(UPnPAction action) {
		_actions.push_back(action);
	}
	vector<UPnPStateVariable> & UPnPService::stateVariables() {
		return _stateVariables;
	}
	UPnPStateVariable UPnPService::getStateVariable(const string & stateVariableName) {
		for (vector<UPnPStateVariable>::iterator iter = _stateVariables.begin(); iter != _stateVariables.end(); iter++) {
			if (iter->name() == stateVariableName) {
				return *iter;
			}
		}
		throw OS::Exception("not found state variable / name: " + stateVariableName, -1, 0);
	}
	void UPnPService::addStateVariable(UPnPStateVariable stateVariable) {
		_stateVariables.push_back(stateVariable);
	}



	UPnPDevice::UPnPDevice() : parent(NULL) {}
	UPnPDevice::UPnPDevice(UPnPDevice * parent) : parent(parent) {}
	UPnPDevice::~UPnPDevice() {}

	void UPnPDevice::setParent(UPnPDevice * parent) {
		this->parent = parent;
	}

	AutoRef<UPnPDevice> UPnPDevice::prepareDevice() {
		AutoRef<UPnPDevice> device(new UPnPDevice(this));
		device->baseUrl() = _baseUrl;
		addDevice(device);
		return device;
	}

	void UPnPDevice::addDevice(AutoRef<UPnPDevice> device) {
		device->setParent(this);
		_devices.push_back(device);
	}

	void UPnPDevice::addService(AutoRef<UPnPService> service) {
		service->setDevice(this);
		_services.push_back(service);
	}

	bool UPnPDevice::hasService(const string & serviceType) {
		for (vector<AutoRef<UPnPService> >::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if ((*iter)->getServiceType() == serviceType) {
				return true;
			}
		}
		return false;
	}
	
	AutoRef<UPnPService> UPnPDevice::getService(const string & serviceType) {
		for (vector<AutoRef<UPnPService> >::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if ((*iter)->getServiceType() == serviceType) {
				return *iter;
			}
		}
		throw OS::Exception("not found service / name: " + serviceType, -1, 0);
	}

	vector<AutoRef<UPnPDevice> > & UPnPDevice::devices() {
		return _devices;
	}

	vector<AutoRef<UPnPService> > & UPnPDevice::services() {
		return _services;
	}

	string UPnPDevice::getUdn() {
		return getProperties()["UDN"];
	}

	void UPnPDevice::setUdn(const string & udn) {
		getProperties()["UDN"] = udn;
	}
	
	string UPnPDevice::getFriendlyName() {
		return getProperties()["friendlyName"];
	}

	void UPnPDevice::setFriendlyName(const string & friendlyName) {
		getProperties()["friendlyName"] = friendlyName;
	}

	string UPnPDevice::getDeviceType() {
		return getProperties()["deviceType"];
	}

	void UPnPDevice::setDeviceType(const string & deviceType) {
		getProperties()["deviceType"] = deviceType;
	}

	Url & UPnPDevice::baseUrl() {
		return _baseUrl;
	}
}
