#include <vector>
#include "UPnPDeviceBuilder.hpp"
#include "UPnPDeviceDeserializer.hpp"
#include "UPnPResourceManager.hpp"

namespace upnp {

	using namespace std;
	using namespace http;
	using namespace osl;

	UPnPDeviceBuilder::UPnPDeviceBuilder(const Url & url)
		: _url(url), _allow_fail_scpd(false) {
	}

	UPnPDeviceBuilder::~UPnPDeviceBuilder() {
	}

	Url & UPnPDeviceBuilder::url() {
		return _url;
	}

	bool & UPnPDeviceBuilder::allow_fail_scpd() {
		return _allow_fail_scpd;
	}

	AutoRef<UPnPDevice> & UPnPDeviceBuilder::device() {
		return _device;
	}

	AutoRef<UPnPDevice> UPnPDeviceBuilder::execute() {
		UPnPResourceManager & resMan = UPnPResourceManager::instance();
		LinkedStringMap meta;
		UPnPResource res = resMan.getResource(_url);
		UPnPDebug::instance().debug("upnp", res.content());
		_device = UPnPDeviceDeserializer::deserializeDevice(res.content());
		_device->meta() = res.meta();
		_device->baseUrl() = _url;
		vector< AutoRef<UPnPService> > services = _device->allServices();
		for (vector< AutoRef<UPnPService> >::iterator iter = services.begin();
			 iter != services.end(); iter++)
		{
			try {
				string content = resMan.getResourceContent(_url.relativePath((*iter)->scpdUrl()));
				UPnPDebug::instance().debug("upnp", content);
				(*iter)->scpd() = UPnPDeviceDeserializer::deserializeScpd(content);
			} catch (Exception e) {
				if(_allow_fail_scpd == false) {
					throw e;
				}
			}
		}
		return _device;
	}


}
