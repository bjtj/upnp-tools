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

		static void deserializeDeviceNode(OS::AutoRef<XML::XmlNode> deviceXml, UPnPDevice & device);
		static void deserializeDeviceProperties(OS::AutoRef<XML::XmlNode> deviceXml, UPnPDevice & device);
		static void deserializeServiceList(OS::AutoRef<XML::XmlNode> deviceXml, UPnPDevice & device);
		static void deserializeServiceProperties(OS::AutoRef<XML::XmlNode> serviceXml, UPnPService * service);
		static UPnPAction deserializeActionNode(OS::AutoRef<XML::XmlNode> actionXml);
		static UPnPArgument deserializeArgumentNode(OS::AutoRef<XML::XmlNode> argumentXml);
		static UPnPStateVariable deserializeStateVariable(OS::AutoRef<XML::XmlNode> stateVariableXml);
		static void deserializeProperties(OS::AutoRef<XML::XmlNode> node, UPnPModelObject & obj);

		static OS::AutoRef<UPnPDevice> deserializeDevice(const std::string & xml);
		static UPnPScpd deserializeScpd(const std::string & xml);
	};
}

#endif
