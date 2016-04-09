#include "UPnPDeviceSerializer.hpp"
#include "XmlUtils.hpp"

namespace UPNP {

	using namespace std;
	using namespace UTIL;
	
	UPnPDeviceSerializer::UPnPDeviceSerializer() {
	}
	UPnPDeviceSerializer::~UPnPDeviceSerializer() {
	}

	string UPnPDeviceSerializer::serializeDeviceDescription(UPnPDevice & device) {
		string ret = "<?xml version=\"1.0\"?>"
			"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
			"<specVersion>"
			"<major>1</major>"
			"<minor>0</minor>"
			"</specVersion>"
			"<device>";

		PropertyMap & props = device.getProperties();
		for (size_t i = 0; i < props.size(); i++) {
			ret.append(XmlUtils::toNameValueTag(props[i]));
		}

		ret.append("<serviceList>");
		
		vector<AutoRef<UPnPService> > & services = device.services();
		for (vector<AutoRef<UPnPService> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			AutoRef<UPnPService> service = *iter;
			
			ret.append("<service>");
			PropertyMap & props = service->getProperties();
			for (size_t i = 0; i < props.size(); i++) {
				ret.append(XmlUtils::toNameValueTag(props[i]));
			}
			ret.append("</service>");
		}
		
		ret.append("</serviceList>");
		ret.append("</root>");
		
		return ret;
	}
	
	string UPnPDeviceSerializer::serializeScpd(UPnPService & service) {
		throw OS::Exception("Not implemeneted");
	}
	
}
