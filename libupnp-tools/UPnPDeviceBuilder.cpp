#include <vector>
#include "UPnPDeviceBuilder.hpp"
#include "UPnPDeviceDeserializer.hpp"
#include "UPnPResourceManager.hpp"

namespace UPNP {

	using namespace std;
	using namespace HTTP;
	using namespace OS;
	using namespace UTIL;

	UPnPDeviceBuilder::UPnPDeviceBuilder(const Url & url)
		: _url(url) {
	}
	UPnPDeviceBuilder::~UPnPDeviceBuilder() {
	}
	Url & UPnPDeviceBuilder::url() {
		return _url;
	}
	AutoRef<UPnPDevice> & UPnPDeviceBuilder::device() {
		return _device;
	}
	AutoRef<UPnPDevice> UPnPDeviceBuilder::execute() {
		UPnPResourceManager & resMan = UPnPResourceManager::instance();
		UPnPDeviceDeserializer deserializer;
		LinkedStringMap meta;
		UPnPResource res = resMan.getResource(_url);
		_device = deserializer.parseDeviceXml(res.content());
		_device->meta() = res.meta();
		_device->baseUrl() = _url;
		vector<UPnPService*> services = _device->allServices();
		for (vector<UPnPService*>::iterator iter = services.begin(); iter != services.end(); iter++) {
			(*iter)->scpd() = deserializer.parseScpdXml(resMan.getResourceContent(_url.relativePath((*iter)->scpdUrl())));
		}
		return _device;
	}

}
