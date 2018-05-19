#include "UPnPModels.hpp"
#include "UPnPExceptions.hpp"
#include <liboslayer/Text.hpp>

namespace upnp {

	using namespace std;
	using namespace osl;
	using namespace http;


	/**
	 * max-age
	 */

	MaxAge::MaxAge(unsigned long second) : _second(second) {
	}

	MaxAge::~MaxAge() {
	}

	unsigned long & MaxAge::second() {
		return _second;
	}

	MaxAge MaxAge::fromString(const string & phrase) {
		if (Text::startsWithIgnoreCase(phrase, "max-age=") == false) {
			throw UPnPParseException("max-age not occurred");
		}
		return MaxAge((unsigned long)Text::toLong(phrase.substr(string("max-age=").size())));
	}

	string MaxAge::toString() const {
		return toString(_second);
	}

	string MaxAge::toString(unsigned long second) {
		return "max-age=" + Text::toString(second);
	}


	/**
	 * callback urls
	 */
	
	CallbackUrls::CallbackUrls(const vector<string> & urls) : _urls(urls) {
	}
	
	CallbackUrls::~CallbackUrls() {
	}
	
	vector<string> & CallbackUrls::urls() {
		return _urls;
	}
	
	CallbackUrls CallbackUrls::fromString(const string & phrase) {
		vector<string> urls;
		string buffer;
		if (phrase.empty()) {
			throw UPnPParseException("empty string");
		}
		for (string::const_iterator iter = phrase.begin(); iter != phrase.end(); iter++) {
			if (*iter == '<') {
				buffer = "";
				for (iter++; iter != phrase.end() && *iter != '>'; iter++) {
					buffer.append(1, *iter);
				}
				try {
					Url::validateUrlFormat(buffer);
				} catch (UrlParseException e) {
					throw UPnPParseException("wrong url format - '" + buffer + "'");
				}
				urls.push_back(buffer);
			}
		}
		return CallbackUrls(urls);
	}
	
	string CallbackUrls::toString() const {
		return toString(_urls);
	}
	
	string CallbackUrls::toString(const vector<string> & urls) {
		string ret;
		for (size_t i = 0; i < urls.size(); i++) {
			if (i != 0) {
				ret.append(" ");
			}
			ret.append("<" + urls[i] + ">");
		}
		return ret;
	}


	/**
	 * second
	 */
	
	Second::Second(unsigned long second) : _second(second) {
	}
	
	Second::~Second() {
	}
	
	unsigned long & Second::second() {
		return _second;
	}

	unsigned long Second::second() const {
		return _second;
	}

	unsigned long Second::milli() const {
		return _second * 1000;
	}
	
	Second Second::fromString(const string & phrase) {
		if (Text::startsWithIgnoreCase(phrase, "Second-") == false) {
			throw UPnPParseException("Not found prefix 'Second-'");
		}
		return Second((unsigned long)Text::toLong(phrase.substr(string("Second-").size())));
	}

	string Second::toString() const {
		return toString(_second);
	}

	string Second::toString(unsigned long second) {
		return "Second-" + Text::toString(second);
	}


	// upnp scpd

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

	string & UPnPDevice::udn() {
		return properties()["UDN"];
	}

	void UPnPDevice::setUdn(const std::string & udn) {
		this->udn() = udn;
		for (vector< AutoRef<UPnPDevice> >::iterator iter = _childDevices.begin();
			 iter != _childDevices.end(); iter++)
		{
			AutoRef<UPnPDevice> device = *iter;
			device->setUdn(udn);
		}
	}

	const string & UPnPDevice::udn() const {
		return properties()["UDN"];
	}

	string & UPnPDevice::friendlyName() {
		return properties()["friendlyName"];
	}

	const string & UPnPDevice::friendlyName() const {
		return properties()["friendlyName"];
	}
	
	string & UPnPDevice::deviceType() {
		return properties()["deviceType"];
	}

	const string & UPnPDevice::deviceType() const {
		return properties()["deviceType"];
	}
	
	Url & UPnPDevice::baseUrl() {
		return _baseUrl;
	}

	string UPnPDevice::formatUrl(const string & url, AutoRef<UPnPService> service) {
		return Text::replaceAll(Text::replaceAll(url, "$udn", udn()),
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
