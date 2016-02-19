#ifndef __UPNP_SESSION_HPP__
#define __UPNP_SESSION_HPP__

#include <string>
#include <vector>
#include <map>
#include <liboslayer/os.hpp>
#include <liboslayer/StringElement.hpp>
#include <liboslayer/XmlParser.hpp>
#include "SSDPHeader.hpp"
#include "UPnPModels.hpp"
#include "HttpUtils.hpp"
#include "XmlUtils.hpp"

namespace UPNP {

	/**
	 *
	 */
	class UPnPSession {
	private:
		static unsigned long idx_seed;
		unsigned long idx;
		std::string udn;
		std::string fn;
		std::string dd;
		bool _completed;
		UPnPDevice device;
		unsigned long creationTime;
		unsigned long updateTime;
		unsigned long sessionTimeout;
	
	public:
		UPnPSession(const std::string & udn) : udn(udn), idx(idx_seed++), _completed(false),
											   creationTime(0), updateTime(0), sessionTimeout(0) {}
		virtual ~UPnPSession() {printf("[%s] session instance destroyed\n", udn.c_str());}

		unsigned long getUniqueSessionId() {
			return idx;
		}

		void setCreationTime(unsigned long creationTime) {
			this->creationTime = creationTime;
		}

		void setUpdateTime(unsigned long updateTime) {
			this->updateTime = updateTime;
		}

		void setSessionTimeout(unsigned long sessionTimeout) {
			this->sessionTimeout = sessionTimeout;
		}

		unsigned long lifetime() {
			return (OS::tick_milli() - creationTime);
		}

		unsigned long duration() {
			return (OS::tick_milli() - updateTime);
		}

		bool outdated() {
			return (duration() >= sessionTimeout);
		}

		void buildDevice(SSDP::SSDPHeader & header) {
			dd = HttpUtils::httpGet(header.getLocation());
			device.baseUrl() = header.getLocation();
			XML::XmlDocument doc = XML::DomParser::parse(dd);
			XML::XmlNode * deviceNode = doc.getRootNode()->getElementByTagName("device");
			if (deviceNode) {
				parseDeviceXmlNode(deviceNode, device);
			}
		}

		void parseDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device) {
			parseDevicePropertiesFromDeviceXmlNode(deviceXml, device);
			parseServiceListFromDeviceXmlNode(deviceXml, device);

			std::vector<XML::XmlNode*> devices =  deviceXml->getElementsByTagNameInDepth("device", 2);
			for (std::vector<XML::XmlNode*>::iterator iter = devices.begin(); iter != devices.end(); iter++) {
				parseDeviceXmlNode(*iter, *device.prepareDevice());
			}
		}

		void parseDevicePropertiesFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device) {
			parsePropertiesFromXmlNode(deviceXml, device);
		}

		void parseServiceListFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device) {
			std::vector<XML::XmlNode*> services = deviceXml->getElementsByTagNameInDepth("service", 2);
			for (std::vector<XML::XmlNode*>::iterator iter = services.begin(); iter != services.end(); iter++) {
				UTIL::AutoRef<UPnPService> service(new UPnPService(NULL));
				parseServicePropertiesFromServiceXmlNode(*iter, &service);
				device.addService(service);
				buildService(*service);
			}
		}

		void parseServicePropertiesFromServiceXmlNode(XML::XmlNode * serviceXml, UPnPService * service) {
			parsePropertiesFromXmlNode(serviceXml, *service);
		}

		void buildService(UPnPService & service) {
			if (service.getDevice()) {
				HTTP::Url u = service.getDevice()->baseUrl().relativePath(service.getScpdUrl());
				std::string scpd = HttpUtils::httpGet(u);
				XML::XmlDocument doc = XML::DomParser::parse(scpd);
				if (doc.getRootNode().nil()) {
					return;
				}
				std::vector<XML::XmlNode*> actions = doc.getRootNode()->getElementsByTagName("action");
				for (std::vector<XML::XmlNode*>::iterator iter = actions.begin(); iter != actions.end(); iter++) {
					service.addAction(parseActionFromActionXml(*iter));
				}
				std::vector<XML::XmlNode*> stateVariables = doc.getRootNode()->getElementsByTagName("stateVariable");
			}
		}

		UPnPAction parseActionFromActionXml(XML::XmlNode * actionXml) {
			UPnPAction action;
			XML::XmlNode * name = actionXml->getElementByTagName("name");
			if (XmlUtils::testNameValueXmlNode(name)) {
				UTIL::NameValue nv = XmlUtils::toNameValue(name);
				action.name() = nv.value();
			}
			std::vector<XML::XmlNode*> arguments = actionXml->getElementsByTagName("argument");
			for (std::vector<XML::XmlNode*>::iterator iter = arguments.begin(); iter != arguments.end(); iter++) {
				action.addArgument(parseArgumentFromArgumentXml(*iter));
			}
			return action;
		}

		UPnPArgument parseArgumentFromArgumentXml(XML::XmlNode * argumentXml) {
			UPnPArgument arg;
			std::vector<XML::XmlNode*> children = argumentXml->children();
			for (std::vector<XML::XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
				if (XmlUtils::testNameValueXmlNode(*iter)) {
					UTIL::NameValue nv = XmlUtils::toNameValue(*iter);
					if (nv.name() == "name") {
						arg.name() = nv.value();
					} else if (nv.name() == "direction") {
						arg.direction() = (nv.value() == "out" ?
										   UPnPArgument::OUT_DIRECTION : UPnPArgument::IN_DIRECTION);
					} else if (nv.name() == "relatedStateVariable") {
						arg.stateVariableName() = nv.value();
					}
				}
			}
			return arg;
		}

		void parsePropertiesFromXmlNode(XML::XmlNode * node, UPnPObject & obj) {
			std::vector<XML::XmlNode*> children = node->children();
			for (std::vector<XML::XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
				if (XmlUtils::testNameValueXmlNode(*iter)) {
					UTIL::NameValue nv = XmlUtils::toNameValue(*iter);
					if (!nv.value().empty()) {
						obj[nv.name()] = nv.value();
					}
				}
			}
		}

		std::string getDeviceDescription() {
			return dd;
		}

		std::string getFriendlyName() {
			return fn;
		}

		bool completed() {
			return _completed;
		}

		UPnPDevice & getDevice() {
			return device;
		}

		std::string toString() {
			return toString(device, 0);
		}

		std::string toString(UPnPDevice & device, int depth) {
			std::string str;

			str.append(depth, ' ');
			if (depth > 0) { str.append(" - "); }
			str.append(device.getUdn() + " (" + device.getFriendlyName() + ")");
			str.append("\n");
			
			std::vector<UTIL::AutoRef<UPnPDevice> > & devices = device.devices();
			for (std::vector<UTIL::AutoRef<UPnPDevice> >::iterator iter = devices.begin(); iter != devices.end(); iter++) {
				str.append(toString(*(*iter), depth + 1));
			}
			return str;
		}
	};

	unsigned long UPnPSession::idx_seed = 0;

	/**
	 *
	 */
	class UPnPSessionManager {
	private:
		std::map<std::string, UTIL::AutoRef<UPnPSession> > sessions;
	public:
		UPnPSessionManager() {}
		virtual ~UPnPSessionManager() {}

		bool has(const std::string & udn) {
			return (sessions.find(udn) != sessions.end());
		}
		void clear() {
			sessions.clear();
		}
		UTIL::AutoRef<UPnPSession> prepareSession(const std::string & udn) {
			if (!has(udn)) {
				sessions[udn] = UTIL::AutoRef<UPnPSession>(new UPnPSession(udn));
			}
			return sessions[udn];
		}
		void remove(const std::string & udn) {
			sessions.erase(udn);
		}
		size_t size() {
			return sessions.size();
		}
		std::vector<std::string> getUdnS() {
			std::vector<std::string> ret;
			for (std::map<std::string, UTIL::AutoRef<UPnPSession> >::iterator iter = sessions.begin(); iter != sessions.end(); iter++) {
				ret.push_back(iter->first);
			}
			return ret;
		}
		UTIL::AutoRef<UPnPSession> operator[] (const std::string & udn) {
			return sessions[udn];
		}
	};

}

#endif
