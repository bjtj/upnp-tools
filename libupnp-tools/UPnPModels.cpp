#include "UPnPModels.hpp"

namespace UPNP {

	using namespace std;
	using namespace HTTP;
	using namespace UTIL;

	UPnPScpd::UPnPScpd() {
	}
	UPnPScpd::~UPnPScpd() {
	}
	vector<UPnPAction> & UPnPScpd::actions() {
		return _actions;
	}
	UPnPAction & UPnPScpd::action(const string & name) {
		for (vector<UPnPAction>::iterator iter = _actions.begin(); iter != _actions.end(); iter++) {
			if (iter->name() == name) {
				return *iter;
			}
		}
		_actions.push_back(UPnPAction(name));
		return *_actions.rbegin();
	}
	bool UPnPScpd::hasAction(const string & name) {
		for (vector<UPnPAction>::iterator iter = _actions.begin(); iter != _actions.end(); iter++) {
			if (iter->name() == name) {
				return true;
			}
		}
		return false;
	}
	vector<UPnPStateVariable> & UPnPScpd::stateVariables() {
		return _stateVariables;
	}
	UPnPStateVariable & UPnPScpd::stateVariable(const string & name) {
		for (vector<UPnPStateVariable>::iterator iter = _stateVariables.begin(); iter != _stateVariables.end(); iter++) {
			if (iter->name() == name) {
				return *iter;
			}
		}

		_stateVariables.push_back(UPnPStateVariable(name));
		return *_stateVariables.rbegin();
	}
	bool UPnPScpd::hasStateVariable(const string & name) {
		for (vector<UPnPStateVariable>::iterator iter = _stateVariables.begin(); iter != _stateVariables.end(); iter++) {
			if (iter->name() == name) {
				return true;
			}
		}
		return false;
	}

	
	
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

	UPnPScpd & UPnPService::scpd() {
		return _scpd;
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
		_embeddedDevices.push_back(device);
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

	vector<AutoRef<UPnPDevice> > & UPnPDevice::embeddedDevices() {
		return _embeddedDevices;
	}

	vector<AutoRef<UPnPService> > & UPnPDevice::services() {
		return _services;
	}

	vector<UPnPDevice*> UPnPDevice::allDevices() {
		vector<UPnPDevice*> ret;
		ret.push_back(this);
		for (vector<AutoRef<UPnPDevice> >::iterator iter = _embeddedDevices.begin(); iter != _embeddedDevices.end(); iter++) {
			vector<UPnPDevice*> v = (*iter)->allDevices();
			ret.insert(ret.end(), v.begin(), v.end());
		}
		return ret;
	}
	
	vector<UPnPService*> UPnPDevice::allServices() {
		vector<UPnPService*> ret;
		for (vector<AutoRef<UPnPService> >::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			ret.push_back(&(*iter));
		}
		for (vector<AutoRef<UPnPDevice> >::iterator iter = _embeddedDevices.begin(); iter != _embeddedDevices.end(); iter++) {
			vector<UPnPService*> v = (*iter)->allServices();
			ret.insert(ret.end(), v.begin(), v.end());
		}
		return ret;
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
