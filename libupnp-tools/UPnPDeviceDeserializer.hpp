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
		void parseDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		void parseDevicePropertiesFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		void parseServiceListFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		void parseServicePropertiesFromServiceXmlNode(XML::XmlNode * serviceXml, UPnPService * service);
		UPnPAction parseActionFromXmlNode(XML::XmlNode * actionXml);
		UPnPArgument parseArgumentFromXmlNode(XML::XmlNode * argumentXml);
		UPnPStateVariable parseStateVariableFromXmlNode(XML::XmlNode * stateVariableXml);
		void parsePropertiesFromXmlNode(XML::XmlNode * node, UPnPModelObject & obj);

		virtual UTIL::AutoRef<UPnPDevice> build(const HTTP::Url & url);
		virtual UTIL::AutoRef<UPnPDevice> parseDeviceXml(const std::string & deviceXml);
		virtual UPnPScpd parseScpdXml(const std::string & scpdXml);
	};
}

#endif
