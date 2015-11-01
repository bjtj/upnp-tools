#ifndef __UPNP_DEVICE_MAKER_HPP__
#define __UPNP_DEVICE_MAKER_HPP__

#include "UPnPDevice.hpp"
#include "XmlDocument.hpp"

namespace UPNP {

	class UPnPDeviceMaker {
	private:
	public:
		UPnPDeviceMaker();
		virtual ~UPnPDeviceMaker();

		static UPnPDevice makeDeviceWithDeviceDescription(const XML::XmlDocument & doc);
		static UPnPDevice makeDeviceWithDeviceNode(const XML::XmlNode & deviceNode);
		static std::vector<UPnPService> makeServiceListFromXmlNode(const XML::XmlNode & deviceNode);
	};

}

#endif