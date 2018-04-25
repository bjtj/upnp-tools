#ifndef __UPNP_DEVICE_DESERIALIZER_HPP__
#define __UPNP_DEVICE_DESERIALIZER_HPP__

#include <liboslayer/AutoRef.hpp>
#include <liboslayer/XmlParser.hpp>
#include "SSDPHeader.hpp"
#include "UPnPModels.hpp"
#include "UPnPDebug.hpp"

namespace upnp {

	/**
	 * @brief 
	 */
	class UPnPDeviceDeserializer {
	private:
	public:
		UPnPDeviceDeserializer();
		virtual ~UPnPDeviceDeserializer();

		static void deserializeDeviceNode(osl::AutoRef<osl::XmlNode> deviceXml, UPnPDevice & device);
		static void deserializeDeviceProperties(osl::AutoRef<osl::XmlNode> deviceXml, UPnPDevice & device);
		static void deserializeServiceList(osl::AutoRef<osl::XmlNode> deviceXml, UPnPDevice & device);
		static void deserializeServiceProperties(osl::AutoRef<osl::XmlNode> serviceXml, UPnPService * service);
		static UPnPAction deserializeActionNode(osl::AutoRef<osl::XmlNode> actionXml);
		static UPnPArgument deserializeArgumentNode(osl::AutoRef<osl::XmlNode> argumentXml);
		static UPnPStateVariable deserializeStateVariable(osl::AutoRef<osl::XmlNode> stateVariableXml);
		static void deserializeProperties(osl::AutoRef<osl::XmlNode> node, UPnPModelObject & obj);

		static osl::AutoRef<UPnPDevice> deserializeDevice(const std::string & xml);
		static UPnPScpd deserializeScpd(const std::string & xml);
	};
}

#endif
