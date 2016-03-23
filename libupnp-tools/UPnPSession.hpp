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
		std::string udn;
		std::string fn;
		std::string dd;
		bool _completed;
		UTIL::AutoRef<UPnPDevice> rootDevice;
		unsigned long creationTime;
		unsigned long updateTime;
		unsigned long sessionTimeout;
	
	public:
		UPnPSession(const std::string & udn);
		virtual ~UPnPSession();
		void setCreationTime(unsigned long creationTime);
		void setUpdateTime(unsigned long updateTime);
		void setSessionTimeout(unsigned long sessionTimeout);
		unsigned long lifetime();
		unsigned long duration();
		bool outdated();
		void buildDevice(SSDP::SSDPHeader & header);
		void parseDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		void parseDevicePropertiesFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		void parseServiceListFromDeviceXmlNode(XML::XmlNode * deviceXml, UPnPDevice & device);
		void parseServicePropertiesFromServiceXmlNode(XML::XmlNode * serviceXml, UPnPService * service);
		std::string getDump(const HTTP::Url & url);
		void buildService(UPnPService & service);
		void parseScpdFromXml(UPnPService & service, const std::string & scpd);
		UPnPAction parseActionFromXml(XML::XmlNode * actionXml);
		UPnPArgument parseArgumentFromXml(XML::XmlNode * argumentXml);
		UPnPStateVariable parseStateVariableFromXml(XML::XmlNode * stateVariableXml);
		void parsePropertiesFromXmlNode(XML::XmlNode * node, UPnPObject & obj);
		std::string getDeviceDescription();
		std::string getFriendlyName();
		bool completed();
		UTIL::AutoRef<UPnPDevice> getRootDevice();
		std::string toString();
		std::string toString(UPnPDevice & device, int depth);
	};
}

#endif
