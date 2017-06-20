#include "UPnPLocationResolver.hpp"
#include <liboslayer/os.hpp>
#include <liboslayer/Uuid.hpp>

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace UTIL;

	UPnPLocationResolver::UPnPLocationResolver() {
	}
	UPnPLocationResolver::~UPnPLocationResolver() {
	}
	string UPnPLocationResolver::makeScpdUrl(AutoRef<UPnPDevice> device, AutoRef<UPnPService> service) {
		Uuid uuid(device->getUdn());
		return "/scpd.xml/" + uuid.getUuid() + "::" + service->serviceType();
	}
	string UPnPLocationResolver::makeControlUrl(AutoRef<UPnPDevice> device, AutoRef<UPnPService> service) {
		Uuid uuid(device->getUdn());
		return "/control/" + uuid.getUuid() + "::" + service->serviceType();
	}
	string UPnPLocationResolver::makeEventSubUrl(AutoRef<UPnPDevice> device, AutoRef<UPnPService> service) {
		Uuid uuid(device->getUdn());
		return "/event/" + uuid.getUuid() + "::" + service->serviceType();
	}
	string UPnPLocationResolver::generalize(const string & url) {
		size_t s = url.find_last_of("/");
		if (s == string::npos) {
			throw Exception("parse failed");
		}
		return url.substr(s + 1);
	}
	void UPnPLocationResolver::resolve(AutoRef<UPnPDevice> device) {
		vector<AutoRef<UPnPService> > services = device->services();
		for (vector<AutoRef<UPnPService> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			(*iter)->scpdUrl() = makeScpdUrl(device, (*iter));
			(*iter)->controlUrl() = makeControlUrl(device, (*iter));
			(*iter)->eventSubUrl() = makeEventSubUrl(device, (*iter));
		}
	}
	void UPnPLocationResolver::resolveRecursive(AutoRef<UPnPDevice> device) {
		resolve(device);
		vector<AutoRef<UPnPDevice> > embeds = device->childDevices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = embeds.begin(); iter != embeds.end(); iter++) {
			resolveRecursive(*iter);
		}
	}
}
