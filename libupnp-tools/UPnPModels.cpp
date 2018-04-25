#include "UPnPModels.hpp"
#include <liboslayer/Text.hpp>

namespace upnp {

	using namespace std;
	using namespace osl;
	using namespace http;

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

	
	UPnPService::UPnPService() {
	}
	
	UPnPService::~UPnPService() {}
	
	string & UPnPService::serviceType() {
		return properties()["serviceType"];
	}

	string & UPnPService::serviceId() {
		return properties()["serviceId"];
	}

	string & UPnPService::scpdUrl() {
		return properties()["SCPDURL"];
	}

	string & UPnPService::controlUrl() {
		return properties()["controlURL"];
	}

	string & UPnPService::eventSubUrl() {
		return properties()["eventSubURL"];
	}

	UPnPScpd & UPnPService::scpd() {
		return _scpd;
	}


	UPnPDevice::UPnPDevice() : parent(NULL) {
	}
	
	UPnPDevice::UPnPDevice(UPnPDevice * parent) : parent(parent) {
	}
	
	UPnPDevice::~UPnPDevice() {
	}

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
		_childDevices.push_back(device);
	}

	void UPnPDevice::addService(AutoRef<UPnPService> service) {
		_services.push_back(service);
	}
	
	AutoRef<UPnPService> UPnPDevice::getService(const string & serviceType) {
		for (vector< AutoRef<UPnPService> >::iterator iter = _services.begin(); iter != _services.end(); iter++) {
			if ((*iter)->serviceType() == serviceType) {
				return *iter;
			}
		}
		for (size_t i = 0; i < _childDevices.size(); i++) {
			AutoRef<UPnPService> service = _childDevices[i]->getService(serviceType);
			if (!service.nil()) {
				return service;
			}
		}
		return AutoRef<UPnPService>();
	}

	AutoRef<UPnPService> UPnPDevice::getServiceWithScpdUrl(const string & scpdUrl) {
		for (vector< AutoRef<UPnPService> >::iterator iter = _services.begin();
			 iter != _services.end(); iter++)
		{
			if ((*iter)->scpdUrl() == scpdUrl) {
				return *iter;
			}
		}
		for (size_t i = 0; i < _childDevices.size(); i++) {
			AutoRef<UPnPService> service = _childDevices[i]->getServiceWithScpdUrl(scpdUrl);
			if (!service.nil()) {
				return service;
			}
		}
		return AutoRef<UPnPService>();
	}

	AutoRef<UPnPService> UPnPDevice::getServiceWithControlUrl(const string & controlUrl) {
		for (vector< AutoRef<UPnPService> >::iterator iter = _services.begin();
			 iter != _services.end(); iter++)
		{
			if ((*iter)->controlUrl() == controlUrl) {
				return *iter;
			}
		}
		for (size_t i = 0; i < _childDevices.size(); i++) {
			AutoRef<UPnPService> service = _childDevices[i]->getServiceWithControlUrl(controlUrl);
			if (!service.nil()) {
				return service;
			}
		}
		return AutoRef<UPnPService>();
	}

	AutoRef<UPnPService> UPnPDevice::getServiceWithEventSubUrl(const string & eventSubUrl) {
		for (vector< AutoRef<UPnPService> >::iterator iter = _services.begin();
			 iter != _services.end(); iter++)
		{
			if ((*iter)->eventSubUrl() == eventSubUrl) {
				return *iter;
			}
		}
		for (size_t i = 0; i < _childDevices.size(); i++) {
			AutoRef<UPnPService> service = _childDevices[i]->getServiceWithEventSubUrl(eventSubUrl);
			if (!service.nil()) {
				return service;
			}
		}
		return AutoRef<UPnPService>();
	}

	vector< AutoRef<UPnPDevice> > & UPnPDevice::childDevices() {
		return _childDevices;
	}

	vector< AutoRef<UPnPService> > & UPnPDevice::services() {
		return _services;
	}

	vector< AutoRef<UPnPDevice> > UPnPDevice::allChildDevices() {
		vector< AutoRef<UPnPDevice> > ret;
		for (vector< AutoRef<UPnPDevice> >::iterator iter = _childDevices.begin(); iter != _childDevices.end(); iter++) {
			vector< AutoRef<UPnPDevice> > v = (*iter)->allChildDevices();
			ret.insert(ret.end(), v.begin(), v.end());
		}
		return ret;
	}
	
	vector< AutoRef<UPnPService> > UPnPDevice::allServices() {
		vector< AutoRef<UPnPService> > ret;
		for (vector< AutoRef<UPnPService> >::iterator iter = _services.begin();
			 iter != _services.end(); iter++)
		{
			ret.push_back(*iter);
		}
				
		for (vector< AutoRef<UPnPDevice> >::iterator iter = _childDevices.begin();
			 iter != _childDevices.end(); iter++)
		{
			vector< AutoRef<UPnPService> > v = (*iter)->allServices();
			ret.insert(ret.end(), v.begin(), v.end());
		}
		return ret;
	}

	UDN UPnPDevice::udn() const {
		return UDN(properties()["UDN"]);
	}

	void UPnPDevice::setUdn(const UDN & udn) {
		properties()["UDN"] = udn.toString();

		for (vector< AutoRef<UPnPDevice> >::iterator iter = _childDevices.begin();
			 iter != _childDevices.end(); iter++) {
			AutoRef<UPnPDevice> device = *iter;
			device->setUdn(udn);
		}
	}

	string & UPnPDevice::friendlyName() {
		return properties()["friendlyName"];
	}
	
	string & UPnPDevice::deviceType() {
		return properties()["deviceType"];
	}
	
	Url & UPnPDevice::baseUrl() {
		return _baseUrl;
	}

	string UPnPDevice::formatUrl(const string & url, AutoRef<UPnPService> service) {
		return Text::replaceAll(Text::replaceAll(url, "$udn", udn().toString()),
								"$serviceType", service->serviceType()); 
	}

	void UPnPDevice::setScpdUrl(const string & scpdUrl) {
		for (vector< AutoRef<UPnPService> >::iterator iter = _services.begin();
			 iter != _services.end(); iter++) {
			AutoRef<UPnPService> service = *iter;
			service->scpdUrl() = formatUrl(scpdUrl, service);
		}

		for (vector< AutoRef<UPnPDevice> >::iterator iter = _childDevices.begin();
			 iter != _childDevices.end(); iter++) {
			AutoRef<UPnPDevice> device = *iter;
			device->setScpdUrl(scpdUrl);
		}
	}
	
	void UPnPDevice::setControlUrl(const string & controlUrl){
		for (vector< AutoRef<UPnPService> >::iterator iter = _services.begin();
			 iter != _services.end(); iter++) {
			AutoRef<UPnPService> service = *iter;
			service->controlUrl() = formatUrl(controlUrl, service);
		}

		for (vector< AutoRef<UPnPDevice> >::iterator iter = _childDevices.begin();
			 iter != _childDevices.end(); iter++) {
			AutoRef<UPnPDevice> device = *iter;
			device->setControlUrl(controlUrl);
		}
	}
	
	void UPnPDevice::setEventSubUrl(const string & eventSubUrl){
		for (vector< AutoRef<UPnPService> >::iterator iter = _services.begin();
			 iter != _services.end(); iter++) {
			AutoRef<UPnPService> service = *iter;
			service->eventSubUrl() = formatUrl(eventSubUrl, service);
		}

		for (vector< AutoRef<UPnPDevice> >::iterator iter = _childDevices.begin();
			 iter != _childDevices.end(); iter++) {
			AutoRef<UPnPDevice> device = *iter;
			device->setEventSubUrl(eventSubUrl);
		}
	}

}
