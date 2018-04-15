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

	void UPnPDeviceDeserializer::parseDeviceXml(const string & xml, UPnPDevice & device) {
		XmlDocument doc = DomParser::parse(xml);
		AutoRef<XmlNode> deviceNode = doc.getRootNode()->getElementByTagName("device");
		if (deviceNode.nil()) {
			throw OS::Exception("cannot build device / wrong device description");
		}
		
		parseDeviceXmlNode(deviceNode, device);
	}

	void UPnPDeviceDeserializer::parseDeviceXmlNode(AutoRef<XmlNode> deviceXml, UPnPDevice & device) {
		parseDevicePropertiesFromDeviceXmlNode(deviceXml, device);
		parseServiceListFromDeviceXmlNode(deviceXml, device);
		vector<AutoRef<XmlNode> > devices =  deviceXml->getElementsByTagNameInDepth("device", 2);
		for (vector<AutoRef<XmlNode> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			parseDeviceXmlNode(*iter, *device.prepareDevice());
		}
	}

	void UPnPDeviceDeserializer::parseDevicePropertiesFromDeviceXmlNode(AutoRef<XmlNode> deviceXml, UPnPDevice & device) {
		parsePropertiesFromXmlNode(deviceXml, device);
	}

	void UPnPDeviceDeserializer::parseServiceListFromDeviceXmlNode(AutoRef<XmlNode> deviceXml, UPnPDevice & device) {
		vector<AutoRef<XmlNode> > services = deviceXml->getElementsByTagNameInDepth("service", 2);
		for (vector<AutoRef<XmlNode> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			AutoRef<UPnPService> service(new UPnPService(NULL));
			parseServicePropertiesFromServiceXmlNode(*iter, &service);
			device.addService(service);
		}
	}

	void UPnPDeviceDeserializer::parseServicePropertiesFromServiceXmlNode(AutoRef<XmlNode> serviceXml, UPnPService * service) {
		parsePropertiesFromXmlNode(serviceXml, *service);
	}

	UPnPAction UPnPDeviceDeserializer::parseActionFromXmlNode(AutoRef<XmlNode> actionXml) {
		UPnPAction action;
		AutoRef<XmlNode> name = actionXml->getElementByTagName("name");
		if (XmlUtils::testKeyValueXmlNode(name)) {
			KeyValue nv = XmlUtils::toKeyValue(name);
			action.name() = nv.value();
		}
		vector<AutoRef<XmlNode> > arguments = actionXml->getElementsByTagName("argument");
		for (vector<AutoRef<XmlNode> >::iterator iter = arguments.begin(); iter != arguments.end(); iter++) {
			action.addArgument(parseArgumentFromXmlNode(*iter));
		}
		return action;
	}

	UPnPArgument UPnPDeviceDeserializer::parseArgumentFromXmlNode(AutoRef<XmlNode> argumentXml) {
		UPnPArgument arg;
		vector<AutoRef<XmlNode> > children = argumentXml->children();
		for (vector<AutoRef<XmlNode> >::iterator iter = children.begin(); iter != children.end(); iter++) {
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
	
	UPnPStateVariable UPnPDeviceDeserializer::parseStateVariableFromXmlNode(AutoRef<XmlNode> stateVariableXml) {
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
		
		vector<AutoRef<XmlNode> > children = stateVariableXml->children();
		for (vector<AutoRef<XmlNode> >::iterator iter = children.begin(); iter != children.end(); iter++) {
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
					vector<AutoRef<XmlNode> > values = (*iter)->children();
					for (vector<AutoRef<XmlNode> >::iterator vi = values.begin(); vi != values.end(); vi++) {
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
	
	void UPnPDeviceDeserializer::parsePropertiesFromXmlNode(AutoRef<XmlNode> node, UPnPModelObject & obj) {
		vector<AutoRef<XmlNode> > children = node->children();
		for (vector<AutoRef<XmlNode> >::iterator iter = children.begin(); iter != children.end(); iter++) {
			if (XmlUtils::testKeyValueXmlNode(*iter)) {
				KeyValue kv = XmlUtils::toKeyValue(*iter);
				if (!kv.value().empty()) {
					obj[kv.key()] = kv.value();
				}
			}
		}
	}

	AutoRef<UPnPDevice> UPnPDeviceDeserializer::parseDeviceXml(const string & deviceXml) {
		debug("upnp", deviceXml);
		AutoRef<UPnPDevice> device(new UPnPDevice);
		XmlDocument doc = DomParser::parse(deviceXml);
		parseDeviceXmlNode(doc.getRootNode()->getElementByTagName("device"), *device);
		return device;
	}
	
	UPnPScpd UPnPDeviceDeserializer::parseScpdXml(const string & scpdXml) {
		debug("upnp", scpdXml);
		if (scpdXml.empty()) {
			return UPnPScpd();
		}
		UPnPScpd scpd;
		XmlDocument doc = DomParser::parse(scpdXml);
		if (doc.getRootNode().nil()) {
			throw OS::Exception("wrong scpd format - xml parse failed");
		}
		vector<AutoRef<XmlNode> > actions = doc.getRootNode()->getElementsByTagName("action");
		for (vector<AutoRef<XmlNode> >::iterator iter = actions.begin(); iter != actions.end(); iter++) {
			UPnPAction action = parseActionFromXmlNode(*iter);
			scpd.action(action.name()) = action;
		}
		vector<AutoRef<XmlNode> > stateVariables = doc.getRootNode()->getElementsByTagName("stateVariable");
		for (vector<AutoRef<XmlNode> >::iterator iter = stateVariables.begin(); iter != stateVariables.end(); iter++) {
			UPnPStateVariable stateVariable = parseStateVariableFromXmlNode(*iter);
			scpd.stateVariable(stateVariable.name()) = stateVariable;
		}
		return scpd;
	}
}
