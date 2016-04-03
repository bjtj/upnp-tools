#include "UPnPDeviceDeserializer.hpp"
#include "XmlUtils.hpp"
#include "UPnPResourceManager.hpp"

namespace UPNP {

	using namespace std;
	using namespace SSDP;
	using namespace XML;
	using namespace HTTP;
	using namespace UTIL;

	UPnPDeviceDeserializer::UPnPDeviceDeserializer() {}
	UPnPDeviceDeserializer::~UPnPDeviceDeserializer() {}

	AutoRef<UPnPDevice> UPnPDeviceDeserializer::buildDevice(SSDPHeader & header) {
		
		XmlDocument doc = DomParser::parse(UPnPResourceManager::getResource(header.getLocation()));
		XmlNode * deviceNode = doc.getRootNode()->getElementByTagName("device");
		if (!deviceNode) {
			throw OS::Exception("cannot build device / wrong device description");
		}
		
		AutoRef<UPnPDevice> device(new UPnPDevice);
		device->baseUrl() = header.getLocation();
		parseDeviceXmlNode(deviceNode, *device);

		return device;
	}

	void UPnPDeviceDeserializer::parseDeviceXmlNode(XmlNode * deviceXml, UPnPDevice & device) {
		parseDevicePropertiesFromDeviceXmlNode(deviceXml, device);
		parseServiceListFromDeviceXmlNode(deviceXml, device);
		vector<XmlNode*> devices =  deviceXml->getElementsByTagNameInDepth("device", 2);
		for (vector<XmlNode*>::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			parseDeviceXmlNode(*iter, *device.prepareDevice());
		}
	}

	void UPnPDeviceDeserializer::parseDevicePropertiesFromDeviceXmlNode(XmlNode * deviceXml, UPnPDevice & device) {
		parsePropertiesFromXmlNode(deviceXml, device);
	}

	void UPnPDeviceDeserializer::parseServiceListFromDeviceXmlNode(XmlNode * deviceXml, UPnPDevice & device) {
		vector<XmlNode*> services = deviceXml->getElementsByTagNameInDepth("service", 2);
		for (vector<XmlNode*>::iterator iter = services.begin(); iter != services.end(); iter++) {
			AutoRef<UPnPService> service(new UPnPService(NULL));
			parseServicePropertiesFromServiceXmlNode(*iter, &service);
			device.addService(service);
			buildService(*service);
		}
	}

	void UPnPDeviceDeserializer::parseServicePropertiesFromServiceXmlNode(XmlNode * serviceXml, UPnPService * service) {
		parsePropertiesFromXmlNode(serviceXml, *service);
	}

	void UPnPDeviceDeserializer::buildService(UPnPService & service) {
		parseScpdFromXml(service, UPnPResourceManager::getResource(service.makeScpdUrl()));
	}

	void UPnPDeviceDeserializer::parseScpdFromXml(UPnPService & service, const string & scpd) {
		XmlDocument doc = DomParser::parse(scpd);
		if (doc.getRootNode().nil()) {
			throw OS::Exception("wrong scpd format", -1, 0);
		}
		vector<XmlNode*> actions = doc.getRootNode()->getElementsByTagName("action");
		for (vector<XmlNode*>::iterator iter = actions.begin(); iter != actions.end(); iter++) {
			service.addAction(parseActionFromXml(*iter));
		}
		vector<XmlNode*> stateVariables = doc.getRootNode()->getElementsByTagName("stateVariable");
		for (vector<XmlNode*>::iterator iter = stateVariables.begin(); iter != stateVariables.end(); iter++) {
			service.addStateVariable(parseStateVariableFromXml(*iter));
		}
	}

	UPnPAction UPnPDeviceDeserializer::parseActionFromXml(XmlNode * actionXml) {
		UPnPAction action;
		XmlNode * name = actionXml->getElementByTagName("name");
		if (XmlUtils::testNameValueXmlNode(name)) {
			NameValue nv = XmlUtils::toNameValue(name);
			action.name() = nv.value();
		}
		vector<XmlNode*> arguments = actionXml->getElementsByTagName("argument");
		for (vector<XmlNode*>::iterator iter = arguments.begin(); iter != arguments.end(); iter++) {
			action.addArgument(parseArgumentFromXml(*iter));
		}
		return action;
	}

	UPnPArgument UPnPDeviceDeserializer::parseArgumentFromXml(XmlNode * argumentXml) {
		UPnPArgument arg;
		vector<XmlNode*> children = argumentXml->children();
		for (vector<XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
			if (XmlUtils::testNameValueXmlNode(*iter)) {
				NameValue nv = XmlUtils::toNameValue(*iter);
				if (nv.name() == "name") {
					arg.name() = nv.value();
				} else if (nv.name() == "direction") {
					arg.direction() = (nv.value() == "out" ? UPnPArgument::OUT_DIRECTION : UPnPArgument::IN_DIRECTION);
				} else if (nv.name() == "relatedStateVariable") {
					arg.stateVariableName() = nv.value();
				}
			}
		}
		return arg;
	}
	UPnPStateVariable UPnPDeviceDeserializer::parseStateVariableFromXml(XmlNode * stateVariableXml) {
		UPnPStateVariable stateVariable;
		vector<XmlNode*> children = stateVariableXml->children();
		for (vector<XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
			if ((*iter)->isElement()) {
				if (XmlUtils::testNameValueXmlNode(*iter)) {
					NameValue nv = XmlUtils::toNameValue(*iter);
					if (nv.name() == "name") {
						stateVariable.name() = nv.value();
					} else if (nv.name() == "dataType") {
						stateVariable.dataType() = nv.value();
					}
				}
				if ((*iter)->tagName() == "allowedValueList") {
					vector<XmlNode*> values = (*iter)->children();
					for (vector<XmlNode*>::iterator vi = values.begin(); vi != values.end(); vi++) {
						if (XmlUtils::testNameValueXmlNode(*vi)) {
							NameValue anv = XmlUtils::toNameValue(*vi);
							if (anv.name() == "allowedValue") {
								stateVariable.addAllowedValue(anv.value());
							}
						}
					}
				}
			}
		}
		return stateVariable;
	}
	void UPnPDeviceDeserializer::parsePropertiesFromXmlNode(XmlNode * node, UPnPObject & obj) {
		vector<XmlNode*> children = node->children();
		for (vector<XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
			if (XmlUtils::testNameValueXmlNode(*iter)) {
				NameValue nv = XmlUtils::toNameValue(*iter);
				if (!nv.value().empty()) {
					obj[nv.name()] = nv.value();
				}
			}
		}
	}
	
}
