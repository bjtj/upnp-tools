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

        static UPnPDevice makeDeviceFromDeviceDescription(const std::string & baseUrl, const XML::XmlDocument & doc);
		static UPnPDevice makeDeviceFromDeviceNode(const std::string & baseUrl, const XML::XmlNode & deviceNode);
		static std::vector<UPnPService> makeServiceListFromXmlNode(const std::string & baseUrl, const XML::XmlNode & deviceNode);
	};

}

#endif