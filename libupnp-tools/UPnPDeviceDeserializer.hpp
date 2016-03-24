#ifndef __UPNP_DEVICE_DESERIALIZER_HPP__
#define __UPNP_DEVICE_DESERIALIZER_HPP__

#include <liboslayer/AutoRef.hpp>
#include <liboslayer/XmlParser.hpp>
#include "SSDPHeader.hpp"
#include "UPnPModels.hpp"

namespace UPNP {
	
	class UPnPDeviceDeserializer {
	private:
	public:
		UPnPDeviceDeserializer();
		virtual ~UPnPDeviceDeserializer();

		static UTIL::AutoRef<UPnPDevice> buildDevice(SSDP::SSDPHeader & header);
		static void parseDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		static void parseDevicePropertiesFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		static void parseServiceListFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		static void parseServicePropertiesFromServiceXmlNode(XML::XmlNode * serviceXml, UPnPService * service);
		static void buildService(UPnPService & service);
		static void parseScpdFromXml(UPnPService & service, const std::string & scpd);
		static UPnPAction parseActionFromXml(XML::XmlNode * actionXml);
		static UPnPArgument parseArgumentFromXml(XML::XmlNode * argumentXml);
		static UPnPStateVariable parseStateVariableFromXml(XML::XmlNode * stateVariableXml);
		static void parsePropertiesFromXmlNode(XML::XmlNode * node, UPnPObject & obj);
	};
}

#endif
