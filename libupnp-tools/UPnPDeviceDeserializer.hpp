#ifndef __UPNP_DEVICE_DESERIALIZER_HPP__
#define __UPNP_DEVICE_DESERIALIZER_HPP__

#include <liboslayer/AutoRef.hpp>
#include <liboslayer/XmlParser.hpp>
#include "SSDPHeader.hpp"
#include "UPnPModels.hpp"
#include "UPnPDebug.hpp"

namespace UPNP {

	/**
	 * @brief 
	 */
	class UPnPDeviceDeserializer : public UPnPDebuggable {
	private:
	public:
		UPnPDeviceDeserializer();
		virtual ~UPnPDeviceDeserializer();

		void parseDeviceXml(const std::string & xml, UPnPDevice & device);
		void parseDeviceXmlNode(OS::AutoRef<XML::XmlNode> deviceXml, UPnPDevice & device);
		void parseDevicePropertiesFromDeviceXmlNode(OS::AutoRef<XML::XmlNode> deviceXml, UPnPDevice & device);
		void parseServiceListFromDeviceXmlNode(OS::AutoRef<XML::XmlNode> deviceXml, UPnPDevice & device);
		void parseServicePropertiesFromServiceXmlNode(OS::AutoRef<XML::XmlNode> serviceXml, UPnPService * service);
		UPnPAction parseActionFromXmlNode(OS::AutoRef<XML::XmlNode> actionXml);
		UPnPArgument parseArgumentFromXmlNode(OS::AutoRef<XML::XmlNode> argumentXml);
		UPnPStateVariable parseStateVariableFromXmlNode(OS::AutoRef<XML::XmlNode> stateVariableXml);
		void parsePropertiesFromXmlNode(OS::AutoRef<XML::XmlNode> node, UPnPModelObject & obj);

		virtual OS::AutoRef<UPnPDevice> build(const HTTP::Url & url);
		virtual OS::AutoRef<UPnPDevice> parseDeviceXml(const std::string & deviceXml);
		virtual UPnPScpd parseScpdXml(const std::string & scpdXml);
	};
}

#endif
