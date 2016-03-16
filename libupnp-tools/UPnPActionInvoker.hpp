#ifndef __UPNP_ACTION_INVOKER_HPP__
#define __UPNP_ACTION_INVOKER_HPP__

#include <string>
#include <map>
#include <vector>
#include <liboslayer/AutoRef.hpp>
#include <liboslayer/StringElement.hpp>
#include <liboslayer/XmlParser.hpp>
#include <libhttp-server/Url.hpp>
#include "HttpUtils.hpp"
#include "XmlUtils.hpp"

#include "UPnPActionRequest.hpp"
#include "UPnPActionResponse.hpp"

namespace UPNP {
	
	class UPnPActionInvoker {
	private:
		HTTP::Url _controlUrl;
		
	public:
		UPnPActionInvoker(HTTP::Url controlUrl) : _controlUrl(controlUrl) {
		}
		virtual ~UPnPActionInvoker() {}

		HTTP::Url & controlUrl() {
			return _controlUrl;
		}
		UPnPActionResponse invoke(UPnPActionRequest & request) {

			UPnPActionResponse response;
			std::string serviceType = request.serviceType();
			std::string actionName = request.actionName();
			
			UTIL::LinkedStringMap headers;
			headers["SOAPACTION"] = ("\"" + serviceType + "#" + actionName + "\"");
			std::string result = HttpUtils::httpPost(_controlUrl, headers, makeSoapRequestContent(request));
			XML::XmlDocument doc = XML::DomParser::parse(result);
			if (doc.getRootNode().nil()) {
				throw OS::Exception("invoke error / wrong format", -1, 0);
			}
			XML::XmlNode * node = doc.getRootNode()->getElementByTagName(actionName + "Response");
			if (!node) {
				throw OS::Exception("invoke error / wrong format", -1, 0);
			}
			std::vector<XML::XmlNode*> children = node->children();
			for (std::vector<XML::XmlNode*>::iterator iter = children.begin(); iter != children.end(); iter++) {
				if (XmlUtils::testNameValueXmlNode(*iter)) {
					UTIL::NameValue nv = XmlUtils::toNameValue(*iter);
					response[nv.name()] = nv.value();
				}
			}
			return response;
		}
		std::string makeSoapRequestContent(UPnPActionRequest & request) {

			std::string actionName = request.actionName();
			std::string serviceType = request.serviceType();
			std::map<std::string, std::string> & parameters = request.parameters();
			
			std::string xml;
			xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n";
			xml.append("<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" "
						   "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n");
			xml.append("<s:Body>\r\n");
			xml.append("<u:" + actionName + " xmlns:u=\"" + serviceType + "\">\r\n");
			for (std::map<std::string, std::string>::iterator iter = parameters.begin(); iter != parameters.end(); iter++) {
				std::string name = iter->first;
				std::string & value = iter->second;
				xml.append("<" + name + ">" + XML::XmlEncoder::encode(value) + "</" + name + ">\r\n");
			}
			xml.append("</u:" + actionName + ">\r\n");
			xml.append("</s:Body>\r\n");
			xml.append("</s:Envelope>");
			return xml;
		}
	};
}

#endif