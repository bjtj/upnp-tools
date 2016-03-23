#include "UPnPSession.hpp"

namespace UPNP {

	using namespace std;
	using namespace UTIL;
	using namespace XML;
	
	UPnPSession::UPnPSession(const string & udn) :
		udn(udn), _completed(false), rootDevice(new UPnPDevice), creationTime(0), updateTime(0), sessionTimeout(0) {
	}
	UPnPSession::~UPnPSession() {
		printf("[%s] session instance destroyed\n", udn.c_str());
	}

	void UPnPSession::setCreationTime(unsigned long creationTime) {
		this->creationTime = creationTime;
	}

	void UPnPSession::setUpdateTime(unsigned long updateTime) {
		this->updateTime = updateTime;
	}

	void UPnPSession::setSessionTimeout(unsigned long sessionTimeout) {
		this->sessionTimeout = sessionTimeout;
	}

	unsigned long UPnPSession::lifetime() {
		return (OS::tick_milli() - creationTime);
	}

	unsigned long UPnPSession::duration() {
		return (OS::tick_milli() - updateTime);
	}

	bool UPnPSession::outdated() {
		return (duration() >= sessionTimeout);
	}

	void UPnPSession::buildDevice(SSDP::SSDPHeader & header) {
		dd = HttpUtils::httpGet(header.getLocation());
		rootDevice->baseUrl() = header.getLocation();
		XmlDocument doc = DomParser::parse(dd);
		XmlNode * deviceNode = doc.getRootNode()->getElementByTagName("device");
		if (deviceNode) {
			parseDeviceXmlNode(deviceNode, *rootDevice);
		}
	}

	void UPnPSession::parseDeviceXmlNode(XmlNode * deviceXml, UPnPDevice & device) {
		parseDevicePropertiesFromDeviceXmlNode(deviceXml, device);
		parseServiceListFromDeviceXmlNode(deviceXml, device);

		vector<XmlNode*> devices =  deviceXml->getElementsByTagNameInDepth("device", 2);
		for (vector<XmlNode*>::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			parseDeviceXmlNode(*iter, *device.prepareDevice());
		}
	}

	void UPnPSession::parseDevicePropertiesFromDeviceXmlNode(XmlNode * deviceXml, UPnPDevice & device) {
		parsePropertiesFromXmlNode(deviceXml, device);
	}

	void UPnPSession::parseServiceListFromDeviceXmlNode(XmlNode * deviceXml, UPnPDevice & device) {
		vector<XmlNode*> services = deviceXml->getElementsByTagNameInDepth("service", 2);
		for (vector<XmlNode*>::iterator iter = services.begin(); iter != services.end(); iter++) {
			AutoRef<UPnPService> service(new UPnPService(NULL));
			parseServicePropertiesFromServiceXmlNode(*iter, &service);
			device.addService(service);
			buildService(*service);
		}
	}

	void UPnPSession::parseServicePropertiesFromServiceXmlNode(XmlNode * serviceXml, UPnPService * service) {
		parsePropertiesFromXmlNode(serviceXml, *service);
	}

	string UPnPSession::getDump(const HTTP::Url & url) {
		return HttpUtils::httpGet(url);
	}

	void UPnPSession::buildService(UPnPService & service) {
		parseScpdFromXml(service, HttpUtils::httpGet(service.makeScpdUrl()));
	}

	void UPnPSession::parseScpdFromXml(UPnPService & service, const string & scpd) {
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

	UPnPAction UPnPSession::parseActionFromXml(XmlNode * actionXml) {
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

	UPnPArgument UPnPSession::parseArgumentFromXml(XmlNode * argumentXml) {
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
	UPnPStateVariable UPnPSession::parseStateVariableFromXml(XmlNode * stateVariableXml) {
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
	void UPnPSession::parsePropertiesFromXmlNode(XmlNode * node, UPnPObject & obj) {
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

	string UPnPSession::getDeviceDescription() {
		return dd;
	}

	string UPnPSession::getFriendlyName() {
		return fn;
	}

	bool UPnPSession::completed() {
		return _completed;
	}

	AutoRef<UPnPDevice> UPnPSession::getRootDevice() {
		return rootDevice;
	}

	string UPnPSession::toString() {
		if (rootDevice.nil()) {
			return "";
		}
		return toString(*rootDevice, 0);
	}

	string UPnPSession::toString(UPnPDevice & device, int depth) {
		string str;

		str.append(depth, ' ');
		if (depth > 0) { str.append(" - "); }
		str.append(device.getUdn() + " (" + device.getFriendlyName() + ")");

		vector<AutoRef<UPnPService> > services = device.services();
		for (vector<AutoRef<UPnPService> >::iterator iter = services.begin(); iter != services.end(); iter++) {
			str.append("\n");
			str.append(depth, ' ');
			str.append(" ** " + (*iter)->getServiceType());

			vector<UPnPAction> actions = (*iter)->actions();
			for (vector<UPnPAction>::iterator aiter = actions.begin(); aiter != actions.end(); aiter++) {
				str.append("\n");
				str.append(depth, ' ');
				str.append("  - " + (*aiter).name());
			}
		}
			
		vector<AutoRef<UPnPDevice> > & devices = device.devices();
		for (vector<AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
			str.append("\n");
			str.append(toString(*(*iter), depth + 1));
		}
		return str;
	}
	
}

