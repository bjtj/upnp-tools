#ifndef __UPNP_DEVICE_DESERIALIZER_HPP__
#define __UPNP_DEVICE_DESERIALIZER_HPP__

#include <liboslayer/AutoRef.hpp>
#include <liboslayer/XmlParser.hpp>
#include "SSDPHeader.hpp"
#include "UPnPModels.hpp"

namespace UPNP {
	
	class UPnPDeviceDeserializer {
	private:
		bool _withScpdBuild;
	public:
		UPnPDeviceDeserializer();
		UPnPDeviceDeserializer(bool withScpdBuild);
		virtual ~UPnPDeviceDeserializer();

		bool & withScpdBuild();

		UTIL::AutoRef<UPnPDevice> buildDevice(const HTTP::Url & url);
		UTIL::AutoRef<UPnPDevice> buildDeviceWithDescriptionXml(const std::string & descriptionXml, const HTTP::Url & url);
		void parseDeviceXml(const std::string & xml, UPnPDevice & device);
		void parseDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		void parseDevicePropertiesFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		void parseServiceListFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		void parseServicePropertiesFromServiceXmlNode(XML::XmlNode * serviceXml, UPnPService * service);
		void buildService(UPnPService & service);
		void parseScpdFromXml(UPnPService & service, const std::string & scpd);
		UPnPAction parseActionFromXmlNode(XML::XmlNode * actionXml);
		UPnPArgument parseArgumentFromXmlNode(XML::XmlNode * argumentXml);
		UPnPStateVariable parseStateVariableFromXmlNode(XML::XmlNode * stateVariableXml);
		void parsePropertiesFromXmlNode(XML::XmlNode * node, UPnPModelObject & obj);
	};
}

#endif
