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
		return "/scpd.xml/" + device->getUdn().toString() + "::" + service->serviceType();
	}

	string UPnPLocationResolver::makeControlUrl(AutoRef<UPnPDevice> device, AutoRef<UPnPService> service) {
		return "/control/" + device->getUdn().toString() + "::" + service->serviceType();
	}

	string UPnPLocationResolver::makeEventSubUrl(AutoRef<UPnPDevice> device, AutoRef<UPnPService> service) {
		return "/event/" + device->getUdn().toString() + "::" + service->serviceType();
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
		vector<AutoRef<UPnPDevice> > children = device->childDevices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = children.begin(); iter != children.end(); iter++) {
			resolveRecursive(*iter);
		}
	}

}
