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
		void parseDeviceXmlNode(UTIL::AutoRef<XML::XmlNode> deviceXml, UPnPDevice & device);
		void parseDevicePropertiesFromDeviceXmlNode(UTIL::AutoRef<XML::XmlNode> deviceXml, UPnPDevice & device);
		void parseServiceListFromDeviceXmlNode(UTIL::AutoRef<XML::XmlNode> deviceXml, UPnPDevice & device);
		void parseServicePropertiesFromServiceXmlNode(UTIL::AutoRef<XML::XmlNode> serviceXml, UPnPService * service);
		UPnPAction parseActionFromXmlNode(UTIL::AutoRef<XML::XmlNode> actionXml);
		UPnPArgument parseArgumentFromXmlNode(UTIL::AutoRef<XML::XmlNode> argumentXml);
		UPnPStateVariable parseStateVariableFromXmlNode(UTIL::AutoRef<XML::XmlNode> stateVariableXml);
		void parsePropertiesFromXmlNode(UTIL::AutoRef<XML::XmlNode> node, UPnPModelObject & obj);

		virtual UTIL::AutoRef<UPnPDevice> build(const HTTP::Url & url);
		virtual UTIL::AutoRef<UPnPDevice> parseDeviceXml(const std::string & deviceXml);
		virtual UPnPScpd parseScpdXml(const std::string & scpdXml);
	};
}

#endif
