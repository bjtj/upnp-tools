#include "UPnPDeviceMaker.hpp"
#include "XmlNodeFinder.hpp"

namespace UPNP {

	using namespace XML;

	UPnPDeviceMaker::UPnPDeviceMaker() {
	}

	UPnPDeviceMaker::~UPnPDeviceMaker() {
	}

	UPnPDevice UPnPDeviceMaker::makeDeviceWithDeviceDescription(XmlDocument doc) {
		UPnPDevice device;

		XmlNodeFinder finder(doc);
		device.setUdn(finder.getContentByTagName("UDN"));
		device.setFriendlyName(finder.getContentByTagName("friendlyName"));

		return device;
	}
}