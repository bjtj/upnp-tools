#include "UPnPDeviceDeserializer.hpp"
#include "XmlUtils.hpp"
#include "UPnPResourceManager.hpp"

namespace UPNP {

	using namespace std;
	using namespace OS;
	using namespace SSDP;
	using namespace XML;
	using namespace HTTP;
	using namespace UTIL;

	UPnPDeviceDeserializer::UPnPDeviceDeserializer() {
	}
	UPnPDeviceDeserializer::~UPnPDeviceDeserializer() {
	}

	void UPnPDeviceDeserializer::deserializeDeviceNode(AutoRef<XmlNode> deviceXml, UPnPDevice & device) {
		deserializeDeviceProperties(deviceXml, device);
		deserializeServiceList(deviceXml, device);
		vector< AutoRef<XmlNode> > devices =  deviceXml->getElementsByTagNameInDepth("device", 2);
		for (vector< AutoRef<XmlNode> >::iterator iter = devices.begin();
			 iter != devices.end(); iter++)
		{
			deserializeDeviceNode(*iter, *device.prepareDevice());
		}
	}

	void UPnPDeviceDeserializer::deserializeDeviceProperties(AutoRef<XmlNode> deviceXml, UPnPDevice & device) {
		deserializeProperties(deviceXml, device);
	}

	void UPnPDeviceDeserializer::deserializeServiceList(AutoRef<XmlNode> deviceXml, UPnPDevice & device) {
		vector< AutoRef<XmlNode> > services = deviceXml->getElementsByTagNameInDepth("service", 2);
		for (vector< AutoRef<XmlNode> >::iterator iter = services.begin();
			 iter != services.end(); iter++)
		{
			AutoRef<UPnPService> service(new UPnPService);
			deserializeServiceProperties(*iter, &service);
			device.addService(service);
		}
	}

	void UPnPDeviceDeserializer::deserializeServiceProperties(AutoRef<XmlNode> serviceXml, UPnPService * service) {
		deserializeProperties(serviceXml, *service);
	}

	UPnPAction UPnPDeviceDeserializer::deserializeActionNode(AutoRef<XmlNode> actionXml) {
		UPnPAction action;
		AutoRef<XmlNode> name = actionXml->getElementByTagName("name");
		if (XmlUtils::testKeyValueXmlNode(name)) {
			KeyValue nv = XmlUtils::toKeyValue(name);
			action.name() = nv.value();
		}
		vector< AutoRef<XmlNode> > arguments = actionXml->getElementsByTagName("argument");
		for (vector< AutoRef<XmlNode> >::iterator iter = arguments.begin();
			 iter != arguments.end(); iter++)
		{
			action.addArgument(deserializeArgumentNode(*iter));
		}
		return action;
	}

	UPnPArgument UPnPDeviceDeserializer::deserializeArgumentNode(AutoRef<XmlNode> argumentXml) {
		UPnPArgument arg;
		vector< AutoRef<XmlNode> > children = argumentXml->children();
		for (vector< AutoRef<XmlNode> >::iterator iter = children.begin();
			 iter != children.end(); iter++)
		{
			if (XmlUtils::testKeyValueXmlNode(*iter)) {
				KeyValue kv = XmlUtils::toKeyValue(*iter);
				if (kv.key() == "name") {
					arg.name() = kv.value();
				} else if (kv.key() == "direction") {
					arg.direction() = (kv.value() == "out" ? UPnPArgument::OUT_DIRECTION : UPnPArgument::IN_DIRECTION);
				} else if (kv.key() == "relatedStateVariable") {
					arg.relatedStateVariable() = kv.value();
				}
			}
		}
		return arg;
	}
	
	UPnPStateVariable UPnPDeviceDeserializer::deserializeStateVariable(AutoRef<XmlNode> stateVariableXml) {
		UPnPStateVariable stateVariable;

		// send events
		if (stateVariableXml->attr("sendEvents") == "no") {
			stateVariable.sendEvents() = false;
		} else {
			stateVariable.sendEvents() = true;
		}

		// multicast
		if (stateVariableXml->attr("multicast") == "yes") {
			stateVariable.multicast() = true;
		} else {
			stateVariable.multicast() = false;
		}
		
		vector< AutoRef<XmlNode> > children = stateVariableXml->children();
		for (vector< AutoRef<XmlNode> >::iterator iter = children.begin();
			 iter != children.end(); iter++)
		{
			if ((*iter)->isElement()) {
				
				// single arguments
				if (XmlUtils::testKeyValueXmlNode(*iter)) {
					KeyValue kv = XmlUtils::toKeyValue(*iter);
					if (kv.key() == "name") {
						stateVariable.name() = kv.value();
					} else if (kv.key() == "dataType") {
						stateVariable.dataType() = kv.value();
					}
				}
				// allowed value list
				if ((*iter)->tagName() == "allowedValueList") {
					vector< AutoRef<XmlNode> > values = (*iter)->children();
					for (vector< AutoRef<XmlNode> >::iterator vi = values.begin();
						 vi != values.end(); vi++)
					{
						if (XmlUtils::testKeyValueXmlNode(*vi)) {
							KeyValue anv = XmlUtils::toKeyValue(*vi);
							if (anv.key() == "allowedValue") {
								stateVariable.addAllowedValue(anv.value());
							}
						}
					}
				}
				// TODO: allowed value range (minimum, maximum, step)
			}
		}
		return stateVariable;
	}
	
	void UPnPDeviceDeserializer::deserializeProperties(AutoRef<XmlNode> node, UPnPModelObject & obj) {
		vector< AutoRef<XmlNode> > children = node->children();
		for (vector< AutoRef<XmlNode> >::iterator iter = children.begin();
			 iter != children.end(); iter++)
		{
			if (XmlUtils::testKeyValueXmlNode(*iter)) {
				KeyValue kv = XmlUtils::toKeyValue(*iter);
				if (!kv.value().empty()) {
					obj[kv.key()] = kv.value();
				}
			}
		}
	}

	AutoRef<UPnPDevice> UPnPDeviceDeserializer::deserializeDevice(const string & xml) {
		// debug("upnp", deviceXml);
		AutoRef<UPnPDevice> device(new UPnPDevice);
		XmlDocument doc = DomParser::parse(xml);
		deserializeDeviceNode(doc.rootNode()->getElementByTagName("device"), *device);
		return device;
	}
	
	UPnPScpd UPnPDeviceDeserializer::deserializeScpd(const string & xml) {
		// debug("upnp", scpdXml);
		if (xml.empty()) {
			return UPnPScpd();
		}
		UPnPScpd scpd;
		XmlDocument doc = DomParser::parse(xml);
		if (doc.rootNode().nil()) {
			throw OS::Exception("wrong scpd format - xml parse failed");
		}
		vector< AutoRef<XmlNode> > actions = doc.rootNode()->getElementsByTagName("action");
		for (vector< AutoRef<XmlNode> >::iterator iter = actions.begin();
			 iter != actions.end(); iter++)
		{
			UPnPAction action = deserializeActionNode(*iter);
			scpd.action(action.name()) = action;
		}
		vector< AutoRef<XmlNode> > stateVariables = doc.rootNode()->getElementsByTagName("stateVariable");
		for (vector< AutoRef<XmlNode> >::iterator iter = stateVariables.begin();
			 iter != stateVariables.end(); iter++)
		{
			UPnPStateVariable stateVariable = deserializeStateVariable(*iter);
			scpd.stateVariable(stateVariable.name()) = stateVariable;
		}
		return scpd;
	}
}
