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

		UPnPDevice makeDeviceWithDeviceDescription(XML::XmlDocument & doc);
		UPnPDevice makeDeviceWithDeviceNode(XML::XmlNode & deviceNode);
	};

}

#endif